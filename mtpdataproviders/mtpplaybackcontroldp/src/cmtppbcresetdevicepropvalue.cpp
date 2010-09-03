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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppbcresetdevicepropvalueTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPBCRESETDEVICEPROPVALUE_NEWL_ENTRY );
    CMTPPbcResetDevicePropValue* self = new (ELeave) CMTPPbcResetDevicePropValue(aFramework, aConnection, aDataProvider);
    OstTraceFunctionExit0( CMTPPBCRESETDEVICEPROPVALUE_NEWL_EXIT );
    return self;
    }

/**
Destructor
*/    
CMTPPbcResetDevicePropValue::~CMTPPbcResetDevicePropValue()
    {    
    OstTraceFunctionEntry0( CMTPPBCRESETDEVICEPROPVALUE_CMTPPBCRESETDEVICEPROPVALUE_ENTRY );
    delete iPbCmd;
    OstTraceFunctionExit0( CMTPPBCRESETDEVICEPROPVALUE_CMTPPBCRESETDEVICEPROPVALUE_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPPBCRESETDEVICEPROPVALUE_CMTPPBCRESETDEVICEPROPVALUE_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPBCRESETDEVICEPROPVALUE_CMTPPBCRESETDEVICEPROPVALUE_EXIT );
    }

/**
SetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcResetDevicePropValue::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPPBCRESETDEVICEPROPVALUE_CHECKREQUESTL_ENTRY );
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
    OstTraceFunctionExit0( CMTPPBCRESETDEVICEPROPVALUE_CHECKREQUESTL_EXIT );
    return respCode;
    }
/**
ResetDevicePropValue request handler.
*/ 	
void CMTPPbcResetDevicePropValue::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPPBCRESETDEVICEPROPVALUE_SERVICEL_ENTRY );

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
    OstTraceFunctionExit0( CMTPPBCRESETDEVICEPROPVALUE_SERVICEL_EXIT );
    }

void CMTPPbcResetDevicePropValue::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPPBCRESETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPBCRESETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aErr %d", aErr );

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
        OstTrace1( TRACE_NORMAL, DUP1_CMTPPBCRESETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aCmd %d", aCmd->PlaybackCommand() );
        }

    OstTraceFunctionExit0( CMTPPBCRESETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
    }
