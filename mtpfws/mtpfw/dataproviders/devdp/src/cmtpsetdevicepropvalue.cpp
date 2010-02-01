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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SetDevicePropValue");)

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
    __FLOG(_L8("~CMTPSetDevicePropValue - Entry"));
    delete iString;
    delete iMtparray;
    delete iData;
    __FLOG(_L8("~CMTPSetDevicePropValue - Exit"));
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    CMTPGetDevicePropDesc::ConstructL();
    iString = CMTPTypeString::NewL();
    iMtparray = CMTPTypeArray::NewL(EMTPTypeAUINT8);
    iData = new(ELeave) TMTPTypeGuid();
    __FLOG(_L8("ConstructL - Exit")); 
    }

/**
Service Battery level property
*/    
void CMTPSetDevicePropValue::ServiceBatteryLevelL()
    {
    __FLOG(_L8("ServiceBatteryLevelL - Entry"));
    SendResponseL(EMTPRespCodeAccessDenied); 
    __FLOG(_L8("ServiceBatteryLevelL - Exit"));   
    }

/**
Service the device friendly name property.
*/   
void CMTPSetDevicePropValue::ServiceDeviceFriendlyNameL()
    {
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Entry"));
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString); 
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Exit"));    
    }
        
/**
Service the synchronisation partner property.
*/ 
void CMTPSetDevicePropValue::ServiceSynchronisationPartnerL()
    {    
    __FLOG(_L8("ServiceSynchronisationPartnerL - Entry")); 
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString);
    __FLOG(_L8("ServiceSynchronisationPartnerL - Exit"));
    }
    
/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPSetDevicePropValue::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry")); 
	TMTPResponseCode responseCode = CMTPGetDevicePropDesc::CheckRequestL();
	
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	if( propCode == EMTPDevicePropCodeBatteryLevel)
		{
		responseCode = EMTPRespCodeAccessDenied;
		}
	__FLOG(_L8("CheckRequestL - Exit")); 
	return responseCode;
	}

/**
Process the transaction response phase.
*/    
TBool CMTPSetDevicePropValue::DoHandleResponsePhaseL()
    {
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));
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
    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));
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
    __FLOG(_L8("CompleteServiceDeviceFriendlyNameL - Entry"));
    iDpSingletons.DeviceDataStore().SetDeviceFriendlyNameL(iString->StringChars());
    SendResponseL(EMTPRespCodeOK);  
    __FLOG(_L8("CompleteServiceDeviceFriendlyNameL - Exit"));  
    }

/**
Processes the synchronisation partner property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceSynchronisationPartnerL()
    {
    __FLOG(_L8("CompleteServiceSynchronisationPartnerL - Entry"));
    iDpSingletons.DeviceDataStore().SetSynchronisationPartnerL(iString->StringChars());
    SendResponseL(EMTPRespCodeOK);
    __FLOG(_L8("CompleteServiceSynchronisationPartnerL - Exit"));
    }

void CMTPSetDevicePropValue::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin)
	{
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
	}
 
/**
Processes the session initiator version info and set the same to session device data store.
*/
void CMTPSetDevicePropValue::CompleteServiceSessionInitiatorVersionInfoL()
	{
	__FLOG(_L8("CompleteServiceSynchronisationPartnerL - Entry"));
	RProcess process;
	RProperty::Set(process.SecureId(), EMTPConnStateKey, iString->StringChars());
	iDpSingletons.DeviceDataStore().SetSessionInitiatorVersionInfoL(iString->StringChars());
	SendResponseL(EMTPRespCodeOK);
	__FLOG(_L8("CompleteServiceSynchronisationPartnerL - Exit"));
	}

/**
Service session initiator property.
*/ 
void CMTPSetDevicePropValue::ServiceSessionInitiatorVersionInfoL()
	{	 
	__FLOG(_L8("SetSessionInitiatorVersionInfoL - Entry")); 
	iString->SetL(KNullDesC);
	ReceiveDataL(*iString);
	__FLOG(_L8("SetSessionInitiatorVersionInfoL - Exit"));
	}


/**
This should be removed no set for percived device type.
*/
void CMTPSetDevicePropValue::CompletePerceivedDeviceTypeL()
    {
    __FLOG(_L8("CompletePerceivedDeviceType - Entry"));
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("CompletePerceivedDeviceType - Exit"));
    }

