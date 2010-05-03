// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <bautils.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpdevdpexclusionmgr.h"
#include "cmtpdevicedatastore.h"
#include "cmtpdevicedp.h"
#include "cmtpdevicedpconfigmgr.h"
#include "cmtpfsenumerator.h"
#include "cmtprequestprocessor.h"
#include "cmtpstoragewatcher.h"
#include "mtpdevicedpconst.h"
#include "mtpdevicedpprocessor.h"
#include "mtpdevdppanic.h"
#include "cmtpconnectionmgr.h"

#include "cmtpextndevdp.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeviceDataProvider");)
static const TInt KMTPDeviceDpSessionGranularity(3);
static const TInt KMTPDeviceDpActiveEnumeration(0);

/**
MTP device data provider plug-in factory method.
@return A pointer to an MTP device data provider plug-in. Ownership IS
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPDeviceDataProvider::NewL(TAny* aParams)
    {
    CMTPDeviceDataProvider* self = new (ELeave) CMTPDeviceDataProvider(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/
CMTPDeviceDataProvider::~CMTPDeviceDataProvider()
    {
    __FLOG(_L8("~CMTPDeviceDataProvider - Entry"));
    iPendingEnumerations.Close();
    TInt count = iActiveProcessors.Count();
    while(count--)
        {
        iActiveProcessors[count]->Release();
        }
    iActiveProcessors.Close();
    delete iStorageWatcher;
    iDevDpSingletons.Close();
    iDpSingletons.Close();
    delete iEnumerator;
    delete iExclusionMgr;

    iExtnPluginMapArray.ResetAndDestroy();
    iExtnPluginMapArray.Close();
	iEvent.Reset();

    delete iDeviceInfoTimer;
    iFrameWork.Close();
    __FLOG(_L8("~CMTPDeviceDataProvider - Exit"));
    __FLOG_CLOSE;
    }

void CMTPDeviceDataProvider::Cancel()
    {

    }

void CMTPDeviceDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8("ProcessEventL - Entry"));
    TInt index = LocateRequestProcessorL(aEvent, aConnection);
    if(index != KErrNotFound)
        {
        iActiveProcessors[index]->HandleEventL(aEvent);
        }
    __FLOG(_L8("ProcessEventL - Exit"));
    }

void CMTPDeviceDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    __FLOG(_L8("ProcessNotificationL - Entry"));
    switch (aNotification)
        {
    case EMTPSessionClosed:
        SessionClosedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;

    case EMTPSessionOpened:
        SessionOpenedL(*reinterpret_cast<const TMTPNotificationParamsSessionChange*>(aParams));
        break;
    case EMTPObjectAdded:
        AddFolderRecursiveL(*reinterpret_cast<const TMTPNotificationParamsFolderChange*>( aParams ));
        break;
    default:
        // Ignore all other notifications.
        break;
        }
    __FLOG(_L8("ProcessNotificationL - Exit"));
    }

void CMTPDeviceDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("ProcessRequestPhaseL - Entry"));
    TUint16 opCode( aRequest.Uint16( TMTPTypeRequest::ERequestOperationCode ) );    
    TInt index = LocateRequestProcessorL(aRequest, aConnection);
    __ASSERT_DEBUG(index != KErrNotFound, Panic(EMTPDevDpNoMatchingProcessor));
    MMTPRequestProcessor* processor = iActiveProcessors[index];
    iActiveProcessor = index;
    iActiveProcessorRemoved = EFalse;
    TBool result = processor->HandleRequestL(aRequest, aPhase);
    if (iActiveProcessorRemoved)
	    {
	    processor->Release(); // destroy the processor
	    }
    else if (result)
	    {
	    processor->Release();    	
	    iActiveProcessors.Remove(index);
	    }
    iActiveProcessor = -1;
    
    __FLOG_VA((_L8("opCode = 0x%x"), opCode));
    __FLOG_VA((_L8("TranPort UID = 0x%x"), iFrameWork.ConnectionMgr().TransportUid().iUid));
    __FLOG_VA((_L8("CommandState = 0x%x"), iCommandState));    
    const static TInt32 KMTPUsbTransportUid = 0x102827B2;

    if((EMTPOpCodeGetDeviceInfo == opCode)&&(KMTPUsbTransportUid == iFrameWork.ConnectionMgr().TransportUid().iUid))
        {
        __FLOG(_L8("EMTPOpCodeGetDeviceInfo == opCode"));
        //If GetDeviceInfo comes and there is no OpenSession before, the timer will start. And tread the host as Mac. 
        //Only the first GetDeviceInfo in one session will start the timer.
        if((EIdle == iCommandState)&&(NULL == iDeviceInfoTimer))
            {
            __FLOG(_L8("EMTPOpCodeGetDeviceInfo == opCode, start timer"));
            iCommandState = EStartDeviceInfoTimer;
            iDeviceInfoTimer = CMTPDeviceInfoTimer::NewL(*this);
            iDeviceInfoTimer->Start();
            }
        else
            {
            __FLOG(_L8("EMTPOpCodeGetDeviceInfo == opCode, Not start timer"));            
            }
        }
    else
       {       
       __FLOG(_L8("EMTPOpCodeGetDeviceInfo != opCode"));
       if((EMTPOpCodeOpenSession == opCode)&&(EIdle == iCommandState))
            {
            __FLOG(_L8("EMTPOpCodeGetDeviceInfo == opCode, set CommandState to be EOpenSession"));
            iCommandState = EOpenSession;
            }
       
       if(iDeviceInfoTimer)
           {
           __FLOG(_L8("iDeviceInfoTimer != NULL, stop timer"));
           delete iDeviceInfoTimer;
           iDeviceInfoTimer = NULL;
           }
       else
           {
           __FLOG(_L8("iDeviceInfoTimer == NULL, NOT stop timer"));            
           }
       }    
    __FLOG(_L8("ProcessRequestPhaseL - Exit"));
    }

void CMTPDeviceDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    __FLOG(_L8("StartObjectEnumerationL - Entry"));
    iPendingEnumerations.AppendL(aStorageId);
    if (iEnumeratingState == EUndefined)
        {
        iDevDpSingletons.DeviceDataStore().StartEnumerationL(aStorageId, *this);
        iEnumeratingState = EEnumeratingDeviceDataStore;
        iStorageWatcher->Start();
        }
    else
        {
    	iEnumeratingState = EEnumeratingFolders;
    	NotifyEnumerationCompleteL(aStorageId, KErrNone);
        }
    __FLOG(_L8("StartObjectEnumerationL - Exit"));
    }

void CMTPDeviceDataProvider::StartStorageEnumerationL()
    {
    __FLOG(_L8("StartStorageEnumerationL - Entry"));
    iStorageWatcher->EnumerateStoragesL();
    Framework().StorageEnumerationCompleteL();
    __FLOG(_L8("StartStorageEnumerationL - Exit"));
    }

void CMTPDeviceDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    __FLOG(_L8("Supported - Entry"));
    TInt mode = Framework().Mode();
    switch (aCategory)
        {
    case EAssociationTypes:
        aArray.Append(EMTPAssociationTypeGenericFolder);        
        break;

    case EDeviceProperties:
        {
        TInt count = sizeof(KMTPDeviceDpSupportedProperties) / sizeof(TUint16);
        for(TInt i = 0; i < count; i++)
            {
            if(( (EModePTP == mode) ||(EModePictBridge == mode) )
				&& ( 0x4000 == (KMTPDeviceDpSupportedProperties[i] & 0xE000)))
            	{
            	//in ptp mode support only ptp properties
            	aArray.Append(KMTPDeviceDpSupportedProperties[i]);
            	}
            else
            	{
            	aArray.Append(KMTPDeviceDpSupportedProperties[i]);
            	}
            }
           
        TInt noOfEtxnPlugins = iExtnPluginMapArray.Count();
        for(TInt i=0; i < noOfEtxnPlugins; i++)
	        {
	        iExtnPluginMapArray[i]->ExtPlugin()->Supported(aCategory, *iExtnPluginMapArray[i]->SupportedOpCodes(), (TMTPOperationalMode)mode );// or pass the incoming array
	        TInt count =iExtnPluginMapArray[i]->SupportedOpCodes()->Count();
       												  														//bcoz it needs to b updated
	        for (TInt r=0; r <count; r++ )
	        	{
	        	aArray.Append((*iExtnPluginMapArray[i]->SupportedOpCodes())[r]);
	        	}
	       }

       TRAP_IGNORE(iPtrDataStore->SetSupportedDevicePropertiesL(aArray));
       }
        break;

    case EEvents:
        {
        TInt count = sizeof(KMTPDeviceDpSupportedEvents) / sizeof(TUint16);
        for(TInt i = 0; i < count; i++)
            {
            TUint16 event = KMTPDeviceDpSupportedEvents[i];
            switch(mode)
                {
            case EModePTP:
            case EModePictBridge:
                // In the initial implementation all device DP events pass this test, but this
                // catches any others added in the future that fall outside this range.
                if(event <= EMTPEventCodePTPEnd)
                    {
                    aArray.Append(event);
                    }
                break;

            case EModeMTP:
                // In the initial implementation all device DP events pass this test, but this
                // catches any others added in the future that fall outside this range.
                if(event <= EMTPEventCodeMTPEnd)
                    {
                    aArray.Append(event);
                    }
                break;

            default:
            	// No other valid modes are defined
                break;
                }
            }
        }
        break;

    case EObjectCaptureFormats:
    case EObjectPlaybackFormats:
  		// Only supports association objects
        aArray.Append(EMTPFormatCodeAssociation);
        break;


    case EOperations:
        {
        TInt count = sizeof(KMTPDeviceDpSupportedOperations) / sizeof(TUint16);
        for(TInt i = 0; i < count; i++)
            {
            aArray.Append(KMTPDeviceDpSupportedOperations[i]);
            }
        }
        break;

    case EStorageSystemTypes:
        aArray.Append(CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
        break;

    case EObjectProperties:
        {
        TInt count(sizeof(KMTPDeviceDpSupportedObjectProperties) / sizeof(KMTPDeviceDpSupportedObjectProperties[0]));
        for (TInt i(0); (i < count); i++)
            {
            aArray.Append(KMTPDeviceDpSupportedObjectProperties[i]);
            }
        }
        break;

    default:
        // Unrecognised category, leave aArray unmodified.
        break;
        }
    __FLOG(_L8("Supported - Exit"));
    }

void CMTPDeviceDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /*aError*/)
	{
    __FLOG(_L8("NotifyEnumerationCompleteL - Entry"));
    __ASSERT_DEBUG((aStorageId == iPendingEnumerations[KMTPDeviceDpActiveEnumeration]), User::Invariant());
    if (iPendingEnumerations.Count() > 0)
        {
        iPendingEnumerations.Remove(KMTPDeviceDpActiveEnumeration);
        }
	switch(iEnumeratingState)
		{
	case EEnumeratingDeviceDataStore:
	case EEnumeratingFolders:
	    iEnumeratingState = EEnumerationComplete;
	    Framework().ObjectEnumerationCompleteL(aStorageId);
		//iEnumerator->StartL(iPendingEnumerations[KMTPDeviceDpActiveEnumeration]);
		break;
	case EEnumerationComplete:
	default:
		__DEBUG_ONLY(User::Invariant());
		break;
		}
    __FLOG(_L8("NotifyEnumerationCompleteL - Exit"));
	}

