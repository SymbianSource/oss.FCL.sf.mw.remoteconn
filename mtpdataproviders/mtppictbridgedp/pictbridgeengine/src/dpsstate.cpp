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

#include <e32debug.h>
#include "dpsstate.h"
#include "dpsstatemachine.h"
#include "dpstransaction.h"
#include "dpsscriptreceiver.h"
#include <pictbridge.h>
 
#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
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
    IF_DEBUG(Print(_L("TDpsIdleState::ScriptSentNotify")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsIdleState::ScriptReceivedNotifyL(TBool aReply)
    {
    if (!aReply)
        {
        IF_DEBUG(Print(_L("TDpsIdleState::ScriptReceived request got")));            
        iStateMachine->Trader()->ParseScriptL(aReply);    
        iStateMachine->SetState(iStateMachine->SendingRepState());
        }
    else
        {
        IF_DEBUG(Print(_L(
            "TDpsIdleState::ScriptReceived should not get reply!!!")));    
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsIdleState::Error(TInt aErr)
    {
    IF_DEBUG(Print(_L("TDpsIdleState::Error %d"), aErr));
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
        IF_DEBUG(Print(_L("TDpsSendingReqState::ScriptSent request")));
        iStateMachine->SetState(iStateMachine->WaitingRepState());
        }
    else
        {
        IF_DEBUG(Print(_L(
            "TDpsSendingReqState::ScriptSent should not get reply!!")));
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingReqState::ScriptReceivedNotifyL(TBool /*aReply*/)
    {
    IF_DEBUG(Print(_L("TDpsSendingReqState::ScriptReceived")));
    IF_DEBUG(Print(_L("**should not reply to the request/reply from host")));
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
    IF_DEBUG(Print(_L("TDpsWaitingRepState::ScriptSent")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsWaitingRepState::ScriptReceivedNotifyL(TBool aReply)
    {
    if (aReply)
        {
        IF_DEBUG(Print(_L("WaitingRepState reply")))
        
        iStateMachine->Trader()->ParseScriptL(aReply);      
        User::RequestComplete(
            iStateMachine->DpsEngine()->OperationRequest(), KErrNone);        
        iStateMachine->Initialize();        
        }
    else
        {
        IF_DEBUG(Print(_L("WaitingRepState should not get request")));
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
        IF_DEBUG(Print(_L("TDpsSendingRepState::ScriptSent reply")));
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
        IF_DEBUG(Print(_L(
            "TDpsSendingRepState::ScriptSent shoul not get request!!!")));
        }
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingRepState::ScriptReceivedNotifyL(TBool /*aReply*/)
    {
    IF_DEBUG(Print(_L("TDpsSendingRepState::ScriptReceived")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void TDpsSendingRepState::Error(TInt /*aErr*/)
    {
    return;
    }
