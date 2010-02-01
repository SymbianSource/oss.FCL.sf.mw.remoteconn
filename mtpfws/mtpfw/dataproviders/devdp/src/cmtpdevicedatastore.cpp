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

#include <badesca.h>
#include <bautils.h>
#include <hal.h>
#include <s32file.h>
#include  <f32file.h>
#include <tz.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtptypefile.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdevicedatastore.h"
#include "cmtpframeworkconfig.h"
#include "mmtpenumerationcallback.h"
#include "mtpdevdppanic.h"
#include "mtpdevicedpconst.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeviceDataStore");)

#ifdef __WINS__
_LIT( KFileName, "c:\\private\\102827a2\\mtpdevice.ico");
#else
_LIT( KFileName, "z:\\private\\102827a2\\mtpdevice.ico");
#endif


// Device property datastore constants.
_LIT(KMTPNoBackupFolder, "nobackup\\");
_LIT(KMTPDevicePropertyStoreFileName, "mtpdevicepropertystore.dat");
_LIT(KMTPDevicePropertyStoreDrive, "c:");
_LIT(KMTPVendorExtensionSetDelimiter, "; ");

//Default Session Initiator Version Information
_LIT(KDefaultSessionInitiatorVersionInfo,"None");
//Default Perceived DeviceType information
static const TUint32 DefaultPerceivedDeviceType = 0;

/*In order to ensure a reasonable RunL() duration, 8 dps at maximum are
  queried in one RunL() invocation.
*/
static const TUint KExtensionSetIterationRunLength = 8;

// The maximum number of extensions each dp support.
static const TUint KExtensionSetGranularity = 4;

//#define for PERCIVED_DEVICETYPE
static const TUint KPercivedDeviceType = 3;
//maximum length for date time string
static const TUint KMaxDateTimeLength = 28;
//minimum length for date time string
static const TUint KMinDateTimeLength = 15;
//position of date and time seperator 'T'
static const TUint KPosDelemT = 8;
//buffer size for reading iconfile
static const TUint KBufferSize = 1024;

//enum for time zone
enum TDevDPTypeTimeOffset
	{
	EDEVDPTypeZero = 0,
	EDEVDPTypeMinus ,
	EDEVDPTypePlus,
	EDEVDPTypeNotDefined,
	};


/**
MTP device information data store factory method.
@return A pointer to an MTP device information data store. Ownership IS
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPDeviceDataStore* CMTPDeviceDataStore::NewL()
    {
    CMTPDeviceDataStore* self = new (ELeave) CMTPDeviceDataStore();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
CMTPDeviceDataStore::~CMTPDeviceDataStore()
    {
    __FLOG(_L8("~CMTPDeviceDataStore - Entry"));
    Cancel();
    delete iDeviceFriendlyNameDefault;
    delete iSyncPartnerNameDefault;
    delete iDeviceFriendlyName;
    delete iSynchronisationPartner;
    delete iTelephony;
    delete iSessionInitiatorVersionInfo;
    delete iDeviceIcon;
    delete iSupportedDevProArray;
    delete iDateTime;
    iDeviceVersion.Close();
    iSerialNumber.Close();
    iSingletons.Close();
    iMTPExtensions.Close();
    __FLOG(_L8("~CMTPDeviceDataStore - Exit"));
    __FLOG_CLOSE;
    }

/**
Check to see if a request such as BatteryLevel is currently
pending for processing. If so it's not advisable to make another
request.

@return ETrue if a request is pending EFalse otherwise.
*/

TBool CMTPDeviceDataStore::RequestPending() const
	{
	return IsActive();
	}

void CMTPDeviceDataStore::BatteryLevelL(TRequestStatus& aStatus, TUint& aBatteryLevel)
    {
    __FLOG(_L8("BatteryLevel - Entry"));

	if (RequestPending())
		{
		// We are already reading battery level
		// leave so we don't set ourselves active twice
		User::Leave(KErrInUse);
		}

    iPendingStatus = &aStatus;
	SetRequestPending(*iPendingStatus);

    if (iTelephony)
        {
        iPendingBatteryLevel    = &aBatteryLevel;
    	iTelephony->GetBatteryInfo(iStatus, iBatteryInfoV1Pckg);
    	SetState(EEnumeratingBatteryLevel);
    	SetActive();
        }
    else
        {
        aBatteryLevel = KMTPDefaultBatteryLevel;
    	SetRequestComplete(*iPendingStatus, KErrNone);
        }
    __FLOG(_L8("BatteryLevel - Exit"));
    }

/**
Provides the MTP device friendly name.
@return The MTP device friendly name.
*/
const TDesC& CMTPDeviceDataStore::DeviceFriendlyName() const
    {
    __FLOG(_L8("DeviceFriendlyName - Entry"));
    __FLOG(_L8("DeviceFriendlyName - Exit"));
    return iDeviceFriendlyName->StringChars();
    }

