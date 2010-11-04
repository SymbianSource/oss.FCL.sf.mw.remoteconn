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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdevicedpTraces.h"
#endif


// Class constants.
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
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_CMTPDEVICEDATAPROVIDER_DES_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_CMTPDEVICEDATAPROVIDER_DES_EXIT );
    }

void CMTPDeviceDataProvider::Cancel()
    {

    }

void CMTPDeviceDataProvider::ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_PROCESSEVENTL_ENTRY );
    TInt index = LocateRequestProcessorL(aEvent, aConnection);
    if(index != KErrNotFound)
        {
        iActiveProcessors[index]->HandleEventL(aEvent);
        }
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_PROCESSEVENTL_EXIT );
    }

void CMTPDeviceDataProvider::ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams)
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_PROCESSNOTIFICATIONL_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_PROCESSNOTIFICATIONL_EXIT );
    }

void CMTPDeviceDataProvider::ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL_ENTRY );
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
    
    OstTrace1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, "opCode = 0x%x", opCode);
    OstTrace1(TRACE_NORMAL, DUP1_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
            "TranPort UID = 0x%x", iFrameWork.ConnectionMgr().TransportUid().iUid);
    OstTrace1(TRACE_NORMAL, DUP2_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, "CommandState = 0x%x", iCommandState);
    const static TInt32 KMTPUsbTransportUid = 0x102827B2;

    if((EMTPOpCodeGetDeviceInfo == opCode)&&(KMTPUsbTransportUid == iFrameWork.ConnectionMgr().TransportUid().iUid))
        {
        OstTrace0(TRACE_NORMAL, DUP3_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, "EMTPOpCodeGetDeviceInfo == opCode");
        //If GetDeviceInfo comes and there is no OpenSession before, the timer will start. And tread the host as Mac. 
        //Only the first GetDeviceInfo in one session will start the timer.
        if((EIdle == iCommandState)&&(NULL == iDeviceInfoTimer))
            {
            OstTrace0(TRACE_NORMAL, DUP4_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
                    "EMTPOpCodeGetDeviceInfo == opCode, start timer");
            
            iCommandState = EStartDeviceInfoTimer;
            iDeviceInfoTimer = CMTPDeviceInfoTimer::NewL(*this);
            iDeviceInfoTimer->Start();
            }
        else
            {
            OstTrace0(TRACE_NORMAL, DUP5_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
                    "EMTPOpCodeGetDeviceInfo == opCode, Not start timer");
            }
        }
    else
       {       
       OstTrace0(TRACE_NORMAL, DUP6_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
               "EMTPOpCodeGetDeviceInfo != opCode");
       if((EMTPOpCodeOpenSession == opCode)&&(EIdle == iCommandState))
            {
            OstTrace0(TRACE_NORMAL, DUP7_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
                    "EMTPOpCodeGetDeviceInfo == opCode, set CommandState to be EOpenSession");
            iCommandState = EOpenSession;
            }
       
       if(iDeviceInfoTimer)
           {
           OstTrace0(TRACE_NORMAL, DUP8_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
                   "iDeviceInfoTimer != NULL, stop timer");
           delete iDeviceInfoTimer;
           iDeviceInfoTimer = NULL;
           }
       else
           {  
           OstTrace0(TRACE_NORMAL, DUP9_CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL, 
                   "iDeviceInfoTimer == NULL, NOT stop timer");
           }
       }    
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_PROCESSREQUESTPHASEL_EXIT );
    }

void CMTPDeviceDataProvider::StartObjectEnumerationL(TUint32 aStorageId, TBool /*aPersistentFullEnumeration*/)
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_STARTOBJECTENUMERATIONL_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_STARTOBJECTENUMERATIONL_EXIT );
    }

void CMTPDeviceDataProvider::StartStorageEnumerationL()
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_STARTSTORAGEENUMERATIONL_ENTRY );
    iStorageWatcher->EnumerateStoragesL();
    Framework().StorageEnumerationCompleteL();
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_STARTSTORAGEENUMERATIONL_EXIT );
    }

void CMTPDeviceDataProvider::Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_SUPPORTED_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_SUPPORTED_EXIT );
    }

void CMTPDeviceDataProvider::NotifyEnumerationCompleteL(TUint32 aStorageId, TInt /*aError*/)
	{
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_ENTRY );
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
	OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_NOTIFYENUMERATIONCOMPLETEL_EXIT );
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
            CleanupStack::PushL(extnpluginMap);
			iExtnPluginMapArray.AppendL(extnpluginMap);
			CleanupStack::Pop(extnpluginMap);
			}

		}
	CleanupStack::PopAndDestroy(&extnUidArray);
	}

