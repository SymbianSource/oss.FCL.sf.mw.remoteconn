// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>

#include "cmtppbcsetdevicepropvalue.h"
#include "mtpplaybackcontroldpconst.h"
#include "cmtpplaybackcontroldp.h"
#include "cmtpplaybackproperty.h"
#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SetPlaybackDevicePropValue");)

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/
MMTPRequestProcessor* CMTPPbcSetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework,
                                                    MMTPConnection& aConnection, 
                                                    CMTPPlaybackControlDataProvider& aDataProvider)
    {
    CMTPPbcSetDevicePropValue* self = new (ELeave) CMTPPbcSetDevicePropValue(aFramework, aConnection, aDataProvider);
    return self;
    }

/**
Destructor
*/    
CMTPPbcSetDevicePropValue::~CMTPPbcSetDevicePropValue()
    {    
    __FLOG(_L8("~CMTPPbcSetDevicePropValue - Entry"));
    delete iPbCmd;
    __FLOG(_L8("~CMTPPbcSetDevicePropValue - Exit"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/    
CMTPPbcSetDevicePropValue::CMTPPbcSetDevicePropValue(MMTPDataProviderFramework& aFramework,
                                               MMTPConnection& aConnection,
                                               CMTPPlaybackControlDataProvider& aDataProvider):
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iPlaybackControlDp(aDataProvider)
    {
    //Open the log system
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }
    
/**
CMTPPbcSetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcSetDevicePropValue::CheckRequestL()
    {
    __FLOG(_L8("CheckRequestL - Entry"));
    TMTPResponseCode respCode = CMTPRequestProcessor::CheckRequestL();
    if(respCode == EMTPRespCodeOK)
        {
        respCode = EMTPRespCodeDevicePropNotSupported;
        TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        const TInt count = sizeof(KMTPPlaybackControlDpSupportedProperties) / sizeof(KMTPPlaybackControlDpSupportedProperties[0]);
        for (TUint i(0); (i < count); i++)
            {
            if (propCode == KMTPPlaybackControlDpSupportedProperties[i])
                {
                respCode = EMTPRespCodeOK;
                break;
                }
            }
        }

    __FLOG(_L8("CheckRequestL - Exit"));
    return respCode;
    }

/**
Process the transaction response phase.
*/    
TBool CMTPPbcSetDevicePropValue::DoHandleResponsePhaseL()
    {
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));

    CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
    //Destroy the previous playback command.
    delete iPbCmd;
    iPbCmd = NULL;
    
    //Get a new playback command.
    TInt result = map.GetPlaybackControlCommand(iData, &iPbCmd);

    if(KErrNone == result)
        {
        MMTPPlaybackControl& control(iPlaybackControlDp.GetPlaybackControlL());
        TRAPD(err, control.CommandL(*iPbCmd, this));
        __ASSERT_ALWAYS((err == KErrNone), SendResponseL(EMTPRespCodeInvalidDevicePropValue));
        }
    else if(KErrNotSupported == result)
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported);
        }
    else
        {
        SendResponseL(EMTPRespCodeInvalidDevicePropValue);
        }

    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));    
    return EFalse;    
    }
    
void CMTPPbcSetDevicePropValue::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
    TMTPDevicePropertyCode propCode(static_cast<TMTPDevicePropertyCode>(Request().
                                    Uint32(TMTPTypeRequest::ERequestParameter1)));
    iData.iOptCode = EMTPOpCodeSetDevicePropValue;
    iData.iDevPropCode = propCode;
    switch(propCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        ReceiveDataL(iData.iPropValInt32);
        }
        break;
    case EMTPDevicePropCodeVolume:
    case EMTPDevicePropCodePlaybackObject:
    case EMTPDevicePropCodePlaybackContainerIndex:
    case EMTPDevicePropCodePlaybackPosition:
        {
        ReceiveDataL(iData.iPropValUint32);        
        }
        break;
        
    default:
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported);
        }
        break;             
        }
    __FLOG(_L8("ServiceL - Exit"));
    }

void CMTPPbcSetDevicePropValue::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    __FLOG(_L8("HandlePlaybackCommandCompleteL - Entry"));
    __FLOG_1(_L8("aErr %d"), aErr);
    
    //Handle error response.
     TMTPResponseCode response;
     switch(aErr)
         {
         case KPlaybackErrNone:
             {
             response = EMTPRespCodeOK;            
             }
             break;
         case KPlaybackErrDeviceUnavailable:
             {
             response = EMTPRespCodeDeviceBusy;
             iPlaybackControlDp.RequestToResetPbCtrl();
             }
             break;
         case KPlaybackErrContextInvalid:
             {
            CMTPPlaybackProperty& prop(iPlaybackControlDp.GetPlaybackProperty());
            if(prop.IsDefaultPropertyValueL(iData))
                {
                response = EMTPRespCodeOK;                
                }
            else
                {
                response = EMTPRespCodeAccessDenied;    
                }   
             }
             break;
         case KPlaybackErrParamInvalid:
             {
             response = EMTPRespCodeInvalidDevicePropValue;
             }
             break;
         default:
             {
             response = EMTPRespCodeDeviceBusy;
             }
             break;
         }
     
     SendResponseL(response);
     
     if(aCmd != NULL)
         {
         __ASSERT_DEBUG((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), Panic(EMTPPBArgumentErr));
         __FLOG_1(_L8("aCmd %d"), aCmd->PlaybackCommand());
         }

    __FLOG(_L8("HandlePlaybackCommandCompleteL - Exit"));
    }
