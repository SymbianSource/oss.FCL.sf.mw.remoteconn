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

#include <obex.h>
#include "obexserverstatemachine.h"

/**
@file
@internalComponent

OBEX Ready State
This is the default state when there is an OBEX connection but no current operation.
A CONNECT will be answered with an OBEX error code
A GET will move the machine to GetOpReceiveSpecification
A PUT will move the machine to PutOpWaitForUser
A SETPATH will move the machine to SetPathOp
A DISCONNECT or ABORT will be processed
*/

TObexServerStateReady::TObexServerStateReady()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "Ready");
	iName = KName;
#endif
	}

void TObexServerStateReady::Entry(CObexServerStateMachine& aContext)
	{
	aContext.Owner().SetCurrentOperation(CObex::EOpIdle);
	}

void TObexServerStateReady::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict
	aContext.Transport().Send(ERespConflict);
	}

void TObexServerStateReady::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStateReady::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.ChangeState(CObexServerStateMachine::EPutOpWaitForUser);
	}

void TObexServerStateReady::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.ChangeState(CObexServerStateMachine::EGetOpReceiveSpecification);
	}

void TObexServerStateReady::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.ChangeState(CObexServerStateMachine::ESetPathOp);
	}

void TObexServerStateReady::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess
	// Report that we're re-synced
	aContext.Transport().Send(ERespSuccess);
	}
	
void TObexServerStateReady::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	// Send response and remain in Ready state
	aContext.Transport().Send(aResponse);
	}


void TObexServerStateReady::ReadActivityDetected(CObexServerStateMachine& aContext)
	/*
	Indicates that a new obex packet is being read. 
	This should only need to be passed up to the user 
	if we are in 'Ready' state.
	*/
	{
	aContext.Owner().SignalReadActivity();
	}

