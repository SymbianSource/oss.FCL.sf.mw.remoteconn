// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @internalTechnology
*/

#include <e32cmn.h>
#include <centralrepository.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypeevent.h>

#include "cmtpimagedp.h"
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "cmtprequestprocessor.h"
#include "mtpimagedprequestprocessor.h"
#include "cmtpimagedpthumbnailcreator.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedpmdeobserver.h"
#include "cmtpimagedprenameobject.h"
#include "cmtpimagedpnewpicturesnotifier.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpTraces.h"
#endif


static const TInt KArrayGranularity = 3;
static const TInt KDeleteObjectGranularity = 2;

//used by hashmap & hashset class
LOCAL_C TUint32 TBuf16Hash(const TBuf<KMaxExtNameLength>& aPtr)
    {
    return DefaultHash::Des16(aPtr);
    }

LOCAL_C TBool TBuf16Ident(const TBuf<KMaxExtNameLength>& aL, const TBuf<KMaxExtNameLength>& aR)
    {
    return DefaultIdentity::Des16(aL, aR);
    }

/**
 Image data provider factory method.
 @return A pointer to a Image data provider object. Ownership is transfered.
 @leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPImageDataProvider::NewL(TAny* aParams)
    {
    CMTPImageDataProvider* self = new (ELeave) CMTPImageDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
 Standard C++ constructor
 
 @param aParams  pointer to MMTPDataProviderFramework
*/
CMTPImageDataProvider::CMTPImageDataProvider(TAny* aParams) :
    CMTPDataProviderPlugin(aParams),
    iActiveProcessors(KArrayGranularity),
    iFormatMappings(&TBuf16Hash, &TBuf16Ident),
    iMimeMappings(&TBuf16Hash, &TBuf16Ident),
    iActiveProcessor(-1),
    iEnumerated(EFalse),
	iDeleteObjectsArray(KDeleteObjectGranularity)
    {
    }

/**
Second-phase construction
*/
void CMTPImageDataProvider::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_CONSTRUCTL_ENTRY );
    
    iNewPicNotifier = CMTPImageDpNewPicturesNotifier::NewL();
    
    //Setup central repository connection
    const TUint32 KUidMTPImageRepositoryValue(0x2001FCA2);
    const TUid KUidMTPImageRepository = {KUidMTPImageRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPImageRepository);    
    
    //Initialize hash map of extention to format code
    TInt count(sizeof(KMTPValidCodeExtensionMappings) / sizeof(KMTPValidCodeExtensionMappings[0]));
    for(TInt i(0); i<count; i++)
        {
        iFormatMappings.Insert(KMTPValidCodeExtensionMappings[i].iExtension, KMTPValidCodeExtensionMappings[i].iFormatCode);
        }    
    
    //Initialize hash map of extension to mime type
    count = sizeof(KMTPExtensionMimeTypeMappings) / sizeof(KMTPExtensionMimeTypeMappings[0]);
    for(TInt i(0); i<count; i++)
        {
        iMimeMappings.Insert(KMTPExtensionMimeTypeMappings[i].iExtension, KMTPExtensionMimeTypeMappings[i].iMimeType);
        }     
    
    //Define RProperty of new pictures for status data provider
	RProcess process;
	TUid tSid = process.SecureId();
    _LIT_SECURITY_POLICY_PASS(KAllowReadAll);
    TInt error = RProperty::Define(tSid, KMTPNewPicKey, RProperty::EInt, KAllowReadAll, KAllowReadAll);
    if (error != KErrNone && error != KErrAlreadyExists)
        {
        LEAVEIFERROR(error,
                OstTrace1( TRACE_ERROR, DUP1_CMTPIMAGEDATAPROVIDER_CONSTRUCTL, 
                        "RProperty define error:%d", error ));
                
        }    
    
    iEnumerationNotified = ETrue;
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_CONSTRUCTL_EXIT );
    }

