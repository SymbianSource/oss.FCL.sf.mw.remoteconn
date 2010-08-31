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

/**
 @file
 @internalComponent
*/
#include <f32file.h> 
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdevicedatastore.h"
#include "cmtpgetdeviceinfo.h"
#include "cmtptypedeviceinfo.h"
#include "rmtpframework.h"
#include "mtpdevdppanic.h"
#include "mtpdevicedpconst.h"
#include "rmtpdevicedpsingletons.h"
#include "cmtpdevicedpconfigmgr.h"
#include "cmtpservicemgr.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetDeviceInfo");)

/**
This identifies, in hundredths, the PTP version this device can support
*/
static const TUint16 KMTPStandardVersion = 100;

/**
This identifies the PTP vendor-extension version that are in use by this device
*/
static const TUint32 KMTPVendorExtensionId = 0x00000006;
static const TUint32 KMTPVendorExtensionId_Mac = 0xFFFFFFFF;

/**
This identifies, in hundredths, the version of the MTP standard that this device supports
*/
static const TUint16 KMTPVersion = 100;    // Currently 100

static const TInt KMTPArrayGranularity = 15;

/**
Two-phase construction method
@param aPlugin    The data provider plugin
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetDeviceInfo::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetDeviceInfo* self = new (ELeave) CMTPGetDeviceInfo(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPGetDeviceInfo::~CMTPGetDeviceInfo()
    {    
    __FLOG(_L8("~CMTPGetDeviceInfo - Entry"));
    delete iDeviceInfo;
    iDpSingletons.Close();
    iSingletons.Close();
    __FLOG(_L8("~CMTPGetDeviceInfo - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPGetDeviceInfo::CMTPGetDeviceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
    
/**
GetDeviceInfo request handler. Build and send device info data set.
*/    
void CMTPGetDeviceInfo::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
    
    if (!iDpSingletons.DeviceDataStore().Enumerated())
        {
        __FLOG(_L8("MTPExtensionReady not ready, reschedule request")); 
        iDpSingletons.DeviceDataStore().RegisterPendingRequest();
        RegisterPendingRequest();
        return;
        }
    
    BuildDeviceInfoL();
    SendDataL(*iDeviceInfo);
    __FLOG(_L8("ServiceL - Exit"));
    }

/**
Second-phase constructor.
*/        
void CMTPGetDeviceInfo::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    iSingletons.OpenL();
    iDpSingletons.OpenL(iFramework);
    iDeviceInfo = CMTPTypeDeviceInfo::NewL();
    __FLOG(_L8("ConstructL - Exit")); 
    }

/**
Populates device info data set
*/
void CMTPGetDeviceInfo::BuildDeviceInfoL()
    {
    __FLOG(_L8("BuildDeviceInfoL - Entry")); 
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());
    iDeviceInfo->SetUint16L(CMTPTypeDeviceInfo::EStandardVersion, KMTPStandardVersion);
    
    if(iDpSingletons.DeviceDataStore().IsConnectMac())
        {
        __FLOG(_L8("Connect Mac = ETrue"));         
        iDeviceInfo->SetUint32L(CMTPTypeDeviceInfo::EMTPVendorExtensionID, KMTPVendorExtensionId_Mac);
        iDeviceInfo->SetUint16L(CMTPTypeDeviceInfo::EMTPVersion, KMTPVersion);        
        RBuf  mtpExtensions;
        mtpExtensions.CleanupClosePushL();
        mtpExtensions.CreateL(0);
        iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::EMTPExtensions, mtpExtensions);
        CleanupStack::PopAndDestroy(&mtpExtensions);
        }
    else
        {
        __FLOG(_L8("Connect Mac = EFalse")); 
        iDeviceInfo->SetUint32L(CMTPTypeDeviceInfo::EMTPVendorExtensionID, KMTPVendorExtensionId);
        iDeviceInfo->SetUint16L(CMTPTypeDeviceInfo::EMTPVersion, KMTPVersion);        
        iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::EMTPExtensions, iDpSingletons.DeviceDataStore().MTPExtensions());  
        }
    
    iDeviceInfo->SetUint16L(CMTPTypeDeviceInfo::EFunctionalMode, EMTPFunctionalModeStandard);    
    iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::EManufacturer, device.Manufacturer());
    iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::EModel, device.Model());
    iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::EDeviceVersion, device.DeviceVersion());
    iDeviceInfo->SetStringL(CMTPTypeDeviceInfo::ESerialNumber, device.SerialNumber());
    
    CMTPDataProviderController& dps(iSingletons.DpController());
    SetSupportedOperationsL(dps);
    SetSupportedEventsL(dps);
    SetSupportedDevicePropertiesL(dps);
    SetSupportedCaptureFormatsL(dps);
    SetSupportedPlaybackFormatsL(dps);
    
    __FLOG(_L8("BuildDeviceInfoL - Exit")); 
    }

