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

#if ( defined __FLOG_ACTIVE && defined __LOG_FUNCTIONS__ )
_LIT8(KLogComponent, "OBEX");
#endif

/**
@file
@internalComponent
*/

/**
Constructs the state machine and the state classes

@param aOwner Server object that owns the state machine
@param aTransport Transport Controller associated with the Server

@return Contructed CObexServerStateMachine object
*/
CObexServerStateMachine* CObexServerStateMachine::NewL(CObexServer& aOwner, CObexTransportControllerBase& aTransport)
	{
	CObexServerStateMachine* self = new(ELeave) CObexServerStateMachine(aOwner, aTransport);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
First phase constructor

@param aOwner Server object that owns the state machine
@param aTransport Transport Controller associated with the Server
*/
CObexServerStateMachine::CObexServerStateMachine(CObexServer& aOwner, CObexTransportControllerBase& aTransport)
	: iCurrentState(&iStates[EDisconnected]), iTransport(aTransport), iOwner(aOwner), iServerStarted(EFalse)
	{
	// Use placement new here to create the state objects and put them into a pre-defined array
	// This is done as the number of states is known at compile time and this avoids heap fragmentation
	new(&iStates[EDisconnected]) TObexServerStateDisconnected();
	new(&iStates[ETransportConnected]) TObexServerStateTransportConnected();
	new(&iStates[EObexConnecting]) TObexServerStateObexConnecting();
	new(&iStates[EWaitForUserPassword]) TObexServerStateWaitForUserPassword();
	new(&iStates[EReady]) TObexServerStateReady();
	new(&iStates[EPutOpWaitForUser]) TObexServerStatePutOpWaitForUser();
	new(&iStates[EPutOpReceiveObject]) TObexServerStatePutOpReceiveObject();
	new(&iStates[EGetOpReceiveSpecification]) TObexServerStateGetOpReceiveSpecification();
	new(&iStates[EGetOpWaitForUser]) TObexServerStateGetOpWaitForUser();
	new(&iStates[EGetOpSendObject]) TObexServerStateGetOpSendObject();
	new(&iStates[ESetPathOp]) TObexServerStateSetPathOp();
	new(&iStates[EPutOpFinal]) TObexServerStatePutOpFinal();
	new(&iStates[EGetOpFinal]) TObexServerStateGetOpFinal();
	new(&iStates[EDisconnecting]) TObexServerStateDisconnecting();
	}

/**
Second phase constructor
*/
void CObexServerStateMachine::ConstructL()
	{
	iSpecObject = CObexNullObject::NewL();
	iHeader = CObexHeader::NewL();
	iCallBack = new(ELeave) CAsyncCallBack(CActive::EPriorityStandard);
	}

/**
Destructor
*/
CObexServerStateMachine::~CObexServerStateMachine()
	{
	// No need to delete state array
	delete iHeader;
	delete iSpecObject;
	delete iPutFinalResponseHeaderSet;
	delete iCallBack;
	}

/**
Move machine to a new state
Note that this function is synchronous, so any code after calling this function
will be executed with the machine in a different state.
Usually the action after changing state should be to return from the current function.

@param aState Index of new state
@return Result of state change
*/
void CObexServerStateMachine::ChangeState(TObexServerOperationStateEnum aState)
	{
	STATE_LOG_2(_L8("Changing from state %S to %S"), &iCurrentState->iName, &iStates[aState].iName);
	iCurrentState = &(iStates[aState]);
	iCurrentStateEnum = aState;
	iCurrentState->Entry(*this);
	}

void CObexServerStateMachine::ControlledTransportDown()
	{
	LOG_FUNC

	iOwner.ControlledTransportDown();
	}

/**
Process a received packet according to the packet's op-code and the current state

@param aPacket Packet to process
@return Result of any state changes
*/
void CObexServerStateMachine::OnPacketReceive(CObexPacket& aPacket)
	{
	if (!iServerStarted)
		{
		__ASSERT_DEBUG(iServerStarted, IrOBEXUtil::Fault(EPacketReceivedWhenServerNotStarted));
		return;
		}

	iLastReceivedPacket = &aPacket;

	if(Transport().IsWriteActive())
 		{
 		FLOG(_L("OnPacketReceive received request whilst writing... dropping connection\r\n"));
		Notification().ObexDisconnectIndication(KNullDesC8);
		// change state before issuing the transport down, as it may cause notifications
		ChangeState(ETransportConnected);
		ControlledTransportDown();
		return;
 		}

	// Initialise the send packet to ensure that we do not
	// accidentally send the same packet as last time!
	iTransport.SendPacket().Init(0);

	switch (aPacket.Opcode())
		{
	case CObex::EOpConnect:
		STATE_LOG(_L8("Calling connect"));
		iCurrentState->Connect(*this, aPacket);
		break;
	case CObex::EOpDisconnect:
		STATE_LOG(_L8("Calling disconnect"));
		iCurrentState->Disconnect(*this, aPacket);
		break;
	case CObex::EOpPut:
		STATE_LOG(_L8("Calling put"));
		iCurrentState->Put(*this, aPacket);
		break;
	case CObex::EOpGet:
		STATE_LOG(_L8("Calling get"));
		iCurrentState->Get(*this, aPacket);
		break;
	case CObex::EOpSetPath:
		STATE_LOG(_L8("Calling SetPath"));
		iCurrentState->SetPath(*this, aPacket);
		break;
	case CObex::EOpAbortNoFBit:
		// Abort does not check target headers (hence no need to send the packet to the event)
		STATE_LOG(_L8("Calling abort"));
		iCurrentState->Abort(*this);
		break;

	default:
		// Unknown packet type
		STATE_LOG(_L8("Unknown packet type"));
		iTransport.Send(ERespNotImplemented);
		break;
		}
	}

/**
Indicates a transport-level connection has been made to the Server
*/
void CObexServerStateMachine::TransportUp()
	{
	// This event could happen while the Server is stopped (as the transport state
	// can be independent of whether the Server has requested a read)
	iCurrentState->TransportUp(*this);
	}

/**
Indicates the transport-level connection to the Server has disappeared
*/
void CObexServerStateMachine::TransportDown()
	{
	// This event could happen while the Server is stopped (as the transport state
	// can be independent of whether the Server has requested a read)
	iCurrentState->Reset(*this);
	}

/**
Indicates a Protocol Error has occurred
*/
void CObexServerStateMachine::Error()
	{
	// This event could happen while the Server is stopped (as the transport state
	// can be independent of whether the Server has requested a read)
	iCurrentState->Reset(*this);
	}

/**
Process an OBEX object received as part of an asynchronous Put/Get indication
This object pointer can be NULL to indicate an error condition
@see MObexServerNotifyAsync::PutRequestInidication
@see MObexServerNotifyAsync::GetRequestInidication
@see CObexServer::RequestIndicationCallback

@param aObject OBEX object to use (NULL to indication an error condition)
@return Result of any state changes
*/
TInt CObexServerStateMachine::RequestNotificationCompleted(CObexBaseObject* aObject)
	{
	__ASSERT_ALWAYS(iCurrentStateEnum == EPutOpWaitForUser ||
					iCurrentStateEnum == EGetOpWaitForUser, IrOBEXUtil::Panic(EInvalidResponseCallback));
	if (!iServerStarted)
		{
		return KErrIrObexServerStopped;
		}

	iCurrentState->RequestNotificationCompleted(*this, aObject);
	return KErrNone;
	}

/**
Process the response received as part of an asynchronous PutComplete/GetComplete/SetPath indication

@param 	aAppResponse Application's response to the indication
@return result of state changes
*/
TInt CObexServerStateMachine::RequestNotificationCompleted(TObexResponse aAppResponse)
	{
	__ASSERT_ALWAYS(iCurrentStateEnum == EPutOpWaitForUser ||
					iCurrentStateEnum == EGetOpWaitForUser, IrOBEXUtil::Panic(EInvalidResponseCallback));
	__ASSERT_ALWAYS(iCurrentState->ValidResponse(aAppResponse), IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp));
	if (!iServerStarted)
		{
		return KErrIrObexServerStopped;
		}

	iCurrentState->RequestNotificationCompleted(*this, aAppResponse);
	return KErrNone;
	}
	