/**
Constructor.
*/
CMTPDeviceDataProvider::CMTPDeviceDataProvider(TAny* aParams) :
    CMTPDataProviderPlugin(aParams),
    iActiveProcessors(KMTPDeviceDpSessionGranularity),
    iActiveProcessor(-1),
    iDeviceInfoTimer(NULL),
    iCommandState(EIdle)
    {

    }

/**
Load devdp extension plugins if present
*/
void CMTPDeviceDataProvider::LoadExtnPluginsL()
	{

	RArray<TUint> extnUidArray;
	CleanupClosePushL(extnUidArray);
	iDevDpSingletons.ConfigMgr().GetRssConfigInfoArrayL( extnUidArray, EDevDpExtnUids);

	TInt count = extnUidArray.Count();
	for (TInt i = 0; i < count; i++)
		{

		CDevDpExtnPluginMap* extnpluginMap = NULL;
		extnpluginMap = CDevDpExtnPluginMap::NewL(*this, TUid::Uid(extnUidArray[i]));

		if(extnpluginMap )
			{
			iExtnPluginMapArray.Append(extnpluginMap);
			}

		}
	CleanupStack::PopAndDestroy(&extnUidArray);
	}

void CMTPDeviceDataProvider::AddFolderRecursiveL( const TMTPNotificationParamsFolderChange& aFolder )
    {
    __FLOG(_L8("AddFolderRecursiveL - Entry"));
    
    TPtrC folderRight( aFolder.iFolderChanged );
    __FLOG_VA((_L16("Folder Addition - DriveAndFullPath:%S"), &folderRight ));
    
    if ( !BaflUtils::FolderExists( Framework().Fs(), folderRight ))
    	{
    	__FLOG(_L8("Folder not exist in file system"));
    	User::Leave( KErrArgument );
    	}
    
    TUint32 parentHandle( KMTPHandleNoParent );
    TUint32 handle( KMTPHandleNoParent );
    TInt pos( KErrNotFound );
    TInt lengthOfRight( folderRight.Length());
    TFileName folderLeft;
    
    _LIT( KRootFolder, "?:\\");
    
    /*
    Go through from beginning.
    when this while end, folderLeft keeps the top
    layer folder which has no handle
    */
    do 
        {
        pos = folderRight.Locate( KPathDelimiter );
        if ( KErrNotFound == pos )
            {
            break;
            }
        folderLeft.Append( folderRight.Left( pos + 1 ));
        lengthOfRight = folderRight.Length()-pos -1;
        folderRight.Set( folderRight.Right( lengthOfRight ));
        
        if ( KErrNotFound != folderLeft.Match( KRootFolder ))
        	{
        	//first time, root folder
        	//continue
        	continue;
        	}
        parentHandle = handle;
        handle = Framework().ObjectMgr().HandleL( folderLeft );
        }
    while( KMTPHandleNone != handle );
    

    if ( KMTPHandleNone == handle )
        {
        __FLOG(_L8("need to add entry into mtp database"));
        
        CMTPObjectMetaData* folderObject = CMTPObjectMetaData::NewL();
        TUint32 storageId = GetStorageIdL( folderLeft );
        
        while( 1 )
            {
            parentHandle = AddEntryL( folderLeft, parentHandle, storageId, *folderObject );
            OnDeviceFolderChangedL( EMTPEventCodeObjectAdded, *folderObject );

            pos = folderRight.Locate( KPathDelimiter );
            lengthOfRight = folderRight.Length()-pos -1;
            if ( KErrNotFound == pos )
            	{
            	break;
            	}
            folderLeft.Append( folderRight.Left( pos + 1  ));
            folderRight.Set( folderRight.Right( lengthOfRight ));
            }
            
        delete folderObject;
        }
    
    __FLOG(_L8("AddFolderRecursiveL - Exit"));
    }
    
