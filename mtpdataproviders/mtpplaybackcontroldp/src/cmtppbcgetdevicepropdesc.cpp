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

#include <mtp/cmtptypedevicepropdesc.h>

#include "cmtppbcgetdevicepropdesc.h"
#include "mtpplaybackcontroldpconst.h"
#include "cmtpplaybackcontroldp.h"
#include "cmtpplaybackproperty.h"
#include "cmtpplaybackmap.h"
#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppbcgetdevicepropdescTraces.h"
#endif


/**
Two-phase constructor.
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/  
MMTPRequestProcessor* CMTPPbcGetDevicePropDesc::NewL(MMTPDataProviderFramework& aFramework,
                                                MMTPConnection& aConnection, 
                                                CMTPPlaybackControlDataProvider& aDataProvider)
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPDESC_NEWL_ENTRY );
    CMTPPbcGetDevicePropDesc* self = new (ELeave) CMTPPbcGetDevicePropDesc(aFramework, aConnection, aDataProvider);
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPDESC_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPbcGetDevicePropDesc::~CMTPPbcGetDevicePropDesc()
    {    
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPDESC_CMTPPBCGETDEVICEPROPDESC_ENTRY );
    delete iPropDesc;
    delete iPbCmd;
    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPDESC_CMTPPBCGETDEVICEPROPDESC_EXIT );
    }

/**
Constructor.
*/    
CMTPPbcGetDevicePropDesc::CMTPPbcGetDevicePropDesc(MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection, 
                                            CMTPPlaybackControlDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iPlaybackControlDp(aDataProvider)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBCGETDEVICEPROPDESC_CMTPPBCGETDEVICEPROPDESC_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPBCGETDEVICEPROPDESC_CMTPPBCGETDEVICEPROPDESC_EXIT );
    }
    
/**
GetDevicePropDesc request validator.
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPPbcGetDevicePropDesc::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPDESC_CHECKREQUESTL_ENTRY );
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

    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPDESC_CHECKREQUESTL_EXIT );
    return respCode;
    }

/**
GetDevicePropDesc request handler.
*/    
void CMTPPbcGetDevicePropDesc::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPDESC_SERVICEL_ENTRY );
    //Destroy the previous playback command.
    delete iPbCmd;
    iPbCmd = NULL;
    
    //Get the device property code
    TMTPDevicePropertyCode propCode(static_cast<TMTPDevicePropertyCode>(Request().
                                    Uint32(TMTPTypeRequest::ERequestParameter1)));
    
    TMTPPbCtrlData data;
    data.iOptCode = EMTPOpCodeGetDevicePropDesc;
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

    OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPDESC_SERVICEL_EXIT );
    }

