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


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"Skip");)

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
    CMTPPbcSkip* self = new (ELeave) CMTPPbcSkip(aFramework, aConnection, aDataProvider);
    return self;
    }

/**
Destructor.
*/    
CMTPPbcSkip::~CMTPPbcSkip()
    {
    __FLOG(_L8("CMTPPbcSkip - Entry"));
    delete iPbCmd;
    __FLOG(_L8("CMTPPbcSkip - Exit"));
    __FLOG_CLOSE;
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
    //Open the log system
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }

/**
CheckRequestL
*/
TMTPResponseCode CMTPPbcSkip::CheckRequestL()
    {
    __FLOG(_L8("CheckRequestL - Entry"));
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
    __FLOG(_L8("CheckRequestL - Exit"));
    return respCode;
    }

/**
CMTPPbcSkip request handler.
*/   
void CMTPPbcSkip::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
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
    __FLOG(_L8("ServiceL - Exit"));
    }

void CMTPPbcSkip::HandlePlaybackCommandCompleteL(CMTPPlaybackCommand* aCmd, TInt aErr)
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
        __FLOG_1(_L8("aCmd %d"), aCmd->PlaybackCommand());
        }

    __FLOG(_L8("HandlePlaybackCommandCompleteL - Exit"));
    }
