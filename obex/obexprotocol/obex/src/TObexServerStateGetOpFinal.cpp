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

Final Get Operation Receive State
Waiting for reply from server app to an async notification GetCompleteIndication

Any OBEX operation will cause a Protocol Error
Chaging the MObexServerNotifyAsync via CObexServer::Start() will cause a Panic
A Reset event (Protocol Error, Server Stop, Transport Down) will cancel the notification and move the machine to Disconnected
The NotificationComplete event send the response back to the client and move the state machine to Ready
*/

TObexServerStateGetOpFinal::TObexServerStateGetOpFinal()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "GetOpFinal");
	iName = KName;
#endif
	}

void TObexServerStateGetOpFinal::Entry(CObexServerStateMachine& aContext)
	{
	aContext.Notification().GetCompleteIndication();
	}

void TObexServerStateGetOpFinal::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::Abort(CObexServerStateMachine& aContext)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateGetOpFinal::Reset(CObexServerStateMachine& aContext)
	{
	// Cancel notification - if it has not already been activated
	if (!aContext.IsCallBackActive())
		{
		aContext.Notification().CancelIndicationCallback();
		}
	aContext.CancelCallBack();
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}

void TObexServerStateGetOpFinal::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

void TObexServerStateGetOpFinal::RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse)
	{
	aContext.SetAppResponse(aAppResponse);
	aContext.CallBack(TObexServerStateGetOpFinal::ProcessNotification);
	}
	
TInt TObexServerStateGetOpFinal::ProcessNotification(TAny* aPtr)
	{
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);

	context.Transport().SendPacket().SetOpcode(context.AppResponse());
	context.Transport().SendPacket().SetFinal();
	context.Transport().Send();
	
	context.ChangeState(CObexServerStateMachine::EReady);
	return KErrNone;
	}

TBool TObexServerStateGetOpFinal::ValidResponse(TObexResponse aResponseCode)
	{
	return (aResponseCode>0 && aResponseCode<=255 && aResponseCode != ERespContinue);
	}
	