TUint32 CMTPDeviceDataProvider::AddEntryL( const TDesC& aPath, TUint32 aParentHandle, TUint32 aStorageId, CMTPObjectMetaData& aObjectInfo  )
    {
    __FLOG(_L8("AddEntryL - Entry"));
    
    TBool isFolder( EFalse );
    BaflUtils::IsFolder( Framework().Fs(), aPath, isFolder );
    
    __ASSERT_ALWAYS( isFolder, User::Leave( KErrArgument ));
    __ASSERT_ALWAYS( aParentHandle != KMTPHandleNone, User::Leave( KErrArgument ));
    __ASSERT_ALWAYS( Framework().StorageMgr().ValidStorageId( aStorageId ), User::Invariant());

    __FLOG_VA((_L16("Add Entry for Path:%S"), &aPath ));
    aObjectInfo.SetUint( CMTPObjectMetaData::EDataProviderId, Framework().DataProviderId() );
    aObjectInfo.SetUint( CMTPObjectMetaData::EFormatCode, EMTPFormatCodeAssociation );
    aObjectInfo.SetUint( CMTPObjectMetaData::EStorageId, aStorageId );
    aObjectInfo.SetDesCL( CMTPObjectMetaData::ESuid, aPath );
    aObjectInfo.SetUint( CMTPObjectMetaData::EFormatSubCode, EMTPAssociationTypeGenericFolder );
    aObjectInfo.SetUint( CMTPObjectMetaData::EParentHandle, aParentHandle );
    aObjectInfo.SetUint( CMTPObjectMetaData::ENonConsumable, EMTPConsumable );
    
    //For example 
    //C:\\Documents\\Sample\\Sample1\\
    //Then "Sample1" is inserted into folderObjects
    TUint length = aPath.Length()-1;//remove '\'
    TPtrC tailFolder( aPath.Ptr(), length );
    TInt pos = tailFolder.LocateReverse( KPathDelimiter ) + 1;
    tailFolder.Set( tailFolder.Right(length - pos));

    aObjectInfo.SetDesCL( CMTPObjectMetaData::EName, tailFolder );
    
    Framework().ObjectMgr().InsertObjectL( aObjectInfo );
    __FLOG(_L8("AddEntryL - Exit"));
    
    return aObjectInfo.Uint( CMTPObjectMetaData::EHandle );
    }

