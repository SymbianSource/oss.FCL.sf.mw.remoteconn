// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef OBEXPACKETTIMER_H
#define OBEXPACKETTIMER_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>

const TInt KLowestPossibleTimerValue = 0;

/** This class handles the timing of Obex reponse packets 
    from the server
*/
class CObexClient;
	
NONSHARABLE_CLASS(CObexPacketTimer) : public CTimer
	{
public:
	static CObexPacketTimer* NewL(CObexClient& aObexClient);
	virtual ~CObexPacketTimer();
	
	void SetTimer(TTimeIntervalMicroSeconds32 anInterval);
	
private:
	CObexPacketTimer(CObexClient& aObexClient);
	virtual void RunL();
	
private:
	CObexClient& iObexClient;
	};
	
#endif // OBEXPACKETTIMER_H
