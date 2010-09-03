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


#include "dpsstatemachine.h"
#include "dpsscriptreceiver.h"
#include "dpsscriptsender.h"
#include "dpstransaction.h"
#include "dpsconst.h"
#include "pictbridge.h"
#include "dpsparam.h"
#include "dpsoperation.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "dpsstatemachineTraces.h"
#endif

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
CDpsStateMachine* CDpsStateMachine::NewL(CDpsEngine* aEngine)
    {
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
    OstTraceFunctionEntry0( CDPSSTATEMACHINE_CDPSSTATEMACHINE_DES_ENTRY );
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
    OstTraceFunctionExit0( CDPSSTATEMACHINE_CDPSSTATEMACHINE_DES_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//	
CDpsStateMachine::CDpsStateMachine(CDpsEngine* aEngine) : 
    iEngine(aEngine),iOperation(EDpsOpEmpty), iEvent(EDpsEvtEmpty), 
    iCurError(KErrNone)
    {
    OstTraceFunctionEntry0( DUP1_CDPSSTATEMACHINE_CDPSSTATEMACHINE_CONS_ENTRY );

    OstTraceFunctionExit0( DUP1_CDPSSTATEMACHINE_CDPSSTATEMACHINE_CONS_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::ConstructL()
    {
    OstTraceFunctionEntry0( CDPSSTATEMACHINE_CONSTRUCTL_ENTRY );
    iIdleState = new(ELeave) TDpsIdleState(this);
    iSendingReqState = new(ELeave) TDpsSendingReqState(this);
    iWaitingRepState = new(ELeave) TDpsWaitingRepState(this);
    iSendingRepState = new(ELeave) TDpsSendingRepState(this);
    iCurState = iIdleState;
    iTrader = CDpsTransaction::NewL(this);
    iScriptReceiver = CDpsScriptReceiver::NewL(this);
    iScriptSender = CDpsScriptSender::NewL(this);	
    OstTraceFunctionExit0( CDPSSTATEMACHINE_CONSTRUCTL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::StartTransactionL(TMDpsOperation* aRequest)
    {
    OstTraceFunctionEntry0( CDPSSTATEMACHINE_STARTTRANSACTIONL_ENTRY );
    iMOperation = aRequest;
    iOperation = (TDpsOperation)iMOperation->iOperation;    
    iTrader->CreateRequestL(aRequest);
    iCurState = iSendingReqState;
    OstTraceFunctionExit0( CDPSSTATEMACHINE_STARTTRANSACTIONL_EXIT );
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
void CDpsStateMachine::Initialize()
    {
    OstTraceFunctionEntry0( CDPSSTATEMACHINE_INITIALIZE_ENTRY );
    iOperation = EDpsOpEmpty;
    iEvent = EDpsEvtEmpty;
    iCurError = KErrNone;
    if (CurState() != IdleState())
        {
        SetState(IdleState());
        } 
    OstTraceFunctionExit0( CDPSSTATEMACHINE_INITIALIZE_EXIT );
    }
