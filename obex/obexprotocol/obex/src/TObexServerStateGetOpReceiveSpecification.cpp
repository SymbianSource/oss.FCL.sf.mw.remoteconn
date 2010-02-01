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

GET Operation Receive Specification State
This state receives the specification object from the client and transfers to GetOpWaitForUser once the object is received
A GET, DISCONNECT or ABORT will be processed
Any other OBEX operation will be answered with an OBEX error code
*/

TObexServerStateGetOpReceiveSpecification::TObexServerStateGetOpReceiveSpecification()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "GetOpReceiveSpecification");
	iName = KName;
#endif
	}

void TObexServerStateGetOpReceiveSpecification::Entry(CObexServerStateMachine& aContext)
	{
	// Reset specification object
	if(aContext.SpecObject()->InitReceive() != KErrNone)
		{
		aContext.Transport().Send(ERespInternalError);
		return;
		}
	aContext.Owner().SetCurrentOperation(CObex::EOpGet);
	// Chain on to Get to start receiving specification
	Get(aContext, aContext.LastReceivedPacket());
	}

void TObexServerStateGetOpReceiveSpecification::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStateGetOpReceiveSpecification::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStateGetOpReceiveSpecification::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespBadRequest and return to Ready
	RespondAndEndOperation(aContext, ERespBadRequest);
	}

void TObexServerStateGetOpReceiveSpecification::Get(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process packet and see if specification is complete
	// Packet contains more headers describing the object to be 'got'
	if(aContext.SpecObject()->ParseNextReceivePacket(aPacket) == CObexBaseObject::EError)
		{
		aContext.Transport().Send(aContext.SpecObject()->GetLastError());
		aContext.ChangeState(CObexServerStateMachine::EReady);
		return;
		}

	if(aPacket.IsFinal())
		{// Time to turn around and start sending our reply
		aContext.ChangeState(CObexServerStateMachine::EGetOpWaitForUser);
		}
	else
		{
		aContext.Transport().Send(ERespContinue);
		}
	}

void TObexServerStateGetOpReceiveSpecification::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStateGetOpReceiveSpecification::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess and return to Ready
	aContext.Notification().AbortIndication();
	RespondAndEndOperation(aContext, ERespSuccess);
	}
	
void TObexServerStateGetOpReceiveSpecification::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	// Send server app response and return to Ready
	RespondAndEndOperation(aContext, aResponse);
	}