/**
Process the response received as part of an asynchronous PutComplete/GetComplete/SetPath indication

@param 	aAppResponse Application's response to the indication
@return result of state changes
*/
TInt CObexServerStateMachine::RequestCompleteNotificationCompleted(TObexResponse aAppResponse)
	{
	__ASSERT_ALWAYS(iCurrentStateEnum == ESetPathOp || iCurrentStateEnum == EGetOpFinal ||
					iCurrentStateEnum == EPutOpFinal, IrOBEXUtil::Panic(EInvalidResponseCallback));
	__ASSERT_ALWAYS(iCurrentState->ValidResponse(aAppResponse), IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp));
	if (!iServerStarted)
		{
		return KErrIrObexServerStopped;
		}

	iCurrentState->RequestCompleteNotificationCompleted(*this, aAppResponse);
	return KErrNone;
	}

/**
Indicates an OBEX level connection has been established

@return Result of any state changes
*/
void CObexServerStateMachine::ConnectionComplete()
	{
	__ASSERT_ALWAYS(iServerStarted, IrOBEXUtil::Fault(EConnectionCompleteWhenServerStopped));

	iCurrentState->ConnectionComplete(*this);
	}

/**
Indicates Server has been started

@param aNotify Notification interface to use
*/
void CObexServerStateMachine::Start(MObexServerNotifyAsync& aNotify)
	{
	iServerStarted = ETrue;
	iNotification = &aNotify;	// state will panic if trying to change interface at an inappropriate point
	iCurrentState->Start(*this);
	}

