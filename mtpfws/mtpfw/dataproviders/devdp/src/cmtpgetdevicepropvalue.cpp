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

#include <mtp/cmtptypestring.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeuint8.h>
#include <mtp/mmtpframeworkconfig.h>
#include "cmtpdevicedatastore.h"
#include "cmtpgetdevicepropvalue.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetDevicePropValue");)

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPGetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetDevicePropValue* self = new (ELeave) CMTPGetDevicePropValue(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
should not delete iMtpArray(ownership is belong to devicedatastore)
*/    
CMTPGetDevicePropValue::~CMTPGetDevicePropValue()
    {
    __FLOG(_L8("~CMTPGetDevicePropValue - Entry"));
    delete iString;    
    delete iData;
    //ownership of the iMtpArray pointer is belongs to devicedatastore so it should not 
    //deleted.    
    __FLOG(_L8("~CMTPGetDevicePropValue - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPGetDevicePropValue::CMTPGetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPGetDevicePropDesc(aFramework, aConnection)
    {
    
    }

/**
Second-phase constructor.
*/
void CMTPGetDevicePropValue::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    CMTPGetDevicePropDesc::ConstructL();
    iString = CMTPTypeString::NewL();
    __FLOG(_L8("ConstructL - Exit")); 
    }

/**
Services the battery level property.
*/    
void CMTPGetDevicePropValue::ServiceBatteryLevelL()
    {
    __FLOG(_L8("ServiceBatteryLevelL - Entry"));
    iBatteryLevel.Set(iBatteryLevelValue);
    SendDataL(iBatteryLevel);
    __FLOG(_L8("ServiceBatteryLevelL - Exit"));
    }

/**
Services the device friendly name property.
*/   
void CMTPGetDevicePropValue::ServiceDeviceFriendlyNameL()
    {
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Entry")); 
    iString->SetL(iDpSingletons.DeviceDataStore().DeviceFriendlyName());
    SendDataL(*iString);  
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Exit"));   
    }
        
/**
Services the synchronisation partner property.
*/ 
void CMTPGetDevicePropValue::ServiceSynchronisationPartnerL()
    {
    __FLOG(_L8("ServiceSynchronisationPartnerL - Entry")); 
    iString->SetL(iDpSingletons.DeviceDataStore().SynchronisationPartner());
    SendDataL(*iString);
    __FLOG(_L8("ServiceSynchronisationPartnerL - Exit"));
    }

/**
Services the ServiceSessionInitiatorVersionInfo property. 
*/ 
void CMTPGetDevicePropValue::ServiceSessionInitiatorVersionInfoL()
   {
   __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Entry")); 
   iString->SetL(iDpSingletons.DeviceDataStore().SessionInitiatorVersionInfo());
   SendDataL(*iString);
   __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Exit"));
   }

/**
Services the ServicePerceivedDeviceType property. 
*/ 
void CMTPGetDevicePropValue::ServicePerceivedDeviceTypeL()
   {
   __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Entry")); 
   iUint32.Set(iDpSingletons.DeviceDataStore().PerceivedDeviceType());
   SendDataL(iUint32);
   __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Exit"));
   }

/**
Services the Date time device property. 
*/ 
void CMTPGetDevicePropValue::ServiceDateTimeL()
  {
  __FLOG(_L8("ServiceDateTime - Entry")); 
  iString->SetL(iDpSingletons.DeviceDataStore().DateTimeL());
  SendDataL(*iString);
  __FLOG(_L8("ServiceDateTime - Exit"));
  }

/**
Services the DiviceIcon property. 
*/ 
void CMTPGetDevicePropValue::ServiceDeviceIconL()
    {
    __FLOG(_L8("DeviceIcon - Entry")); 
    //iMtpArray is not owned by this class DO NOT DELET IT.
    iMtpArray = &(iDpSingletons.DeviceDataStore().DeviceIcon());
    SendDataL(*iMtpArray);
    __FLOG(_L8("DeviceIcon - Exit"));
    }

/**
Services the ServicePerceivedDeviceType property. 
*/ 
void CMTPGetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {
    __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Entry"));    
    iUint8.Set(GetFormatOrdered());
    SendDataL(iUint8);
    __FLOG(_L8("ServiceSessionInitiatorVersionInfo - Exit"));
    }

 void CMTPGetDevicePropValue::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin)
    {
    MMTPType* mtptype = NULL;
    aExtnDevplugin->GetDevPropertyL((TMTPDevicePropertyCode)aPropCode, &mtptype);

    if(NULL != mtptype)
        {
        SendDataL(*mtptype);	
        }
    else
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported); 	
        }
    }
 
 /*
 *Service Supported FuntionalID.
 */
 void CMTPGetDevicePropValue::ServiceFunctionalIDL()
     {
     __FLOG(_L8("ServiceFuntionalIDL - Entry")); 

    delete iData;
    iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentFuncationalID); 
    
     SendDataL(*iData); 
     __FLOG(_L8("ServiceFuntionalIDL - Exit")); 
     }

 /*
 *Service Supported ModelID.
 */
 void CMTPGetDevicePropValue::ServiceModelIDL()
     {
     __FLOG(_L8("ServiceModelIDL - Entry")); 
     
     delete iData;
     iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentModelID); 
 	
     SendDataL(*iData); 
     __FLOG(_L8("ServiceModelIDL - Exit")); 
     }

 /*
 *Service Supported UseDeviceStage.
 */
 void CMTPGetDevicePropValue::ServiceUseDeviceStageL()
     {
     __FLOG(_L8("ServiceUseDeviceStageL - Entry")); 
 	iUint8.Set(1);
 	SendDataL(iUint8); 
     __FLOG(_L8("ServiceUseDeviceStageL - Exit")); 
     }

  














    

    


       

    






