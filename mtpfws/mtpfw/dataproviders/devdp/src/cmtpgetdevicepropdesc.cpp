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

#include <mtp/cmtptypedevicepropdesc.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpframeworkconfig.h>
#include <centralrepository.h>

#include "cmtpdevicedatastore.h"
#include "cmtpgetdevicepropdesc.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"
#include "cmtpdevicedpconfigmgr.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetdevicepropdescTraces.h"
#endif


// Class constants.

_LIT(KSpace, " ");

/**
Two-phase constructor.
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/  
MMTPRequestProcessor* CMTPGetDevicePropDesc::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetDevicePropDesc* self = new (ELeave) CMTPGetDevicePropDesc(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPGetDevicePropDesc::~CMTPGetDevicePropDesc()
    {    
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_CMTPGETDEVICEPROPDESC_DES_ENTRY );
    delete iData;
    delete iPropDesc;
    delete iRepository;
    iDpSingletons.Close();
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_CMTPGETDEVICEPROPDESC_DES_EXIT );
    }

/**
Constructor.
*/    
CMTPGetDevicePropDesc::CMTPGetDevicePropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {    
    }
    
/**
GetDevicePropDesc request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPGetDevicePropDesc::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_CHECKREQUESTL_ENTRY );
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
    if((respCode != EMTPRespCodeOK) && iDpSingletons.DeviceDataStore().ExtnDevicePropDp())//2113 
        {
        respCode = EMTPRespCodeOK;
        }
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_CHECKREQUESTL_EXIT );
    return respCode;
    }

/**
GetDevicePropDesc request handler.
*/    
void CMTPGetDevicePropDesc::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEL_ENTRY );  
    iPropCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    //before performing any operation will check the properties are supported or 
    //not if not then return EMTPRespCodeDevicePropNotSupported
    const CMTPTypeArray *mtpArray = &(iDpSingletons.DeviceDataStore().GetSupportedDeviceProperties());
    RArray <TUint> supportedArray;	    
    mtpArray->Array(supportedArray);
    OstTrace1(TRACE_NORMAL, CMTPGETDEVICEPROPDESC_SERVICEL, 
            "No of elements in supported property array = %d ", supportedArray.Count());
    if(KErrNotFound == supportedArray.Find(iPropCode))
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported);       
        OstTrace0(TRACE_NORMAL, DUP1_CMTPGETDEVICEPROPDESC_SERVICEL, "CMTPGetDevicePropDesc::EMTPRespCodeDevicePropNotSupported ");
        }
    else
        {
        switch (iPropCode)
            {
            case EMTPDevicePropCodeBatteryLevel:
            if (iDpSingletons.DeviceDataStore().RequestPending())
                {
                // BatteryLevel already pending - return busy code
                SendResponseL(EMTPRespCodeDeviceBusy);
                }
            else
                {
                iDpSingletons.DeviceDataStore().BatteryLevelL(iStatus, iBatteryLevelValue);
                SetActive();	
                }
            break;
            
            case EMTPDevicePropCodeSynchronizationPartner:
                ServiceSynchronisationPartnerL();
            break;
            
            case EMTPDevicePropCodeDeviceFriendlyName:
                ServiceDeviceFriendlyNameL();
            break;
            
            case EMTPDevicePropCodeSessionInitiatorVersionInfo:
                ServiceSessionInitiatorVersionInfoL();
            break;
            
            case EMTPDevicePropCodePerceivedDeviceType:
                ServicePerceivedDeviceTypeL();
            break;
            
            case EMTPDevicePropCodeDateTime:
                ServiceDateTimeL();
            break;
            
            case EMTPDevicePropCodeDeviceIcon:
                ServiceDeviceIconL();
            break;

            case EMTPDevicePropCodeSupportedFormatsOrdered:
            ServiceSupportedFormatsOrderedL();
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
            if(iDpSingletons.DeviceDataStore().ExtnDevicePropDp())
                {
                HandleExtnServiceL(iPropCode, iDpSingletons.DeviceDataStore().ExtnDevicePropDp());
                }
            else 
                SendResponseL(EMTPRespCodeDevicePropNotSupported); 
            break;
            }
        }
    supportedArray.Close();
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEL_EXIT );
    }
    
    
void  CMTPGetDevicePropDesc::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin )
	{
	OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_HANDLEEXTNSERVICEL_ENTRY );
	//call	 plugin ->desc
	MMTPType* mtptype;
	if(KErrNone == aExtnDevplugin->GetDevPropertyDescL((TMTPDevicePropertyCode)aPropCode, &mtptype))
	{
	SendDataL(*mtptype);	
	}
	else
	{
	SendResponseL(EMTPRespCodeDevicePropNotSupported); 	
	}

	
	OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_HANDLEEXTNSERVICEL_EXIT );
	}