/**
Provides the default MTP device friendly name.
@return The default MTP device friendly name.
*/
const TDesC& CMTPDeviceDataStore::DeviceFriendlyNameDefault() const
    {
    __FLOG(_L8("DeviceFriendlyNameDefault - Entry"));
    __FLOG(_L8("DeviceFriendlyNameDefault - Exit"));
    return *iDeviceFriendlyNameDefault;
    }

/**
Provides the device firmware version identifier
@return The device firmware version identifier .
*/
const TDesC& CMTPDeviceDataStore::DeviceVersion() const
    {
    __FLOG(_L8("DeviceVersion - Entry"));
    __FLOG(_L8("DeviceVersion - Exit"));
    return iDeviceVersion;
    }

/**
Provides the device manufacturer name.
@return The device manufacturer name.
*/
const TDesC& CMTPDeviceDataStore::Manufacturer() const
    {
    __FLOG(_L8("Manufacturer - Entry"));
    __ASSERT_DEBUG(Enumerated(), Panic(EMTPDevDpInvalidState));
    __FLOG(_L8("Manufacturer - Exit"));
    return iPhoneIdV1.iManufacturer;
    }

/**
Provides the device model identifier.
@return The device model identifier.
*/
const TDesC& CMTPDeviceDataStore::Model() const
    {
    __FLOG(_L8("Model - Entry"));
    __ASSERT_DEBUG(Enumerated(), Panic(EMTPDevDpInvalidState));
    __FLOG(_L8("Model - Exit"));
    return iPhoneIdV1.iModel;
    }

/**
Provides the MTP Vendor Extension string.
@return The MTP Vendor Extension string.
*/
const TDesC& CMTPDeviceDataStore::MTPExtensions() const
    {
    __FLOG(_L8("MTPExtensions - Entry"));
    __FLOG(_L8("MTPExtensions - Exit"));
    return iMTPExtensions;
    }

/**
Provides the device serial number.
@return The device serial number.
*/
const TDesC& CMTPDeviceDataStore::SerialNumber() const
    {
    __FLOG(_L8("SerialNumber - Entry"));
    __ASSERT_DEBUG(Enumerated(), Panic(EMTPDevDpInvalidState));
    __FLOG(_L8("SerialNumber - Exit"));
    return iPhoneIdV1.iSerialNumber;
    }

/**
Provides the MTP synchronisation partner name.
@return The MTP synchronisation partner name.
*/
const TDesC& CMTPDeviceDataStore::SynchronisationPartner() const
    {
    __FLOG(_L8("SynchronisationPartner - Entry"));
    __FLOG(_L8("SynchronisationPartner - Exit"));
    return iSynchronisationPartner->StringChars();
    }

/**
Provides the default MTP synchronisation partner name.
@return The default MTP synchronisation partner name.
*/
const TDesC& CMTPDeviceDataStore::SynchronisationPartnerDefault() const
    {
    __FLOG(_L8("SynchronisationPartnerDefault - Entry"));
    __FLOG(_L8("SynchronisationPartnerDefault - Exit"));
    return *iSyncPartnerNameDefault;
    }

/**
Sets the MTP device friendly name.
@param aName The new MTP device friendly name.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::SetDeviceFriendlyNameL(const TDesC& aName)
    {
    __FLOG(_L8("SetDeviceFriendlyNameL - Entry"));
    iDeviceFriendlyName->SetL(aName);
    StoreL();
    __FLOG(_L8("SetDeviceFriendlyNameL - Exit"));
    }

/**
Sets the synchronisation partner name.
@param aName The new MTP synchronisation partner name.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::SetSynchronisationPartnerL(const TDesC& aName)
    {
    __FLOG(_L8("SetSynchronisationPartnerL - Entry"));
    iSynchronisationPartner->SetL(aName);
    StoreL();
    __FLOG(_L8("SetSynchronisationPartnerL - Exit"));
    }

/**
Initiates the MTP device data provider's device information data store
enumeration sequence. The sequence is concluded when ObjectEnumerationCompleteL
is signalled to the MTP data provider framework layer.
@param aStorageId The MTP StorageId to be enumerated.
@param aCallback Callback to be called when enumeration completes.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::StartEnumerationL(TUint32 aStorageId, MMTPEnumerationCallback& aCallback)
    {
    __FLOG(_L8("StartEnumerationL - Entry"));
    if (State() != EUndefined)
        {
        aCallback.NotifyEnumerationCompleteL(aStorageId, KErrNone);
        }
    else
        {
        iStorageId = aStorageId;
        iCallback = &aCallback;
        Schedule(EEnumeratingDevicePropertyStore);
        }
    __FLOG(_L8("StartEnumerationL - Exit"));
    }

void CMTPDeviceDataStore::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));
    if (iTelephony)
        {
        switch (State())
            {
        case EEnumeratingPhoneId:
            iTelephony->CancelAsync(CTelephony::EGetPhoneIdCancel);
            break;

        case EEnumeratingBatteryLevel:
            iTelephony->CancelAsync(CTelephony::EGetBatteryInfoCancel);
            break;

        default:
            // Nothing to do.
            break;
            }
        }
    __FLOG(_L8("DoCancel - Exit"));
    }

/**
Handles leaves occurring in RunL.
@param aError leave error code
@return KErrNone
*/
TInt CMTPDeviceDataStore::RunError(TInt aError)
    {
    __FLOG(_L8("RunError - Entry"));
    __FLOG_VA((_L8("Error = %d, State = %d"), aError, State()));
	aError = aError;	// suppress compiler warning

    switch (State())
        {
    case EEnumeratingDevicePropertyStore:
        // Error restoring device properties; use defaults.
        Schedule(EEnumeratingDeviceVersion);
        break;

    case EEnumeratingDeviceVersion:
        // Error enumerating software build information; use default.
        Schedule(EEnumeratingPhoneId);
        break;

    case EEnumeratingPhoneId:
        // Error enumerating telephony device ID information; use defaults.
        Schedule(EEnumeratingVendorExtensions);
        break;

    case EEnumeratingVendorExtensions:
        Schedule(EEnumerated);
    	break;

    case EEnumeratingBatteryLevel:
    	// This case will never occur
    case EUndefined:
    default:
        __DEBUG_ONLY(Panic(EMTPDevDpInvalidState));
        break;
        }

    __FLOG(_L8("RunError - Exit"));
    return KErrNone;
    }

