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

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDataProvider");)

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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDataProvider::ConstructL"));
    
    iPropertyMgr = CMTPImageDpObjectPropertyMgr::NewL(Framework());
    iThumbnailManager = CMTPImageDpThumbnailCreator::NewL();
    iMdeObserver = CMTPImageDpMdeObserver::NewL(Framework(), *this);
    iMdeObserver->SubscribeForChangeNotificationL();
    
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
    
    //Define RProperty of new pictures for status data provider
    _LIT_SECURITY_POLICY_PASS(KAllowReadAll);
    TInt error = RProperty::Define(TUid::Uid(KMTPServerUID), KMTPNewPicKey, RProperty::EInt, KAllowReadAll, KAllowReadAll);
    if (error != KErrNone && error != KErrAlreadyExists)
        {
        __FLOG_1(_L8("CMTPImageDataProvider::ConstructL - RProperty define error:%d"), error);
        User::LeaveIfError(error);
        }    
    
    __FLOG(_L8("<< CMTPImageDataProvider::ConstructL"));
    }

/**
 Destructor
*/
CMTPImageDataProvider::~CMTPImageDataProvider()
    {
    __FLOG(_L8(">> ~CMTPImageDataProvider"));      
    
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
    
    iFormatMappings.Close();
    
    //Try to delete objects in array
    HandleDeleteObjectsArray();
    iDeleteObjectsArray.ResetAndDestroy();
    
    __FLOG(_L8("<< ~CMTPImageDataProvider"));
    __FLOG_CLOSE;
    }

void CMTPImageDataProvider::Cancel()
    {
    __FLOG(_L8(">> Cancel"));
    __FLOG(_L8("<< Cancel"));
    }

/**
 Process the event from initiator
 
 @param aEvent       The event to be processed
 @param aConnection  The connection from which the event comes
*/
void CMTPImageDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8(">> ProcessEventL"));
    
    //Try to delete objects in array
    HandleDeleteObjectsArray();
    
    TInt idx(LocateRequestProcessorL(aEvent, aConnection));
    
    if (idx != KErrNotFound)
        {
        iActiveProcessors[idx]->HandleEventL(aEvent);
        }

    __FLOG(_L8("<< ProcessEventL"));
    }

/**
Process the notification from framework
@param aNotification  The notification to be processed
@param aParams        Notification parmenter
*/
void CMTPImageDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    __FLOG(_L8(">> ProcessNotificationL"));

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
 
    __FLOG(_L8("<< ProcessNotificationL"));
    }

/**
 Process the request from initiator
 
 @param aPhase       The request transaction phase
 @param aRequest     The request to be processed
 @param aConnection  The connection from which the request comes
*/   
void CMTPImageDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {    
    __FLOG(_L8(">> ProcessRequestPhaseL"));    
    
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
    __FLOG(_L8("<< ProcessRequestPhaseL"));
    }

/**
 Starts the enumeration of the image dp
*/
void CMTPImageDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    __FLOG(_L8(">> StartObjectEnumerationL"));
    
    if (aStorageId == KMTPStorageAll)
        {
        /*
         * Query previous image object count for calculation of new pictures when MTP startup
         * 
         */
        iPrePictures = QueryImageObjectCountL();
        iEnumerated = ETrue;
        __FLOG_1(_L16("CMTPImageDpEnumerator::CompleteEnumeration - Previous Pics: %d"), iPrePictures);
        }

    NotifyEnumerationCompleteL(aStorageId, KErrNone);
    
    __FLOG(_L8("<< StartObjectEnumerationL"));
    }


/**
Starts enumerate imagedp storage, just declare complete
*/
void CMTPImageDataProvider::StartStorageEnumerationL()
    {
    __FLOG(_L8(">> StartStorageEnumerationL"));   
    NotifyStorageEnumerationCompleteL();
    __FLOG(_L8("<< StartStorageEnumerationL"));        
    }

