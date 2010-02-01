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

GET Operation Wait State
This state issues the Get indication to the Application on entry and then waits for the response.

Any OBEX operation will be cause a Protocol Error
Chaging the MObexServerNotifyAsync via CObexServer::Start() will cause a Panic
A Reset event (Protocol Error, Server Stop, Transport Down) will cancel the notification and move the machine to Disconnected
The NotificationComplete event will queue a async one-shot to process the returned object, moving the machine to GetOpSendObject
*/

#ifdef __FLOG_ACTIVE
_LIT8(KGetWaitCmpt, "getwait");

#define GETWAIT_LOG(str) CObexLog::Write(KGetWaitCmpt, str)
#else
#define GETWAIT_LOG(str)
#endif


TObexServerStateGetOpWaitForUser::TObexServerStateGetOpWaitForUser()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "GetOpWaitForUser");
	iName = KName;
#endif
	}

void TObexServerStateGetOpWaitForUser::Entry(CObexServerStateMachine& aContext)
	{
	// Send notification to the Application
	aContext.SetTransObject(NULL);
	
	GETWAIT_LOG(_L8("Upcalling..."));
	aContext.Notification().GetRequestIndication(aContext.SpecObject());
	}

void TObexServerStateGetOpWaitForUser::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Abort(CObexServerStateMachine& aContext)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}
	
void TObexServerStateGetOpWaitForUser::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	// Raise Protocol Error (which will cause a Reset and cancel the notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

void TObexServerStateGetOpWaitForUser::Reset(CObexServerStateMachine& aContext)
	{
	// Cancel notification - if it has not already been activated
	if (!aContext.IsCallBackActive())
		{
		aContext.Notification().CancelIndicationCallback();
		}
	aContext.CancelCallBack();
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}

void TObexServerStateGetOpWaitForUser::RequestNotificationCompleted(CObexServerStateMachine& aContext, CObexBaseObject* aObject)
	{
	aContext.SetTransObject(aObject);
	aContext.CallBack(TObexServerStateGetOpWaitForUser::ProcessNotification);
	}

void TObexServerStateGetOpWaitForUser::RequestNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse)
	{
	aContext.SetAppResponse(aAppResponse);
	aContext.CallBack(TObexServerStateGetOpWaitForUser::ProcessErrorNotification);	
	}

/*static*/ TInt TObexServerStateGetOpWaitForUser::ProcessNotification(TAny* aPtr)
	{
	// Get hold of the Context and the returned object
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);
	
	GETWAIT_LOG(_L8("Process notification"));
	CObexBaseObject* object = context.TransObject();
	
	//Check and initialise object, move to GetOpSendObject
	if (!object)
		{
		RespondAndEndOperation(context, ERespForbidden);
		}
	else if (object->InitSend(ERespContinue) != KErrNone)
		{
		RespondAndEndOperation(context, ERespInternalError);
		}
	else
		{
		//ConnectionID is compulsory if Target header was used at connection
		if ( context.Owner().CheckObjectForConnectionId(*(context.SpecObject())))
			{
			context.ChangeState(CObexServerStateMachine::EGetOpSendObject);
			}
		else  //connectionID was incorrect or not received
			{
			context.Transport().Send(ERespServiceUnavailable); 
			}
		}
	return KErrNone;
	}

/*static*/ TInt TObexServerStateGetOpWaitForUser::ProcessErrorNotification(TAny* aPtr)
	{
	// Get hold of the Context and the returned object
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);
	
	RespondAndEndOperation(context, context.AppResponse());
	
	return KErrNone;
	}

TBool TObexServerStateGetOpWaitForUser::ValidResponse(TObexResponse aResponseCode)
	{
	return (aResponseCode>0 && aResponseCode<=255 &&  aResponseCode != ERespContinue && 
	   aResponseCode != ERespSuccess  && aResponseCode != ERespCreated  && aResponseCode != ERespAccepted);  
	}