void CMTPDeviceDataStore::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    switch (State())
        {
    case EEnumeratingDevicePropertyStore:
        RestoreL();
        Schedule(EEnumeratingDeviceVersion);
        break;

    case EEnumeratingDeviceVersion:
        {
        TInt buildNo(0);
        User::LeaveIfError(HAL::Get(HALData::EManufacturerSoftwareBuild, buildNo));
        __FLOG_VA((_L8("EManufacturerSoftwareBuild = %d "), buildNo));
        iDeviceVersion.Format(_L("%d"), buildNo);
        Schedule(EEnumeratingPhoneId);
        }
        break;

    case EEnumeratingPhoneId:
        if (!iTelephony)
            {
            iTelephony  = CTelephony::NewL();
            iTelephony->GetPhoneId(iStatus, iPhoneIdV1Pckg);
            SetActive();
            }
        else
            {
            StoreFormattedSerialNumber(iPhoneIdV1.iSerialNumber);
            Schedule(EEnumeratingVendorExtensions);
            }
        break;

    case EEnumeratingVendorExtensions:
    	{
    	TBool isCompleted = EFalse;
    	AppendMTPExtensionSetsL(isCompleted);
    	if (isCompleted)
    		{
    		Schedule(EEnumerated);
    		}
    	else
    		{
	        Schedule(EEnumeratingVendorExtensions);
    		}
    	}
    	break;

    case EEnumeratingBatteryLevel:
        *iPendingBatteryLevel   = iBatteryInfoV1.iChargeLevel;
    	SetRequestComplete(*iPendingStatus, KErrNone);
        SetState(EEnumerated);
        break;

    case EEnumerated:
        if (iCallback)
            {
            iCallback->NotifyEnumerationCompleteL(iStorageId, KErrNone);
            iCallback = NULL;
            iStorageId = KMTPNotSpecified32;
            }
        break;

case EEnumeratedBatteryLevel :
	*iPendingBatteryLevel	= iBatteryInfoV1.iChargeLevel;
	SetRequestComplete(*iPendingStatus, KErrNone);
	SetState(EEnumerated);
	break;
	
    case EUndefined:
    default:
        __DEBUG_ONLY(Panic(EMTPDevDpInvalidState));
        break;
        }
    __FLOG(_L8("RunL - Exit"));
    }

/**
Constructor.
@param aConnectionMgr The MTP connection manager interface.
*/
CMTPDeviceDataStore::CMTPDeviceDataStore() :
    CActive(EPriorityStandard),
	iBatteryInfoV1Pckg(iBatteryInfoV1),
	iPhoneIdV1Pckg(iPhoneIdV1)
    {
    CActiveScheduler::Add(this);
    }

/**
Second phase constructor.
*/
void CMTPDeviceDataStore::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    iSingletons.OpenL();

    /*
    Set the default values.

        1.  Device friendly name.
    */
    iDeviceFriendlyNameDefault = iSingletons.FrameworkConfig().ValueL(CMTPFrameworkConfig::EDeviceFriendlyName);
    iDeviceFriendlyName = CMTPTypeString::NewL(*iDeviceFriendlyNameDefault);

    //  2.  Synchronization partner name.
    iSyncPartnerNameDefault = iSingletons.FrameworkConfig().ValueL(CMTPFrameworkConfig::ESynchronizationPartnerName);
    iSynchronisationPartner = CMTPTypeString::NewL(*iSyncPartnerNameDefault);

    //  3.  Device Version.
    iDeviceVersion.CreateL(KMTPDefaultDeviceVersion);

    //  4.  Manufacturer.
    iPhoneIdV1.iManufacturer    = KMTPDefaultManufacturer;

    //  5.  Model.
    iPhoneIdV1.iModel           = KMTPDefaultModel;

    //  6.  Serial Number.
    StoreFormattedSerialNumber(KMTPDefaultSerialNumber);

    //  7.  Vendor Extensions
    iMTPExtensions.CreateL(KMTPMaxStringCharactersLength);

   // 8. Session Initiator version Info
    iSessionInitiatorVersionInfo = CMTPTypeString::NewL(KDefaultSessionInitiatorVersionInfo);

   //9. Percived device type property.
   //value for mobile handset is 0x00000003
   iPerceivedDeviceType.Set(KPercivedDeviceType);

   //10 date time property. no need to creat the string right now, create the
   //date time string whe it is requested.
   TBuf<30> dateTimeString;
   iDateTime = CMTPTypeString::NewL(dateTimeString);

   //11 Device Icon property, Load the icon into Auint Atrray
   LoadDeviceIconL();

    __FLOG(_L8("ConstructL - Exit"));
    }