void CMTPPbcGetDevicePropDesc::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aErr %d", aErr );
    

    TBool useDefault = EFalse;
    switch(aErr)
        {
        case KPlaybackErrNone:
            {
            __ASSERT_DEBUG((aCmd != NULL), Panic(EMTPPBDataNullErr));
            __ASSERT_DEBUG((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), Panic(EMTPPBArgumentErr));
            __ASSERT_ALWAYS_OST((aCmd != NULL), OstTrace0( TRACE_ERROR, DUP2_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL, "Error argument" ), User::Leave(KErrArgument));
            __ASSERT_ALWAYS_OST((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), OstTrace0( TRACE_ERROR, DUP3_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL, "Error argument" ), User::Leave(KErrArgument));
            OstTrace1( TRACE_NORMAL, DUP1_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aCmd %d", aCmd->PlaybackCommand());
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
            OstTraceFunctionExit0( CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
            return;

        default:
            {
            SendResponseL(EMTPRespCodeDeviceBusy);
            }
            OstTraceFunctionExit0( DUP1_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
            return;
        }

    delete iPropDesc;
    iPropDesc = NULL;
    
    CMTPPlaybackProperty& property(iPlaybackControlDp.GetPlaybackProperty());
    TMTPDevicePropertyCode propCode = static_cast<TMTPDevicePropertyCode>
                                      (Request().Uint32(TMTPTypeRequest::ERequestParameter1));

    switch (propCode)
        {
        case EMTPDevicePropCodeVolume:
            {
            TMTPPbDataVolume volSet(1,0,1,1,1);
            if(useDefault)
                {
                 property.GetDefaultVolSet(volSet);
                }
            else
                {
                volSet = aCmd->ParamL().VolumeSetL();
                property.SetDefaultVolSetL(volSet);
                }

            CMTPTypeDevicePropDescRangeForm* form = CMTPTypeDevicePropDescRangeForm::NewLC(EMTPTypeUINT32);
            form->SetUint32L(CMTPTypeDevicePropDescRangeForm::EMaximumValue, volSet.MaxVolume());
            form->SetUint32L(CMTPTypeDevicePropDescRangeForm::EMinimumValue, volSet.MinVolume());
            form->SetUint32L(CMTPTypeDevicePropDescRangeForm::EStepSize, volSet.Step());
            iPropDesc = CMTPTypeDevicePropDesc::NewL(propCode,
                                                     CMTPTypeDevicePropDesc::EReadWrite, 
                                                     CMTPTypeDevicePropDesc::ERangeForm,
                                                     form);

            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, volSet.DefaultVolume());
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, volSet.CurrentVolume());
            CleanupStack::PopAndDestroy(form);
            SendDataL(*iPropDesc); 
            }
            break;

        case EMTPDevicePropCodePlaybackRate:
            {
            CMTPTypeDevicePropDescEnumerationForm* form = CMTPTypeDevicePropDescEnumerationForm::
                                                          NewLC(EMTPTypeINT32);
            CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());

            TInt32 val = map.PlaybackRateL(EPlayStateBackwardSeeking);
            TMTPTypeInt32 value(val);
            form->AppendSupportedValueL(value);
            
            val = map.PlaybackRateL(EPlayStatePaused);
            value.Set(val);
            form->AppendSupportedValueL(value);
            
            val = map.PlaybackRateL(EPlayStatePlaying);
            value.Set(val);
            form->AppendSupportedValueL(value);
            
            val = map.PlaybackRateL(EPlayStateForwardSeeking);
            value.Set(val);
            form->AppendSupportedValueL(value);

            iPropDesc = CMTPTypeDevicePropDesc::NewL(propCode,
                                                     CMTPTypeDevicePropDesc::EReadWrite, 
                                                     CMTPTypeDevicePropDesc::EEnumerationForm,
                                                     form);

            property.GetDefaultPropertyValueL(propCode, val);
            iPropDesc->SetInt32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, val);

            if(!useDefault)
                {
                TMTPPlaybackState state = static_cast<TMTPPlaybackState>(aCmd->ParamL().Uint32L());
                val = map.PlaybackRateL(state);
                }
            iPropDesc->SetInt32L(CMTPTypeDevicePropDesc::ECurrentValue, val);
            CleanupStack::PopAndDestroy(form);

            SendDataL(*iPropDesc); 
            }
            break;
        
        case EMTPDevicePropCodePlaybackObject:
            {
            iPropDesc = CMTPTypeDevicePropDesc::NewL(propCode);
            TUint32 val = 0;
            property.GetDefaultPropertyValueL(propCode, val);
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, val);
            CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
            if(!useDefault)
                {
                val = map.ObjectHandleL(aCmd->ParamL().SuidSetL().Suid());                
                }
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, val);
            SendDataL(*iPropDesc); 
            }
            break;

        case EMTPDevicePropCodePlaybackContainerIndex:
            {
            iPropDesc = CMTPTypeDevicePropDesc::NewL(propCode);
            TUint32 val = 0;
            property.GetDefaultPropertyValueL(propCode, val);
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, val);
            if(!useDefault)
                {
                val = aCmd->ParamL().Uint32L();                
                }
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, val);
            SendDataL(*iPropDesc); 
            }
            break;

        case EMTPDevicePropCodePlaybackPosition:
            {
            iPropDesc = CMTPTypeDevicePropDesc::NewL(propCode);
            TUint32 val = 0;
            CMTPPlaybackProperty& property(iPlaybackControlDp.GetPlaybackProperty());
            property.GetDefaultPropertyValueL(propCode, val);
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, val);
            if(!useDefault)
                {
                val = aCmd->ParamL().Uint32L();                
                }
            iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, val);
            SendDataL(*iPropDesc);
            }
            break;
        
        default:
            {
            LEAVEIFERROR( KErrArgument, 
                                    OstTrace0( TRACE_ERROR, DUP4_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL, "Error argument" ));
            } 
            break;
        }

    OstTraceFunctionExit0( DUP2_CMTPPBCGETDEVICEPROPDESC_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
    }