TUint32 CMTPDeviceDataProvider::GetStorageIdL( const TDesC& aPath )
    {
    __FLOG(_L8("GetStorageId - Entry"));

    TChar driveLetter = aPath[0];
    TInt drive;
    User::LeaveIfError( Framework().Fs().CharToDrive( driveLetter, drive ));
    	
    __FLOG(_L8("GetStorageId - Exit"));
    
    return Framework().StorageMgr().FrameworkStorageId( static_cast<TDriveNumber>( drive ));
    }

void CMTPDeviceDataProvider::OnDeviceFolderChangedL( TMTPEventCode aEventCode, CMTPObjectMetaData& aObjectInfo )
    {
    __FLOG(_L8("OnDeviceFolderChangedL - Entry"));
    
    iEvent.Reset();
    
    switch( aEventCode )
        {
    case EMTPEventCodeObjectAdded:
        {
        __FLOG(_L8("Send event for object add"));
        iEvent.SetUint16( TMTPTypeEvent::EEventCode, EMTPEventCodeObjectAdded );
        iEvent.SetUint32( TMTPTypeEvent::EEventSessionID, KMTPSessionAll );
        iEvent.SetUint32( TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone );
        TUint32 handle = aObjectInfo.Uint( CMTPObjectMetaData::EHandle );
        iEvent.SetUint32( TMTPTypeEvent::EEventParameter1, handle );
        }
        break;
    default:
        break;
        }
    
    Framework().SendEventL(iEvent);
    
    __FLOG(_L8("OnDeviceFolderChangedL - Exit"));
    }

