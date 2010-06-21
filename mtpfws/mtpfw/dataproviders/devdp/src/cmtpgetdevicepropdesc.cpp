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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetDevicePropDesc");)

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
    __FLOG(_L8("~CMTPGetDevicePropDesc - Entry"));
    delete iData;
    delete iPropDesc;
    delete iRepository;
    iDpSingletons.Close();
    __FLOG(_L8("~CMTPGetDevicePropDesc - Exit"));
    __FLOG_CLOSE;
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
    if((respCode != EMTPRespCodeOK) && iDpSingletons.DeviceDataStore().ExtnDevicePropDp())//2113 
        {
        respCode = EMTPRespCodeOK;
        }
    __FLOG(_L8("CheckRequestL - Exit"));
    return respCode;
    }

/**
GetDevicePropDesc request handler.
*/    
void CMTPGetDevicePropDesc::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));    
    iPropCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    //before performing any operation will check the properties are supported or 
    //not if not then return EMTPRespCodeDevicePropNotSupported
    const CMTPTypeArray *mtpArray = &(iDpSingletons.DeviceDataStore().GetSupportedDeviceProperties());
    RArray <TUint> supportedArray;	    
    mtpArray->Array(supportedArray);
    __FLOG_VA((_L8("No of elements in supported property array = %d "), supportedArray.Count()));	
    if(KErrNotFound == supportedArray.Find(iPropCode))
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported);       
        __FLOG(_L8("CMTPGetDevicePropDesc::EMTPRespCodeDevicePropNotSupported "));	  
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
    __FLOG(_L8("ServiceL - Exit"));
    }
    
    
void  CMTPGetDevicePropDesc::HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin )
	{
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

	
	}
void CMTPGetDevicePropDesc::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));
    if (iPropCode == EMTPDevicePropCodeBatteryLevel)
        {
        iDpSingletons.DeviceDataStore().Cancel();
        }
    __FLOG(_L8("DoCancel - Exit"));
    }
    
void CMTPGetDevicePropDesc::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    if (iPropCode == EMTPDevicePropCodeBatteryLevel)
        {
        ServiceBatteryLevelL();
        }
    else
        {
        __DEBUG_ONLY(Panic(EMTPDevDpUnknownDeviceProperty));
        }
    __FLOG(_L8("RunL - Exit"));
    }

/**
Second-phase constructor.
*/        
void CMTPGetDevicePropDesc::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPGetDevicePropDesc: ConstructL - Entry")); 
    iDpSingletons.OpenL(iFramework);
	const TUint32 KUidMTPRepositoryValue(0x10282FCC);
    const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
    iRepository = CRepository::NewL(KUidMTPRepository);
    __FLOG(_L8("CMTPGetDevicePropDesc: ConstructL - Exit")); 
    }

/**
Services the battery level property.
*/        
void CMTPGetDevicePropDesc::ServiceBatteryLevelL()
    {
    __FLOG(_L8("ServiceBatteryLevelL - Entry")); 
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
    __FLOG(_L8("ServiceBatteryLevelL - Exit")); 
    }

/**
Services the device friendly name property.
*/    
void CMTPGetDevicePropDesc::ServiceDeviceFriendlyNameL()
    {
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Entry"));
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
    __FLOG(_L8("ServiceDeviceFriendlyNameL - Exit")); 
    }
        
/**
Services the synchronisation partner property.
*/    
void CMTPGetDevicePropDesc::ServiceSynchronisationPartnerL()
    {
    __FLOG(_L8("ServiceSynchronisationPartnerL - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSynchronizationPartner);
    
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.SynchronisationPartnerDefault());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.SynchronisationPartner());
    
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServiceSynchronisationPartnerL - Exit")); 
    }