/**
 Destructor
*/
CMTPImageDataProvider::~CMTPImageDataProvider()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_CMTPIMAGEDATAPROVIDER_DES_ENTRY );
    
    // delete all processor instances
    TUint count(iActiveProcessors.Count());
    while (count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();
     
    // image dp unsubscribe from MDS
    if(iMdeObserver)
        {
        TRAP_IGNORE(iMdeObserver->UnsubscribeForChangeNotificationL());
        delete iMdeObserver;
        }        
    delete iThumbnailManager;
    delete iPropertyMgr;       
    delete iRepository;   
    delete iRenameObject;
    delete iNewPicNotifier;
    
    iFormatMappings.Close();
    iMimeMappings.Close();
    
    //Try to delete objects in array
    HandleDeleteObjectsArray();
    iDeleteObjectsArray.ResetAndDestroy();
    iNewPicHandles.Reset();

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_CMTPIMAGEDATAPROVIDER_DES_EXIT );
    }

void CMTPImageDataProvider::Cancel()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_CANCEL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_CANCEL_EXIT );
    }

/**
 Process the event from initiator
 
 @param aEvent       The event to be processed
 @param aConnection  The connection from which the event comes
*/
void CMTPImageDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_PROCESSEVENTL_ENTRY );
    
    //Try to delete objects in array
    HandleDeleteObjectsArray();
    
    TInt idx(LocateRequestProcessorL(aEvent, aConnection));
    
    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_PROCESSEVENTL_EXIT );
    }

/**
Process the notification from framework
@param aNotification  The notification to be processed
@param aParams        Notification parmenter
*/
void CMTPImageDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_PROCESSNOTIFICATIONL_ENTRY );

    switch (aNotification)
        {
    case EMTPSessionClosed:
        SessionClosedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
        
    case EMTPSessionOpened:
        SessionOpenedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
        
    case EMTPRenameObject:
        RenameObjectL(*reinterpret_cast<const TMTPNotificationParamsHandle*>(aParams));
        break;
        
    default:
        // Ignore all other notifications.
        break;
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_PROCESSNOTIFICATIONL_EXIT );
    }

/**
 Process the request from initiator
 
 @param aPhase       The request transaction phase
 @param aRequest     The request to be processed
 @param aConnection  The connection from which the request comes
*/   
void CMTPImageDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_PROCESSREQUESTPHASEL_ENTRY );
    
    //Try to handle objects which need to be deleted
    HandleDeleteObjectsArray();
    
    TInt idx(LocateRequestProcessorL(aRequest, aConnection));
    __ASSERT_DEBUG((idx != KErrNotFound), Panic(EMTPImageDpNoMatchingProcessor));
    MMTPRequestProcessor* processor(iActiveProcessors[idx]);
    iActiveProcessor = idx;
    iActiveProcessorRemoved = EFalse;    
    TBool result(processor->HandleRequestL(aRequest, aPhase));
    if (iActiveProcessorRemoved)
        {
        processor->Release(); // destroy the processor
        }    
    else if (result)    //destroy the processor
        {
        processor->Release();
        iActiveProcessors.Remove(idx);
        }
 
    iActiveProcessor = -1;
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_PROCESSREQUESTPHASEL_EXIT );
    }

/**
 Starts the enumeration of the image dp
*/
void CMTPImageDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_STARTOBJECTENUMERATIONL_ENTRY );
    
    TBool isComplete = ETrue;
    iEnumerationNotified = EFalse;

    if (aStorageId == KMTPStorageAll)
        {
        /*
         * framework notify data provider to enumerate
         * 
         */
        if (iPropertyMgr == NULL)
            {
            iPropertyMgr = CMTPImageDpObjectPropertyMgr::NewL(Framework(), *this);
            isComplete = EFalse;
            }
  
        iEnumerated = ETrue;
        }

    if (isComplete)
        {
        NotifyEnumerationCompleteL(aStorageId, KErrNone);
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_STARTOBJECTENUMERATIONL_EXIT );
    }


