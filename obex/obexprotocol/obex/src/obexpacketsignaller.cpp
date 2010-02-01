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

#include <obexfinalpacketobserver.h>
#include "logger.h"
#include "obexpacketsignaller.h"
#include "OBEXUTIL.H"
#include "obexfaults.h"
#include "obexclient.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/** Constructor.
This Active Object has a priority of EPriorityHigh + 1. This is to ensure 
that, when a final packet is sent, the RunLs are executed in the following order:
1/ signaller (EObexFinalPacketStarted)
2/ activeRW (the final packet actually being sent)
3/ signaller (EObexFinalPacketFinished)
The priority chosen is EPriorityHigh + 1 because the priorities of the 
transport controllers' writer (CActiveRW) AOs tends to be EPriorityHigh. 
As long as this is the case, our guarantee holds. Our documentation says that 
new licensee transport controllers must have writers with priorities that 
continue to support this guarantee.
*/
CObexPacketSignaller::CObexPacketSignaller() : CActive(CActive::EPriorityHigh + 1)
	{
	}
	
/** Factory function.

@return A fully constructed CObexPacketSignaller.
*/
CObexPacketSignaller* CObexPacketSignaller::NewL()
	{
	CObexPacketSignaller* signaller = new(ELeave)CObexPacketSignaller();
	return signaller;
	}

/** Destructor.
*/	
CObexPacketSignaller::~CObexPacketSignaller()
	{
	Cancel();
	}
	
/** Sets observer to be notified of packet process events.

This clears any events that are currently outstanding.

@param aObserver The observer to receive events.  May be set to NULL.
*/
void CObexPacketSignaller::SetFinalPacketObserver(MObexFinalPacketObserver* aObserver)
	{
	LOG1(_L8("CObexPacketSignaller::SetObserver(0x%x)"), aObserver);
	
	// Clear outstanding events for the final packet observer
	// If there's an outstanding final packet event this means 
	// it won't get signalled.
	iOutstandingEvents &= ~(EObexFinalPacketStarted | EObexFinalPacketFinished);
	
	// Add or remove from scheduler if necessary
	if(!iFinalPacketObserver &&!iReadActivityObserver && aObserver)
		{
		CActiveScheduler::Add(this);
		}
	else if(iFinalPacketObserver && !iReadActivityObserver && !aObserver)
		{
		Deque();
		}
	
	iFinalPacketObserver = aObserver;
	}
	
void CObexPacketSignaller::SetReadActivityObserver(MObexReadActivityObserver* aObserver)
	{
	LOG1(_L8("CObexPacketSignaller::SetObserver(0x%x)"), aObserver);
	
	// Clear outstanding events for the read activity observer
	// If there's an outstanding read activity event this means 
	// it won't get signalled.
	iOutstandingEvents &= ~EObexReadActivityDetected;
	
	//UpdateScheduler();
	// Add or remove from scheduler if necessary
	if(!iFinalPacketObserver &&!iReadActivityObserver && aObserver)
		{
		CActiveScheduler::Add(this);
		}
	else if(iReadActivityObserver && !iFinalPacketObserver && !aObserver)
		{
		Deque();
		}
	
	iReadActivityObserver = aObserver;
	}

	
/** Signal that an event has occurred.

@param aEvent The event that has occurred.
*/	
void CObexPacketSignaller::Signal(TObexPacketProcessEvent aEvent)
	{
	LOG1(_L8("CObexPacketSignaller::Signal(%d)"), aEvent);
	__ASSERT_DEBUG((aEvent != 0), IrOBEXUtil::Panic(EBadPacketProcessEvent));

	TBool appropriateObserver = EFalse;
	switch(aEvent)
		{
		case EObexFinalPacketStarted:
		case EObexFinalPacketFinished:
			{
			if(iFinalPacketObserver)
				{
				appropriateObserver = ETrue;
				}
			}
			break;
		case EObexReadActivityDetected:
			{
			if(iReadActivityObserver)
				{
				appropriateObserver = ETrue;
				}
			}
			break;
		default:
		    break; //drop - not for us
		}

	if(appropriateObserver)
		{
		iOutstandingEvents |= aEvent;
		
		if(!IsActive())
			{
			SetActive();
			TRequestStatus* status = &iStatus;
			User::RequestComplete(status, KErrNone);
			}
		}
	}
	
/** Active object completion function.
*/
void CObexPacketSignaller::RunL()
	{
	LOG1(_L8("CObexPacketSignaller::RunL() with iOutstandingEvents %d"), iOutstandingEvents);

	__ASSERT_DEBUG(iFinalPacketObserver || !(iOutstandingEvents  & (EObexFinalPacketStarted | EObexFinalPacketFinished)), 
				  IrOBEXUtil::Panic(ENoPacketProcessObserverSet));
	__ASSERT_DEBUG(iReadActivityObserver || !(iOutstandingEvents & EObexReadActivityDetected), 
				  IrOBEXUtil::Panic(ENoPacketProcessObserverSet));
	// Signal each event in order
	// Should be safe against an observer becoming
	// unset as if it is unset then associated outstanding events are
	// cleared.
	if(iOutstandingEvents & EObexFinalPacketStarted)
		{
		iOutstandingEvents &= ~EObexFinalPacketStarted;
		if(iFinalPacketObserver!=NULL)
			{
			iFinalPacketObserver->MofpoFinalPacketStarted();
			}
		}
		
	if(iOutstandingEvents & EObexFinalPacketFinished)
		{
		iOutstandingEvents &= ~EObexFinalPacketFinished;
		if(iFinalPacketObserver!=NULL)
			{
			iFinalPacketObserver->MofpoFinalPacketFinished();
			}
		}
	
	if(iOutstandingEvents & EObexReadActivityDetected)
		{
		iOutstandingEvents &= ~EObexReadActivityDetected;
		if(iReadActivityObserver!=NULL)
			{
			iReadActivityObserver->MoraoReadActivity();
			}
		}
	
	// If we've been signalled as a result of client's notification
	// handling events may be outstanding...
	if(iOutstandingEvents)
		{
		SetActive();
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);	
		}
	}
	
/** From CActive.
*/	
void CObexPacketSignaller::DoCancel()
	{
	iOutstandingEvents = 0;
	}

