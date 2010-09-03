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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppbcgetdevicepropvalueTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPVALUE_NEWL_ENTRY );
    CMTPPbcGetDevicePropValue* self = new (ELeave) CMTPPbcGetDevicePropValue(aFramework, aConnection, aDataProvider);
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPVALUE_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPbcGetDevicePropValue::~CMTPPbcGetDevicePropValue()
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPVALUE_CMTPPBCGETDEVICEPROPVALUE_ENTRY );
    delete iPbCmd;
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPVALUE_CMTPPBCGETDEVICEPROPVALUE_EXIT );
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
    OstTraceFunctionEntry0( DUP1_CMTPPBCGETDEVICEPROPVALUE_CMTPPBCGETDEVICEPROPVALUE_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPBCGETDEVICEPROPVALUE_CMTPPBCGETDEVICEPROPVALUE_EXIT );
    }

/**
CMTPPbcGetDevicePropValue request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcGetDevicePropValue::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPVALUE_CHECKREQUESTL_ENTRY );
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
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPVALUE_CHECKREQUESTL_EXIT );
    return respCode;
    }

/**
CMTPPbcGetDevicePropValue request handler.
*/   
void CMTPPbcGetDevicePropValue::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPVALUE_SERVICEL_ENTRY );
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
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPVALUE_SERVICEL_EXIT );
    }

void CMTPPbcGetDevicePropValue::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aErr %d", aErr );
    

    //Handle the error
    TBool useDefault = EFalse;
    switch(aErr)
        {
        case KPlaybackErrNone:
            {
            __ASSERT_DEBUG((aCmd != NULL), Panic(EMTPPBDataNullErr));
            __ASSERT_DEBUG((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), Panic(EMTPPBArgumentErr));
            __ASSERT_ALWAYS_OST((aCmd != NULL), OstTrace0( TRACE_ERROR, DUP2_CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "Error argument" ), User::Leave(KErrArgument));
            __ASSERT_ALWAYS_OST((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), OstTrace0( TRACE_ERROR, DUP3_CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "Error argument" ), User::Leave(KErrArgument));
            OstTrace1( TRACE_NORMAL, DUP1_CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aCmd %d", aCmd->PlaybackCommand() );
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
            OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
            return;
        default:
            {
            SendResponseL(EMTPRespCodeDeviceBusy);
            }
            OstTraceFunctionExit0( DUP1_CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
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
    OstTraceFunctionExit0( DUP2_CMTPPBCGETDEVICEPROPVALUE_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
    }

