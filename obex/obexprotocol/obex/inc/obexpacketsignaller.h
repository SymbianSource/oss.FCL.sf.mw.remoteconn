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

#ifndef OBEXPACKETSIGNALLER_H
#define OBEXPACKETSIGNALLER_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>
#include <obexfinalpacketobserver.h>
#include <obexreadactivityobserver.h>
#include <obex/transport/mobextransportnotify.h>

const TUint KObexPacketSignallerInterestingEvents = EObexFinalPacketStarted | EObexFinalPacketFinished | EObexReadActivityDetected;
const TUint KObexPacketSignallerInterestingClientEvents = EObexFinalPacketStarted | EObexFinalPacketFinished;

/** This class handles packet process events.  

Currently the only events handled are those for final packet 
notification and read activity.
*/
NONSHARABLE_CLASS(CObexPacketSignaller) : public CActive
	{
public:
	static CObexPacketSignaller* NewL();
	virtual ~CObexPacketSignaller();

	void SetFinalPacketObserver(MObexFinalPacketObserver* aObserver);
	void SetReadActivityObserver(MObexReadActivityObserver* aObserver);
	void Signal(TObexPacketProcessEvent aEvent);
	
private:
	CObexPacketSignaller();
	
	void RunL();
	void DoCancel();
	
	void NotifyObserver(TObexPacketProcessEvent aEvent);

private:
	MObexFinalPacketObserver* iFinalPacketObserver;
	MObexReadActivityObserver* iReadActivityObserver;
	TObexPacketProcessEvents iOutstandingEvents;	
	};
	
#endif // OBEXPACKETSIGNALLER_H
