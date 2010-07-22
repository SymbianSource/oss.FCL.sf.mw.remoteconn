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

#include "cmtppbcresetdevicepropvalue.h"
#include "mtpplaybackcontroldpconst.h"
#include "cmtpplaybackcontroldp.h"
#include "cmtpplaybackproperty.h"
#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ResetPlaybackDevicePropValue");)

/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPPbcResetDevicePropValue::NewL(MMTPDataProviderFramework& aFramework, 
                                                    MMTPConnection& aConnection, 
                                                    CMTPPlaybackControlDataProvider& aDataProvider)
    {
    CMTPPbcResetDevicePropValue* self = new (ELeave) CMTPPbcResetDevicePropValue(aFramework, aConnection, aDataProvider);
    return self;
    }

/**
Destructor
*/    
CMTPPbcResetDevicePropValue::~CMTPPbcResetDevicePropValue()
    {    
    __FLOG(_L8("~CMTPPbcResetDevicePropValue - Entry"));
    delete iPbCmd;
    __FLOG(_L8("~CMTPPbcResetDevicePropValue - Exit"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/    
CMTPPbcResetDevicePropValue::CMTPPbcResetDevicePropValue(MMTPDataProviderFramework& aFramework, 
                                                    MMTPConnection& aConnection,
                                                    CMTPPlaybackControlDataProvider& aDataProvider):
                                                    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
                                                    iPlaybackControlDp(aDataProvider)
    {
    //Open the log system
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcResetDevicePropValue::CheckRequestL()
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
ResetDevicePropValue request handler.
*/ 	
void CMTPPbcResetDevicePropValue::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));

    CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
    //Destroy the previous playback command.
    delete iPbCmd;
    iPbCmd = NULL;
    
    //Get a new playback command.
    iData.iOptCode = EMTPOpCodeResetDevicePropValue;
    TMTPDevicePropertyCode propCode(static_cast<TMTPDevicePropertyCode>(Request().
                                    Uint32(TMTPTypeRequest::ERequestParameter1)));
    iData.iDevPropCode = propCode;
    CMTPPlaybackProperty& property(iPlaybackControlDp.GetPlaybackProperty());
    property.GetDefaultPropertyValueL(iData);

    TInt result = map.GetPlaybackControlCommand(iData, &iPbCmd);

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

void CMTPPbcResetDevicePropValue::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
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
            response = EMTPRespCodeAccessDenied;
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
