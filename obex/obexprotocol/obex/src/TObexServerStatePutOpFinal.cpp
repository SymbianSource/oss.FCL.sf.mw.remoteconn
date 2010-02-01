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

/**
@file
@internalComponent

Final PUT Operation Receive State
Waiting for reply from server app to an async notification PutCompleteIndication

Any OBEX operation will cause a Protocol Error
Chaging the MObexServerNotifyAsync via CObexServer::Start() will cause a Panic
A Reset event (Protocol Error, Server Stop, Transport Down) will cancel the notification and move the machine to Disconnected
The NotificationComplete event send the response back to the client and mvoe the state machine to Ready
*/

TObexServerStatePutOpFinal::TObexServerStatePutOpFinal()
	{
#ifdef __FLOG_ACTIVE
	_LIT8(KName, "PutOpFinalPut");
	iName = KName;
#endif
	}

void TObexServerStatePutOpFinal::Entry(CObexServerStateMachine& aContext)
	{
	// Chain on to Put() to start receiving object
	//return Put(aContext, aContext.LastReceivedPacket());
	aContext.Notification().PutCompleteIndication();
	}

void TObexServerStatePutOpFinal::Connect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::Disconnect(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::Put(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::Get(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::SetPath(CObexServerStateMachine& aContext, CObexPacket& /*aPacket*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::Abort(CObexServerStateMachine& aContext)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}
	
void TObexServerStatePutOpFinal::OverrideRequestHandling(CObexServerStateMachine& aContext, TObexResponse /*aResponse*/)
	{
	aContext.Owner().Error(KErrIrObexPacketDuringWait);
	}

void TObexServerStatePutOpFinal::Start(CObexServerStateMachine& /*aContext*/)
	{
	// Panic - trying to change interface during wait
	IrOBEXUtil::Panic(EChangeInterfaceDuringWait);
	}

// Utility function to handle the optional header set to send at the completion of the Put
void TObexServerStatePutOpFinal::PrepareFinalResponseHeaderSet(CObexHeader* aHeader, CObexHeaderSet& aHeaderSet, CObexPacket& aPacket)
	{
	//	We have some additional headers to send, on a best effort
	//	basis...
	TObexInternalHeader intHeader;
	aHeaderSet.First();
	TBool insertedOK=ETrue;

	while(insertedOK && (aHeaderSet.This(aHeader)==KErrNone))
		{
		switch(aHeader->Type())
			{
		case CObexHeader::EUnicode:
			intHeader.Set(aHeader->HI(), aHeader->AsUnicode());
			break;
			
		case CObexHeader::EByteSeq:
			intHeader.Set(aHeader->HI(), (const_cast<TUint8*> ((aHeader->AsByteSeq()).Ptr())), (aHeader->AsByteSeq()).Size());
			break;
			
		case CObexHeader::EByte:
			intHeader.Set(aHeader->HI(), aHeader->AsByte());
			break;
			
		case CObexHeader::EFourByte:
			intHeader.Set(aHeader->HI(), aHeader->AsFourByte());
			break;
			}

		insertedOK=aPacket.InsertData(intHeader); 
							
		(void)aHeaderSet.Next();
		}
	}
	
void TObexServerStatePutOpFinal::Reset(CObexServerStateMachine& aContext)
	{
	// Cancel notification - if it has not already been activated
	if (!aContext.IsCallBackActive())
		{
		aContext.Notification().CancelIndicationCallback();
		}
	aContext.CancelCallBack();
	aContext.ChangeState(CObexServerStateMachine::EDisconnected);
	}
	
void TObexServerStatePutOpFinal::RequestCompleteNotificationCompleted(CObexServerStateMachine& aContext, TObexResponse aAppResponse)
	{
	aContext.SetAppResponse(aAppResponse);
	aContext.CallBack(TObexServerStatePutOpFinal::ProcessNotification);
	}
	
TInt TObexServerStatePutOpFinal::ProcessNotification(TAny* aPtr)
	{
	CObexServerStateMachine& context = *static_cast<CObexServerStateMachine*>(aPtr);
	
	context.Transport().SendPacket().Init(context.AppResponse());
	context.Transport().SendPacket().SetFinal();	//	Because it is
	
	// Check for additional headers to send
	if(context.PutFinalResponseHeaderSet())
		{
		PrepareFinalResponseHeaderSet(	context.GetHeader(),
						*(context.PutFinalResponseHeaderSet()),
						context.Transport().SendPacket());
		// delete the headers now they've been used
		context.SetPutFinalResponseHeaderSet(NULL);
		}
	
	context.Transport().Send();
	context.ChangeState(CObexServerStateMachine::EReady);
	
	return KErrNone;
	}

TBool TObexServerStatePutOpFinal::ValidResponse(TObexResponse aResponseCode)
	{
	return (aResponseCode>0 && aResponseCode<=255 && aResponseCode != ERespContinue);
	}

