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
#include "obexsetpathdata.h"

/**
@file
@internalComponent

SETPATH Operation State
This state processes the SetPath operation, the SetPathIndication is asynchronous.
So the state has to wait for RequestCompleteIndicationCallback before sending 
response and transition to the ready state. 
*/

TObexServerStateSetPathOp::TObexServerStateSetPathOp()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "SetPathOp");
	iName = KName;
#endif
	}

void TObexServerStateSetPathOp::Entry(CObexServerStateMachine& aContext)
	{
	TObexSetPathData data;
	TBool authChallengePresent = EFalse;
	CObexPacket& aPacket = aContext.LastReceivedPacket();

	if(!aPacket.ExtractData(data))
		{
		aContext.Transport().Send(ERespBadRequest);
		aContext.ChangeState(CObexServerStateMachine::EReady);
		return;
		}
	TInt packetSize = aPacket.PacketSize();
	// Check Connection ID
	if (aContext.Owner().CheckPacketForConnectionId(aPacket))
		{
		CObex::TSetPathInfo info(data);

		// Search for any name headers
		TObexInternalHeader header;
		while(aPacket.ExtractData(header))
			{
			if(!info.iNamePresent && header.HI() == TObexInternalHeader::EName && header.GetHVText(info.iName) == KErrNone)
				{
				info.iNamePresent = ETrue;
				}
			if (header.HI() == TObexInternalHeader::EAuthChallenge)
				{
				authChallengePresent = ETrue;	
				}
			}
		if (authChallengePresent) // Auth challenge is present, send unauthorized response
			{
			aContext.Transport().Send(ERespUnauthorized);
			aContext.ChangeState(CObexServerStateMachine::EReady);
			}
		else 
			{
			//There is no need to add a new state to wait for the RequestCompleteIndicationCallback because there is 
			//no other actions required for SetPath. So just wait in this state.
			aContext.Notification().SetPathIndication(info, KNullDesC8);
			}
		
		}
	else //bad or missing ConnectionID
		{
		aContext.Transport().Send(ERespServiceUnavailable);
		aContext.ChangeState(CObexServerStateMachine::EReady);
		}
	}

void TObexServerStateSetPathOp::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::Abort(CObexServerStateMachine& aContext)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}
	
void TObexServerStateSetPathOp::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStateSetPathOp::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

void TObexServerStateSetPathOp::Reset(CObexServerStateMachine& aContext)
	{
	// Cancel notification - if it has not already been activated
	if (!aContext.IsCallBackActive())
		{
		aContext.Notification().CancelIndicationCallback();
		}
	aContext.CancelCallBack();
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}

void TObexServerStateSetPathOp::RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse)
	{
	aContext.SetAppResponse(aAppResponse);
	aContext.CallBack(TObexServerStateSetPathOp::ProcessNotification);
	}
	
TInt TObexServerStateSetPathOp::ProcessNotification(TAny* aPtr)
	{
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);

	context.Transport().Send(context.AppResponse());
	context.ChangeState(CObexServerStateMachine::EReady);
	
	return KErrNone;
	}

TBool TObexServerStateSetPathOp::ValidResponse(TObexResponse aResponseCode)
	{
	return (aResponseCode>0 && aResponseCode<=255 && aResponseCode != ERespContinue);
	}

