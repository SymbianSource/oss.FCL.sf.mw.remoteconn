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

#ifndef OBEX_SERVER_OPERATION_STATES_H
#define OBEX_SERVER_OPERATION_STATES_H

/**
@file
@internalComponent

Defines base state class and derived state classes for each operation
*/

#include <obextypes.h>
#include "logger.h"

static const TInt KMaxStatenameLength = 32;

/**
Defines the state class interface and default operations for events
Also provides utility functions for common actions
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerOperationState)
	{
public:
	// Default implementation of events
	virtual void Entry(CObexServerStateMachine& aContext);
	
	virtual void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	
	virtual void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	
	virtual void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	
	virtual void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	
	virtual void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	
	virtual void Abort(CObexServerStateMachine& aContext);
	
	virtual void TransportUp(CObexServerStateMachine& aContext);
	
	virtual void Start(CObexServerStateMachine& aContext);
	
	virtual void Reset(CObexServerStateMachine& aContext);
	
	virtual void RequestNotificationCompleted(CObexServerStateMachine& aContext, CObexBaseObject* aObject);

	virtual void RequestNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);

	virtual void RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	
	virtual void ConnectionComplete(CObexServerStateMachine& aContext);
	
	virtual void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);

	virtual TBool ValidResponse(TObexResponse aResponseCode);

	virtual void WriteComplete(CObexServerStateMachine& aContext);
	
	virtual void ReadActivityDetected(CObexServerStateMachine& aContext);


protected:
	// Utility functions
	static void PerformDisconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	static void RespondAndEndOperation(CObexServerStateMachine& aContext, TObexResponse aResponseCode);

#ifdef __FLOG_ACTIVE
public:
	TBuf8<KMaxStatenameLength> iName;
#endif
	};


// State class definitions - defines methods for all the non-default events
// default events are provided by TObexServerOperationState

/**
Initial unconnected state
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateDisconnected) : public TObexServerOperationState
	{
public:
	TObexServerStateDisconnected();
	void Entry(CObexServerStateMachine& aContext);
	void TransportUp(CObexServerStateMachine& aContext);
	void Reset(CObexServerStateMachine& aContext);
	};

/**
Idle transport connected state (no OBEX connection)
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateTransportConnected) : public TObexServerOperationState
	{
public:
	TObexServerStateTransportConnected();
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	};

/**
Processes an OBEX connection attempt
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateObexConnecting) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStateObexConnecting();
	void Entry(CObexServerStateMachine& aContext);
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	};

/**
Waits for a password from the Application as part of connection attempt
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateWaitForUserPassword) : public TObexServerOperationState
	{
public:
	//Failure
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	//Functional
	TObexServerStateWaitForUserPassword();
	void ConnectionComplete(CObexServerStateMachine& aContext);
	};

/**
Idle OBEX connected state
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateReady) : public TObexServerOperationState
	{
public:
	TObexServerStateReady();
	void Entry(CObexServerStateMachine& aContext);
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	void ReadActivityDetected(CObexServerStateMachine& aContext);
	};

/**
Waits for object to return to Client as part of PUT operation
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStatePutOpWaitForUser) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStatePutOpWaitForUser();
	void Entry(CObexServerStateMachine& aContext);
	void Reset(CObexServerStateMachine& aContext);
	void RequestNotificationCompleted(CObexServerStateMachine& aContext, CObexBaseObject* aObject);
	void RequestNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	TBool ValidResponse(TObexResponse aResponseCode);
	
	// Call-back function used by asynchronous one-shot
	static TInt ProcessNotification(TAny* aPtr);
	static TInt ProcessErrorNotification(TAny* aPtr);
	};

/**
Receives object from Client as part of PUT operation
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStatePutOpReceiveObject) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	//Functional
	TObexServerStatePutOpReceiveObject();
	void Entry(CObexServerStateMachine& aContext);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	};

/**
Recevies from Client specification of object to GET
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateGetOpReceiveSpecification) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	//Functional
	TObexServerStateGetOpReceiveSpecification();
	void Entry(CObexServerStateMachine& aContext);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	};

/**
Waits for object to return to Client as part of GET operation
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateGetOpWaitForUser) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStateGetOpWaitForUser();
	void Entry(CObexServerStateMachine& aContext);
	void Reset(CObexServerStateMachine& aContext);
	void RequestNotificationCompleted(CObexServerStateMachine& aContext, CObexBaseObject* aObject);
	void RequestNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	TBool ValidResponse(TObexResponse aResponseCode);
	
	// Call-back function used by asynchronous one-shot
	static TInt ProcessNotification(TAny* aPtr);
	static TInt ProcessErrorNotification(TAny* aPtr);
	};

/**
Returns requested object to Client as part of GET operation
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateGetOpSendObject) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStateGetOpSendObject();
	void Entry(CObexServerStateMachine& aContext);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	};

/**
Performs SETPATH operation and waits for user interaction
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateSetPathOp) : public TObexServerOperationState
	{
public:
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStateSetPathOp();
	void Entry(CObexServerStateMachine& aContext);
	void Reset(CObexServerStateMachine& aContext);
	void RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	TBool ValidResponse(TObexResponse aResponseCode);
	
	static TInt ProcessNotification(TAny* aPtr);
	};

/**
Waits for user interaction after receiving the final PUT
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStatePutOpFinal) : public TObexServerOperationState
	{
public:
	//Failure cases	
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Functional
	TObexServerStatePutOpFinal();
	void Entry(CObexServerStateMachine& aContext);
	void Reset(CObexServerStateMachine& aContext);
	void RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	TBool ValidResponse(TObexResponse aResponseCode);
	
	static TInt ProcessNotification(TAny* aPtr);
private:
	static void PrepareFinalResponseHeaderSet(CObexHeader* aHeader, CObexHeaderSet& aHeaderSet, CObexPacket& aPacket);
	};

/**
Waits for user interaction after receiving the final Get
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateGetOpFinal) : public TObexServerOperationState
	{
public:
	
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void Start(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);
	
	//Fucntional
	TObexServerStateGetOpFinal();
	void Reset(CObexServerStateMachine& aContext);
	void Entry(CObexServerStateMachine& aContext);
	void RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse);
	TBool ValidResponse(TObexResponse aResponseCode);
	
	static TInt ProcessNotification(TAny* aPtr);
	};
		
/**
Disconnecting state
@see TObexServerOperationState
@see CObexServerStateMachine
*/
NONSHARABLE_CLASS(TObexServerStateDisconnecting) : public TObexServerOperationState
	{
public:
	TObexServerStateDisconnecting();
	
	//Failure cases
	void Connect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Disconnect(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Put(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Get(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void SetPath(CObexServerStateMachine& aContext, CObexPacket& aPacket);
	void Abort(CObexServerStateMachine& aContext);
	void OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse aResponse);

	//Functional
	void WriteComplete(CObexServerStateMachine& aContext);
	};

#endif	// OBEX_SERVER_OPERATION_STATES_H

