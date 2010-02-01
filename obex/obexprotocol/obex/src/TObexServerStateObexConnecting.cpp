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

OBEX Connecting State
This state is entered during an OBEX connection attempt. The authentication state machine is
invoked via ParseConnectPacket and PrepareConnect packet.
A successful CONNECT will move the machine to Ready.
A CONNECT requiring a user password will move the machine to WaitForUserPassword
Other CONNECT results may stay in this state or move the machine to TransportConnected
A DISCONNECT will be processed
An ABORT will cause a Protocol Error
Any other OBEX operation will be answered with an OBEX error code
*/

TObexServerStateObexConnecting::TObexServerStateObexConnecting()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "ObexConnecting");
	iName = KName;
#endif
	}

void TObexServerStateObexConnecting::Entry(CObexServerStateMachine& aContext)
	{
	// Reset the Connect State Machine
	aContext.Owner().SetConnectState(CObex::EConnTransport);
	// Chain on to Connect to process packet
	Connect(aContext, aContext.LastReceivedPacket());
	}

void TObexServerStateObexConnecting::Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process connect packet
	// This will return KErrNone if the packet is OK
	//                  a positive number if an OBEX error should be returned to the Client
	//				    a negative number if a Symbian error should be raised (as a Protocol Error)
	TInt parseConnectResult = aContext.Owner().ParseConnectPacket(aPacket);
	if (parseConnectResult == KErrNone)
		{
		FLOG(_L("OnPacketReceive ParseConnectPacket succesfull\r\n"));
		aContext.Transport().SendPacket().Init(CObex::EOpConnect); 
		// PrepareConnectPacket will check the present state and request
		// Auth, or check Auth as necessary and will return:
		//		An error & connect state set to EWaitForUserInput if a User Password is required  
		//		No error and connect state set to EConnObex if the connection was successful
		//		Connect state set to EConnTransport if there is an error packet to send back to the client
		//		Other connect states will return a packet to send and stay in the current state
		TInt err = aContext.Owner().PrepareConnectPacket(aContext.Transport().SendPacket());
		if ((err == KErrNone)	|| (aContext.Owner().GetConnectState() == CObex::EConnTransport)
								|| (aContext.Owner().GetConnectState() == CObex::EWaitForUserInput))
			{
			FLOG(_L("OnPacketReceive PrepareConnectPacket SUCCESS\r\n"));
			// If not waiting for user input, send the resultant packet
			if ( aContext.Owner().GetConnectState() != CObex::EWaitForUserInput )
				{
				aContext.Transport().SendPacket().SetFinal();
				aContext.Transport().Send();
				}
			// Move this (operation) state machine according to result of authentication state machine
			switch (aContext.Owner().GetConnectState())
				{
				case CObex::EConnTransport:
					aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
					break;
				case CObex::EConnObex:
					aContext.ChangeState(CObexServerStateMachine::EReady);
					break;
				case CObex::EWaitForUserInput:
					aContext.ChangeState(CObexServerStateMachine::EWaitForUserPassword);
					break;
				}
			}
		else 
			{
			FTRACE( if (err)
						{
						FPrint(_L("OnPacketReceive PrepareConnectPacket FAILED"));
						}
					else
						{
						FPrint(_L("OnPacketReceive PrepareConnectPacket OK but state is %d"), aContext.Owner().GetConnectState());
						}
					);
			// Raising a Protocol Error will cause a Reset event resulting in a move to Disconnected.
			// So any code after this call will potentially be executed in a different state.
			aContext.Owner().Error(err);
			}
		}
	else if (parseConnectResult > 0) // so it's an OBEX error code
		{
		FLOG(_L("OnPacketReceive ParseConnectPacket FAILED (OBEX error)\r\n"));
		
		aContext.Transport().SendPacket().Init(parseConnectResult); 
		TInt rsp = aContext.Owner().PrepareErroredConnectPacket(aContext.Transport().SendPacket());
		if (rsp == KErrNone)	
   			{
			aContext.Transport().SendPacket().SetFinal();
			aContext.Transport().Send();
			aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
			}			
		else
			{
			aContext.Owner().Error(rsp);	
			}
		}
	else // parseConnectResult < 0 so it's a Symbian error code
		{
		FLOG(_L("OnPacketReceive ParseConnectPacket FAILED (Symbian error)\r\n"));
		// Raising a Protocol Error will cause a Reset event resulting in a move to Disconnected.
		// So any code after this call will potentially be executed in a different state.
		aContext.Owner().Error(KErrAbort);
		}
	}

void TObexServerStateObexConnecting::Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket)
	{
	// Process disconnect
	PerformDisconnect(aContext, aPacket);
	}

void TObexServerStateObexConnecting::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespBadRequest and return to ETransportConnected
	aContext.Transport().Send(ERespBadRequest);
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}

void TObexServerStateObexConnecting::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to ETransportConnected
	aContext.Transport().Send(ERespConflict);
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}

void TObexServerStateObexConnecting::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Send ERespConflict and return to ETransportConnected
	aContext.Transport().Send(ERespConflict);
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}

void TObexServerStateObexConnecting::Abort(CObexServerStateMachine& aContext)
	{
	// Send ERespSuccess
	// Any other response would, according to the spec,
	// require the Obex client to bring down the transport.
	// Our attempt is to be resilient if an 'Abort'
	// is sent erroneously whilst we are in this state.
	aContext.Transport().Send(ERespSuccess);
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}
	
void TObexServerStateObexConnecting::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse)
	{
	// Send the server applications response and return to Transport Connected
	aContext.Transport().Send(aResponse);
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}