/**
Populates the supported operations field in the device info data set
It enumerates the installed data provider plugins and retrieves the supported operations
@param aDpController    A reference to the data provider controller
*/    
void CMTPGetDeviceInfo::SetSupportedOperationsL(CMTPDataProviderController& aDpController)
    {
    __FLOG(_L8("SetSupportedOperationsL - Entry")); 
    
    TInt count = aDpController.Count();    
    RArray<TUint> supportedOperations(KMTPArrayGranularity);
    CleanupClosePushL(supportedOperations);
    const TInt32 KMTPImplementationUidDeviceDp(0x102827AF);
    const TInt32 KMTPImplementationUidFileDp(0x102827B0);
    const TInt32 KMTPImplementationUidProxyDp(0x102827B1);
    const TInt32 KMTPFrameworkDpCount(3);
    TBool bOnlyInternalDpLoad = count > KMTPFrameworkDpCount ? EFalse : ETrue;
    while(count--)
        {
        TInt32 uid = aDpController.DataProviderByIndexL(count).ImplementationUid().iUid;
        // The filter is added for licencee's request which will filtrate the symbian's internal
        // dp's supported enhance mode operations to make licencee's dp work.
        // Every new internal dp need add it's implementation id here to filtrate and increase
        // the KMTPFrameworkDpCount number.
        if ((uid == KMTPImplementationUidDeviceDp ||
            uid == KMTPImplementationUidFileDp ||
            uid == KMTPImplementationUidProxyDp) && !bOnlyInternalDpLoad)
            {
            AddToArrayWithFilterL(supportedOperations, aDpController.DataProviderByIndexL(count).SupportedCodes(EOperations));
            }
        else
            {
            AddToArrayL(supportedOperations, aDpController.DataProviderByIndexL(count).SupportedCodes(EOperations));
            }
        }

    CMTPTypeArray* mtpOperationsArray = CMTPTypeArray::NewL(EMTPTypeAUINT16, supportedOperations);
    CleanupStack::PopAndDestroy(&supportedOperations);
    CleanupStack::PushL(mtpOperationsArray); //unnecessary if Set operation below does not leave,         
    iDeviceInfo->SetL(CMTPTypeDeviceInfo::EOperationsSupported, *mtpOperationsArray);
    CleanupStack::PopAndDestroy(mtpOperationsArray);  
    __FLOG(_L8("SetSupportedOperationsL - Exit"));      
    }

