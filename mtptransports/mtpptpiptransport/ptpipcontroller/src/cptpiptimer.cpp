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

CPTPIPTimer::CPTPIPTimer(CPTPIPController& aController) : CTimer(CActive::EPriorityStandard)
	{
	iController=&aController;
	}

CPTPIPTimer* CPTPIPTimer::NewLC(CPTPIPController& aController)
	{
	CPTPIPTimer* self=new (ELeave) CPTPIPTimer(aController);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CPTPIPTimer* CPTPIPTimer::NewL(CPTPIPController& aController)
	{
	CPTPIPTimer* self = NewLC(aController);
	CleanupStack::Pop(self);
	return self;
	}

void CPTPIPTimer::ConstructL()
	{
	CTimer::ConstructL();
	CActiveScheduler::Add(this);
	}

void CPTPIPTimer::IssueRequest(TInt aTimerValue)
	{	
	CTimer::After(aTimerValue * KTimerMultiplier);
	}

void CPTPIPTimer::RunL()
	{
 	iController->OnTimeOut();
	}
