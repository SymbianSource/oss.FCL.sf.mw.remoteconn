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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ResetDevicePropValue");)

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
    __FLOG(_L8("~CMTPResetDevicePropValue - Entry"));
        iDpSingletons.Close();
        delete iData;
        delete iRepository;
    __FLOG(_L8("~CMTPResetDevicePropValue - Exit"));
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    iDpSingletons.OpenL(iFramework);
	const TUint32 KUidMTPRepositoryValue(0x10282FCC);
    const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPRepository);
    __FLOG(_L8("ConstructL - Exit")); 
    }

/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPResetDevicePropValue::CheckRequestL()
    {
    __FLOG(_L8("CheckRequestL - Entry"));
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
    __FLOG(_L8("CheckRequestL - Exit"));
    return respCode;
    }
/**
ResetDevicePropValue request handler.
*/ 	
void CMTPResetDevicePropValue::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
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
    __FLOG(_L8("SetSessionInitiatorVersionInfo - Entry")); 
    iDpSingletons.DeviceDataStore().SetSessionInitiatorVersionInfoL( iDpSingletons.DeviceDataStore().SessionInitiatorVersionInfoDefault());
    SendResponseL(EMTPRespCodeOK);
    __FLOG(_L8("SetSessionInitiatorVersionInfo - Exit"));
    }

/**
*Service the PerceivedDeviceType property.
*it is not set type it should be removed 
**/
void CMTPResetDevicePropValue::ServicePerceivedDeviceTypeL()
    {  
    __FLOG(_L8("ServicePerceivedDeviceType - Entry")); 
    //PerceivedDeviceType is of type get only .
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("ServicePerceivedDeviceType - Exit"));
    }

/**
Service the Date Time property.
*/ 
void CMTPResetDevicePropValue::ServiceDateTimeL()
    {  
    __FLOG(_L8("ServiceDateTime - Entry")); 
    SendResponseL(EMTPRespCodeOperationNotSupported);
    __FLOG(_L8("ServiceDateTime - Exit"));
    }


/*
*Service the Device Icon property and CompleteDeviceIcon.
As of now implemented as device property of type get.
*/   
void CMTPResetDevicePropValue::ServiceDeviceIconL()
    {  
    __FLOG(_L8("ServiceDeviceIcon - Entry")); 
    //DeviceIcon property is implemented as get only .
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("ServiceDeviceIcon - Exit"));
    }

/*
*ServiceSupportedFormatsOrdered property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceSupportedFormatsOrderedL()
    {  
    __FLOG(_L8("ServiceSupportedFormatsOrdered - Entry"));  
    //no need to recive this data beacuse it is Get property
    //iDpSingletons.DeviceDataStore().SetFormatOrdered( iDpSingletons.DeviceDataStore().FormatOrderedDefault());
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("ServicePerceivedDeviceType - Exit"));
    }

/*
*FunctionalID property. 
*/
void CMTPResetDevicePropValue::ServiceFunctionalIDL()
    {	 
    __FLOG(_L8("ServiceFunctionalIDL - Entry")); 
    delete iData;
    iData = GetGUIDL( MMTPFrameworkConfig::EDeviceDefaultFuncationalID ); 
    SaveGUID(MMTPFrameworkConfig::EDeviceCurrentFuncationalID, *iData);
    SendResponseL(EMTPRespCodeOK);
    __FLOG(_L8("ServiceFunctionalIDL - Exit"));
    }

/*
*ModelID property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceModelIDL()
    {	 
    __FLOG(_L8("ServiceModelIDL - Entry")); 	
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("ServiceModelIDL - Exit"));
    }

/*
*UseDeviceStage property. it is get only type.
*/
void CMTPResetDevicePropValue::ServiceUseDeviceStageL()
    {	 
    __FLOG(_L8("ServiceUseDeviceStageL - Entry")); 	
    SendResponseL(EMTPRespCodeAccessDenied);
    __FLOG(_L8("ServiceUseDeviceStageL - Exit"));
    }

TMTPTypeGuid* CMTPResetDevicePropValue::GetGUIDL(const TUint aKey)
    {
    TBuf<KGUIDFormatStringLength> buf;
    
    User::LeaveIfError(iRepository->Get(aKey,buf));

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
