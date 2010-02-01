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

Disconnected State
This is the ground state for the machine: transport is disconnected, operation is EOpIdle
Machine will move to TransportConnnected state on receiving a TransportUp
*/

TObexServerStateDisconnected::TObexServerStateDisconnected()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "Disconnected");
	iName = KName;
#endif
	}

void TObexServerStateDisconnected::Entry(CObexServerStateMachine& aContext)
	{
	aContext.Owner().SetCurrentOperation(CObex::EOpIdle);
	}

void TObexServerStateDisconnected::TransportUp(CObexServerStateMachine& aContext)
	{
	aContext.ChangeState(CObexServerStateMachine::ETransportConnected);
	}

void TObexServerStateDisconnected::Reset(CObexServerStateMachine& /*aContext*/)
	{
	}