/**
Starts enumerate imagedp storage, just declare complete
*/
void CMTPImageDataProvider::StartStorageEnumerationL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_STARTSTORAGEENUMERATIONL_ENTRY );
    NotifyStorageEnumerationCompleteL();   
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_STARTSTORAGEENUMERATIONL_EXIT );
    }

/**
Defines the supported operations and formats of the data provider

@param aCategory Defines what MTP is quering the DP about
@param aArray Supported() edits array to append supported features
*/
void CMTPImageDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_SUPPORTED_ENTRY );

    switch (aCategory) 
        {        
    case EEvents:
        {
        TInt count(sizeof(KMTPImageDpSupportedEvents) / sizeof(KMTPImageDpSupportedEvents[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPImageDpSupportedEvents[i]);
            OstTrace1( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_SUPPORTED, 
                    "   CMTPImageDataProvider::Supported Events %d added", KMTPImageDpSupportedEvents[i] );
            }  
        }
        break;
    case EObjectPlaybackFormats: // formats that can be placed on the device
	/*intentional fall through*/ 
    case EObjectCaptureFormats: // formats the device generates
        {
        TInt count(sizeof(KMTPValidCodeExtensionMappings) / sizeof(KMTPValidCodeExtensionMappings[0]));
        for(TInt i(0); (i < count); i++)
            {
            OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDATAPROVIDER_SUPPORTED, 
                    "   CMTPImageDataProvider::Supported we have formatCode %d", KMTPValidCodeExtensionMappings[i].iFormatCode  );
            if(aArray.Find(KMTPValidCodeExtensionMappings[i].iFormatCode)==KErrNotFound) // KMTPValidCodeExtensionMappings may contain format code more than once
                {
                aArray.Append(KMTPValidCodeExtensionMappings[i].iFormatCode);
                OstTrace1( TRACE_NORMAL, DUP2_CMTPIMAGEDATAPROVIDER_SUPPORTED, 
                        "   CMTPImageDataProvider::Supported formatCode %d added", KMTPValidCodeExtensionMappings[i].iFormatCode );
                }
            }
        }
        break;
    case EObjectProperties:
        {
        TInt count(sizeof(KMTPImageDpSupportedProperties) / sizeof(KMTPImageDpSupportedProperties[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPImageDpSupportedProperties[i]);
            OstTrace1( TRACE_NORMAL, DUP3_CMTPIMAGEDATAPROVIDER_SUPPORTED, 
                    "   CMTPImageDataProvider::Supported property %d added", KMTPImageDpSupportedProperties[i] );
            
            }   
        }
        break; 

    case EOperations:
        {
        TInt count(sizeof(KMTPImageDpSupportedOperations) / sizeof(KMTPImageDpSupportedOperations[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPImageDpSupportedOperations[i]);
            OstTrace1( TRACE_NORMAL, DUP4_CMTPIMAGEDATAPROVIDER_SUPPORTED, 
                    "   CMTPImageDataProvider::Supported operation %d added", KMTPImageDpSupportedOperations[i] );
            }   
        }
        break;  

    case EStorageSystemTypes:
        aArray.Append(CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
        break; 

    default:    
        // Unrecognised category, leave aArray unmodified.
        break;
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_SUPPORTED_EXIT );
    }

/**
Defines the supported vendor extension info of the data provider

@param aCategory Defines what MTP is quering the DP about
@param aStrings Supported() edits array to append supported vendor info
*/
void CMTPImageDataProvider::SupportedL(TMTPSupportCategory aCategory, CDesCArray& aStrings) const
    {
    switch (aCategory) 
        {
    case EFolderExclusionSets:
        {
        //do nothing
        }
        break;
        
    case EFormatExtensionSets:
        {
        _LIT(KFormatExtensionJpg, "0x3801:jpg:::3");//3 means file dp will enumerate all image files instead of image dp.
        aStrings.AppendL(KFormatExtensionJpg);
        _LIT(KFormatExtensionJpe, "0x3801:jpe:::3");
        aStrings.AppendL(KFormatExtensionJpe);
        _LIT(KFormatExtensionJpeg, "0x3801:jpeg:::3");
        aStrings.AppendL(KFormatExtensionJpeg);
        
        /*
         * bmp files
         */
        _LIT(KFormatExtensionBmp, "0x3804:bmp:::3");
        aStrings.AppendL(KFormatExtensionBmp);
        
        /*
         * gif files
         */
        _LIT(KFormatExtensionGif, "0x3807:gif:::3");
        aStrings.AppendL(KFormatExtensionGif);
        
        /*
         * png files
         */
        _LIT(KFormatExtensionPng, "0x380B:png:::3");
        aStrings.AppendL(KFormatExtensionPng);
        
        /*
         * tif, tiff files
         */
        /*
        _LIT(KFormatExtensionTif, "0x380D:tif::3");
        aStrings.AppendL(KFormatExtensionTif);
        _LIT(KFormatExtensionTiff, "0x380D:tiff::3");
        aStrings.AppendL(KFormatExtensionTiff);
        */                 
        }
        break;
        
    default:
        // Unrecognised category, leave aArray unmodified.
        break;
        }
    }

/**
Notify framework the image dp has completed enumeration
*/
void CMTPImageDataProvider::NotifyStorageEnumerationCompleteL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_NOTIFYSTORAGEENUMERATIONCOMPLETEL_ENTRY );   
    Framework().StorageEnumerationCompleteL();          
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_NOTIFYSTORAGEENUMERATIONCOMPLETEL_EXIT );
    }

