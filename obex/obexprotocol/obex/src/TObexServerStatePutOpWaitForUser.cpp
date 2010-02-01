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

PUT Operation Wait State
This state issues the Put indication to the Application on entry and then waits for the response.

Any OBEX operation will be cause a Protocol Error
Chaging the MObexServerNotifyAsync via CObexServer::Start() will cause a Panic
A Reset event (Protocol Error, Server Stop, Transport Down) will cancel the notification and move the machine to Disconnected
The NotificationComplete event will queue a async one-shot to process the returned object, moving the machine to PutOpReceiveObject
*/

TObexServerStatePutOpWaitForUser::TObexServerStatePutOpWaitForUser()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "PutOpWaitForUser");
	iName = KName;
#endif
	}

void TObexServerStatePutOpWaitForUser::Entry(CObexServerStateMachine& aContext)
	{
	// Send notification to the Application
	aContext.SetTransObject(NULL);
	
	aContext.Notification().PutRequestIndication();
	}

void TObexServerStatePutOpWaitForUser::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Abort(CObexServerStateMachine& aContext)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}
	
void TObexServerStatePutOpWaitForUser::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

void TObexServerStatePutOpWaitForUser::Reset(CObexServerStateMachine& aContext)
	{
	// Cancel notification - if it has not already been activated
	if (!aContext.IsCallBackActive())
		{
		aContext.Notification().CancelIndicationCallback();
		}
	aContext.CancelCallBack();
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}

void TObexServerStatePutOpWaitForUser::RequestNotificationCompleted(CObexServerStateMachine& aContext, CObexBaseObject* aObject)
	{
	aContext.SetTransObject(aObject);
	aContext.CallBack(TObexServerStatePutOpWaitForUser::ProcessNotification);
	}

void TObexServerStatePutOpWaitForUser::RequestNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse)
	{
	aContext.SetAppResponse(aAppResponse);
	aContext.CallBack(TObexServerStatePutOpWaitForUser::ProcessErrorNotification);
	}

/*static*/ TInt TObexServerStatePutOpWaitForUser::ProcessNotification(TAny* aPtr)
	{
	// Get hold of the Context and the returned object
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);
	
	CObexBaseObject* object = context.TransObject();
	
	//Check and initialise object, move to PutOpReceiveObject
	if(!object)
		{
		RespondAndEndOperation(context, ERespForbidden);
		}
	else if (object->InitReceive() != KErrNone)
		{
		RespondAndEndOperation(context, ERespInternalError);
		}
	else
		{
		context.Owner().SetCurrentOperation(CObex::EOpPut);
		context.ChangeState(CObexServerStateMachine::EPutOpReceiveObject);
		}
	
	return KErrNone;
	}

/*static*/ TInt TObexServerStatePutOpWaitForUser::ProcessErrorNotification(TAny* aPtr)
	{
	// Get hold of the Context and the returned object
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);
	
	RespondAndEndOperation(context, context.AppResponse());
	
	return KErrNone;
	}

TBool TObexServerStatePutOpWaitForUser::ValidResponse(TObexResponse aResponseCode)
	{
	return (aResponseCode>0 && aResponseCode<=255 &&  aResponseCode != ERespContinue && 
	   aResponseCode != ERespSuccess  && aResponseCode != ERespCreated  && aResponseCode != ERespAccepted);  
	}