/**
Second phase constructor.
*/
void CMTPDeviceDataProvider::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    iDevDpSingletons.OpenL(Framework());
    iPtrDataStore = &(iDevDpSingletons.DeviceDataStore());
    iDpSingletons.OpenL(Framework());

    iExclusionMgr = CMTPDevDpExclusionMgr::NewL(Framework());
    iDpSingletons.SetExclusionMgrL(*iExclusionMgr);

    iStorageWatcher = CMTPStorageWatcher::NewL(Framework());

    const TUint KProcessLimit = iDevDpSingletons.ConfigMgr().UintValueL(CMTPDeviceDpConfigMgr::EEnumerationIterationLength);

 	TRAPD(err, LoadExtnPluginsL());
 //	__ASSERT_DEBUG((err == KErrNone), Panic(_L("Invalid resource file ")));
 	if(KErrNone != err)
		{
	    __FLOG(_L8("\nTere is an issue in loading the plugin !!!!!\n"));
		}

    iEnumerator = CMTPFSEnumerator::NewL(Framework(), iDpSingletons.ExclusionMgrL(), *this, KProcessLimit);
    iFrameWork.OpenL();
    
    __FLOG(_L8("ConstructL - Exit"));

    }

void CMTPDeviceDataProvider::OnDevicePropertyChangedL (TMTPDevicePropertyCode& aPropCode)
	{
	iEvent.Reset();
	iEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeDevicePropChanged );
	iEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
	iEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
	iEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aPropCode);
	Framework().SendEventL(iEvent);
	}