/**
Indicates if the device information data store is in the EEnumerated state.
@return ETrue if the device data store state is enumerated, otherwiese EFalse.
*/
TBool CMTPDeviceDataStore::Enumerated() const
    {
    __FLOG(_L8("Enumerated - Entry"));
    TInt32 state(State());
    __FLOG(_L8("Enumerated - Exit"));
    return (state & EEnumerated);
    }

/**
Externalizes device properties to the device property store.
@param aWriteStream the stream to externalize the device properties
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::ExternalizeL(RWriteStream& aWriteStream) const
    {
    __FLOG(_L8("ExternalizeL - Entry"));
    aWriteStream.WriteInt32L(KMTPDevicePropertyStoreVersion);
    aWriteStream << DeviceFriendlyName();
    aWriteStream << SynchronisationPartner();
    __FLOG(_L8("ExternalizeL - Exit"));
    }

/**
Internalises device properties from the device property store.
@param aReadStream The device property store input data stream.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::InternalizeL(RReadStream& aReadStream)
    {
    __FLOG(_L8("InternalizeL - Entry"));
    RBuf buf;
    buf.CleanupClosePushL();

    /*
    Read the device property store version. This is present for future
    expansion but is currently ignored.
    */
    TInt32 version(aReadStream.ReadInt32L());

    // Read the device friendly name.
    buf.CreateL(aReadStream, KMTPMaxStringCharactersLength);
    iDeviceFriendlyName->SetL(buf);
    buf.Close();

    // Read the synchronisation partner name.
    buf.CreateL(aReadStream, KMTPMaxStringCharactersLength);
    iSynchronisationPartner->SetL(buf);
    buf.Close();

    CleanupStack::Pop();  //buf
    __FLOG(_L8("InternalizeL - Exit"));
    }

/**
Provides the full pathname of the device property store.
@return The full pathname of the device property store.
*/
const TDesC& CMTPDeviceDataStore::PropertyStoreName()
    {
    __FLOG(_L8("PropertyStoreName - Entry"));
    if (iPropertyStoreName.Length() == 0)
        {
        iSingletons.Fs().PrivatePath(iPropertyStoreName);
        iPropertyStoreName.Insert(0, KMTPDevicePropertyStoreDrive);
        iPropertyStoreName.Append(KMTPNoBackupFolder);
        iPropertyStoreName.Append(KMTPDevicePropertyStoreFileName);
        }

    __FLOG(_L8("PropertyStoreName - Exit"));
    return iPropertyStoreName;
    }

/**
Query all dps for the MTP Vendor extensions set.
@return ETrue if finished querying all the dps, otherwise EFalse.
*/
void CMTPDeviceDataStore::AppendMTPExtensionSetsL(TBool& aCompleted)
    {
    __FLOG(_L8("AppendMTPExtensionSetsL - Entry"));
	CMTPDataProviderController& dps(iSingletons.DpController());
	const TInt count = Min<TInt>(iCurrentDpIndex + KExtensionSetIterationRunLength, dps.Count());
	aCompleted = EFalse;
	CDesCArraySeg* extensions = new (ELeave) CDesCArraySeg(KExtensionSetGranularity);
	CleanupStack::PushL(extensions);
	while (!aCompleted && (iCurrentDpIndex < count))
		{
		CMTPDataProvider& dp(dps.DataProviderByIndexL(iCurrentDpIndex));
		dp.Plugin().SupportedL(EVendorExtensionSets, *extensions);
		//append the mtp extension string.
		TInt n = extensions->Count();
		TInt len = iMTPExtensions.Length();
		for (TInt i = 0; i < n; i++)
			{
			len += (*extensions)[i].Length() + KMTPVendorExtensionSetDelimiter().Length();
			if (len > KMTPMaxStringCharactersLength)
				{
				__FLOG(_L8("MTP Extensions set exceeded the maximum MTP String length"));
				// End querying dps when the extension set exceeds the maximum mtp string length.
				aCompleted = ETrue;
				break;
				}
			else
				{
				if ( KErrNotFound == iMTPExtensions.Find((*extensions)[i]) )
					{
					iMTPExtensions.Append((*extensions)[i]);
					iMTPExtensions.Append(KMTPVendorExtensionSetDelimiter);
					}
				}
			}
		extensions->Reset();
		iCurrentDpIndex++;
		}
	CleanupStack::PopAndDestroy(extensions);

	if(!aCompleted && iCurrentDpIndex >= dps.Count())
		{
		aCompleted = ETrue;
		}

	__FLOG(_L8("AppendMTPExtensionSetsL - Exit"));
    }