/**
Populates the supported events field in the device info data set
It enumerates the installed data provider plugins and retrieves the supported events
@param aDpController    A reference to the data provider controller
*/    
void CMTPGetDeviceInfo::SetSupportedEventsL(CMTPDataProviderController& aDpController)
    {
    __FLOG(_L8("SetSupportedEventsL - Entry"));
    TInt count = aDpController.Count();    
    RArray<TUint> supportedEvents(KMTPArrayGranularity);
    CleanupClosePushL(supportedEvents);
    while(count--)
        {
        AddToArrayL(supportedEvents, aDpController.DataProviderByIndexL(count).SupportedCodes(EEvents));
        }
    
    CMTPTypeArray* mtpEventArray = CMTPTypeArray::NewL(EMTPTypeAUINT16, supportedEvents);
    CleanupStack::PopAndDestroy(&supportedEvents);
    CleanupStack::PushL(mtpEventArray); //unnecessary if Set operation below does not leave,         
    iDeviceInfo->SetL(CMTPTypeDeviceInfo::EEventsSupported, *mtpEventArray);
    CleanupStack::PopAndDestroy(mtpEventArray);  
    __FLOG(_L8("SetSupportedEventsL - Exit"));  
    }

/**
Populates the supported device properties field in the device info data set
*/       
void CMTPGetDeviceInfo::SetSupportedDevicePropertiesL(CMTPDataProviderController& aDpController)
	{ 
	__FLOG(_L8("SetSupportedDevicePropertiesL - Entry"));	
    TInt count = aDpController.Count();    
    RArray<TUint> supportedOperations(KMTPArrayGranularity);
    CleanupClosePushL(supportedOperations);
    const TInt32 KMTPImplementationUidDeviceDp(0x102827AF);
    const TInt32 KMTPImplementationUidFileDp(0x102827B0);
    const TInt32 KMTPImplementationUidProxyDp(0x102827B1);
    const TInt32 KMTPFrameworkDpCount(3);
    TBool bOnlyInternalDpLoad = count > KMTPFrameworkDpCount ? EFalse : ETrue;
    while(count--)
        {
        TInt32 uid = aDpController.DataProviderByIndexL(count).ImplementationUid().iUid;
        // The filter is added for licencee's request which will filtrate the symbian's internal
        // dp's supported enhance mode operations to make licencee's dp work.
        // Every new internal dp need add it's implementation id here to filtrate and increase
        // the KMTPFrameworkDpCount number.
        if ((uid == KMTPImplementationUidDeviceDp ||
            uid == KMTPImplementationUidFileDp ||
            uid == KMTPImplementationUidProxyDp) && !bOnlyInternalDpLoad)
            {
            AddToArrayWithFilterL(supportedOperations, aDpController.DataProviderByIndexL(count).SupportedCodes(EDeviceProperties));
            }
        else
            {
            AddToArrayL(supportedOperations, aDpController.DataProviderByIndexL(count).SupportedCodes(EDeviceProperties));
            }
        }

    CMTPTypeArray* mtpOperationsArray = CMTPTypeArray::NewL(EMTPTypeAUINT16, supportedOperations);
    CleanupStack::PopAndDestroy(&supportedOperations);
    CleanupStack::PushL(mtpOperationsArray); //unnecessary if Set operation below does not leave,         
    iDeviceInfo->SetL(CMTPTypeDeviceInfo::EDevicePropertiesSupported, *mtpOperationsArray);
    CleanupStack::PopAndDestroy(mtpOperationsArray);  
	
	__FLOG(_L8("SetSupportedDevicePropertiesL - Exit"));  
	}

