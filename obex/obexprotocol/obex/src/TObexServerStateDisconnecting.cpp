// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#if ( defined __FLOG_ACTIVE && defined __LOG_FUNCTIONS__ )
_LIT8(KLogComponent, "OBEX");
#endif

/**
@file
@internalComponent

Disconnecting State
In this state, the server has received a (correctly-addressed) disconnect 
request from the client, and placed an asynchronous request on the transport 
to ACK it.
This state is interested in the completion of that send request so it can (a) 
complete the client's notifications, (b) move to 'transport connected' state 
and (c) pull down the transport.
*/

TObexServerStateDisconnecting::TObexServerStateDisconnecting()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "Disconnecting");
	iName = KName;
#endif
	}

void TObexServerStateDisconnecting::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::Abort(CObexServerStateMachine& aContext)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}
	
void TObexServerStateDisconnecting::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	aContext.Owner().Error(KErrIrObexBadEvent);
	}

void TObexServerStateDisconnecting::WriteComplete(CObexServerStateMachine& aContext)
	{
	LOG_FUNC

	aContext.Notification().ObexDisconnectIndication(KNullDesC8);
	// ControlledTransportDown() may raise a TransportDown event immediately, 
	// re-entering the state machine, so we need to be in the correct state 
	// (TransportConnected) before the call.
	// Also need to keep current operation == EOpDisconnect until transport is 
	// happy (which is done in CObex) so ETransportConnected does not set 
	// iCurrentOperation to EOpIdle.
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	aContext.ControlledTransportDown();
	}