/**
Loads device properties from the device property store.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::RestoreL()
    {
    __FLOG(_L8("RestoreL - Entry"));
    RFs& fs(iSingletons.Fs());
    if(BaflUtils::FileExists(fs, PropertyStoreName()))
        {
        CFileStore* store(CDirectFileStore::OpenLC(fs, PropertyStoreName(), EFileRead));
        RStoreReadStream instream;
        instream.OpenLC(*store, store->Root());
        InternalizeL(instream);
        CleanupStack::PopAndDestroy(2, store); // instream, store
        }
    __FLOG(_L8("RestoreL - Exit"));
    }

/**
Schedules the device information data store to transition to the specified state.
@param aState The new data stream state.
*/
void CMTPDeviceDataStore::Schedule(TInt32 aState)
    {
    __FLOG(_L8("Schedule - Entry"));
    SetState(aState);
    SetRequestPending(iStatus);
    SetActive();
    SetRequestComplete(iStatus,KErrNone);
    __FLOG(_L8("Schedule - Exit"));
    }

/**
Completes the supplied asynchronous request completion status.
*/
void CMTPDeviceDataStore::SetRequestComplete(TRequestStatus& aRequest, TUint aErr)
    {
    __FLOG(_L8("CompleteRequest - Entry"));
    TRequestStatus* status(&aRequest);
    User::RequestComplete(status, aErr);
    __FLOG(_L8("CompleteRequest - Exit"));
    }


/**
Initialises the supplied asynchronous request completion status.
*/
void CMTPDeviceDataStore::SetRequestPending(TRequestStatus& aRequest)
    {
    __FLOG(_L8("SetRequestPending - Entry"));
    aRequest = KRequestPending;
    __FLOG(_L8("SetRequestPending - Exit"));
    }

/**
Sets the device information data store state variable.
@param aState The new data stream state.
*/
void CMTPDeviceDataStore::SetState(TInt32 aState)
    {
    __FLOG(_L8("SetState - Entry"));
    iState = ((EEnumerated & iState) | aState);
    __FLOG_VA((_L8("State set to 0x%08X"), iState));
    __FLOG(_L8("SetState - Exit"));
    }

/**
Provides the device information data store state variable value.
@return The device information data store state variable value.
*/
TInt32 CMTPDeviceDataStore::State() const
    {
    __FLOG(_L8("State - Entry"));
    __FLOG_VA((_L8("State = 0x%08X"), iState));
    __FLOG(_L8("State - Exit"));
    return iState;
    }

/**
Stores device properties in the device property store.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPDeviceDataStore::StoreL()
    {
    __FLOG(_L8("StoreL - Entry"));
    CFileStore* store(CDirectFileStore::ReplaceLC(iSingletons.Fs(), PropertyStoreName(), EFileWrite));
    store->SetTypeL(KDirectFileStoreLayoutUid);
    RStoreWriteStream outstream;
    TStreamId id = outstream.CreateLC(*store);
    ExternalizeL(outstream);
    outstream.CommitL();
    CleanupStack::PopAndDestroy(&outstream);
    store->SetRootL(id);
    store->CommitL();
    CleanupStack::PopAndDestroy(store);
    __FLOG(_L8("StoreL - Exit"));
    }

/**
Formats the specified serial number as a valid MTP Serial Number string. The
MTP specification recommends that the serial number always be represented as
exactly 32 characters, with leading zeros as required.
*/
void CMTPDeviceDataStore::StoreFormattedSerialNumber(const TDesC& aSerialNo)
    {
    __FLOG(_L8("FormatSerialNumber - Entry"));
    TBuf<KMTPSerialNumberLength> formatted;
    if (aSerialNo.Length() < KMTPSerialNumberLength)
        {
        formatted = aSerialNo;
        }
    else
        {
        /*
        Supplied serial number data is greater than or equal to the required
        32 characters. Extract the least significant 32 characters.
        */
        formatted = aSerialNo.Right(KMTPSerialNumberLength);
        }

    // Store the formatted serial number.
    iPhoneIdV1.iSerialNumber = formatted;

    __FLOG(_L8("FormatSerialNumber - Exit"));
    }

/**
* Get method for Session initiator version info(0xD406).
*
* @return TDesC& : session initiator version info
*/
const TDesC& CMTPDeviceDataStore::SessionInitiatorVersionInfo() const
	{
	__FLOG(_L8("SessionInitiatorVersionInfo - Entry:Exit"));
	return iSessionInitiatorVersionInfo->StringChars();
	}

/**
* Get method for Session initiator version info Default value.
*
* @return TDesC& : session initiator version info default.
*/
const TDesC& CMTPDeviceDataStore::SessionInitiatorVersionInfoDefault() const
	{
	__FLOG(_L8("SessionInitiatorVersionInfoDefault - Entry:Exit"));
	return KDefaultSessionInitiatorVersionInfo;
	}

