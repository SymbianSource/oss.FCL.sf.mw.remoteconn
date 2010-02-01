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

GET Operation Send State
This state sends the object to the client and transfers to Ready once the exchange is finished
A GET, DISCONNECT or ABORT will be processed
Any other OBEX operation will be answered with an OBEX error code
*/

TObexServerStateGetOpSendObject::TObexServerStateGetOpSendObject()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "GetOpSendObject");
	iName = KName;
#endif
	}

void TObexServerStateGetOpSendObject::Entry(CObexServerStateMachine& aContext)
	{
	// Chain on to Get() to start sending object
	Get(aContext, aContext.LastReceivedPacket());
	}

void TObexServerStateGetOpSendObject::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStateGetOpSendObject::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStateGetOpSendObject::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespBadRequest and return to Ready
	RespondAndEndOperation(aContext, ERespBadRequest);
	}

void TObexServerStateGetOpSendObject::Get(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	if(!aPacket.IsFinal())
		{
		// Raising a Protocol Error will cause a Reset event resulting in a move to Disconnected.
		// So any code after this call will potentially be executed in a different state
		aContext.Owner().Error(KErrCommsOverrun);
		return;
		}

	// This section prepares the next packet to send, if an error is detected or it is the final packet
	// the machine is moved to EReady.
	TInt err = KErrGeneral;
	if(aContext.TransObject()->PrepareNextSendPacket(aContext.Transport().SendPacket()) == CObexBaseObject::EError ||
		(err = aContext.Notification().GetPacketIndication()) != KErrNone)
		{
		aContext.Transport().SendPacket().Init(IrOBEXUtil::ObexResponse(err, ERespInternalError));
		aContext.ChangeState(CObexServerStateMachine::EReady);
		}
	else if(aContext.Transport().SendPacket().IsFinal())
		{// This is the last packet of the object being returned to client
		aContext.ChangeState(CObexServerStateMachine::EGetOpFinal);
		return;	
		}
	// Send the resultant packet
	aContext.Transport().SendPacket().SetFinal();
	aContext.Transport().Send();
	}

void TObexServerStateGetOpSendObject::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStateGetOpSendObject::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess and return to Ready
	aContext.Notification().AbortIndication();
	RespondAndEndOperation(aContext, ERespSuccess);
	}
	
void TObexServerStateGetOpSendObject::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	// Send server app response and return to Ready
	RespondAndEndOperation(aContext, aResponse);
	}

