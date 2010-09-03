// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include "cptptimer.h"
#include "cmtppictbridgeprinter.h"
#include "cptpsession.h"
#include "cptpserver.h"
#include "ptpdef.h"
#include "mtppictbridgedpconst.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptptimerTraces.h"
#endif


// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
CPtpTimer* CPtpTimer::NewL(CPtpSession& aSession)
    {
    CPtpTimer* self = new(ELeave) CPtpTimer(aSession);
    CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
    return self;
    }

// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//    
CPtpTimer::CPtpTimer(CPtpSession& aSession) : CTimer(EPriorityStandard),
    iSession(aSession)
    {
    CActiveScheduler::Add(this);    
    }
    
// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
void CPtpTimer::ConstructL()    
    {
    OstTraceFunctionEntry0( CPTPTIMER_CONSTRUCTL_ENTRY );   
    CTimer::ConstructL();
    OstTraceFunctionExit0( CPTPTIMER_CONSTRUCTL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
CPtpTimer::~CPtpTimer()
    {
    OstTraceFunctionEntry0( CPTPTIMER_CPTPTIMER_DES_ENTRY );
    Cancel();
    OstTraceFunctionExit0( CPTPTIMER_CPTPTIMER_DES_EXIT );
    }
    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
void CPtpTimer::RunL()
    {
    OstTraceFunctionEntry0( CPTPTIMER_RUNL_ENTRY );
    if (iStatus.Int() == KErrNone)
        {
        OstTrace0( TRACE_NORMAL, CPTPTIMER_RUNL, "--- timer expired, because of:" );

        if (iSession.ServerP()->Printer()->Status() == CMTPPictBridgePrinter::ENotConnected) // must be DPS discovery, since no other service is supported
            {
            OstTrace0( TRACE_NORMAL, DUP1_CPTPTIMER_RUNL, "--- Dps printer not available" );
            iSession.ServerP()->Printer()->NoDpsDiscovery();
            iSession.IsDpsPrinterCompleted(EPrinterNotAvailable);
            }
        else if (iSession.ServerP()->Printer()->SendObjectPending())
            {
            OstTrace0( TRACE_NORMAL, DUP2_CPTPTIMER_RUNL, "---SendObject timeout" );
            iSession.ServerP()->Printer()->DpsFileSent(KErrTimedOut);
            }
        else 
            {
            OstTrace0( TRACE_NORMAL, DUP3_CPTPTIMER_RUNL, "---something else, do not care" );
            }    
        }
    else if (iStatus.Int() == KErrCancel)
        {
        OstTrace0( TRACE_NORMAL, DUP4_CPTPTIMER_RUNL, "--- RunL Cancelled." );
        }
    else 
        {
        OstTrace1( TRACE_WARNING, DUP5_CPTPTIMER_RUNL, "!!!Error: Err %d returned.", iStatus.Int() );
        }
    OstTraceFunctionExit0( CPTPTIMER_RUNL_EXIT );
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
#ifdef OST_TRACE_COMPILER_IN_USE
TInt CPtpTimer::RunError(TInt aErr)
#else
TInt CPtpTimer::RunError(TInt /*aErr*/)
#endif
    {
    OstTraceFunctionEntry0( CPTPTIMER_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CPTPTIMER_RUNERROR, 
            "error code %d", aErr);
    OstTraceFunctionExit0( CPTPTIMER_RUNERROR_EXIT );
    return KErrNone;
    }