/**
* Set method for Session initiator version info(0xD406).
*
* @Param TDesC& : session initiator version info from the initiator
*/
void CMTPDeviceDataStore::SetSessionInitiatorVersionInfoL(const TDesC& aVerInfo)
	{
	 __FLOG(_L8("SetDeviceFriendlyNameL - Entry"));
	 iSessionInitiatorVersionInfo->SetL(aVerInfo);
	 StoreL();
	 __FLOG(_L8("SetDeviceFriendlyNameL - Exit"));
	}

/**
* Get method for PerceivedDeviceTypeDefault(0x00000000	Generic).
*
* @return TUint32: return value for PerceivedDeviceTypeDefault
*/
TUint32 CMTPDeviceDataStore::PerceivedDeviceTypeDefault() const
	{
	__FLOG(_L8("SessionInitiatorVersionInfoDefault - Entry:Exit"));
	return DefaultPerceivedDeviceType;
	}

/**
* Get method for PerceivedDeviceType(0x00000003 Mobile Handset).
* possible values for PerceivedDeviceType are
* 0x00000000	Generic.
* 0x00000001	Still Image/Video Camera.
* 0x00000002	Media (Audio/Video) Player.
* 0x00000003	Mobile Handset.
* 0x00000004	Digital Video Camera.
* 0x00000005	Personal Information Manager/Personal Digital Assistant.
* 0x00000006	Audio Recorder.
* @return TUint32: return value for PerceivedDeviceType.
*/
TUint32 CMTPDeviceDataStore::PerceivedDeviceType() const
	{
	__FLOG(_L8("SessionInitiatorVersionInfo - Entry:Exit"));
	return iPerceivedDeviceType.Value();
	}

/**
* Get method for Date time.
* @return TDesC: const date time string YYYYMMDDThhmmss.s
**/
const TDesC& CMTPDeviceDataStore::DateTimeL()
	{
	__FLOG(_L8("DateTime - Entry:Exit"));
	TBuf<30> dateTimeString;
	DateTimeToStringL(dateTimeString);
	iDateTime->SetL(dateTimeString);
	__FLOG(_L8("DateTime -Exit"));
	return iDateTime->StringChars();
	}

/**
* This method to set the date time on MTP device
* incoming date time string will be having a format YYYYMMDDThhmmss.s
* it need to modify accordingly to set the time.
*
*@Param aDateTime : Date time string.
* Some modification need to be done on this method(minor change).
**/
TInt CMTPDeviceDataStore::SetDateTimeL(const TDesC& aDateTime )
    {
    __FLOG(_L8("SetDateTime - Entry"));
    TBuf<30> dateTime;
    TInt offset = User::UTCOffset().Int();
	//get actul time to set, offset  + ,- or UTC and offset from UTC in seconds.
    TInt errorCode = ValidateString(aDateTime, dateTime, offset);
    if(KErrNone == errorCode)
	{
	    StringToDateTime(dateTime);
	    iDateTime->SetL(dateTime);
	    StoreL();
	    //now set the system time by calling user SetUTCTime
	    TTime tt;
	    errorCode = tt.Set(dateTime);
            // tt is currently in an unknown time zone -- now adjust to UTC.
	    if(KErrNone == errorCode)
	   	{
                  //if date time is YYYYMMDDTHHMMSS.S or append with 'Z' then
		  TTimeIntervalSeconds utcOffset(offset);
        	  // Subtract seconds ahead, to get to UTC timezone
         	  tt -= utcOffset;
        	  __FLOG(_L8("Setting UTC time"));
        	  errorCode = User::SetUTCTime(tt);
                  __FLOG_STMT(TBuf<30> readable;)
        	  __FLOG_STMT(tt.FormatL(readable, _L("%F%Y%M%DT%H%T%SZ"));)
        	  __FLOG_1(_L("Time now: %S"), &readable);
                }
        }
        __FLOG_1(_L8("SetDateTime - Exit %d"), errorCode);

	return errorCode;
    }


/**
*This method will create a string that is compatible for MTP datet time .
* Format("YYYYMMDDThhmmss.s") microsecond part is not implemented
* yet but that can be done easly. one more function can be implemented
* for appending 0s
**/
void CMTPDeviceDataStore::DateTimeToStringL(TDes& aDateTime)
    {
    __FLOG(_L8("DateTimeToString - Entry"));
    //get home time and convert it to string
    TTime tt;
    tt.UniversalTime();    
    _LIT(KFormat,"%F%Y%M%DT%H%T%SZ");
    tt.FormatL(aDateTime, KFormat);
    __FLOG(_L8("DateTimeToString - Exit"));
	}

