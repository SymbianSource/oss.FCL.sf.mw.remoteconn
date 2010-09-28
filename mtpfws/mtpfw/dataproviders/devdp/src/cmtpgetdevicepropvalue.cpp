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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetdevicepropvalueTraces.h"
#endif


// Class constants.

_LIT(KSpace, " ");

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
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_CMTPGETDEVICEPROPVALUE_DES_ENTRY );
    delete iString;    
    delete iData;
    //ownership of the iMtpArray pointer is belongs to devicedatastore so it should not 
    //deleted.    
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_CMTPGETDEVICEPROPVALUE_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_CONSTRUCTL_ENTRY );
    CMTPGetDevicePropDesc::ConstructL();
    iString = CMTPTypeString::NewL();
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_CONSTRUCTL_EXIT );
    }

/**
Services the battery level property.
*/    
void CMTPGetDevicePropValue::ServiceBatteryLevelL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEBATTERYLEVELL_ENTRY );
    iBatteryLevel.Set(iBatteryLevelValue);
    SendDataL(iBatteryLevel);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEBATTERYLEVELL_EXIT );
    }

/**
Services the device friendly name property.
*/   
void CMTPGetDevicePropValue::ServiceDeviceFriendlyNameL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEDEVICEFRIENDLYNAMEL_ENTRY );
    //if device friendly name is blank, which means it is the first time the device get connected,
    //, so will use "manufacture + model id" firstly; if neither manufacture nor model
    //id not able to be fetched by API, then use the default device friendly name     if ( iDpSingletons.DeviceDataStore().DeviceFriendlyName().Length()<=0 )
    if ( iDpSingletons.DeviceDataStore().DeviceFriendlyName().Length()<=0 ) 
        {
        if ( iDpSingletons.DeviceDataStore().Manufacturer().Compare(KMTPDefaultManufacturer) && 
                iDpSingletons.DeviceDataStore().Model().Compare(KMTPDefaultModel) )
            {
            HBufC* friendlyName = HBufC::NewLC( iDpSingletons.DeviceDataStore().Manufacturer().Length()+1+  iDpSingletons.DeviceDataStore().Model().Length());
            TPtr ptrName = friendlyName->Des();
            ptrName.Copy( iDpSingletons.DeviceDataStore().Manufacturer());
            ptrName.Append(KSpace);
            ptrName.Append( iDpSingletons.DeviceDataStore().Model());
            iDpSingletons.DeviceDataStore().SetDeviceFriendlyNameL(ptrName);
            iString->SetL(ptrName);
            CleanupStack::PopAndDestroy(friendlyName);
            }
        else
            {
            iString->SetL(iDpSingletons.DeviceDataStore().DeviceFriendlyNameDefault());
            }
        }
    else
        {
        iString->SetL(iDpSingletons.DeviceDataStore().DeviceFriendlyName());
        }
      
    SendDataL(*iString);   
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEDEVICEFRIENDLYNAMEL_EXIT );
    }
        
/**
Services the synchronisation partner property.
*/ 
void CMTPGetDevicePropValue::ServiceSynchronisationPartnerL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICESYNCHRONISATIONPARTNERL_ENTRY );
    iString->SetL(iDpSingletons.DeviceDataStore().SynchronisationPartner());
    SendDataL(*iString);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICESYNCHRONISATIONPARTNERL_EXIT );
    }

/**
Services the ServiceSessionInitiatorVersionInfo property. 
*/ 
void CMTPGetDevicePropValue::ServiceSessionInitiatorVersionInfoL()
   {
   OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_ENTRY );
   iString->SetL(iDpSingletons.DeviceDataStore().SessionInitiatorVersionInfo());
   SendDataL(*iString);
   OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_EXIT );
   }

/**
Services the ServicePerceivedDeviceType property. 
*/ 
void CMTPGetDevicePropValue::ServicePerceivedDeviceTypeL()
   {
   OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_ENTRY );
   iUint32.Set(iDpSingletons.DeviceDataStore().PerceivedDeviceType());
   SendDataL(iUint32);
   OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_EXIT );
   }

/**
Services the Date time device property. 
*/ 
void CMTPGetDevicePropValue::ServiceDateTimeL()
  {
  OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEDATETIMEL_ENTRY );
  iString->SetL(iDpSingletons.DeviceDataStore().DateTimeL());
  SendDataL(*iString);
  OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEDATETIMEL_EXIT );
  }

/**
Services the DiviceIcon property. 
*/ 
void CMTPGetDevicePropValue::ServiceDeviceIconL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEDEVICEICONL_ENTRY );
    //iMtpArray is not owned by this class DO NOT DELET IT.
    iMtpArray = &(iDpSingletons.DeviceDataStore().DeviceIcon());
    SendDataL(*iMtpArray);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEDEVICEICONL_EXIT );
    }

/**
Services the ServicePerceivedDeviceType property. 
*/ 
void CMTPGetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_ENTRY );  
    iUint8.Set(GetFormatOrdered());
    SendDataL(iUint8);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_EXIT );
    }

 void CMTPGetDevicePropValue::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin)
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_HANDLEEXTNSERVICEL_ENTRY );
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
    OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_HANDLEEXTNSERVICEL_EXIT );
    }
 
 /*
 *Service Supported FuntionalID.
 */
 void CMTPGetDevicePropValue::ServiceFunctionalIDL()
     {
     OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_ENTRY );

    delete iData;
    iData = NULL;
    iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentFuncationalID); 
    
     SendDataL(*iData); 
     OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_EXIT );
     }

 /*
 *Service Supported ModelID.
 */
 void CMTPGetDevicePropValue::ServiceModelIDL()
     {
     OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEMODELIDL_ENTRY );
     
     delete iData;
     iData = NULL;
     iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentModelID); 
 	
     SendDataL(*iData); 
     OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEMODELIDL_EXIT );
     }

 /*
 *Service Supported UseDeviceStage.
 */
 void CMTPGetDevicePropValue::ServiceUseDeviceStageL()
     {
     OstTraceFunctionEntry0( CMTPGETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_ENTRY );
 	iUint8.Set(1);
 	SendDataL(iUint8); 
     OstTraceFunctionExit0( CMTPGETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_EXIT );
     }

  














    

    


       

    






