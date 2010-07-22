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
* Description:  This class implements the dps state machine. 
*
*/


#include <e32debug.h>
#include "dpsstatemachine.h"
#include "dpsscriptreceiver.h"
#include "dpsscriptsender.h"
#include "dpstransaction.h"
#include "dpsconst.h"
#include "pictbridge.h"
#include "dpsparam.h"
#include "dpsoperation.h"

#ifdef _DEBUG
#	define IF_DEBUG(t) {RDebug::t;}
#else
#	define IF_DEBUG(t)
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsStateMachine* CDpsStateMachine::NewL(CDpsEngine* aEngine)
    {
    IF_DEBUG(Print(_L("CDpsStateMachine::NewL")));
    CDpsStateMachine* self = new(ELeave) CDpsStateMachine(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;	
    }
	
// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsStateMachine::~CDpsStateMachine()
    {
    IF_DEBUG(Print(_L(">>>~CDpsStateMachine")));
    delete iTrader;
    iTrader = NULL;
    delete iScriptReceiver;
    iScriptReceiver = NULL;
    delete iScriptSender;
    iScriptSender = NULL;
    
    delete iIdleState; iIdleState = NULL;
    delete iSendingReqState; iSendingReqState = NULL;
    delete iWaitingRepState; iWaitingRepState = NULL;
    delete iSendingRepState; iSendingRepState = NULL;	
    IF_DEBUG(Print(_L("<<<~CDpsStateMachine")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsStateMachine::CDpsStateMachine(CDpsEngine* aEngine) : 
    iEngine(aEngine),iOperation(EDpsOpEmpty), iEvent(EDpsEvtEmpty), 
    iCurError(KErrNone)
    {
    IF_DEBUG(Print(_L(">>>CDpsStateMachine::Ctor")));
    
    IF_DEBUG(Print(_L("<<<CDpsStateMachine::Ctor")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::ConstructL()
    {
    IF_DEBUG(Print(_L(">>>CDpsStateMachine::ConstructL")));	
    iIdleState = new(ELeave) TDpsIdleState(this);
    iSendingReqState = new(ELeave) TDpsSendingReqState(this);
    iWaitingRepState = new(ELeave) TDpsWaitingRepState(this);
    iSendingRepState = new(ELeave) TDpsSendingRepState(this);
    iCurState = iIdleState;
    iTrader = CDpsTransaction::NewL(this);
    iScriptReceiver = CDpsScriptReceiver::NewL(this);
    iScriptSender = CDpsScriptSender::NewL(this);	
    IF_DEBUG(Print(_L("<<<CDpsOperator::ConstructL")));	
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::StartTransactionL(TMDpsOperation* aRequest)
    {
    IF_DEBUG(Print(_L(">>>CDpsStateMachine::StartTransaction")));
    iMOperation = aRequest;
    iOperation = (TDpsOperation)iMOperation->iOperation;    
    iTrader->CreateRequestL(aRequest);
    iCurState = iSendingReqState;
    IF_DEBUG(Print(_L("<<<CDpsStateMachine::StartTransaction")));
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::Initialize()
    {
    IF_DEBUG(Print(_L(">>>CDpsStateMachine::Initialize")));
    iOperation = EDpsOpEmpty;
    iEvent = EDpsEvtEmpty;
    iCurError = KErrNone;
    if (CurState() != IdleState())
        {
        SetState(IdleState());
        }
    IF_DEBUG(Print(_L("<<<CDpsStateMachine::Initialize")));    
    }