/**
*This method will convert MTP date time format ("YYYYMMDDThhmmss.s")to
*TTime time format YYYYMMDD:hhmmss.ssssss. Right now microsecond part is
* not implemented.
**/
void CMTPDeviceDataStore::StringToDateTime(TDes& aDateTime )
	{
    __FLOG(_L8("StringToDateTime - Entry"));
	TBuf<30> newTime;
	_LIT(KDlemMTP,"T");
	_LIT(KDlemTTime,":");
	TInt pos = aDateTime.Find(KDlemMTP)	;
	if((KErrNotFound != pos) && (KPosDelemT == pos))
		{
		const TInt KYearDigits = 4;
                const TInt KMonthDigits = 2;
                const TInt KDayDigits = 2;

                TInt month = 0;
                TLex monthLex(aDateTime.Mid(KYearDigits, KMonthDigits));
                //coverity[unchecked_value]
                monthLex.Val(month);
                
                TInt day = 0;
                TLex dayLex(aDateTime.Mid(KYearDigits+KMonthDigits, KDayDigits));
                //coverity[unchecked_value]
                dayLex.Val(day);

                _LIT(KDateFormat, "%S%02d%02d");
                TPtrC year(aDateTime.Left(KYearDigits));
                newTime.AppendFormat(KDateFormat, &year, month - 1, day - 1);

	        newTime.Append(KDlemTTime);
		newTime.Append(aDateTime.Mid(pos + 1));
		aDateTime.Copy(newTime);
		}
	else
		{
		_LIT(KPanic, "date time ");
		User::Panic(KPanic, 3);
		}
        __FLOG_1(_L("Processed DateTime: %S"), &aDateTime);	
	__FLOG(_L8("StringToDateTime - Exit"));
	}

/**
*This method to validate the incoming date time string
*Any incoming string from intiator supposed to be either  YYYYMMDDThhmmss.s
* or YYYYMMDDT(Z/+/-)hhmmss.s form
*1.validation is done based on minimum length of the string
*2.based on the carector and its position.
*char allowded are 'Z', 'T', '.','+' and '-'
*/
TInt CMTPDeviceDataStore::ValidateString(const TDesC& aDateTimeStr, TDes& aDateTime, TInt &aOffsetVal)
	{
        __FLOG(_L8("ValidateString - Entry"));
        __FLOG_1(_L("Supplied date: %S"), &aDateTimeStr);
	_LIT(KDlemMTP,"T");
	TInt errCode = KErrNone;
	TInt pos = aDateTimeStr.Find(KDlemMTP);
        TInt nullCharPos = aDateTimeStr.Locate('\0');
	if ( KErrNotFound != nullCharPos )
		{		
		aDateTime.Copy( aDateTimeStr.Left(nullCharPos));
		}
	else
		{
		aDateTime.Copy(aDateTimeStr);
		}

	//1.validation is done based on minimum length of the string and pos
	if((KErrNotFound ==  pos )	|| (aDateTimeStr.Length() > KMaxDateTimeLength) ||
	    (aDateTimeStr.Length() < KMinDateTimeLength) || pos != KPosDelemT)
		{
                __FLOG_2(_L8("Invalid. pos: %d, len: %d"), pos, aDateTimeStr.Length());
		errCode = KErrGeneral;
		}
	else
		{
		//validation based on the carector and its position.
		for(TInt i =0 ; i< aDateTimeStr.Length(); i ++)
		{
		//any other string other than 'Z', 'T', '.','+' and '-' or positon is wrong return
		if(('0' > aDateTimeStr[i]) || ('9' < aDateTimeStr[i]))
			{
			switch(aDateTimeStr[i])
				{
				case 'T':
				case 't':
				if(i != pos)
					{
                                        __FLOG_1(_L8("Invalid. 'T' encountered at offset %d"), i);
					//error char at rong position
					errCode = KErrGeneral;
					}//else fine
				break;
				case 'Z':
				case 'z':
					aOffsetVal = 0;
                                        aDateTime.Copy(aDateTimeStr.Mid(0, i));
					//error char at wrong position
					if(i <KMinDateTimeLength)
						{
                                                __FLOG_1(_L8("Invalid. 'Z' encountered at offset %d"), i);
						//error char at wrong position
						errCode = KErrGeneral;
						}//else fine
					break;
				case '+':
				case '-':
					if(i < KMinDateTimeLength)
						{
						//error char at wrong position
						errCode = KErrGeneral;
                                                __FLOG_1(_L8("Invalid. '+/-' encountered at offset %d"), i);
						break;
						}
					else
						{
						const TInt KHoursDigits = 2;
                                                const TInt KMinutesDigits = 2;
                                                const TInt KSecondsPerMinute = 60;
                                                const TInt KSecondsPerHour = KSecondsPerMinute * 60;
						aDateTime.Copy(aDateTimeStr.Mid(0, i));

                                                TUint hourOffset = 0;
                                                TLex hourOffsetLex(aDateTimeStr.Mid(i+1, KHoursDigits));
                                                hourOffsetLex.Val(hourOffset);

                                                TUint minuteOffset = 0;
                                                TLex minuteOffsetLex(aDateTimeStr.Mid(i+KHoursDigits+1, KMinutesDigits));
                                                minuteOffsetLex.Val(minuteOffset);
						if ((hourOffset > 23) || (minuteOffset > 59))
                                                     {
                                                     errCode = KErrGeneral;
                                                     __FLOG_2(_L8("Invalid. Hour(%d) or Minute(%d) offset out of range."), hourOffset, minuteOffset);
                                                     break;
                                                     }

                                                     
                                                aOffsetVal = (hourOffset * KSecondsPerHour) + (minuteOffset * KSecondsPerMinute);
                                                if ('-' == aDateTimeStr[i])
							{
					                aOffsetVal = -aOffsetVal;
							}
                                                __FLOG_1(_L8("Info: Timezone offset %d seconds"), aOffsetVal);	
						}

					break;
				case '.':
				case '\0':
				if(i < KMinDateTimeLength)
					{
					//error char at wrong position
					errCode = KErrGeneral;
                                        __FLOG_1(_L8("Invalid. '.' or NULL at offset %d"), i);
					}
				break;
				default :
				//wrong char
				errCode = KErrGeneral;
                                __FLOG_2(_L8("Invalid. Character %04x at offset %d"), aDateTimeStr[i], i);
				break;
				}
		}
		if(KErrNone != errCode)
			{
		        __FLOG_2(_L("Processed date: %S, TimeZone: %ds ahead"), &aDateTimeStr, aOffsetVal);
                        __FLOG_1(_L8("ValidateString - Exit %d"), errCode);
			return errCode;
			}
		}
	}
	__FLOG_2(_L("Processed date: %S, TimeZone: %ds ahead"), &aDateTimeStr, aOffsetVal);
        __FLOG_1(_L8("ValidateString - Exit %d"), errCode);
	return errCode;
	}

