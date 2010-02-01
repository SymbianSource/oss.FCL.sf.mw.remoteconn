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
*/

/**
Called by the state machine when it has moved into the state
Default action - do nothing and return no error
@param aContext Reference to state machine object
*/
void TObexServerOperationState::Entry(CObexServerStateMachine& /*aContext*/)
	{
	}

/**
Connection packet to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
@param aPacket Connect packet
*/
void TObexServerOperationState::Connect(CObexServerStateMachine& /*aContext*/, CObexPacket& /*aPacket*/)
	{
	IrOBEXUtil::Fault(EConnectInBadState);
	}

/**
Disconnect packet to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
@param aPacket Disconnect packet
*/
void TObexServerOperationState::Disconnect(CObexServerStateMachine& /*aContext*/, CObexPacket& /*aPacket*/)
	{
	IrOBEXUtil::Fault(EDisconnectInBadState);
	}

/**
Put packet to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
@param aPacket Put packet
*/
void TObexServerOperationState::Put(CObexServerStateMachine& /*aContext*/, CObexPacket& /*aPacket*/)
	{
	IrOBEXUtil::Fault(EPutInBadState);
	}

/**
Get packet to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
@param aPacket Get packet
*/
void TObexServerOperationState::Get(CObexServerStateMachine& /*aContext*/, CObexPacket& /*aPacket*/)
	{
	IrOBEXUtil::Fault(EGetInBadState);
	}

/**
SetPath packet to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
@param aPacket SetPath packet
*/
void TObexServerOperationState::SetPath(CObexServerStateMachine& /*aContext*/, CObexPacket& /*aPacket*/)
	{
	IrOBEXUtil::Fault(ESetPathInBadState);
	}

/**
Abort request to be processed
Default action - fault (event should not happen for this state)
@param aContext Reference to state machine object
*/
void TObexServerOperationState::Abort(CObexServerStateMachine& /*aContext*/)
	{
	IrOBEXUtil::Fault(EAbortInBadState);
	}

/**
Transport up notification
Default action - do nothing or fault (event should not happen for this state)
@param aContext Reference to state machine object
*/
void TObexServerOperationState::TransportUp(CObexServerStateMachine& /*aContext*/)
	{
	__ASSERT_DEBUG(EFalse, IrOBEXUtil::Fault(ETransportUpInBadState));
	}

/**
Server Start notification
Default action - do nothing and return no error
@param aContext Reference to state machine object
*/
void TObexServerOperationState::Start(CObexServerStateMachine& /*aContext*/)
	{
	}

/**
Reset notification - state machine should go back to the initial (disconnected) state
Default action - change to Disconnected state
@param aContext Reference to state machine object
*/
void TObexServerOperationState::Reset(CObexServerStateMachine& aContext)
	{
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}

/**
Asynchronous notification complete - the Application has return an object for the current operation
Default action - panic as there is no operation waiting for a notification
@param aContext Reference to state machine object
@param aObject Pointer to object returned from the Application
@panic ENoNotificationToComplete
*/
void TObexServerOperationState::RequestNotificationCompleted(CObexServerStateMachine& /*aContext*/, CObexBaseObject* /*aObject*/)
	{
	IrOBEXUtil::Panic(ENoNotificationToComplete);
	}
	
/**
Asynchronous notification complete - the Application has return an object for the current operation
Default action - panic as there is no operation waiting for a notification
@param aContext Reference to state machine object
@param aObject Pointer to object returned from the Application
@panic ENoNotificationToComplete
*/	
void TObexServerOperationState::RequestNotificationCompleted(CObexServerStateMachine& /*aContext*/, TObexResponse /*aAppResponse*/)
	{
	IrOBEXUtil::Panic(ENoNotificationToComplete);
	}

/**
Asynchronous notification complete - the Application has return an object for the current operation
Default action - panic as there is no operation waiting for a notification
@param aContext Reference to state machine object
@param aObject Pointer to object returned from the Application
@panic ENoNotificationToComplete
*/	
void TObexServerOperationState::RequestCompleteNotificationCompleted(CObexServerStateMachine& /*aContext*/, TObexResponse /*aAppResponse*/)
	{
	IrOBEXUtil::Panic(ENoNotificationToComplete);
	}


/**
Connection notification - an OBEX connection has been made
Default action - panic. This should not be passed to the state machine in this state.
@param aContext Reference to state machine object
*/
void TObexServerOperationState::ConnectionComplete(CObexServerStateMachine& /*aContext*/)
	{
	IrOBEXUtil::Fault(EConnectionCompleteInBadState);
	}

/**
A user has requested to override the handling of a request packet.
@param aContext Reference to state machine object
@param aResponse Response code to send to Obex Client (this must not be a success code)
*/	
void TObexServerOperationState::OverrideRequestHandling(CObexServerStateMachine& /*aContext*/, TObexResponse /*aResponse*/)
	{
	IrOBEXUtil::Panic(EOverrideRequestHandlingInBadState);
	}

// Utility functions - common operations performed by states

/**
Check disconnect packet for the correct connection ID (if required) and disconnect the Client
If the check is not sucessful, return to the Idle state if there is an OBEX connection
or the TransportConnected state otherwise
@param aContext Reference to state machine object
@param aPacket Disconnect packet
*/
/*static*/ void TObexServerOperationState::PerformDisconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	FLOG(_L("OnPacketReceive DisConnect OP code\r\n"));
	aContext.Owner().SetCurrentOperation(CObex::EOpDisconnect);

	// Check for connection ID before performing disconnect
	if (aContext.Owner().CheckPacketForConnectionId(aPacket))
		{
		aContext.Transport().Send(ERespSuccess);
		aContext.ChangeState(CObexServerStateMachine::EDisconnecting);
		}
	else
		{
		// Incorrect connection ID - abandon current operation
		aContext.Transport().Send(ERespServiceUnavailable);
		// Target states depends on whether there is a current OBEX connection
		if (aContext.Owner().GetConnectState() == CObex::EConnObex)
			{
			aContext.ChangeState(CObexServerStateMachine::EReady);
			}
		else
			{
			aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
			}
		}
	}

/**
End the current operation, sending the given response code to the Client and moving to the Idle state
@panic ObexFault ERespondAndEndOpWithoutObexConnection if there is no OBEX connection
@param aContext Reference to state machine object
@param aResponseCode Response code to send to the Client
*/
/*static*/ void TObexServerOperationState::RespondAndEndOperation(CObexServerStateMachine& aContext, TObexResponse aResponseCode)
	{
	// This function assumes that there is an OBEX connection, so it goes back to EReady - the assert checks this
	__ASSERT_DEBUG((aContext.Owner().GetConnectState() == CObex::EConnObex),IrOBEXUtil::Fault(ERespondAndEndOpWithoutObexConnection));

	aContext.Transport().Send(aResponseCode);
	aContext.ChangeState(CObexServerStateMachine::EReady);
	}
	
TBool TObexServerOperationState::ValidResponse(TObexResponse /*aResponseCode*/)
	{
	return EFalse;
	}

void TObexServerOperationState::WriteComplete(CObexServerStateMachine& /*aContext*/)
	{
	}

/**
Indicates that a new obex packet is being read. Do nothing in default state.
*/
void TObexServerOperationState::ReadActivityDetected(CObexServerStateMachine& /*aContext*/)
	{
	}