/**
Defines the supported operations and formats of the data provider

@param aCategory Defines what MTP is quering the DP about
@param aArray Supported() edits array to append supported features
*/
void CMTPImageDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    __FLOG(_L8(">> Supported"));

    switch (aCategory) 
        {        
    case EEvents:
        {
        TInt count(sizeof(KMTPImageDpSupportedEvents) / sizeof(KMTPImageDpSupportedEvents[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPImageDpSupportedEvents[i]);
            __FLOG_VA((_L("   CMTPImageDataProvider::Supported Events %d added"), KMTPImageDpSupportedEvents[i]));
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
            __FLOG_VA((_L("   CMTPImageDataProvider::Supported we have formatCode %d"), KMTPValidCodeExtensionMappings[i].iFormatCode ));
            if(aArray.Find(KMTPValidCodeExtensionMappings[i].iFormatCode)==KErrNotFound) // KMTPValidCodeExtensionMappings may contain format code more than once
                {
                aArray.Append(KMTPValidCodeExtensionMappings[i].iFormatCode);
                __FLOG_VA((_L("   CMTPImageDataProvider::Supported formatCode %d added"), KMTPValidCodeExtensionMappings[i].iFormatCode));
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
            __FLOG_VA((_L("   CMTPImageDataProvider::Supported property %d added"), KMTPImageDpSupportedProperties[i]));
            }   
        }
        break; 

    case EOperations:
        {
        TInt count(sizeof(KMTPImageDpSupportedOperations) / sizeof(KMTPImageDpSupportedOperations[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPImageDpSupportedOperations[i]);
            __FLOG_VA((_L("   CMTPImageDataProvider::Supported operation %d added"), KMTPImageDpSupportedOperations[i]));
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

    __FLOG(_L8("<< Supported"));
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
        _LIT(KFormatExtensionJpg, "0x3801:jpg::3");//3 means file dp will enumerate all image files instead of image dp.
        aStrings.AppendL(KFormatExtensionJpg);
        _LIT(KFormatExtensionJpe, "0x3801:jpe::3");
        aStrings.AppendL(KFormatExtensionJpe);
        _LIT(KFormatExtensionJpeg, "0x3801:jpeg::3");
        aStrings.AppendL(KFormatExtensionJpeg);   
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
    __FLOG(_L8(">> NotifyStorageEnumerationCompleteL"));    
    Framework().StorageEnumerationCompleteL();    
    __FLOG(_L8("<< NotifyStorageEnumerationCompleteL"));        
    }

CMTPImageDpThumbnailCreator& CMTPImageDataProvider::ThumbnailManager() const
	{
    __ASSERT_DEBUG(iThumbnailManager, User::Invariant());
	return *iThumbnailManager;
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
#ifdef __FLOG_ACTIVE  
void CMTPImageDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError)
#else
void CMTPImageDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /*aError*/)
#endif // __FLOG_ACTIVE
    {
    __FLOG(_L8(">> NotifyEnumerationCompletedL"));
    __FLOG_VA((_L8("Enumeration of storage 0x%08X completed with error status %d"), aStorageId, aError));
        
    Framework().ObjectEnumerationCompleteL(aStorageId);
    
    __FLOG(_L8("<< HandleEnumerationCompletedL"));
    }

/**
Find or create a request processor that can process the request

@param aRequest    The request to be processed
@param aConnection The connection from which the request comes

@return the index of the found/created request processor
*/
TInt CMTPImageDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8(">> LocateRequestProcessorL"));        
    
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
 
    __FLOG(_L8("<< LocateRequestProcessorL"));
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
    __FLOG(_L8(">> LocateRequestProcessorL"));
        
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

    __FLOG(_L8("<< LocateRequestProcessorL"));
    return idx;
    }

/**
 Notify the data provider that the session has been closed

 @param aSessionId    The session Id closed
 @param aConnection   The connection of the sesssion
*/
void CMTPImageDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    __FLOG_VA((_L8(">> SessionClosedL SessionID = %d"), aSession.iMTPId));
    
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
    
    __FLOG(_L8("<< SessionClosedL"));
    }

/**
 Notify the data provider that the session opened
 
 @param aSessionId    The session Id opened
 @param aConnection   The connection of the sesssion
*/
#ifdef __FLOG_ACTIVE
void CMTPImageDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPImageDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    __FLOG_VA((_L8(">> SessionOpenedL SessionID = %d"), aSession.iMTPId));
    
    if (iEnumerated)
        {
        /**
         * Get image object count from framework and calculate the new pictures
         */
        TUint curPictures = QueryImageObjectCountL();
        TInt  newPictures = curPictures - iPrePictures;
        
        __FLOG_2(_L16("CMTPImageDpEnumerator::CompleteEnumeration - Previous Pics:%d, New Pics: %d"), iPrePictures, newPictures);
        if (newPictures >= 0)
            {
            MTPImageDpUtilits::UpdateNewPicturesValue(*this, newPictures, ETrue);  
            }
        else
            {
            MTPImageDpUtilits::UpdateNewPicturesValue(*this, 0, ETrue);  
            }
        
        iEnumerated = EFalse;
        }
    
    __FLOG(_L8("<< SessionOpenedL "));
    }

/**
 Notify the data provider that the folder name has been changed
 
 @param aParam   The Rename notification
*/
void CMTPImageDataProvider::RenameObjectL(const TMTPNotificationParamsHandle& aParam)
    {
    __FLOG_VA((_L16(">> RenameObjectL Handle: %u, Old name: %S"), aParam.iHandleId, &aParam.iFileName));
    
    if (!iRenameObject)
        {
        iRenameObject = CMTPImageDpRenameObject::NewL(Framework(), *this);
        }

    iRenameObject->StartL(aParam.iHandleId, aParam.iFileName);    
    
    __FLOG(_L8("<< RenameObjectL "));
    }

/**
 Find format code according to its extension name 
*/
TMTPFormatCode CMTPImageDataProvider::FindFormatL(const TDesC& aExtension)
    {
    TMTPFormatCode* ret = iFormatMappings.Find(aExtension);
    User::LeaveIfNull(ret);
    
    return *ret;
    }

/**
 Query image object count from current framework 
*/
TUint CMTPImageDataProvider::QueryImageObjectCountL()
    { 
    TMTPObjectMgrQueryParams params(KMTPStorageAll, EMTPFormatCodeEXIFJPEG, KMTPHandleNone);    
    return Framework().ObjectMgr().CountL(params);
    }

TBool CMTPImageDataProvider::GetCacheParentHandle(const TDesC& aParentPath, TUint32& aParentHandle)
    {
    TBool ret = EFalse;
    
    if (iParentCache.iPath.Compare(aParentPath) == 0)
        {
        aParentHandle = iParentCache.iHandle;
        ret = ETrue;
        }
    else
        {
        aParentHandle = KMTPHandleNone;
        }
    
    return ret;
    }

void CMTPImageDataProvider::SetCacheParentHandle(const TDesC& aParentPath, TUint32 aParentHandle)
    {
    iParentCache.iPath.Copy(aParentPath);
    iParentCache.iHandle = aParentHandle;
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
        __FLOG_2(_L8("delete left objects %d error code is %d \n"), i, err );
        
        if ( err == KErrNone )
            {
            iDeleteObjectsArray.Remove(i);
            --i;
            delete object;
            object = NULL;
            }
        }
    }