void CMTPGetDevicePropDesc::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_DOCANCEL_ENTRY );
    if (iPropCode == EMTPDevicePropCodeBatteryLevel)
        {
        iDpSingletons.DeviceDataStore().Cancel();
        }
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_DOCANCEL_EXIT );
    }
    
void CMTPGetDevicePropDesc::RunL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_RUNL_ENTRY );
    if (iPropCode == EMTPDevicePropCodeBatteryLevel)
        {
        ServiceBatteryLevelL();
        }
    else
        {
        __DEBUG_ONLY(Panic(EMTPDevDpUnknownDeviceProperty));
        }
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_RUNL_EXIT );
    }

/**
Second-phase constructor.
*/        
void CMTPGetDevicePropDesc::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_CONSTRUCTL_ENTRY );
    iDpSingletons.OpenL(iFramework);
	const TUint32 KUidMTPRepositoryValue(0x10282FCC);
    const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPRepository);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_CONSTRUCTL_EXIT );
    }

/**
Services the battery level property.
*/        
void CMTPGetDevicePropDesc::ServiceBatteryLevelL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEBATTERYLEVELL_ENTRY );
    CMTPTypeDevicePropDescRangeForm* form = CMTPTypeDevicePropDescRangeForm::NewLC(EMTPTypeUINT8);
    form->SetUint8L(CMTPTypeDevicePropDescRangeForm::EMinimumValue, 0);
    form->SetUint8L(CMTPTypeDevicePropDescRangeForm::EMaximumValue, 100);
    form->SetUint8L(CMTPTypeDevicePropDescRangeForm::EStepSize, 10);
    
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeBatteryLevel, *form);
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, 0);    
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::ECurrentValue, iBatteryLevelValue);
    CleanupStack::PopAndDestroy(form);

    SendDataL(*iPropDesc);
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEBATTERYLEVELL_EXIT );
    }

/**
Services the device friendly name property.
*/    
void CMTPGetDevicePropDesc::ServiceDeviceFriendlyNameL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEDEVICEFRIENDLYNAMEL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeDeviceFriendlyName);
    
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.DeviceFriendlyNameDefault());
    
    //if device friendly name is blank, which means it is the first time the device get connected,
    //, so will use "manufacture + model id" firstly; if neither manufacture nor model
    //id not able to be fetched by API, then use the default device friendly name 
    if ( device.DeviceFriendlyName().Length()<=0 )
        {
        if ( device.Manufacturer().Compare(KMTPDefaultManufacturer) && device.Model().Compare(KMTPDefaultModel) )
            {
            HBufC* friendlyName = HBufC::NewLC(device.Manufacturer().Length()+1+ device.Model().Length());
            TPtr ptrName = friendlyName->Des();
            ptrName.Copy(device.Manufacturer());
            ptrName.Append(KSpace);
            ptrName.Append(device.Model());
            device.SetDeviceFriendlyNameL(ptrName);
            iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, ptrName);
            CleanupStack::PopAndDestroy(friendlyName);
            }
        else
            {
            iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.DeviceFriendlyNameDefault());
            }
        }
    else
        {
        iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.DeviceFriendlyName());
        }
    
    SendDataL(*iPropDesc);    
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEDEVICEFRIENDLYNAMEL_EXIT );
    }
        
/**
Services the synchronisation partner property.
*/    
void CMTPGetDevicePropDesc::ServiceSynchronisationPartnerL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICESYNCHRONISATIONPARTNERL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSynchronizationPartner);
    
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.SynchronisationPartnerDefault());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.SynchronisationPartner());
    
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICESYNCHRONISATIONPARTNERL_EXIT );
    }

/**
*Services the synchronisation partner property. 
*/    
void CMTPGetDevicePropDesc::ServiceSessionInitiatorVersionInfoL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICESESSIONINITIATORVERSIONINFOL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    // this property is of type set or get 
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSessionInitiatorVersionInfo, 0x01/*set/get*/, 0x00, NULL);   
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());   
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.SessionInitiatorVersionInfoDefault());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.SessionInitiatorVersionInfo());
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICESESSIONINITIATORVERSIONINFOL_EXIT );
    }

/**
*Services the Service Perceived Device Type. 
*/    
void CMTPGetDevicePropDesc::ServicePerceivedDeviceTypeL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEPERCEIVEDDEVICETYPEL_ENTRY ); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodePerceivedDeviceType, 0x00/*get only*/, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());    
    iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.PerceivedDeviceTypeDefault());
    iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, device.PerceivedDeviceType());
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEPERCEIVEDDEVICETYPEL_EXIT );
    }