/**
*This method is to load/Store the deviceIcon to the  CMTPTypeArray iDeviceIcon
*  some improvment is possible here.
*/
void CMTPDeviceDataStore::LoadDeviceIconL()
	{
	RFile rFile;
	RFs  rFs;
	//device icon should be stored in an array of type uint8
	iDeviceIcon = CMTPTypeArray::NewL(EMTPTypeAUINT8);
	TInt error = rFs.Connect();
	if(error == KErrNone )
		{
		error = rFile.Open(rFs, KFileName, EFileRead);
		//for buffer size we could have used VolumeIOParam()
		//metho to get optimal cluseter size, it could be(1, 2, 4 kb)
		//but it is not make much difference in performance if we use 1kb buffer
		TBuf8<KBufferSize> buffer;
		if(error == KErrNone )
			{
			TUint length =0;
			while(ETrue)
				{
				error = rFile.Read(buffer, KBufferSize);
				length = buffer.Length();
				if((0 != length) && ( KErrNone == error))
					{
					for(TUint index = 0; index < length; index++ )
						{
							iDeviceIcon->AppendUintL(buffer[index]);
						}
					}
				else
					{
					break;
					}
				}
			}
		rFile.Close();
		}
		rFs.Close();
	}

/**
*Get method fro DeviceIcon, return value is a type unint array.
*
*@return iDeviceIcon: array of uint
*/
const CMTPTypeArray& CMTPDeviceDataStore::DeviceIcon()
    {
     return *iDeviceIcon;
    }

/**
*This method is to store the supported device properties
*/
void CMTPDeviceDataStore::SetSupportedDevicePropertiesL(RArray<TUint>& aSupportedDevProps)
    {
    CheckDeviceIconProperties(aSupportedDevProps);
    delete iSupportedDevProArray;
    iSupportedDevProArray = NULL;
    iSupportedDevProArray =  CMTPTypeArray::NewL(EMTPTypeAUINT16, aSupportedDevProps);
    }
/**
*This method is to store the devicedp reference
*/
void CMTPDeviceDataStore::SetExtnDevicePropDp(MExtnDevicePropDp* aExtnDevicePropDp)
    {
    iExtnDevicePropDp = aExtnDevicePropDp;
    };

/**
*This method returns the devicedp reference
*/
MExtnDevicePropDp* CMTPDeviceDataStore::ExtnDevicePropDp()
{
	return iExtnDevicePropDp;
};


/**
*This method to get supported device properties
**/
const CMTPTypeArray& CMTPDeviceDataStore::GetSupportedDeviceProperties()
    {
    return *iSupportedDevProArray;
    }

/**
This method will remove the property that is not supported
As of now, will check the device icon property support
*/
void CMTPDeviceDataStore::CheckDeviceIconProperties( RArray<TUint> &aSupportedDeviceProperties)
	{
	//store the index value if device property.
	TUint index = aSupportedDeviceProperties.Find(EMTPDevicePropCodeDeviceIcon);
	if(KErrNotFound != (TInt)index)
	{
	RFs rFs;
	RFile rFile;
	TInt error = rFs.Connect();
	if(KErrNone == error)
		{
		error = rFile.Open(rFs, KFileName, EFileRead);
		}//else do nothing.
	if(KErrNone != error)
		{
		//control reach here only when there is no file exists or any other error situation
		//so remove the property from the list
		aSupportedDeviceProperties.Remove(index);
		}//else do nothing
	rFile.Close();
	rFs.Close();
	}//else nothing to do.
	}

