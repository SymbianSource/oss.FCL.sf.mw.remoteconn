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
#include <mtp/rmtpclient.h>
#include <e32property.h>
#include <mtp/mmtpframeworkconfig.h>
#include "cmtpdevicedatastore.h"
#include "cmtpsetdevicepropvalue.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsetdevicepropvalueTraces.h"
#endif


// Class constants.

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPSetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPSetDevicePropValue* self = new (ELeave) CMTPSetDevicePropValue(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPSetDevicePropValue::~CMTPSetDevicePropValue()
    {    
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_CMTPSETDEVICEPROPVALUE_DES_ENTRY );
    delete iString;
    delete iMtparray;
    delete iData;
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_CMTPSETDEVICEPROPVALUE_DES_EXIT );
    }

/**
Standard c++ constructor
*/    
CMTPSetDevicePropValue::CMTPSetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPGetDevicePropDesc(aFramework, aConnection)
    {
    
    }
    
/**
Second-phase construction
*/    
void CMTPSetDevicePropValue::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_CONSTRUCTL_ENTRY );
    CMTPGetDevicePropDesc::ConstructL();
    iString = CMTPTypeString::NewL();
    iMtparray = CMTPTypeArray::NewL(EMTPTypeAUINT8);
    iData = new(ELeave) TMTPTypeGuid();
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_CONSTRUCTL_EXIT );
    }

/**
Service Battery level property
*/    
void CMTPSetDevicePropValue::ServiceBatteryLevelL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEBATTERYLEVELL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied); 
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEBATTERYLEVELL_EXIT );
    }

/**
Service the device friendly name property.
*/   
void CMTPSetDevicePropValue::ServiceDeviceFriendlyNameL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEDEVICEFRIENDLYNAMEL_ENTRY );
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString); 
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEDEVICEFRIENDLYNAMEL_EXIT );
    }
        
/**
Service the synchronisation partner property.
*/ 
void CMTPSetDevicePropValue::ServiceSynchronisationPartnerL()
    {    
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICESYNCHRONISATIONPARTNERL_ENTRY );
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICESYNCHRONISATIONPARTNERL_EXIT );
    }
    
/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPSetDevicePropValue::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPGetDevicePropDesc::CheckRequestL();
	
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	if( propCode == EMTPDevicePropCodeBatteryLevel)
		{
		responseCode = EMTPRespCodeAccessDenied;
		}
	OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_CHECKREQUESTL_EXIT );
	return responseCode;
	}

/**
Process the transaction response phase.
*/    
TBool CMTPSetDevicePropValue::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_DOHANDLERESPONSEPHASEL_ENTRY );
    MExtnDevicePropDp* extnDevplugin = iDpSingletons.DeviceDataStore().ExtnDevicePropDp();
    TUint32 propCode(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    switch(propCode)
        {
    case EMTPDevicePropCodeSynchronizationPartner:
        CompleteServiceSynchronisationPartnerL();
        break;
        
    case EMTPDevicePropCodeDeviceFriendlyName:
        CompleteServiceDeviceFriendlyNameL();
        break;

	case EMTPDevicePropCodeSessionInitiatorVersionInfo:
		CompleteServiceSessionInitiatorVersionInfoL();
		break;
	case EMTPDevicePropCodeDateTime:	
		CompleteServiceDateTimeL();
		break;
	
	case EMTPDevicePropCodeSupportedFormatsOrdered:		
		CompleteServiceSupportedFormatsOrderedL();
		break;
	
	case EMTPDevicePropCodeDeviceIcon:		
		CompleteDeviceIconL();
		break;
	case EMTPDevicePropCodePerceivedDeviceType:		
		CompletePerceivedDeviceTypeL();
		break;
	case EMTPDevicePropCodeFunctionalID:		
		CompleteServiceFunctionalIDL();
		break;
	case EMTPDevicePropCodeModelID:		
		CompleteServiceModelIDL();
		break;
	case EMTPDevicePropCodeUseDeviceStage:		
		CompleteServiceUseDeviceStageL();
		break;
    default:
 		if(extnDevplugin)
		{
		SendResponseL(extnDevplugin->SetDevicePropertyL());
		}
		else 
		{
		SendResponseL(EMTPRespCodeDevicePropNotSupported); 
		}
        break;             
        }
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;    
    }
    
TBool CMTPSetDevicePropValue::HasDataphase() const
	{
	return ETrue;
	}
    
/**
Processes the device friendly name property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceDeviceFriendlyNameL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEDEVICEFRIENDLYNAMEL_ENTRY );
    iDpSingletons.DeviceDataStore().SetDeviceFriendlyNameL(iString->StringChars());
    SendResponseL(EMTPRespCodeOK);  
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEDEVICEFRIENDLYNAMEL_EXIT );
    }

/**
Processes the synchronisation partner property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceSynchronisationPartnerL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESYNCHRONISATIONPARTNERL_ENTRY );
    iDpSingletons.DeviceDataStore().SetSynchronisationPartnerL(iString->StringChars());
    SendResponseL(EMTPRespCodeOK);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESYNCHRONISATIONPARTNERL_EXIT );
    }

void CMTPSetDevicePropValue::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin)
	{
	OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_HANDLEEXTNSERVICEL_ENTRY );
	MMTPType* ammtptype = NULL;
	aExtnDevplugin->GetDevicePropertyContainerL((TMTPDevicePropertyCode)aPropCode, &ammtptype);	
	if(ammtptype != NULL)
	{
	ReceiveDataL(*ammtptype);
	}
	else
	{
	SendResponseL(EMTPRespCodeDevicePropNotSupported);	
	}
	OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_HANDLEEXTNSERVICEL_EXIT );
	}
 
/**
Processes the session initiator version info and set the same to session device data store.
*/
void CMTPSetDevicePropValue::CompleteServiceSessionInitiatorVersionInfoL()
	{
	OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESESSIONINITIATORVERSIONINFOL_ENTRY );
	RProcess process;
	RProperty::Set(process.SecureId(), EMTPConnStateKey, iString->StringChars());
	iDpSingletons.DeviceDataStore().SetSessionInitiatorVersionInfoL(iString->StringChars());
	SendResponseL(EMTPRespCodeOK);
	OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESESSIONINITIATORVERSIONINFOL_EXIT );
	}

