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
#include <mtp/tmtptyperequest.h>

#include "cmtppbcgetdevicepropvalue.h"
#include "mtpplaybackcontroldpconst.h"
#include "cmtpplaybackcontroldp.h"
#include "cmtpplaybackproperty.h"
#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetPlaybackDevicePropValue");)

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPPbcGetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework, 
                                                   MMTPConnection& aConnection, 
                                                   CMTPPlaybackControlDataProvider& aDataProvider)
    {
    CMTPPbcGetDevicePropValue* self = new (ELeave) CMTPPbcGetDevicePropValue(aFramework, aConnection, aDataProvider);
    return self;
    }

/**
Destructor.
*/    
CMTPPbcGetDevicePropValue::~CMTPPbcGetDevicePropValue()
    {
    __FLOG(_L8("~CMTPPbcGetDevicePropValue - Entry"));
    delete iPbCmd;
    __FLOG(_L8("~CMTPPbcGetDevicePropValue - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPPbcGetDevicePropValue::CMTPPbcGetDevicePropValue(MMTPDataProviderFramework& aFramework, 
                                                MMTPConnection& aConnection,
                                                CMTPPlaybackControlDataProvider& aDataProvider):
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iPlaybackControlDp(aDataProvider)
    {
    //Open the log system
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

/**
CMTPPbcGetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcGetDevicePropValue::CheckRequestL()
    {
    __FLOG(_L8("CheckRequestL - Entry"));
    TMTPResponseCode respCode = CMTPRequestProcessor::CheckRequestL();
    if(respCode == EMTPRespCodeOK)
        {
        respCode = EMTPRespCodeDevicePropNotSupported;
        TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        const TInt count = sizeof(KMTPPlaybackControlDpSupportedProperties) / 
                           sizeof(KMTPPlaybackControlDpSupportedProperties[0]);
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
CMTPPbcGetDevicePropValue request handler.
*/   
void CMTPPbcGetDevicePropValue::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
    //Destroy the previous playback command.
    delete iPbCmd;
    iPbCmd = NULL;
    
    //Get the device property code
    TMTPDevicePropertyCode propCode(static_cast<TMTPDevicePropertyCode>(Request().
                                    Uint32(TMTPTypeRequest::ERequestParameter1)));
    
    TMTPPbCtrlData data;
    data.iOptCode = EMTPOpCodeGetDevicePropValue;
    data.iDevPropCode = propCode;

    //Get a new playback command.
    CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
    TInt result = map.GetPlaybackControlCommand(data, &iPbCmd);

    if(KErrNone == result)
        {
        MMTPPlaybackControl& control(iPlaybackControlDp.GetPlaybackControlL());
        TRAPD(err, control.CommandL(*iPbCmd, this));
        __ASSERT_ALWAYS((err == KErrNone), SendResponseL(EMTPRespCodeParameterNotSupported));
        }
    else if(KErrNotSupported == result)
        {
        SendResponseL(EMTPRespCodeDevicePropNotSupported);
        }
    else
        {
        SendResponseL(EMTPRespCodeParameterNotSupported);
        }
    __FLOG(_L8("ServiceL - Exit"));
    }

void CMTPPbcGetDevicePropValue::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    __FLOG(_L8("HandlePlaybackCommandCompleteL - Entry"));
    __FLOG_1(_L8("aErr %d"), aErr);

    //Handle the error
    TBool useDefault = EFalse;
    switch(aErr)
        {
        case KPlaybackErrNone:
            {
            __ASSERT_DEBUG((aCmd != NULL), Panic(EMTPPBDataNullErr));
            __ASSERT_DEBUG((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), Panic(EMTPPBArgumentErr));
            __ASSERT_ALWAYS((aCmd != NULL), User::Leave(KErrArgument));
            __ASSERT_ALWAYS((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), User::Leave(KErrArgument));
            __FLOG_1(_L8("aCmd %d"), aCmd->PlaybackCommand());
            }
            break;
        case KPlaybackErrContextInvalid:
            {
            useDefault = ETrue;
            }
            break;
        case KPlaybackErrDeviceUnavailable:
            {
            iPlaybackControlDp.RequestToResetPbCtrl();
            SendResponseL(EMTPRespCodeDeviceBusy);
            }
            return;
        default:
            {
            SendResponseL(EMTPRespCodeDeviceBusy);
            }
            return;
        }

    CMTPPlaybackProperty& property(iPlaybackControlDp.GetPlaybackProperty());
    TMTPDevicePropertyCode propCode(static_cast<TMTPDevicePropertyCode>
                                   (Request().Uint32(TMTPTypeRequest::ERequestParameter1)));
   
    switch(propCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
        TInt32 val;
        if(useDefault)
            {
            property.GetDefaultPropertyValueL(propCode, val);
            }
        else
            {
            TMTPPlaybackState state = static_cast<TMTPPlaybackState>(aCmd->ParamL().Uint32L());
            val = map.PlaybackRateL(state);
            }
        iInt32.Set(val);
        SendDataL(iInt32);
        }
        break;

    case EMTPDevicePropCodePlaybackObject:
        {
        CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
        TUint32 val;
        if(useDefault)
            {
            property.GetDefaultPropertyValueL(propCode, val);
            }
        else
            {
            val = map.ObjectHandleL(aCmd->ParamL().SuidSetL().Suid());
            }
        iUint32.Set(val);
        SendDataL(iUint32);
        }
        break;

    case EMTPDevicePropCodeVolume:
    case EMTPDevicePropCodePlaybackContainerIndex:
    case EMTPDevicePropCodePlaybackPosition:
        {
        TUint32 val;
        if(useDefault)
            {
            property.GetDefaultPropertyValueL(propCode, val);
            }
        else
            {
            val = aCmd->ParamL().Uint32L();
            }
        iUint32.Set(val);
        SendDataL(iUint32);
        }
        break;

    default:
        SendResponseL(EMTPRespCodeDevicePropNotSupported);
        break;             
        }
    __FLOG(_L8("HandlePlaybackCommandCompleteL - Exit"));
    }

