/*
* Copyright (c) 2006, 2007 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  These classes implement dps states. 
*
*/

#include "dpsstate.h"
#include "dpsstatemachine.h"
#include "dpstransaction.h"
#include "dpsscriptreceiver.h"
#include <pictbridge.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsstateTraces.h"
#endif

  
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsIdleState::TDpsIdleState(CDpsStateMachine* aStateMachine) :
    iStateMachine(aStateMachine) {}
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsIdleState::ScriptSentNotifyL(TBool /*aReply*/)
    {
    OstTraceFunctionEntry0( TDPSIDLESTATE_SCRIPTSENTNOTIFYL_ENTRY );
    OstTraceFunctionExit0( TDPSIDLESTATE_SCRIPTSENTNOTIFYL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsIdleState::ScriptReceivedNotifyL(TBool aReply)
    {
    if (!aReply)
        {
        OstTrace0( TRACE_NORMAL, TDPSIDLESTATE_SCRIPTRECEIVEDNOTIFYL, "request got" );
        iStateMachine->Trader()->ParseScriptL(aReply);    
        iStateMachine->SetState(iStateMachine->SendingRepState());
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_TDPSIDLESTATE_SCRIPTRECEIVEDNOTIFYL, "should not get reply!!!" );
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsIdleState::Error(TInt aErr)
    {
    OstTrace1( TRACE_NORMAL, TDPSIDLESTATE_ERROR, "Error %d", aErr );
    if ((aErr != KErrCancel) && (aErr != KErrNotReady))
        {
        iStateMachine->Trader()->HandleHostRequestError(aErr);
        iStateMachine->SetState(iStateMachine->SendingRepState());    
        }
    
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsSendingReqState::TDpsSendingReqState(CDpsStateMachine* aStateMachine) :
    iStateMachine(aStateMachine) {}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingReqState::ScriptSentNotifyL(TBool aReply)
    {
    if (!aReply)
        {
        OstTrace0( TRACE_NORMAL, TDPSSENDINGREQSTATE_SCRIPTSENTNOTIFYL, " request" );
        iStateMachine->SetState(iStateMachine->WaitingRepState());
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_TDPSSENDINGREQSTATE_SCRIPTSENTNOTIFYL, " should not get reply!!" );
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingReqState::ScriptReceivedNotifyL(TBool /*aReply*/)
    {
    OstTrace0( TRACE_NORMAL, TDPSSENDINGREQSTATE_SCRIPTRECEIVEDNOTIFYL, "**should not reply to the request/reply from host" );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingReqState::Error(TInt aErr)
    {
    User::RequestComplete(iStateMachine->DpsEngine()->OperationRequest(), 
                          aErr);
    iStateMachine->Initialize();                          
    return;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsWaitingRepState::TDpsWaitingRepState(CDpsStateMachine* aStateMachine) :
    iStateMachine(aStateMachine) {}


// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsWaitingRepState::ScriptSentNotifyL(TBool /*aReply*/)
    {    
    OstTraceFunctionEntry0( TDPSWAITINGREPSTATE_SCRIPTSENTNOTIFYL_ENTRY );
    OstTraceFunctionExit0( TDPSWAITINGREPSTATE_SCRIPTSENTNOTIFYL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsWaitingRepState::ScriptReceivedNotifyL(TBool aReply)
    {
    if (aReply)
        {
        OstTrace0( TRACE_NORMAL, TDPSWAITINGREPSTATE_SCRIPTRECEIVEDNOTIFYL, "WaitingRepState reply" );

        iStateMachine->Trader()->ParseScriptL(aReply);      
        User::RequestComplete(
            iStateMachine->DpsEngine()->OperationRequest(), KErrNone);        
        iStateMachine->Initialize();        
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_TDPSWAITINGREPSTATE_SCRIPTRECEIVEDNOTIFYL, "WaitingRepState should not get request" );
        // collision happened, we do nothing because the host will
        // eventually handle this by sending the correct response. but we need
        // to subscribe for the event again.
        iStateMachine->ScriptReceiver()->WaitForReceive();
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsWaitingRepState::Error(TInt aErr)
    {
    // this is not tested
    User::RequestComplete(iStateMachine->DpsEngine()->OperationRequest(), 
                          aErr);
    iStateMachine->Initialize();                          
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsSendingRepState::TDpsSendingRepState(CDpsStateMachine* aStateMachine) :
    iStateMachine(aStateMachine) {}

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingRepState::ScriptSentNotifyL(TBool aReply)
    {
    if (aReply)
        {
        OstTrace0( TRACE_NORMAL, TDPSSENDINGREPSTATE_SCRIPTSENTNOTIFYL, "reply" );
        if (KErrNone == iStateMachine->CurError())
            {
            // this is the normal situation, inform the client
            User::RequestComplete(iStateMachine->DpsEngine()->EventRequest(), 
                              KErrNone);    
            }
        else
            {
            // this is the error situation, do not inform the client. and
            // we need to subscribe for the new event.
            iStateMachine->ScriptReceiver()->WaitForReceive();
            }
        iStateMachine->Initialize();
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_TDPSSENDINGREPSTATE_SCRIPTSENTNOTIFYL, "shoul not get request!!!" );
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingRepState::ScriptReceivedNotifyL(TBool /*aReply*/)
    {
    OstTraceFunctionEntry0( TDPSSENDINGREPSTATE_SCRIPTRECEIVEDNOTIFYL_ENTRY );
    OstTraceFunctionExit0( TDPSSENDINGREPSTATE_SCRIPTRECEIVEDNOTIFYL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingRepState::Error(TInt /*aErr*/)
    {
    return;
    }
