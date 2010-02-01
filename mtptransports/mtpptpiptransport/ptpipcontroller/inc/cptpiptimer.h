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
// cptpiptransport.h
// 
//

/**
 @internalComponent
*/


#ifndef CPTPIPTIMER_H_
#define CPTPIPTIMER_H_

#include <e32base.h>
#include <e32def.h>

const TUint KInitialTimeoutInSeconds = 30;
const TInt KTimerMultiplier = 1000000;


/**
The CPTPIPTimer class sets up a timeout time for operations.
*/

class CPTPIPController;

class CPTPIPTimer: public CTimer
	{
public:	
	static CPTPIPTimer* NewLC(CPTPIPController& aController);
	static CPTPIPTimer* NewL(CPTPIPController& aController);	
	void IssueRequest(TInt aTimerValue);
	void RunL();

private:	
	CPTPIPTimer(CPTPIPController& aController);
	void ConstructL();	
	
private:
	CPTPIPController* iController;	
	};
	
#endif /*CPTPIPTIMER_H_*/
