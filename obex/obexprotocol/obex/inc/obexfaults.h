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

#ifndef OBEX_FAULTS_H
#define OBEX_FAULTS_H

/**
@file
@internalComponent

This file contains details of all Obex panics which may be raised as a result
of an internal programming error.
*/


/**
Category for Obex faults
*/
_LIT(KObexFaultCategory, "ObexFault");


/**
Obex fault codes
*/
enum TObexFaultCode
	{
	/** USB has entered an unknown state.  This probably means TUsbcDeviceState
	has changed without Obex being updated accordingly.
	@see TUsbcDeviceState
	@see CObexUsbHandler
	*/
	EUnknownUsbState				= 0,
	
	
	/** The object still has references to it on destruction.
	@see CRefCounted
	*/
	ERefCountNonZero				= 1,
	
	
	/** An attempt has been made to construct an unknown buffering details
	object.
	@see TObexBufferingDetails
	*/
 	EBadBufferDetailsVersion		= 2,
 	
 	
	/** No receive packet is available to return.
	@see CObexServerStateMachine
	*/
	ENoReceivePacketAvailable		= 3,
	
	
	/** No notifier has been set to notify of an event.
	Server should not be started without a notifier, so event handling is
	not expected to occur.
	@see CObexServerStateMachine
	*/
	ENoNotifierAvailable			= 4,
	
	
	/** Synchronous state wrapper has been errored when calling
	CObexServer::RequestIndicationCallback().
	@see CObexServerSyncWrapper
	*/
	ESyncWrapperCallbackError		= 5,


	/** An unexpected event has been sent to the state machine
	@see CObexServer
	@see CObexServerStateMachine
	*/
	EServerStateMachineBadEvent		= 6,

	/** CreateTransportInfoL has been given an unrecognised 
	TObexProtocolInfo::iTransport, and has therefore been unable to create a 
	TObexTransportInfo.
	@see IrOBEXUtil
	*/
	EUtilNoTransportInfo			= 7,

	/** CObex::TransportDown() has been called
	This is a legacy function that is retained for backwards compatibility.
	It does not do anything and should not be called
	@see CObex::TransportDown()
	*/
	ETransportDownCalled			= 8,

	/** RespondAndEndOperation has been called, but the authentication
	state machine does not think there is an OBEX connection
	@see TObexServerOperationState::RespondAndEndOperation
	*/
	ERespondAndEndOpWithoutObexConnection	= 9,

	/** CObexClient::ClientCommandL() does not recognise the operation
	code value it is supplied.
	@see CObexClient::ClientCommandL()
	*/
	EClientCommandOpUnrecognised		= 10,

	/** CObexErrorEngine::LastError() does not recognise the error set
	argument it is supplied.
	@see CObexErrorEngine::LastError()
	*/
	EErrorSetUnrecognised			= 11,

	/** CObexErrorEngine::LastError() does not recognise the underlying
	error value that has been set.
	@see CObexErrorEngine::LastError()
	*/
	EUnderlyingErrorUnrecognised		= 12,

	/** The last underlying error is set twice during the lifetime
	of a client operation.
	@see CObexClient
	*/
	ELastErrorSetTwice			= 13,

	/** The last underlying error was not set during the lifetime
	of a client operation.
	@see CObexClient
	*/
	ELastErrorNotSet			= 14,
	
	/** OnPacketReceive has been called but the state machine does not
	believe the server to have been started.
	@see CObexServerStateMachine::OnPacketReceive
	*/
	EPacketReceivedWhenServerNotStarted = 15,
	
	/** ConnectionComplete has been called but the state machine does
	not believe the server to have been started.
	@see CObexServerStateMachine::ConnectionComplete
	*/
	EConnectionCompleteWhenServerStopped = 16,
	
	/** Put received in bad state.  Note that all states where a put
	is expected to be possible (even if due to invalid remote 
	behaviour) override the base implementation.
	@see TObexServerOperationState::Put
	*/
	EPutInBadState = 17,
	
	/** Get received in bad state.  Note that all states where a get
	is expected to be possible (even if due to invalid remote 
	behaviour) override the base implementation.
	@see TObexServerOperationState::Get
	*/
	EGetInBadState = 18,
	
	/** SetPath received in bad state.  Note that all states where a 
	setpath is expected to be possible (even if due to invalid remote 
	behaviour) override the base implementation.
	@see TObexServerOperationState::SetPath
	*/
	ESetPathInBadState = 19,
	
	/** Abort received in bad state.  Note that all states where an
	abort is expected to be possible (even if due to invalid remote 
	behaviour) override the base implementation.
	@see TObexServerOperationState::Abort
	*/
	EAbortInBadState = 20,
	
	/** ConnectionComplete received in bad state.  Note that all states where
	this is expected to be possible (even if due to invalid remote 
	behaviour) override the base implementation.
	@see TObexServerOperationState::ConnectionComplete
	*/
	EConnectionCompleteInBadState = 21,
	
	/** TransportUp received in bad state.  Note that all states where
	this is expected to be possible override the base implementation.
	@see TObexServerOperationState::TransportUp
	*/
	ETransportUpInBadState = 22,
	
	/** Connect received in bad state.  Note that all states where
	this is expected to be possible override the base implementation.
	@see TObexServerOperationState::Connect
	*/
	EConnectInBadState = 23,
	
	/** Disconnect received in bad state.  Note that all states where
	this is expected to be possible override the base implementation.
	@see TObexServerOperationState::Disconnect
	*/
	EDisconnectInBadState = 24,

	/** In the current implementation, the transport controller should be 
	created at CObex::ConstructL time and persist until ~CObex.
	*/
	ETransportControllerNotCreated = 25,
	};

#endif
