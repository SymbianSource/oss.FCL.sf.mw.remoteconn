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
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mmtpframeworkconfig.h>
#include <centralrepository.h>

#include "cmtpdevicedatastore.h"
#include "cmtpresetdevicepropvalue.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpresetdevicepropvalueTraces.h"
#endif


// Class constants.

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPResetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPResetDevicePropValue* self = new (ELeave) CMTPResetDevicePropValue(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPResetDevicePropValue::~CMTPResetDevicePropValue()
    {    
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_CMTPRESETDEVICEPROPVALUE_DES_ENTRY );
    iDpSingletons.Close();
    delete iData;
    delete iRepository;
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_CMTPRESETDEVICEPROPVALUE_DES_EXIT );
    }

/**
Standard c++ constructor
*/    
CMTPResetDevicePropValue::CMTPResetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
 CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {

    }
    
/**
Second-phase construction
*/    
void CMTPResetDevicePropValue::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_CONSTRUCTL_ENTRY );
    iDpSingletons.OpenL(iFramework);
	const TUint32 KUidMTPRepositoryValue(0x10282FCC);
    const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPRepository);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_CONSTRUCTL_EXIT );
    }

/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPResetDevicePropValue::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_CHECKREQUESTL_ENTRY );
    TMTPResponseCode respCode(EMTPRespCodeDevicePropNotSupported);
    iPropCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    const TInt count = sizeof(KMTPDeviceDpSupportedProperties) / sizeof(KMTPDeviceDpSupportedProperties[0]);
    for (TUint i(0); ((respCode != EMTPRespCodeOK) && (i < count)); i++)
        {
        if (iPropCode == KMTPDeviceDpSupportedProperties[i])
            {
            respCode = EMTPRespCodeOK;
            }
        }

    if(iDpSingletons.DeviceDataStore().ExtnDevicePropDp())
        {
        respCode = EMTPRespCodeOK;
        }
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_CHECKREQUESTL_EXIT );
    return respCode;
    }
/**
ResetDevicePropValue request handler.
*/ 	
void CMTPResetDevicePropValue::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEL_ENTRY );
    iPropCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    MExtnDevicePropDp* extnDevplugin = iDpSingletons.DeviceDataStore().ExtnDevicePropDp();
    switch (iPropCode)
        {
        //Added all new prpoerties here we have to add other properties that not present here*/
        case EMTPDevicePropCodeSessionInitiatorVersionInfo:
            ServiceSessionInitiatorVersionInfoL();
        break;
        case EMTPDevicePropCodePerceivedDeviceType:
            ServicePerceivedDeviceTypeL();
        break;

        case EMTPDevicePropCodeDeviceIcon:
            ServiceDeviceIconL();
        break;

        case EMTPDevicePropCodeSupportedFormatsOrdered:
            ServiceSupportedFormatsOrderedL();
        break;

        case EMTPDevicePropCodeDateTime:
            ServiceDateTimeL();
        break;
        case EMTPDevicePropCodeFunctionalID:
        	ServiceFunctionalIDL();
        break;
        case EMTPDevicePropCodeModelID:
        	ServiceModelIDL();
        break;
        case EMTPDevicePropCodeUseDeviceStage:
        	ServiceUseDeviceStageL();
        break;
        default:         
            if(extnDevplugin)
                { 
                HandleExtnServiceL(iPropCode, extnDevplugin);
                }
            else 
                { 
                SendResponseL(EMTPRespCodeDevicePropNotSupported);
                }
        break;   
        }
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEL_EXIT );
    } 

 void CMTPResetDevicePropValue::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin)
    {
	  if(aExtnDevplugin->ResetDevPropertyL((TMTPDevicePropertyCode)aPropCode) == KErrNone)
	  {
	  SendResponseL(EMTPRespCodeOK);
	  }
	  else
	  {
	  SendResponseL(EMTPRespCodeDevicePropNotSupported);
	  }

    }

/**
Service session initiator property.
*/ 
void CMTPResetDevicePropValue::ServiceSessionInitiatorVersionInfoL()
    {  
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_ENTRY );
    iDpSingletons.DeviceDataStore().SetSessionInitiatorVersionInfoL( iDpSingletons.DeviceDataStore().SessionInitiatorVersionInfoDefault());
    SendResponseL(EMTPRespCodeOK);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_EXIT );
    }

/**
*Service the PerceivedDeviceType property.
*it is not set type it should be removed 
**/
void CMTPResetDevicePropValue::ServicePerceivedDeviceTypeL()
    {  
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_ENTRY );
    //PerceivedDeviceType is of type get only .
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_EXIT );
    }

/**
Service the Date Time property.
*/ 
void CMTPResetDevicePropValue::ServiceDateTimeL()
    {  
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEDATETIMEL_ENTRY );
    SendResponseL(EMTPRespCodeOperationNotSupported);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEDATETIMEL_EXIT );
    }


/*
*Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/   
void CMTPResetDevicePropValue::ServiceDeviceIconL()
    {  
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEDEVICEICONL_ENTRY );
    //DeviceIcon property is implemented as get only .
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEDEVICEICONL_EXIT );
    }

/*
*ServiceSupportedFormatsOrdered property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {  
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_ENTRY );
    //no need to recive this data beacuse it is Get property
    //iDpSingletons.DeviceDataStore().SetFormatOrdered( iDpSingletons.DeviceDataStore().FormatOrderedDefault());
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_EXIT );
    }

/*
*FunctionalID property. 
*/
void CMTPResetDevicePropValue::ServiceFunctionalIDL()
    {	 
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_ENTRY );
    delete iData;
    iData = NULL;
    iData = GetGUIDL( MMTPFrameworkConfig::EDeviceDefaultFuncationalID ); 
    SaveGUID(MMTPFrameworkConfig::EDeviceCurrentFuncationalID, *iData);
    SendResponseL(EMTPRespCodeOK);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_EXIT );
    }

/*
*ModelID property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceModelIDL()
    {	 
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEMODELIDL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEMODELIDL_EXIT );
    }

/*
*UseDeviceStage property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceUseDeviceStageL()
    {	 
    OstTraceFunctionEntry0( CMTPRESETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPRESETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_EXIT );
    }

TMTPTypeGuid* CMTPResetDevicePropValue::GetGUIDL(const TUint aKey)
    {
    TBuf<KGUIDFormatStringLength> buf;
    
    LEAVEIFERROR(iRepository->Get(aKey,buf),
            OstTrace1( TRACE_ERROR, CMTPRESETDEVICEPROPVALUE_GETGUIDL, "get from iRepository with key %d", aKey ));

    TMTPTypeGuid* ret = new (ELeave) TMTPTypeGuid( buf );
    
    return ret;
    }

void CMTPResetDevicePropValue::SaveGUID( const TUint aKey,  TMTPTypeGuid& aValue )
    {
	TBuf<KGUIDFormatStringLength> buf;
	if(aValue.ToString(buf) == KErrNone)
		{
		iRepository->Set(aKey,buf);
		}
    }