/**
Indicates Server has been stopped
*/
void CObexServerStateMachine::Stop()
	{
	iCurrentState->Reset(*this);
	iServerStarted = EFalse;
	}
	
/**
Indication that the Obex server application has chosen to override the
handling of the request packet that has been received.

@param aResponse The response the server application has indicated that should
				 be sent to the Obex client.  The actual sending of the response
				 is delegated to the individual states to handle as appropriate.
@return Result of any state changes
*/
void CObexServerStateMachine::OverrideRequestHandling(TObexResponse aResponse)
	{
	iCurrentState->OverrideRequestHandling(*this, aResponse);
	}

/**
Indicates that a write of a packet has been completed successfully at the 
transport level.
*/
void CObexServerStateMachine::WriteComplete()
	{
	LOG_FUNC

	iCurrentState->WriteComplete(*this);
	}

/**
Indicates that a new obex packet is being read.
*/
void CObexServerStateMachine::ReadActivityDetected()
	{
	LOG_FUNC

	iCurrentState->ReadActivityDetected(*this);
	}

/**
@return Last packet processed by the state machine
*/
CObexPacket& CObexServerStateMachine::LastReceivedPacket() const
	{
	__ASSERT_DEBUG(iLastReceivedPacket, IrOBEXUtil::Fault(ENoReceivePacketAvailable));
	return *iLastReceivedPacket;
	}

/**
This sets pointer of the object received from the application
and so can be NULL to indicate an error

@see CObexServerStateMachine::NotificationComplete
@return Specification Object - used to describe the OBEX object to Get
*/
CObexBaseObject* CObexServerStateMachine::SpecObject() const
	{
	return iSpecObject;
	}

/**
This returns pointer to the object received from the application
and so can be NULL to indicate an error

@see CObexServerStateMachine::NotificationComplete
@return Transfer object to exchange with the Client
*/
CObexBaseObject* CObexServerStateMachine::TransObject() const
	{
	return iTransObject;
	}

/**
@param aTransObject New transfer object to exchange with the Client
*/
void CObexServerStateMachine::SetTransObject(CObexBaseObject* aTransObject)
	{
	iTransObject = aTransObject;
	}

/**
@return Transport Controller associated with the Server
*/
CObexTransportControllerBase& CObexServerStateMachine::Transport() const
	{
	return iTransport;
	}

/**
@return Server object associated with the State Machine
*/
CObexServer& CObexServerStateMachine::Owner() const
	{
	return iOwner;
	}

/**
@return Notification interface associated with the Server
*/
MObexServerNotifyAsync& CObexServerStateMachine::Notification() const
	{
	__ASSERT_DEBUG(iNotification, IrOBEXUtil::Fault(ENoNotifierAvailable));
	return *iNotification;
	}

/**
@return Final Header set to send at the end of a Put operation
*/
CObexHeaderSet* CObexServerStateMachine::PutFinalResponseHeaderSet()
	{
	return iPutFinalResponseHeaderSet;
	}

/**
@param aHeaderSet Final Header set to send at the end of a Put operation
*/
void CObexServerStateMachine::SetPutFinalResponseHeaderSet(CObexHeaderSet* aHeaderSet)
	{
	delete iPutFinalResponseHeaderSet;
	iPutFinalResponseHeaderSet = aHeaderSet;
	}

/**
@return Internal header object (used for sizing the final response header set)
*/
CObexHeader* CObexServerStateMachine::GetHeader()
	{
	return iHeader;
	}

/**
Activate one-shot call-back to run from the Active Scheduler
@param aFunction Pointer to the function to call
*/
void CObexServerStateMachine::CallBack(TInt (*aFunction)(TAny* aPtr))
	{
	iCallBack->Set(TCallBack(aFunction, this));
	iCallBack->CallBack();
	}

/**
Cancel one-shot call-back
*/
void CObexServerStateMachine::CancelCallBack()
	{
	iCallBack->Cancel();
	}

/**
@return If one-shot call-back is active
*/
TBool CObexServerStateMachine::IsCallBackActive() const
	{
	return iCallBack->IsActive();
	}

TObexResponse CObexServerStateMachine::AppResponse() const
	{
	return iAppResponse;
	}

void CObexServerStateMachine::SetAppResponse(TObexResponse aAppResponse)
	{
	__ASSERT_DEBUG(aAppResponse>0 && aAppResponse<=255, IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp ));
	
	//remove the final bit, sorry about the horrible double casting. 
	aAppResponse = static_cast<TObexResponse>(static_cast<TInt> (aAppResponse) & 0x7F); 
	iAppResponse = aAppResponse;
	}