CMTPImageDpThumbnailCreator* CMTPImageDataProvider::ThumbnailManager()
    {
    if(NULL == iThumbnailManager)
        {
        TRAP_IGNORE(iThumbnailManager = CMTPImageDpThumbnailCreator::NewL(*this));
        }
    return iThumbnailManager;
    }

CMTPImageDpObjectPropertyMgr& CMTPImageDataProvider::PropertyMgr()const
	{
	__ASSERT_DEBUG(iPropertyMgr, User::Invariant());
	return *iPropertyMgr;	
	}

CRepository& CMTPImageDataProvider::Repository() const
    {
    __ASSERT_DEBUG(iRepository, User::Invariant());
    return *iRepository;
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//
#ifdef OST_TRACE_COMPILER_IN_USE  
void CMTPImageDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError)
#else
void CMTPImageDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /*aError*/)
#endif // OST_TRACE_COMPILER_IN_USE
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL, 
            "Enumeration of storage 0x%08X completed with error status %d", aStorageId, (TInt32)aError );

    if (!iEnumerationNotified)
        {
        iEnumerationNotified = ETrue;
        Framework().ObjectEnumerationCompleteL(aStorageId);
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_EXIT );
    }

/**
Find or create a request processor that can process the request

@param aRequest    The request to be processed
@param aConnection The connection from which the request comes

@return the index of the found/created request processor
*/
TInt CMTPImageDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );     
    
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i(0); (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            idx = i;
            break;
            }
        }
        
    if (idx == KErrNotFound)
        {
        MMTPRequestProcessor* processor = MTPImageDpProcessor::CreateL(Framework(), aRequest, aConnection,*this);
        CleanupReleasePushL(*processor);
        iActiveProcessors.AppendL(processor);
        CleanupStack::Pop();
        idx = count;
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
    return idx;
    }

/**
Find or create a request processor that can process the event

@param aEvent    The event to be processed
@param aConnection The connection from which the request comes

@return the index of the found/created request processor
*/
TInt CMTPImageDataProvider::LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( DUP1_CMTPIMAGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_ENTRY );
        
    TInt idx(KErrNotFound);
    TInt count(iActiveProcessors.Count());
    for (TInt i(0); (i < count); i++)
        {
        if (iActiveProcessors[i]->Match(aEvent, aConnection))
            {
            idx = i;
            break;
            }
        }    

    OstTraceFunctionExit0( DUP1_CMTPIMAGEDATAPROVIDER_LOCATEREQUESTPROCESSORL_EXIT );
    return idx;
    }