/**
Service session initiator property.
*/ 
void CMTPSetDevicePropValue::ServiceSessionInitiatorVersionInfoL()
	{	 
	OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_ENTRY );
	iString->SetL(KNullDesC);
	ReceiveDataL(*iString);
	OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICESESSIONINITIATORVERSIONINFOL_EXIT );
	}


/**
This should be removed no set for percived device type.
*/
void CMTPSetDevicePropValue::CompletePerceivedDeviceTypeL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETEPERCEIVEDDEVICETYPEL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETEPERCEIVEDDEVICETYPEL_EXIT );
    }

/**
*Service the PerceivedDeviceType property.
*it is not set type it should be removed 
**/
void CMTPSetDevicePropValue::ServicePerceivedDeviceTypeL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_ENTRY );
    ReceiveDataL(iUint32);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEPERCEIVEDDEVICETYPEL_EXIT );
    }

/**
Processes the Date Time property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceDateTimeL()
	{
	OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEDATETIMEL_ENTRY );
												
	//validate the incoming date time string first and then set it.
	if(KErrNone == iDpSingletons.DeviceDataStore().SetDateTimeL(iString->StringChars()) )
		{
		SendResponseL(EMTPRespCodeOK);
		}
	else
		{
		SendResponseL(EMTPRespCodeInvalidDataset);
		}
	
	OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEDATETIMEL_EXIT );
	}

/**
Service the Date Time property. 
*/ 
void CMTPSetDevicePropValue::ServiceDateTimeL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEDATETIMEL_ENTRY );
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEDATETIMEL_EXIT );
    }

/*
*Complete Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/
void CMTPSetDevicePropValue::CompleteDeviceIconL()
    {   	
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETEDEVICEICONL_ENTRY );
    //it is Get only device property
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETEDEVICEICONL_EXIT );
    }

/*
*Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/   
void CMTPSetDevicePropValue::ServiceDeviceIconL()
    {    
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEDEVICEICONL_ENTRY );
    //no need to recive this data beacuse it is Get property
    ReceiveDataL(*iMtparray);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEDEVICEICONL_EXIT );
    }
       

/*
*Processes the PerceivedDeviceType property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceSupportedFormatsOrderedL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESUPPORTEDFORMATSORDEREDL_ENTRY );
    //it is Get only device property
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICESUPPORTEDFORMATSORDEREDL_EXIT );
    }

/*
*ServiceSupportedFormatsOrdered property. it is get only type.
*/
void CMTPSetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_ENTRY );
    //no need to recive this data beacuse it is Get property
    ReceiveDataL(iUint8);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICESUPPORTEDFORMATSORDEREDL_EXIT );
    }

/*
*Processes the FunctionalID property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceFunctionalIDL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEFUNCTIONALIDL_ENTRY );

    TPtrC8 ptr(NULL,0);
    if ( KMTPChunkSequenceCompletion == iData->FirstReadChunk(ptr) )
   		{
   		SaveGUID( MMTPFrameworkConfig::EDeviceCurrentFuncationalID, *iData );
   		SendResponseL(EMTPRespCodeOK);
    	}
    
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEFUNCTIONALIDL_EXIT );
    }

/*
*FunctionalID property. 
*/
void CMTPSetDevicePropValue::ServiceFunctionalIDL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_ENTRY );
    ReceiveDataL(*iData);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEFUNCTIONALIDL_EXIT );
    }

/*
*Processes the ModelID property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceModelIDL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEMODELIDL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEMODELIDL_EXIT );
    }

/*
*ModelID property. 
*/
void CMTPSetDevicePropValue::ServiceModelIDL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEMODELIDL_ENTRY );	
    ReceiveDataL(*iData);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEMODELIDL_EXIT );
    }

/*
*Processes the UseDeviceStage property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceUseDeviceStageL()
    {
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEUSEDEVICESTAGEL_ENTRY );
    SendResponseL(EMTPRespCodeAccessDenied);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_COMPLETESERVICEUSEDEVICESTAGEL_EXIT );
    }

/*
*UseDeviceStage property. 
*/
void CMTPSetDevicePropValue::ServiceUseDeviceStageL()
    {	 
    OstTraceFunctionEntry0( CMTPSETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_ENTRY );
    ReceiveDataL(iUint8);
    OstTraceFunctionExit0( CMTPSETDEVICEPROPVALUE_SERVICEUSEDEVICESTAGEL_EXIT );
    }
