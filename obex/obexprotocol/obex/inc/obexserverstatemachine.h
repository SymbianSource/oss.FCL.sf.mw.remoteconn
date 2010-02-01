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


#ifndef OBEX_SERVER_STATE_MACHINE_H
#define OBEX_SERVER_STATE_MACHINE_H

#include <obextypes.h>
#include <obex/internal/obexpacket.h>
#include <obex/transport/obextransportcontrollerbase.h>
#include "obexserveroperationstates.h"
#include "OBEXUTIL.H"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KStateMachineCmpt, "statemachine");

#define STATE_LOG(str) CObexLog::Write(KStateMachineCmpt, str)
#define STATE_LOG_2(str, a, b) CObexLog::WriteFormat(KStateMachineCmpt, str, a, b)
#else
#define STATE_LOG(str)
#define STATE_LOG_2(str, a, b)
#endif

/**
The OBEX Server state machine's context.
This class holds all the state context information and acts
as the interface between CObexServer and the current state
@see TObexServerOperationState
*/
NONSHARABLE_CLASS(CObexServerStateMachine) : public CBase
	{
public:
	enum TObexServerOperationStateEnum
		{
		EDisconnected,
		ETransportConnected,
		EObexConnecting,
		EWaitForUserPassword,
		EReady,
		EPutOpWaitForUser,
		EPutOpReceiveObject,
		EGetOpReceiveSpecification,
		EGetOpWaitForUser,
		EGetOpSendObject,
		ESetPathOp,
		EPutOpFinal,
		EGetOpFinal,
		EDisconnecting,
		EEndOfStates
		};
	
	static CObexServerStateMachine* NewL(CObexServer& aOwner, CObexTransportControllerBase& aTransport);
	~CObexServerStateMachine();
	
	
	// Events
	void ChangeState(TObexServerOperationStateEnum aState);
	void OnPacketReceive(CObexPacket& aPacket);
	void TransportUp();
	void TransportDown();
	void Error();
	TInt RequestNotificationCompleted(CObexBaseObject* aObject);
	TInt RequestNotificationCompleted(TObexResponse aAppResponse);
	TInt RequestCompleteNotificationCompleted(TObexResponse aAppResponse);
	void ConnectionComplete();
	void Start(MObexServerNotifyAsync& aNotify);
	void Stop();
	void OverrideRequestHandling(TObexResponse aResponse);
	void WriteComplete();	
	void ReadActivityDetected();
	
	// Getter/setter functions
	CObexPacket& LastReceivedPacket() const; // no setter needed as is only ever set in OnPacketReceive()
	
	// Object specifiying object to GET
	CObexBaseObject* SpecObject() const; 	// Don't need a setter as we own
	
	// Object returned from Application for GET/PUT
	CObexBaseObject* TransObject() const;
	void SetTransObject(CObexBaseObject* aTransObject);
	
	// Access to CObexServer members
	CObexTransportControllerBase& Transport() const;
	CObexServer& Owner() const;
	MObexServerNotifyAsync& Notification() const;
	
	// Header set for final Put response
	CObexHeaderSet* PutFinalResponseHeaderSet();
	void SetPutFinalResponseHeaderSet(CObexHeaderSet* aHeaderSet);
	CObexHeader* GetHeader(); // don't need setter as we own

	// Call-back used to wrap NotificationComplete event
	void CallBack(TInt (*aFunction)(TAny* aPtr));
	void CancelCallBack();
	TBool IsCallBackActive() const;
	TObexResponse AppResponse() const;
	void SetAppResponse(TObexResponse aAppResponse);

	void ControlledTransportDown();

private:
	CObexServerStateMachine(CObexServer& aOwner, CObexTransportControllerBase& aTransport);
	void ConstructL();
	
private:
	TFixedArray<TObexServerOperationState, EEndOfStates> iStates;	
	TObexServerOperationState* iCurrentState;
	
	CObexPacket* iLastReceivedPacket;
	CObexBaseObject* iSpecObject;
	CObexBaseObject* iTransObject;
	CObexTransportControllerBase& iTransport;
	
	CObexServer& iOwner;
	MObexServerNotifyAsync* iNotification;

	CObexHeaderSet* iPutFinalResponseHeaderSet;
	CObexHeader* iHeader;

	TBool iServerStarted;
	
	CAsyncCallBack* iCallBack;
	TObexResponse iAppResponse;
	TObexServerOperationStateEnum iCurrentStateEnum;
	};

#endif	// OBEX_SERVER_STATE_MACHINE_H