/**
*Services the synchronisation partner property. 
*/    
void CMTPGetDevicePropDesc::ServiceSessionInitiatorVersionInfoL()
    {
    __FLOG(_L8("ServiceSessionInitiatorVersionInfoL - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    // this property is of type set or get 
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSessionInitiatorVersionInfo, 0x01/*set/get*/, 0x00, NULL);   
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());   
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.SessionInitiatorVersionInfoDefault());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.SessionInitiatorVersionInfo());
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServiceSessionInitiatorVersionInfoL - Exit")); 
    }

/**
*Services the Service Perceived Device Type. 
*/    
void CMTPGetDevicePropDesc::ServicePerceivedDeviceTypeL()
    {
    __FLOG(_L8("ServicePerceivedDeviceType - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodePerceivedDeviceType, 0x00/*get only*/, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());    
    iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.PerceivedDeviceTypeDefault());
    iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, device.PerceivedDeviceType());
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServicePerceivedDeviceType - Exit")); 
    }

/**
Services the Date Time property. 
*/    
void CMTPGetDevicePropDesc::ServiceDateTimeL()
    {
    __FLOG(_L8("ServicePerceivedDeviceType - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeDateTime, 0x01/*get/set*/, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());    
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.DateTimeL());
    iPropDesc->SetStringL(CMTPTypeDevicePropDesc::ECurrentValue, device.DateTimeL());
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServicePerceivedDeviceType - Exit")); 
    }

/**
Services the Date Time property. 
*/	  
void CMTPGetDevicePropDesc::ServiceDeviceIconL()
    {
    __FLOG(_L8("ServiceDeviceIcon - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeDeviceIcon, 0x00, 0x00, NULL);
    CMTPDeviceDataStore& device(iDpSingletons.DeviceDataStore());	 
    //need to think of which one to be used for default.
    iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, device.DeviceIcon());
    iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, device.DeviceIcon());
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServiceDeviceIcon- Exit"));   
    }

/*
*Service Supported format ordered.
*/
void CMTPGetDevicePropDesc::ServiceSupportedFormatsOrderedL()
    {
    __FLOG(_L8("ServiceSupportedFormatsOrdered - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeSupportedFormatsOrdered, 0x00, 0x00, NULL);    
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, (TUint8)FORMAT_UNORDERED);
    iPropDesc->SetUint8L(CMTPTypeDevicePropDesc::ECurrentValue, GetFormatOrdered());
    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServiceSupportedFormatsOrdered - Exit")); 
    }

/*
*Service Supported FuntionalID.
*/
void CMTPGetDevicePropDesc::ServiceFunctionalIDL()
    {
    __FLOG(_L8("ServiceFuntionalIDL - Entry")); 
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
    __FLOG(_L8("ServiceFuntionalIDL - Exit")); 
    }

/*
*Service Supported ModelID.
*/
void CMTPGetDevicePropDesc::ServiceModelIDL()
    {
    __FLOG(_L8("ServiceModelIDL - Entry")); 
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
    __FLOG(_L8("ServiceModelIDL - Exit")); 
    }

/*
*Service Supported UseDeviceStage.
*/
void CMTPGetDevicePropDesc::ServiceUseDeviceStageL()
    {
    __FLOG(_L8("ServiceUseDeviceStageL - Entry")); 
    delete iPropDesc;
    iPropDesc = NULL;
    iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodeUseDeviceStage, 0, 0, NULL); 
    
    TMTPTypeUint8 *data = new (ELeave)TMTPTypeUint8(1);
    iPropDesc->SetL(CMTPTypeDevicePropDesc::EFactoryDefaultValue, *data);
    iPropDesc->SetL(CMTPTypeDevicePropDesc::ECurrentValue, *data);
    
    delete data;

    SendDataL(*iPropDesc); 
    __FLOG(_L8("ServiceUseDeviceStageL - Exit")); 
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
		__FLOG_VA((_L8("GetRssConfigArray returned with %d"), error));
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
    
    User::LeaveIfError(iRepository->Get(aKey,buf));

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