/**
 Notify the data provider that the session has been closed

 @param aSessionId    The session Id closed
 @param aConnection   The connection of the sesssion
*/
void CMTPImageDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_SESSIONCLOSEDL, "SessionID = %d", aSession.iMTPId );
    
    TInt count = iActiveProcessors.Count();
    while(count--)
        {
        MMTPRequestProcessor* processor = iActiveProcessors[count];
        TUint32 sessionId = processor->SessionId();
        if((sessionId == aSession.iMTPId) && (processor->Connection().ConnectionId() == aSession.iConnection.ConnectionId()))
            {
            iActiveProcessors.Remove(count);
            if (count == iActiveProcessor)
                {
                iActiveProcessorRemoved = ETrue;
                }
            else
                {
                processor->Release();
                }
            }
        }
    
    /**
     * We clear property manager cache when receiving session close notification from framework every times
     */
    iPropertyMgr->ClearAllCache();

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

/**
 Notify the data provider that the session opened
 
 @param aSessionId    The session Id opened
 @param aConnection   The connection of the sesssion
*/
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPImageDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPImageDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_SESSIONOPENEDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_SESSIONOPENEDL, "SessionID = %d", aSession.iMTPId );
    
    if (iEnumerated)
        {
        /**
         * Get image object count from framework and calculate the new pictures
         */
        TUint newPictures = QueryImageObjectCountL();        
        iNewPicNotifier->SetNewPictures(newPictures);
        OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDATAPROVIDER_SESSIONOPENEDL, 
                "CMTPImageDpEnumerator::CompleteEnumeration - New Pics: %d", newPictures );
        iEnumerated = EFalse;
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_SESSIONOPENEDL_EXIT );
    }

/**
 Notify the data provider that the folder name has been changed
 
 @param aParam   The Rename notification
*/
void CMTPImageDataProvider::RenameObjectL(const TMTPNotificationParamsHandle& aParam)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_RENAMEOBJECTL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_RENAMEOBJECTL, 
            "Handle: %u, Old name: %S", aParam.iHandleId, aParam.iFileName);
    
    if (!iRenameObject)
        {
        iRenameObject = CMTPImageDpRenameObject::NewL(Framework(), *this);
        }

    iRenameObject->StartL(aParam.iHandleId, aParam.iFileName);    

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_RENAMEOBJECTL_EXIT );
    }

/**
 Find format code according to its extension name 
*/
TMTPFormatCode CMTPImageDataProvider::FindFormat(const TDesC& aExtension)
    {
    TMTPFormatCode* ret = iFormatMappings.Find(aExtension);
    if (ret == NULL)
        {
        return EMTPFormatCodeUndefined;
        }
    else
        {
        return *ret;
        }
    }

/**
 Find mime type according to its extension name 
*/
const TDesC& CMTPImageDataProvider::FindMimeType(const TDesC& aExtension)
    {
    /**
     * copy file extension by insensitive case
     */
    if (aExtension.Length() > KMaxExtNameLength)
        {
        return KNullDesC;
        }
    
    TBuf<KMaxExtNameLength> extension;
    extension.CopyLC(aExtension);
    
    const TDesC* ret = iMimeMappings.Find(extension);
    if (ret == NULL)
        {
        return KNullDesC;
        }
    else
        {
        return *ret;
        }
    }

