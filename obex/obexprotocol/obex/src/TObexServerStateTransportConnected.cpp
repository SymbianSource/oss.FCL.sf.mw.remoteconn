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

Transport Connected State
This is the state where there is a transport connect, but no OBEX connection.
A CONNECT will move the machine to ObexConnecting
A DISCONNECT will be processed
An ABORT will cause a Protocol Error
Any other OBEX operation will be answered with an OBEX error code
*/

TObexServerStateTransportConnected::TObexServerStateTransportConnected()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "TransportConnected");
	iName = KName;
#endif
	}

void TObexServerStateTransportConnected::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.ChangeState(CObexServerStateMachine::EObexConnecting);
	}

void TObexServerStateTransportConnected::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStateTransportConnected::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespBadRequest
	aContext.Transport().Send(ERespBadRequest);
	}

void TObexServerStateTransportConnected::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict
	aContext.Transport().Send(ERespConflict);
	}

void TObexServerStateTransportConnected::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict
	aContext.Transport().Send(ERespConflict);
	}

void TObexServerStateTransportConnected::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess
	// Any other response would, according to the spec,
	// require the Obex client to bring down the transport.
	// Our attempt is to be resilient if an 'Abort'
	// is sent erroneously whilst we are in this state.
	aContext.Transport().Send(ERespSuccess);
	}

void TObexServerStateTransportConnected::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	aContext.Transport().Send(aResponse);
	// no need to change state, it's just as if the packet never arrived.
	}