void CMTPDeviceDataProvider::AddFolderRecursiveL( const TMTPNotificationParamsFolderChange& aFolder )
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_ADDFOLDERRECURSIVEL_ENTRY );
    
    TPtrC folderRight( aFolder.iFolderChanged );
    OstTraceExt1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_ADDFOLDERRECURSIVEL, 
            "Folder Addition - DriveAndFullPath:%S", folderRight);
    
    if ( !BaflUtils::FolderExists( Framework().Fs(), folderRight ))
    	{
        OstTrace0(TRACE_NORMAL, DUP1_CMTPDEVICEDATAPROVIDER_ADDFOLDERRECURSIVEL, 
                "Folder not exist in file system");
    	User::Leave( KErrArgument );
    	}
    
    TUint32 parentHandle( KMTPHandleNoParent );
    TUint32 handle( KMTPHandleNoParent );
    TInt pos( KErrNotFound );
    TInt lengthOfRight( folderRight.Length());
    TFileName folderLeft;
    
    // get root path of storage
    TInt driveNumber;
    User::LeaveIfError(Framework().Fs().CharToDrive(folderRight[0], driveNumber));
    RBuf rootDirPath;
    rootDirPath.CreateL(KMaxFileName);
    rootDirPath.CleanupClosePushL();
    iDevDpSingletons.ConfigMgr().GetRootDirPathL(driveNumber, rootDirPath);
    rootDirPath.Insert(0, folderRight.Mid(0, 2));// get drive:
    
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
        
        if ( rootDirPath.FindF(folderLeft) != KErrNotFound)
        	{
        	//first time, root folder
        	//continue
        	continue;
        	}
        parentHandle = handle;
        handle = Framework().ObjectMgr().HandleL( folderLeft );
        }
    while( KMTPHandleNone != handle );
    CleanupStack::PopAndDestroy(&rootDirPath);

    if ( KMTPHandleNone == handle )
        {
        OstTrace0(TRACE_NORMAL, DUP2_CMTPDEVICEDATAPROVIDER_ADDFOLDERRECURSIVEL, 
                "need to add entry into mtp database");
        
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
    
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_ADDFOLDERRECURSIVEL_EXIT );
    }
    
TUint32 CMTPDeviceDataProvider::AddEntryL( const TDesC& aPath, TUint32 aParentHandle, TUint32 aStorageId, CMTPObjectMetaData& aObjectInfo  )
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_ADDENTRYL_ENTRY );
    
    TBool isFolder( EFalse );
    BaflUtils::IsFolder( Framework().Fs(), aPath, isFolder );
    
    __ASSERT_ALWAYS( isFolder, User::Leave( KErrArgument ));
    __ASSERT_ALWAYS( aParentHandle != KMTPHandleNone, User::Leave( KErrArgument ));
    __ASSERT_ALWAYS( Framework().StorageMgr().ValidStorageId( aStorageId ), User::Leave( KErrArgument ));

    OstTraceExt1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_ADDENTRYL, "Add Entry for Path:%S", aPath);
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_ADDENTRYL_EXIT );
    return aObjectInfo.Uint( CMTPObjectMetaData::EHandle );
    }

TUint32 CMTPDeviceDataProvider::GetStorageIdL( const TDesC& aPath )
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_GETSTORAGEIDL_ENTRY );

    TChar driveLetter = aPath[0];
    TInt drive;
    LEAVEIFERROR( Framework().Fs().CharToDrive( driveLetter, drive ),
            OstTraceExt1( TRACE_ERROR, CMTPDEVICEDATAPROVIDER_GETSTORAGEIDL, "can't convert driver letter %c to drive!", driveLetter));

    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_GETSTORAGEIDL_EXIT );
    
    return Framework().StorageMgr().FrameworkStorageId( static_cast<TDriveNumber>( drive ));
    }

void CMTPDeviceDataProvider::OnDeviceFolderChangedL( TMTPEventCode aEventCode, CMTPObjectMetaData& aObjectInfo )
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_ONDEVICEFOLDERCHANGEDL_ENTRY );
    
    iEvent.Reset();
    
    switch( aEventCode )
        {
    case EMTPEventCodeObjectAdded:
        {
        OstTrace0(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_ONDEVICEFOLDERCHANGEDL, "Send event for object add");
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
    
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_ONDEVICEFOLDERCHANGEDL_EXIT );
    }

