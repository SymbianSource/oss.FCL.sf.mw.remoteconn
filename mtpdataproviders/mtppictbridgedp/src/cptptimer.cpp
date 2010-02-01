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
    __FLOG_OPEN(KMTPSubsystem, KPtpServerLog);
    __FLOG(_L8("CPtpTimer::ConstructL"));        
    CTimer::ConstructL();
    }

// --------------------------------------------------------------------------
// 
// 
// --------------------------------------------------------------------------
//
CPtpTimer::~CPtpTimer()
    {
    __FLOG(_L8("CPtpTimer::~"));        
    Cancel();
    __FLOG_CLOSE;
    }
    
// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
//    
void CPtpTimer::RunL()
    {
    __FLOG(_L8(">>>CPtpTimer::RunL"));
    if (iStatus.Int() == KErrNone)
        {
        __FLOG(_L8("--- timer expired, because of:"));

        if (iSession.ServerP()->Printer()->Status() == CMTPPictBridgePrinter::ENotConnected) // must be DPS discovery, since no other service is supported
            {
            __FLOG(_L8("--- Dps printer not available"));            
            iSession.ServerP()->Printer()->NoDpsDiscovery();
            iSession.IsDpsPrinterCompleted(EPrinterNotAvailable);
            }
        else if (iSession.ServerP()->Printer()->SendObjectPending())
            {
            __FLOG(_L8("---SendObject timeout"));
            iSession.ServerP()->Printer()->DpsFileSent(KErrTimedOut);
            }
        else 
            {
            __FLOG(_L8("---something else, do not care"));
            }    
        }
    else if (iStatus.Int() == KErrCancel)
        {
        __FLOG(_L8("--- RunL Cancelled."));
        }
    else 
        {
        __FLOG_VA((_L8("!!!Error: Err %d returned."), iStatus.Int()));
        }
    __FLOG(_L8("<<<CPtpTimer::RunL"));	
    }

// --------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------
#ifdef __FLOG_ACTIVE
TInt CPtpTimer::RunError(TInt aErr)
#else
TInt CPtpTimer::RunError(TInt /*aErr*/)
#endif
    {
    __FLOG_VA((_L8(">>>CPtpTimer::RunError %d"), aErr));
    return KErrNone;
    }