/**
Services the Date Time property. 
*/    
void CMTPGetDevicePropDesc::ServiceDateTimeL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEDATETIMEL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeDateTime, 0x01/*get/set*/, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());    
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.DateTimeL());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.DateTimeL());
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEDATETIMEL_EXIT );
    }

/**
Services the Date Time property. 
*/	  
void CMTPGetDevicePropDesc::ServiceDeviceIconL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEDEVICEICONL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeDeviceIcon, 0x00, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());	 
    //need to think of which one to be used for default.
    iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.DeviceIcon());
    iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, device.DeviceIcon());
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEDEVICEICONL_EXIT );
    }

/*
*Service Supported format ordered.
*/
void CMTPGetDevicePropDesc::ServiceSupportedFormatsOrderedL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICESUPPORTEDFORMATSORDEREDL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSupportedFormatsOrdered, 0x00, 0x00, NULL);    
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, (TUint8)FORMAT_UNORDERED);
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::ECurrentValue, GetFormatOrdered());
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICESUPPORTEDFORMATSORDEREDL_EXIT );
    }

/*
*Service Supported FuntionalID.
*/
void CMTPGetDevicePropDesc::ServiceFunctionalIDL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEFUNCTIONALIDL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeFunctionalID, 1, 0, NULL); 
    
    delete iData;
	iData = GetGUIDL( MMTPFrameworkConfig::EDeviceDefaultFuncationalID ); 
	iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, *iData);
	delete iData;
	iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentFuncationalID); 
	iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, *iData);
	
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEFUNCTIONALIDL_EXIT );
    }

/*
*Service Supported ModelID.
*/
void CMTPGetDevicePropDesc::ServiceModelIDL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEMODELIDL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeModelID, 0, 0, NULL);   
    
    delete iData;
    iData = GetGUIDL(MMTPFrameworkConfig::EDeviceDefaultModelID);  
    iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, *iData);
    
    delete iData;
    iData = GetGUIDL(MMTPFrameworkConfig::EDeviceCurrentModelID); 
	iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, *iData);
	
    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEMODELIDL_EXIT );
    }

/*
*Service Supported UseDeviceStage.
*/
void CMTPGetDevicePropDesc::ServiceUseDeviceStageL()
    {
    OstTraceFunctionEntry0( CMTPGETDEVICEPROPDESC_SERVICEUSEDEVICESTAGEL_ENTRY );
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeUseDeviceStage, 0, 0, NULL); 
    
    TMTPTypeUint8 *data = new (ELeave)TMTPTypeUint8(1);
    iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, *data);
    iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, *data);
    
    delete data;

    SendDataL(*iPropDesc); 
    OstTraceFunctionExit0( CMTPGETDEVICEPROPDESC_SERVICEUSEDEVICESTAGEL_EXIT );
    }

/*
*This method to set the supported format order.
*this value will be set by getdevice info based on the formats present in the
*mtpdevicedp_config.rss file.
*/
TUint8 CMTPGetDevicePropDesc::GetFormatOrdered()
    {
    TUint8 formatOrdered;
    RArray<TUint> orderedFormats(8);
    CleanupClosePushL(orderedFormats);  
    TRAPD(error,iDpSingletons.ConfigMgr().GetRssConfigInfoArrayL(orderedFormats, EDevDpFormats));
	if(error!=KErrNone)
		{
		OstTrace1(TRACE_WARNING, CMTPGETDEVICEPROPDESC_GETFORMATORDERED,
		        "GetRssConfigArray returned with %d", error);
		}
    if(orderedFormats.Count() > 0)
        {
        formatOrdered = (TUint8)FORMAT_ORDERED;
        }
    else
        {
        formatOrdered = (TUint8)FORMAT_UNORDERED; 
        }
    CleanupStack::PopAndDestroy(&orderedFormats);   
    return formatOrdered;
    }

TMTPTypeGuid* CMTPGetDevicePropDesc::GetGUIDL(const TUint aKey)
    {
    TBuf<KGUIDFormatStringLength> buf;
    
    LEAVEIFERROR(iRepository->Get(aKey,buf),
    OstTrace1( TRACE_ERROR, CMTPGETDEVICEPROPDESC_GETGUIDL, "can't get from iRepository. the parameter is %d", aKey));

    TMTPTypeGuid* ret = new (ELeave) TMTPTypeGuid( buf );
    
    return ret;
    }

void CMTPGetDevicePropDesc::SaveGUID( const TUint aKey,  TMTPTypeGuid& aValue )
    {
	TBuf<KGUIDFormatStringLength> buf;
	if(aValue.ToString(buf) == KErrNone)
		{
		iRepository->Set(aKey,buf);
		}
    }