/**
Second phase constructor.
*/
void CMTPDeviceDataProvider::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_CONSTRUCTL_ENTRY );
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
	    OstTrace0(TRACE_ERROR, CMTPDEVICEDATAPROVIDER_CONSTRUCTL, 
	            "\nTere is an issue in loading the plugin !!!!!\n");
		}

    iEnumerator = CMTPFSEnumerator::NewL(Framework(), iDpSingletons.ExclusionMgrL(), *this, KProcessLimit);
    iFrameWork.OpenL();

    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_CONSTRUCTL_EXIT );
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
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_LOCATEREQUESTPROCESSORL_TMTPTYPEREQUEST_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_LOCATEREQUESTPROCESSORL_TMTPTYPEREQUEST_EXIT );
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
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_LOCATEREQUESTPROCESSORL_TMTPTYPEEVENT_ENTRY );
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_LOCATEREQUESTPROCESSORL_TMTPTYPEEVENT_EXIT );
    return index;
    }

/**
Cleans up outstanding request processors when a session is closed.
@param aSession notification parameter block
*/
void CMTPDeviceDataProvider::SessionClosedL(const TMTPNotificationParamsSessionChange& aSession)
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_SESSIONCLOSEDL_ENTRY );
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
    OstTrace1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_SESSIONCLOSEDL, 
            "current state is =%d", iCommandState);
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
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_SESSIONCLOSEDL_EXIT );
    }

/**
Prepares for a newly-opened session.
@param aSession notification parameter block
*/
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPDeviceDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession)
#else
void CMTPDeviceDataProvider::SessionOpenedL(const TMTPNotificationParamsSessionChange& /*aSession*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_SESSIONOPENEDL_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_SESSIONOPENEDL, "SessionID = %d", aSession.iMTPId);
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_SESSIONOPENEDL_EXIT);
    }

void CMTPDeviceDataProvider::SetConnectMac()
    {
    OstTraceFunctionEntry0( CMTPDEVICEDATAPROVIDER_SETCONNECTMAC_ENTRY );
    iDevDpSingletons.DeviceDataStore().SetConnectMac(ETrue); 
    OstTrace1(TRACE_NORMAL, CMTPDEVICEDATAPROVIDER_SETCONNECTMAC,
            "previous state = %d, current is ESetIsMac", iCommandState);
    iCommandState = ESetIsMac; 
    OstTraceFunctionExit0( CMTPDEVICEDATAPROVIDER_SETCONNECTMAC_EXIT );
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
    }
    
/**
Starts the deviceinfo timer.
*/
// DeviceInfo delay, in microseconds. 5s
const TUint KMTPDeviceInfoDelay = (1000000 * 5);
void CMTPDeviceInfoTimer::Start()
    {
    OstTraceFunctionEntry0( CMTPDEVICEINFOTIMER_START_ENTRY );

    After(KMTPDeviceInfoDelay);
    iState = EStartTimer;
    OstTraceFunctionExit0( CMTPDEVICEINFOTIMER_START_EXIT );
    }
    
void CMTPDeviceInfoTimer::RunL()
    {
    OstTraceFunctionEntry0( CMTPDEVICEINFOTIMER_RUNL_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPDEVICEINFOTIMER_RUNL,
            "iStatus == %d", iStatus.Int());

    switch(iState)
        {
        case EStartTimer:
            OstTrace0(TRACE_NORMAL, DUP1_CMTPDEVICEINFOTIMER_RUNL, "CMTPDeviceInfoTimer::RunL - EStartTimer");
            // Open the USB device interface.
			TInt err;
			err = iLdd.Open(0);
			if ( err != KErrNone )
			{
				OstTrace0( TRACE_ERROR, DUP4_CMTPDEVICEINFOTIMER_RUNL, "Open the USB device interface error!" );
				Panic(EMTPDevDpConnectionNotFound);
			}
            iLdd.ReEnumerate(iStatus);
            iDeviceProvider.SetConnectMac();
            iState = EUSBReEnumerate;
            SetActive();
            break;
        case EUSBReEnumerate:          
            OstTrace0(TRACE_NORMAL, DUP2_CMTPDEVICEINFOTIMER_RUNL, "CMTPDeviceInfoTimer::RunL - EUSBReEnumerate");
            break;
        default:
            OstTrace0(TRACE_NORMAL, DUP3_CMTPDEVICEINFOTIMER_RUNL, "CMTPDeviceInfoTimer::RunL - default");
            break;
        }
    OstTraceFunctionExit0( CMTPDEVICEINFOTIMER_RUNL_EXIT );
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
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    }
