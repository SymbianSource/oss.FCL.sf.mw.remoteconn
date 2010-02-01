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
#include <obex/internal/obexinternalheader.h>
#include "obexserverstatemachine.h"

/**
@file
@internalComponent

PUT Operation Receive State
This state receive the object from the client and transfers to Ready once the exchange is finished
A PUT, DISCONNECT or ABORT will be processed
Any other OBEX operation will be answered with an OBEX error code
*/

TObexServerStatePutOpReceiveObject::TObexServerStatePutOpReceiveObject()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "PutOpReceiveObject");
	iName = KName;
#endif
	}

void TObexServerStatePutOpReceiveObject::Entry(CObexServerStateMachine& aContext)
	{
	// Chain on to Put() to start receiving object
	Put(aContext, aContext.LastReceivedPacket());
	}

void TObexServerStatePutOpReceiveObject::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStatePutOpReceiveObject::Put(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	//Initialise 'err' to KErrNone:
	//if ParseNextReceivePacket returns an error, we want 
	//IrOBEXUtil::ObexResponse to return that error unchanged
	//See: IrOBEXUtil::ObexResponse
	TInt err = KErrNone; 

	// Process Put packet into receive object
	if(aContext.TransObject()->ParseNextReceivePacket(aPacket) == CObexBaseObject::EError 
		||(err = aContext.Notification().PutPacketIndication()) != KErrNone)
		{// Error in receive
		aContext.Transport().Send(IrOBEXUtil::ObexResponse(err, aContext.TransObject()->GetLastError()));
		aContext.Notification().ErrorIndication(KErrGeneral);
		aContext.ChangeState(CObexServerStateMachine::EReady);
		}

	//ConnectionID is compulsory if Target header was used at connection
	else if ( aContext.Owner().CheckObjectForConnectionId(*(aContext.TransObject())))
		{
		if(aPacket.IsFinal())
			{// Successfully received
			aContext.ChangeState(CObexServerStateMachine::EPutOpFinal);
			}
		else // not final packet
			{
			aContext.Transport().Send(ERespContinue);
			}
		}
	else
		{
		aContext.Transport().Send(ERespServiceUnavailable); //connectionID was incorrect or not received
		}
	}


void TObexServerStatePutOpReceiveObject::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStatePutOpReceiveObject::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStatePutOpReceiveObject::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to Ready
	RespondAndEndOperation(aContext, ERespConflict);
	}

void TObexServerStatePutOpReceiveObject::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess and return to Ready
	aContext.Notification().AbortIndication();
	RespondAndEndOperation(aContext, ERespSuccess);
	}
	
void TObexServerStatePutOpReceiveObject::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	// Send server app response and return to Ready
	RespondAndEndOperation(aContext, aResponse);
	}