/**
Populates the supported capture formats field in the device info data set
It enumerates the installed data provider plugins and retrieves the supported capture formats
@param aDpController    A reference to the data provider controller
*/    
void CMTPGetDeviceInfo::SetSupportedCaptureFormatsL(CMTPDataProviderController& aDpController)
    {
    __FLOG(_L8("SetSupportedCaptureFormatsL - Entry"));
    TInt count = aDpController.Count();    
    RArray<TUint> supportedCaptureFormats(KMTPArrayGranularity);
    CleanupClosePushL(supportedCaptureFormats);
//get the formt supported information from the devdp rss file
  //and store it into the CArrayFix by passing the reference of the array.
  RArray<TUint> orderedFormats(KMTPArrayGranularity);
  CleanupClosePushL(orderedFormats);
  //get the count form the formt supported struct.
  TRAPD(errorCode,iDpSingletons.ConfigMgr().GetRssConfigInfoArrayL(orderedFormats, EDevDpFormats));
  if(KErrNone != errorCode)
	  {
	  __FLOG(_L8("There is an issue in reading format info from rss file "));
	  }
	
    while(count--)
        {
        AddToArrayL(supportedCaptureFormats, aDpController.DataProviderByIndexL(count).SupportedCodes(EObjectCaptureFormats));
        }
	TInt insertIndex = 0;
	//Remove the existing formats from the supportedPlaybackFormats and insert the 
	//elements from orderedFormats at the begining.	
	for(TInt orderedElements = 0; orderedElements <orderedFormats.Count(); orderedElements++ )
		{
	TInt index = supportedCaptureFormats.Find(orderedFormats[orderedElements]);	  
	if(KErrNotFound != index)
		{
		supportedCaptureFormats.Remove(index);
		//insert at insertindex position to make sure that value inserted at begening.
		supportedCaptureFormats.InsertL(orderedFormats[orderedElements], insertIndex);
		insertIndex++;
		}//else nothing 	  	
		}
	
	//The formats which are defined in Device Service Extension should not be reported by GetDeviceInfo
	RemoveServiceFormat(supportedCaptureFormats);
	
#ifdef _DEBUG
	for(TInt i =0 ; i < supportedCaptureFormats.Count(); i++)
	{
	__FLOG_VA((_L8("Playback formats = %d"), supportedCaptureFormats[i]));
	}
#endif 
	//before deleting make sure all the elements are added to supportedPlaybackFormats
	CleanupStack::PopAndDestroy(&orderedFormats);

    CMTPTypeArray* mtpCaptureFormatArray = CMTPTypeArray::NewL(EMTPTypeAUINT16, supportedCaptureFormats);
    CleanupStack::PopAndDestroy(&supportedCaptureFormats);
    CleanupStack::PushL(mtpCaptureFormatArray); //unnecessary if Set operation below does not leave,         
    iDeviceInfo->SetL(CMTPTypeDeviceInfo::ECaptureFormats, *mtpCaptureFormatArray);
    CleanupStack::PopAndDestroy(mtpCaptureFormatArray); 
    __FLOG(_L8("SetSupportedCaptureFormatsL - Exit"));   
    }

/**
Populates the supported playback formats field in the device info data set
It enumerates the installed data provider plugins and retrieves the supported playback formats
@param aDpController    A reference to the data provider controller
*/    
void CMTPGetDeviceInfo::SetSupportedPlaybackFormatsL(CMTPDataProviderController& aDpController)
    {
    __FLOG(_L8("SetSupportedPlaybackFormatsL - Entry"));
    TInt count = aDpController.Count();    
    RArray<TUint> supportedPlaybackFormats(KMTPArrayGranularity);	
    CleanupClosePushL(supportedPlaybackFormats);
	
    //get the formt supported information from the devdp rss file
    //and store it into the CArrayFix by passing the reference of the array.
    RArray<TUint> orderedFormats(KMTPArrayGranularity);
    CleanupClosePushL(orderedFormats);
    //get the count form the formt supported struct.
    TRAPD(errorCode,iDpSingletons.ConfigMgr().GetRssConfigInfoArrayL(orderedFormats, EDevDpFormats));
	if(KErrNone != errorCode)
		{
		__FLOG(_L8("There is an issue in reading format info from rss file "));
		}

    while(count--)
        {
        AddToArrayL(supportedPlaybackFormats, aDpController.DataProviderByIndexL(count).SupportedCodes(EObjectPlaybackFormats));       
        }

	TInt insertIndex = 0;
	//Remove the existing formats from the supportedPlaybackFormats and insert the 
	//elements from orderedFormats at the begining.	
 	for(TInt orderedElements = 0; orderedElements <orderedFormats.Count(); orderedElements++ )
 		{
		TInt index = supportedPlaybackFormats.Find(orderedFormats[orderedElements]);	  
		if(KErrNotFound != index)
			{
			supportedPlaybackFormats.Remove(index);
			//insert at insertindex position to make sure that value inserted at begening.
			supportedPlaybackFormats.InsertL(orderedFormats[orderedElements], insertIndex);
			insertIndex++;
			}//else nothing 	  	
 		}
 	
 	//The formats which are defined in Device Service Extension should not be reported by GetDeviceInfo
 	RemoveServiceFormat(supportedPlaybackFormats);
 	
#ifdef _DEBUG
	for(TInt i =0 ; i < supportedPlaybackFormats.Count(); i++)
	{
	__FLOG_VA((_L8("Playback formats = %d"), supportedPlaybackFormats[i]));
	}
#endif 
    //before deleting make sure all the elements are added to supportedPlaybackFormats
    CleanupStack::PopAndDestroy(&orderedFormats);
	
    CMTPTypeArray* mtpPlaybackFormatArray = CMTPTypeArray::NewL(EMTPTypeAUINT16, supportedPlaybackFormats);    
    CleanupStack::PopAndDestroy(&supportedPlaybackFormats);
    CleanupStack::PushL(mtpPlaybackFormatArray); //unnecessary if Set operation below does not leave,         
    iDeviceInfo->SetL(CMTPTypeDeviceInfo::EPlaybackFormats, *mtpPlaybackFormatArray);
    CleanupStack::PopAndDestroy(mtpPlaybackFormatArray);  
    __FLOG(_L8("SetSupportedPlaybackFormatsL - Exit"));  
    }

