// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// cptpiptransport.cpp
// 
//

/**
 @internalComponent
*/

#include "cptpiptimer.h"
#include "cptpipcontroller.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpiptimerTraces.h"
#endif


CPTPIPTimer::CPTPIPTimer(CPTPIPController& aController) : CTimer(CActive::EPriorityStandard)
	{
	OstTraceFunctionEntry0( CPTPIPTIMER_CPTPIPTIMER_ENTRY );
	iController=&aController;
	OstTraceFunctionExit0( CPTPIPTIMER_CPTPIPTIMER_EXIT );
	}

CPTPIPTimer* CPTPIPTimer::NewLC(CPTPIPController& aController)
	{
	OstTraceFunctionEntry0( CPTPIPTIMER_NEWLC_ENTRY );
	CPTPIPTimer* self=new (ELeave) CPTPIPTimer(aController);
	CleanupStack::PushL(self);
	self->ConstructL();
	OstTraceFunctionExit0( CPTPIPTIMER_NEWLC_EXIT );
	return self;
	}

CPTPIPTimer* CPTPIPTimer::NewL(CPTPIPController& aController)
	{
	OstTraceFunctionEntry0( CPTPIPTIMER_NEWL_ENTRY );
	CPTPIPTimer* self = NewLC(aController);
	CleanupStack::Pop(self);
	OstTraceFunctionExit0( CPTPIPTIMER_NEWL_EXIT );
	return self;
	}

void CPTPIPTimer::ConstructL()
	{
	OstTraceFunctionEntry0( CPTPIPTIMER_CONSTRUCTL_ENTRY );
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
	OstTraceFunctionExit0( CPTPIPTIMER_CONSTRUCTL_EXIT );
	}

void CPTPIPTimer::IssueRequest(TInt aTimerValue)
	{	
	OstTraceFunctionEntry0( CPTPIPTIMER_ISSUEREQUEST_ENTRY );
	CTimer::After(aTimerValue * KTimerMultiplier);
	OstTraceFunctionExit0( CPTPIPTIMER_ISSUEREQUEST_EXIT );
	}

void CPTPIPTimer::RunL()
	{
 	OstTraceFunctionEntry0( CPTPIPTIMER_RUNL_ENTRY );
 	iController->OnTimeOut();
	OstTraceFunctionExit0( CPTPIPTIMER_RUNL_EXIT );
	}
