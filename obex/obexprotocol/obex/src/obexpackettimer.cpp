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

/**
 @file
 @released
 @internalComponent
*/

#include "obexpackettimer.h"
#include "obexclient.h"


/*
 * CObexPacketTimer. Will RunL if the remote obex server has not send any packet for a certain time.
 * Duration is set by CObexClient::SetCommandTimeout
 */

CObexPacketTimer::CObexPacketTimer(CObexClient& aObexClient) 
	: CTimer(CActive::EPriorityStandard), iObexClient(aObexClient)
	{
	CActiveScheduler::Add(this);		
	}

CObexPacketTimer* CObexPacketTimer::NewL(CObexClient& aObexClient)
	{
	CObexPacketTimer* timer = new(ELeave)CObexPacketTimer(aObexClient);
	CleanupStack::PushL(timer);
	timer->ConstructL();
	CleanupStack::Pop(timer);
	return timer;
	}

CObexPacketTimer::~CObexPacketTimer()
	{
	Cancel();
	}
	
void CObexPacketTimer::RunL()
	{
	if(iStatus==KErrNone)
		{
		//Timeout 
 		iObexClient.TimeOutCompletion();
		}
	}

void CObexPacketTimer::SetTimer(TTimeIntervalMicroSeconds32 anInterval)
	{
	//Check if a timeout should be started on the request packet.
	if(anInterval.Int()>KLowestPossibleTimerValue)
		{
		Cancel();

		After(anInterval);
		}
	}

