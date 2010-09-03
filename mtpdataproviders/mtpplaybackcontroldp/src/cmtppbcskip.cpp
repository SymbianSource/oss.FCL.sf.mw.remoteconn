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

#include "cmtppbcskip.h"
#include "mtpplaybackcontroldpconst.h"
#include "cmtpplaybackmap.h"
#include "cmtpplaybackcontroldp.h"
#include "cmtpplaybackproperty.h"
#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppbcskipTraces.h"
#endif


/**
Two-phase constructor.
@param aPlugin  The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object.
*/  
MMTPRequestProcessor* CMTPPbcSkip::NewL(MMTPDataProviderFramework& aFramework,
                                     MMTPConnection& aConnection,
                                     CMTPPlaybackControlDataProvider& aDataProvider)
    {
    OstTraceFunctionEntry0( CMTPPBCSKIP_NEWL_ENTRY );
    CMTPPbcSkip* self = new (ELeave) CMTPPbcSkip(aFramework, aConnection, aDataProvider);
    OstTraceFunctionExit0( CMTPPBCSKIP_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPbcSkip::~CMTPPbcSkip()
    {
    OstTraceFunctionEntry0( CMTPPBCSKIP_CMTPPBCSKIP_ENTRY );
    delete iPbCmd;
    OstTraceFunctionExit0( CMTPPBCSKIP_CMTPPBCSKIP_EXIT );
    }

/**
Constructor.
*/    
CMTPPbcSkip::CMTPPbcSkip(MMTPDataProviderFramework& aFramework,
                   MMTPConnection& aConnection,
                   CMTPPlaybackControlDataProvider& aDataProvider):
                   CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
                   iPlaybackControlDp(aDataProvider)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBCSKIP_CMTPPBCSKIP_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPBCSKIP_CMTPPBCSKIP_EXIT );
    }

/**
CheckRequestL
*/
TMTPResponseCode CMTPPbcSkip::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPPBCSKIP_CHECKREQUESTL_ENTRY );
    TMTPResponseCode respCode = CMTPRequestProcessor::CheckRequestL();
    if(respCode == EMTPRespCodeOK)
        {
        respCode = EMTPRespCodeInvalidParameter;
        TUint32 step = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        if(step != 0)
            {
            respCode = EMTPRespCodeOK;
            }
        }
    OstTraceFunctionExit0( CMTPPBCSKIP_CHECKREQUESTL_EXIT );
    return respCode;
    }

/**
CMTPPbcSkip request handler.
*/   
void CMTPPbcSkip::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPPBCSKIP_SERVICEL_ENTRY );
    CMTPPlaybackMap& map(iPlaybackControlDp.GetPlaybackMap());
    MMTPPlaybackControl& control(iPlaybackControlDp.GetPlaybackControlL());

    TMTPPbCtrlData data;
    data.iOptCode = EMTPOpCodeSkip;
    data.iPropValInt32 = static_cast<TInt32>(Request().Uint32(TMTPTypeRequest::ERequestParameter1));

    TInt result = map.GetPlaybackControlCommand(data, &iPbCmd);
    
    if(KErrNone == result)
        {
        TRAPD(err, control.CommandL(*iPbCmd, this));
        __ASSERT_ALWAYS((err == KErrNone), SendResponseL(EMTPRespCodeInvalidParameter));
        }
    else
        {
        SendResponseL(EMTPRespCodeInvalidParameter);
        }
    OstTraceFunctionExit0( CMTPPBCSKIP_SERVICEL_EXIT );
    }

void CMTPPbcSkip::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPPBCSKIP_HANDLEPLAYBACKCOMMANDCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPPBCSKIP_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aErr %d", aErr );
    
    //Handle error response.
    TMTPResponseCode response;
    switch(aErr)
        {
        case KPlaybackErrNone:
            {
            response = EMTPRespCodeOK;            
            }
            break;
        case KPlaybackErrDeviceBusy:
            {
            response = EMTPRespCodeDeviceBusy;            
            }
            break;
        case KPlaybackErrDeviceUnavailable:
            {
            response = EMTPRespCodeDeviceBusy;
            iPlaybackControlDp.RequestToResetPbCtrl();
            }
            break;
        default:
            {
            response = EMTPRespCodeInvalidParameter;
            }
            break;
        }

    SendResponseL(response);
    
    if(aCmd != NULL)
        {
        __ASSERT_DEBUG((aCmd->PlaybackCommand() == iPbCmd->PlaybackCommand()), Panic(EMTPPBArgumentErr));
        OstTrace1( TRACE_NORMAL, DUP1_CMTPPBCSKIP_HANDLEPLAYBACKCOMMANDCOMPLETEL, "aCmd %d", aCmd->PlaybackCommand() );
        }

    OstTraceFunctionExit0( CMTPPBCSKIP_HANDLEPLAYBACKCOMMANDCOMPLETEL_EXIT );
    }
