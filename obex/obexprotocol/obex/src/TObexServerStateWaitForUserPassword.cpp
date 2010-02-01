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

CONNECT Operation Wait State
This waits for the authentication state machine to receive a password from the user (CObexServer::UserPasswordL) which is indicated by ConnectionComplete

Any OBEX operation will be cause a Protocol Error (which will then cause a reset event)
Changing the MObexServerNotifyAsync via CObexServer::Start() will cause a Panic
A Reset event (Protocol Error, Server Stop, Transport Down) will cause a transition to Disconnected
*/

TObexServerStateWaitForUserPassword::TObexServerStateWaitForUserPassword()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "WaitForUserPassword");
	iName = KName;
#endif
	}

void TObexServerStateWaitForUserPassword::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::Abort(CObexServerStateMachine& aContext)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}
	
void TObexServerStateWaitForUserPassword::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	// Raise Protocol Error (no way to cancel the password request notification)
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateWaitForUserPassword::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

void TObexServerStateWaitForUserPassword::ConnectionComplete(CObexServerStateMachine& aContext)
	{
	// Move to Ready
	aContext.ChangeState(CObexServerStateMachine::EReady);
	}