/**
*Service the PerceivedDeviceType property.
*it is not set type it should be removed 
**/
void CMTPSetDevicePropValue::ServicePerceivedDeviceTypeL()
    {	 
    __FLOG(_L8("ServicePerceivedDeviceType - Entry")); 	
    ReceiveDataL(iUint32);
    __FLOG(_L8("ServicePerceivedDeviceType - Exit"));
    }

/**
Processes the Date Time property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceDateTimeL()
	{
	__FLOG(_L8("CompleteDateTime - Entry"));
	//validate the incoming date time string first and then set it.
	if(KErrNone == iDpSingletons.DeviceDataStore().SetDateTimeL(iString->StringChars()) )
		{
		SendResponseL(EMTPRespCodeOK);
		}
	else
		{
		SendResponseL(EMTPRespCodeInvalidDataset);
		}
	
	__FLOG(_L8("CompleteDateTime - Exit"));
	}

/**
Service the Date Time property. 
*/ 
void CMTPSetDevicePropValue::ServiceDateTimeL()
    {	 
    __FLOG(_L8("ServiceDateTime - Entry")); 
    iString->SetL(KNullDesC);
    ReceiveDataL(*iString);
    __FLOG(_L8("ServiceDateTime - Exit"));
    }

/*
*Complete Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/
void CMTPSetDevicePropValue::CompleteDeviceIconL()
    {   	
    __FLOG(_L8("CompleteDeviceIcon - Entry"));
    //it is Get only device property
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("CompleteDeviceIcon - Exit"));
    }

/*
*Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/   
void CMTPSetDevicePropValue::ServiceDeviceIconL()
    {    
    __FLOG(_L8("ServiceDeviceIcon - Entry")); 	
    //no need to recive this data beacuse it is Get property
    ReceiveDataL(*iMtparray);
    __FLOG(_L8("ServiceDeviceIcon - Exit"));
    }
       

/*
*Processes the PerceivedDeviceType property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceSupportedFormatsOrderedL()
    {
    __FLOG(_L8("CompleteServiceSupportedFormatsOrdered - Entry"));
    //it is Get only device property
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("CompleteServiceSupportedFormatsOrdered - Exit"));
    }

/*
*ServiceSupportedFormatsOrdered property. it is get only type.
*/
void CMTPSetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {	 
    __FLOG(_L8("ServiceSupportedFormatsOrdered - Entry")); 	
    //no need to recive this data beacuse it is Get property
    ReceiveDataL(iUint8);
    __FLOG(_L8("ServicePerceivedDeviceType - Exit"));
    }

/*
*Processes the FunctionalID property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceFunctionalIDL()
    {
    __FLOG(_L8("CompleteServiceFunctionalIDL - Entry"));

    TPtrC8 ptr(NULL,0);
    if ( KMTPChunkSequenceCompletion == iData->FirstReadChunk(ptr) )
   		{
   		SaveGUID( MMTPFrameworkConfig::EDeviceCurrentFuncationalID, *iData );
   		SendResponseL(EMTPRespCodeOK);
    	}
    
    __FLOG(_L8("CompleteServiceFunctionalIDL - Exit"));
    }

/*
*FunctionalID property. 
*/
void CMTPSetDevicePropValue::ServiceFunctionalIDL()
    {	 
    __FLOG(_L8("ServiceFunctionalIDL - Entry")); 	
    ReceiveDataL(*iData);
    __FLOG(_L8("ServiceFunctionalIDL - Exit"));
    }

/*
*Processes the ModelID property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceModelIDL()
    {
    __FLOG(_L8("CompleteServiceModelIDL - Entry"));
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("CompleteServiceModelIDL - Exit"));
    }

/*
*ModelID property. 
*/
void CMTPSetDevicePropValue::ServiceModelIDL()
    {	 
    __FLOG(_L8("ServiceModelIDL - Entry")); 	
    ReceiveDataL(*iData);
    __FLOG(_L8("ServiceModelIDL - Exit"));
    }

/*
*Processes the UseDeviceStage property transaction response phase.
*/
void CMTPSetDevicePropValue::CompleteServiceUseDeviceStageL()
    {
    __FLOG(_L8("CompleteServiceUseDeviceStageL - Entry"));
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("CompleteServiceUseDeviceStageL - Exit"));
    }

/*
*UseDeviceStage property. 
*/
void CMTPSetDevicePropValue::ServiceUseDeviceStageL()
    {	 
    __FLOG(_L8("ServiceUseDeviceStageL - Entry")); 	
    ReceiveDataL(iUint8);
    __FLOG(_L8("ServiceUseDeviceStageL - Exit"));
    }
