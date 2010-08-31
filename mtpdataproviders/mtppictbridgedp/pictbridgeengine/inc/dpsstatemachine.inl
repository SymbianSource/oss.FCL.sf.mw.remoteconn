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
* Description:  inline functions of CDpsStatemachine
*
*/


#include "dpsstate.h"

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsStateMachine::ScriptSentNotifyL(TBool aReply)
    {
    iCurState->ScriptSentNotifyL(aReply);
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::ScriptReceivedNotifyL(TBool aReply)
    {
    iCurState->ScriptReceivedNotifyL(aReply);
    }
        
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::Error(TInt err)
    {
    iCurError = err;
    iCurState->Error(err);
    }
        
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsStateMachine::SetState(MDpsState* aState)
    {
    iCurState = aState;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
MDpsState* CDpsStateMachine::IdleState() const
    {
    return iIdleState;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
MDpsState* CDpsStateMachine::SendingReqState() const
    {
    return iSendingReqState;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
MDpsState* CDpsStateMachine::WaitingRepState() const
    {
    return iWaitingRepState; 
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
MDpsState* CDpsStateMachine::SendingRepState() const
    {
    return iSendingRepState;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsScriptReceiver* CDpsStateMachine::ScriptReceiver() const
    {
    return iScriptReceiver;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//                
CDpsScriptSender* CDpsStateMachine::ScriptSender() const
    {
    return iScriptSender;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsTransaction* CDpsStateMachine::Trader() const
    {
    return iTrader;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsOperation CDpsStateMachine::Operation() const
    {
    return iOperation;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::SetOperation(TDpsOperation aOp)
    {
    iOperation = aOp;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TMDpsOperation* CDpsStateMachine::MOperation() const
    {
    return iMOperation;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
TDpsEvent CDpsStateMachine::Event() const
    {
    return iEvent;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
void CDpsStateMachine::SetEvent(TDpsEvent aEvent)
    {
    iEvent = aEvent;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//    
MDpsState* CDpsStateMachine::CurState() const
    {
    return iCurState;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//        
CDpsEngine* CDpsStateMachine::DpsEngine() const
    {
    return iEngine;
    }
    
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//            
TInt CDpsStateMachine::CurError() const
    {
    return iCurError;
    }