/**
 *This method will return the reference to MMTPDataProviderFramework from CMTPDataProviderPlugin
 */
MMTPDataProviderFramework& CMTPDeviceDataProvider::DataProviderFramework ()
	{
	return Framework();
	}

TInt CMTPDeviceDataProvider::FindExtnPlugin(TUint aOpcode)
	{
	TInt noOfEtxnPlugins = iExtnPluginMapArray.Count();
	for(TInt i=0; i < noOfEtxnPlugins; i++)
		{
		if(iExtnPluginMapArray[i]->SupportedOpCodes()->Find(aOpcode)!= KErrNotFound)
			{
			return i;
			}
		}
	return KErrNotFound;
	}
/**
Find or create a request processor that can process the request
@param aRequest    The request to be processed
@param aConnection The connection from which the request comes
@return the index of the found/created request processor
*/
TInt CMTPDeviceDataProvider::LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __FLOG(_L8("LocateRequestProcessorL - Entry"));
    TInt index = KErrNotFound;
    TInt count = iActiveProcessors.Count();
    for(TInt i = 0; i < count; i++)
        {
        if(iActiveProcessors[i]->Match(aRequest, aConnection))
            {
            index = i;
            break;
            }
        }
    if(index == KErrNotFound)
        {
        MMTPRequestProcessor* processor = MTPDeviceDpProcessor::CreateL(Framework(), aRequest, aConnection);
        __ASSERT_DEBUG(processor, Panic(EMTPDevDpNoMatchingProcessor));
        CleanupReleasePushL(*processor);
        iActiveProcessors.AppendL(processor);
        TUint16 operationCode(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));

       if (operationCode >= EMTPOpCodeGetDevicePropDesc && operationCode <=EMTPOpCodeResetDevicePropValue)
		{
		TUint propCode = aRequest.Uint32(TMTPTypeRequest::ERequestParameter1);
		TInt foundplugin = FindExtnPlugin (propCode);
		if(foundplugin!= KErrNotFound)
			{
			iDevDpSingletons.DeviceDataStore().SetExtnDevicePropDp(iExtnPluginMapArray[foundplugin]->ExtPlugin());
			}
		}
	        CleanupStack::Pop();
	        index = count;
        }
    __FLOG(_L8("LocateRequestProcessorL - Exit"));
    return index;
    }

