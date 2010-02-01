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
 @internalComponent
*/

#ifndef __OBEXERRORENGINE_H
#define __OBEXERRORENGINE_H

#include <obex/internal/mobexclienterrorresolver.h>

/**
Implements the Obex client error resolver extension functionality.
Keeps a record of the last underlying client error that occurred.
Resolves the underlying error to an error set specified by the user.
*/
NONSHARABLE_CLASS(CObexErrorEngine) : public CBase, public MObexClientErrorResolver
	{
public:
	/**
	The Obex client sets a detailed "underlying" error on the error
	engine for resolving to a specific error set later on.  This
	mechanism adds extensibility to different error sets.
	*/
	enum TObexClientUnderlyingError
		{
		/**
		No error in the last operation.
		@see CObexClient::OnPacketReceive()
		*/
		ENoError = 0,

		/**
		The last operation failed to bring the transport up.
		@see CObexClient::ClientCommandL()
		@see CObexClient::CompleteRequest()
		*/
		ETransportUpFailed = 1,

		/**
		Tried to connect but already connected.
		@see CObexClient::ClientCommandL()
		*/
		EAlreadyConnected = 2,

		/**
		Failed to insert local connection info into connect packet.
		@see CObexClient::PrepareConnectPacket()
		*/
		ECannotInsertConnectInfo = 3,

		/**
		Connection challenge received but unable to ask the user for a password.
		@see CObexClient::PrepareConnectPacket()
		*/
		EChallengeRejected = 4,

		/**
		The client's connect state was set to an erroneous value.
		@see CObexClient::PrepareConnectPacket()
		*/
		EPrepareConnectPacketIncorrectState = 5,

		/**
		A packet was received while the client was sending.
		@see CObexClient::OnPacketReceive()
		*/
		EResponseWhileWriting = 6,

		/**
		Failed to extract remote connection info from server connect packet.
		@see CObexClient::ParseConnectPacket()
		*/
		ECannotExtractConnectInfo = 7,

		/**
		Cannot process the challenge/challenge response from the server.
		@see CObexClient::ParseConnectPacket()
		*/
		ECannotProcessChallenge = 8,

		/**
		Challenge response received but the client did not request one.
		@see CObexClient::ParseConnectPacket()
		*/
		EUnexpectedChallengeResponse = 9,

		/**
		Unauthorised opcode in server connect packet and there is no challenge header.
		This means that the server failed to authenticate the client.
		@see CObexClient::ParseConnectPacket()
		*/
		EChallengeAbsent = 10,

		/**
		The server connect packet contained an unknown opcode.
		@see CObexClient::ParseConnectPacket()
		*/
		EBadOpcodeInConnectPacket = 11,

		/**
		The client operation timed out.
		@see CObexClient::TimeoutCompletion()
		*/
		EResponseTimeout = 12,

		/**
		The client operation was aborted by the user.
		@see CObexClient::OnPacketReceive()
		*/
		EAborted = 13,

		/**
		The transport went down while an operation was outstanding.
		@see CObexClient::OnTransportDown()
		@see CObexClient::CompleteRequest()
		*/
		EOpOutstandingOnTransportDown = 14,

		/**
		The server refused the disconnection request
		because it did not recognise the connection ID.
		@see CObexClient::OnPacketReceive()
		*/
		EBadConnectionId = 15,

		/**
		The client attempted an operation other than connect while disconnected.
		@see CObexClient::ClientCommandL()
		*/
		EDisconnected = 16,

		/**
		Could not initialise the object to be sent in the Put/Get request.
		@see CObexClient::ClientCommandL()
		*/
		ECannotInitialiseObject = 17,

		/**
		Could not set the connection ID on the
		object to be sent in the Put/Get request.
		@see CObexClient::ClientCommandL()
		*/
		ECannotSetConnectionId = 18,

		/**
		Could not prepare the next send packet in the Put/Get request.
		@see CObexClient::ClientCommandL()
		*/
		ECannotPreparePacket = 19,

		/**
		Multipacket response from server to Put/Get (not GetResponse) request.
		@see CObexClient::OnPacketReceive()
		*/
		EMultipacketResponse = 20,

		/**
		The response from the server contained an error code.
		@see CObexClient::OnPacketReceive()
		*/
		EErrorResponseFromServer = 21,

		/**
		Could not extract the header from the final Put response packet.
		@see CObexClient::OnPacketReceive()
		*/
		ECannotExtractFinalPutHeader = 22,

		/**
		The opcode of the Put response packet from the server is
		not consistent with the progress of the Put operation.
		@see CObexClient::OnPacketReceive()
		*/
		EPutOutOfSync = 23,

		/**
		The opcode of the Get response packet from the server is Success
		but the client hasn't finished sending all the Get request packets.
		@see CObexClient::OnPacketReceive()
		*/
		EGetPrematureSuccess = 24,

		/**
		Could not parse a GetResponse packet from the server.
		@see CObexClient::OnPacketReceive()
		*/
		EGetResponseParseError = 25,
		};

	static CObexErrorEngine* NewL();

	// Implements MObexClientErrorResolver::LastError().
	virtual TUint LastError(TObexClientErrorResolutionSetType aErrorSet) const;

	void SetLastError(TObexClientUnderlyingError aError);

private:
	CObexErrorEngine();

private:
	TObexClientUnderlyingError iLastError;
	};

#endif