/**
 Query image object count from current framework 
*/
TUint CMTPImageDataProvider::QueryImageObjectCountL()
    { 
    RMTPObjectMgrQueryContext   context;
    RArray<TUint>               handles;
    TMTPObjectMgrQueryParams    params(KMTPStorageAll, KMTPFormatsAll, KMTPHandleNone, Framework().DataProviderId());
    CleanupClosePushL(context);
    CleanupClosePushL(handles);    
    
    iNewPicHandles.Reset();
    
    do
        {
        /*
         * Speed up query performance, avoid to duplicated copy object handle between RArrays
         */
        Framework().ObjectMgr().GetObjectHandlesL(params, context, handles);
        }
    while (!context.QueryComplete());    
    
    CMTPObjectMetaData* objMetadata = CMTPObjectMetaData::NewLC();
    
    TUint newPictures = 0;
    TInt count = handles.Count();
    for (TInt i(0); i<count; ++i)
        {
        Framework().ObjectMgr().ObjectL(handles[i], *objMetadata);
        if (MTPImageDpUtilits::IsNewPicture(*objMetadata))
            {
            ++newPictures;
            iNewPicHandles.Append(handles[i]);
            }
        }
    
    
    CleanupStack::PopAndDestroy(objMetadata);
    CleanupStack::PopAndDestroy(&handles);
    CleanupStack::PopAndDestroy(&context);

    return newPictures;
    }

void CMTPImageDataProvider::AppendDeleteObjectsArrayL(const TDesC& aSuid)
    {
    iDeleteObjectsArray.AppendL(aSuid.AllocLC());
    CleanupStack::Pop();
    }

void CMTPImageDataProvider::HandleDeleteObjectsArray()
    {
    for ( TInt i = 0; i < iDeleteObjectsArray.Count(); i++ )
        {
        HBufC* object = iDeleteObjectsArray[i];
        TInt err = Framework().Fs().Delete(object->Des());
        OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_HANDLEDELETEOBJECTSARRAY, 
                "delete left objects %d error code is %d \n", i, err  );
        
        
        if ( err == KErrNone )
            {
            iDeleteObjectsArray.Remove(i);
            --i;
            delete object;
            object = NULL;
            }
        }
    }

void CMTPImageDataProvider::IncreaseNewPictures(TInt aCount)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_INCREASENEWPICTURES_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_INCREASENEWPICTURES, "New Pictures: %d", aCount );

    iNewPicNotifier->IncreaseCount(aCount);    
 
    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_INCREASENEWPICTURES_EXIT );
    }

void CMTPImageDataProvider::DecreaseNewPictures(TInt aCount)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_DECREASENEWPICTURES_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDATAPROVIDER_DECREASENEWPICTURES, "New Pictures: %d", aCount );

    iNewPicNotifier->DecreaseCount(aCount);

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_DECREASENEWPICTURES_EXIT );
    }

void CMTPImageDataProvider::ResetNewPictures()
	{
	OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_RESETNEWPICTURES_ENTRY );

	iNewPicNotifier->SetNewPictures(0);
	
	TInt count = iNewPicHandles.Count();
	if (!count)
		{
		OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_RESETNEWPICTURES_EXIT );
		return;
		}

	CMTPObjectMetaData* objMetadata = CMTPObjectMetaData::NewLC();
	
	for (TInt i(0); i<count; ++i)
	{
		Framework().ObjectMgr().ObjectL(iNewPicHandles[i], *objMetadata);
		MTPImageDpUtilits::UpdateObjectStatusToOldL(Framework(), *objMetadata);
	}
	
	iNewPicHandles.Reset();
	CleanupStack::PopAndDestroy(objMetadata);

	OstTraceFunctionExit0( DUP1_CMTPIMAGEDATAPROVIDER_RESETNEWPICTURES_EXIT );
	}

void CMTPImageDataProvider::HandleMdeSessionCompleteL(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDATAPROVIDER_HANDLEMDESESSIONCOMPLETEL_ENTRY );

    NotifyEnumerationCompleteL(KMTPStorageAll, KErrNone);
    if (aError == KErrNone)
        {
        iMdeObserver = CMTPImageDpMdeObserver::NewL(Framework(), *this);
        iMdeObserver->SubscribeForChangeNotificationL();
        }

    OstTraceFunctionExit0( CMTPIMAGEDATAPROVIDER_HANDLEMDESESSIONCOMPLETEL_EXIT );
    }