/**
Finds a request processor that can process the event
@param aEvent    The event to be processed
@param aConnection The connection from which the request comes
@return the index of the found request processor, KErrNotFound if not found
*/
TInt CMTPDeviceDataProvider::LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    __FLOG(_L8("LocateRequestProcessorL - Entry"));
    TInt index = KErrNotFound;
    TInt count = iActiveProcessors.Count();
    for(TInt i = 0; i < count; i++)
	{
	if(iActiveProcessors[i]->Match(aEvent, aConnection))
		{
		index = i;
		break;
		}
	}
    __FLOG(_L8("LocateRequestProcessorL - Exit"));
    return index;
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPDeviceDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    __FLOG(_L8("SessionClosedL - Entry"));
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
    __FLOG_VA((_L8("current state is =%d"), iCommandState));    
    if(iCommandState != EIdle)
        {
        if(iDeviceInfoTimer)
            {
            delete iDeviceInfoTimer;
            iDeviceInfoTimer = NULL;
            }
        iCommandState = EIdle;
        iDevDpSingletons.DeviceDataStore().SetConnectMac(EFalse);
        }
    __FLOG(_L8("SessionClosedL - Exit"));
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef __FLOG_ACTIVE
void CMTPDeviceDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPDeviceDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    __FLOG(_L8("SessionOpenedL - Entry"));
    __FLOG_VA((_L8("SessionID = %d"), aSession.iMTPId));
    __FLOG(_L8("SessionOpenedL - Exit"));
    }

void CMTPDeviceDataProvider::SetConnectMac()
    {
    __FLOG(_L8("SetConnectMac - Entry"));   
    iDevDpSingletons.DeviceDataStore().SetConnectMac(ETrue);
    __FLOG_VA((_L8("previous state = %d, current is ESetIsMac"), iCommandState));    
    iCommandState = ESetIsMac;
    __FLOG(_L8("SetConnectMac - Exit"));     
    }

/**
CMTPDeviceInfoTimer factory method. 
@return A pointer to a new CMTPDeviceInfoTimer instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPDeviceInfoTimer* CMTPDeviceInfoTimer::NewL(CMTPDeviceDataProvider& aDeviceProvider)
    {
    CMTPDeviceInfoTimer* self = new (ELeave) CMTPDeviceInfoTimer(aDeviceProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor.
*/
CMTPDeviceInfoTimer::~CMTPDeviceInfoTimer()
    {
    Cancel();
    iLdd.Close();
    __FLOG_CLOSE;    
    }
    
/**
Starts the deviceinfo timer.
*/
// DeviceInfo delay, in microseconds. 5s
const TUint KMTPDeviceInfoDelay = (1000000 * 5);
void CMTPDeviceInfoTimer::Start()
    {
    __FLOG(_L8("CMTPDeviceInfoTimer::Start - Entry"));    

    After(KMTPDeviceInfoDelay);
    iState = EStartTimer;
    __FLOG(_L8("CMTPDeviceInfoTimer::Start - Exit"));     
    }
    
void CMTPDeviceInfoTimer::RunL()
    {
    __FLOG(_L8("CMTPDeviceInfoTimer::RunL - Entry"));
    __FLOG_VA((_L8("iStatus == %d"), iStatus.Int()));    

    switch(iState)
        {
        case EStartTimer:
            __FLOG(_L8("CMTPDeviceInfoTimer::RunL - EStartTimer"));
            // Open the USB device interface.
            User::LeaveIfError(iLdd.Open(0));
            iLdd.ReEnumerate(iStatus);
            iDeviceProvider.SetConnectMac();
            iState = EUSBReEnumerate;
            SetActive();
            break;
        case EUSBReEnumerate:          
            __FLOG(_L8("CMTPDeviceInfoTimer::RunL - EUSBReEnumerate"));            
            break;
        default:
            __FLOG(_L8("CMTPDeviceInfoTimer::RunL - default")); 
            break;
        }
    __FLOG(_L8("CMTPDeviceInfoTimer::RunL - Exit"));    
    }
    
/** 
Constructor
*/
CMTPDeviceInfoTimer::CMTPDeviceInfoTimer(CMTPDeviceDataProvider& aDeviceProvider) : 
    CTimer(EPriorityNormal),iDeviceProvider(aDeviceProvider),iState(EIdle)
    {
    
    }    

/**
Second phase constructor.
*/    
void CMTPDeviceInfoTimer::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    }