/**
Add elements from source array to the destination array without duplication
@param aDestArray    The destination array
@param aSrcArray    The source array
*/
void CMTPGetDeviceInfo::AddToArrayL(RArray<TUint>& aDestArray, const RArray<TUint>& aSrcArray) const
    {
    TInt count(aSrcArray.Count());
    for (TInt i(0); (i < count); i++)
        {
        // Apply filter
        if(aSrcArray[i] == EMTPOpCodeResetDevicePropValue)
            {
            __FLOG_VA((_L8("Filter ignored operation: %d"), aSrcArray[i]));
            continue;
            }
        
        TInt err(aDestArray.InsertInOrder(aSrcArray[i]));
        if ((err != KErrNone) && (err != KErrAlreadyExists))
            {
            User::Leave(err);
            }
        }
    }

/**
Add elements from source array to the destination array without duplication
User filter to help licensee's data provider's supported operation.
@param aDestArray    The destination array
@param aSrcArray    The source array
*/
void CMTPGetDeviceInfo::AddToArrayWithFilterL(RArray<TUint>& aDestArray, const RArray<TUint>& aSrcArray) const
    {
    TInt count(aSrcArray.Count());
    for (TInt i(0); (i < count); i++)
        {
        // Apply filter
        if(aSrcArray[i] == EMTPOpCodeGetObjectPropList ||
           aSrcArray[i] == EMTPOpCodeSetObjectPropList||
           aSrcArray[i] == EMTPOpCodeSendObjectPropList ||
			aSrcArray[i] == EMTPOpCodeGetFormatCapabilities )
            {
            __FLOG_VA((_L8("Filter ignored operation: %d"), aSrcArray[i]));
            continue;
            }

        TInt err(aDestArray.InsertInOrder(aSrcArray[i]));
        if ((err != KErrNone) && (err != KErrAlreadyExists))
            {
            User::Leave(err);
            }
        }
    }


void CMTPGetDeviceInfo::RemoveServiceFormat(RArray<TUint>& aSupportedCaptureFormats)
	{
	__FLOG(_L8("RemovetServiceFormat - Entry"));
	
	TInt count = aSupportedCaptureFormats.Count();
	count--;
	while(count>=0)
		{
        if(iSingletons.ServiceMgr().IsServiceFormatCode( aSupportedCaptureFormats[count] ))
            aSupportedCaptureFormats.Remove(count);
        count--;
		}
	__FLOG(_L8("RemovetServiceFormat - Exit")); 
	}
