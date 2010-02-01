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

#include <obex/transport/mobextransportnotify.h>
#include <obexbttransportinfo.h>
#include "ObexRfcommConnector.h"
#include "ObexActiveRfcommConnector.h"
#include "ObexListenerRfcommConnector.h"
#include "logger.h"
#include "obexbtfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "RFCOMM");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "RfcommConnector");
#endif

CObexRfcommConnector* CObexRfcommConnector::NewL(MObexTransportNotify& aOwner, 
						 TBTSockAddr& aAddr)
	{
	LOG_STATIC_FUNC_ENTRY

	CObexRfcommConnector* self = new(ELeave) CObexRfcommConnector(aOwner, aAddr);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CObexRfcommConnector::CObexRfcommConnector(MObexTransportNotify& aObserver, 
										   TBTSockAddr& aAddr)
 :	CObexConnector(aObserver),
	iAddr(aAddr)
	{
	LOG_FUNC
	LOG(_L8("\taAddr:"));
	LOGHEXDESC(aAddr);
	}

void CObexRfcommConnector::ConstructL()
	{
	LOG_FUNC

	LEAVEIFERRORL(iSocketServ.Connect());
	LEAVEIFERRORL(iSocketServ.FindProtocol(_L("RFCOMM"), iProtocolDesc));

	iConnector = new(ELeave) CObexActiveRfcommConnector(*this, iSocketServ, iSocket, iProtocolDesc, iAddr);
	iReceiver = new(ELeave) CObexListenerRfcommConnector(*this, iSocketServ, iSocket, iProtocolDesc, iAddr);

	BaseConstructL();
	}

CObexRfcommConnector::~CObexRfcommConnector()
	{
	LOG_FUNC

	delete iConnector;
	delete iReceiver;
	iSocket.Close();
	iSocketServ.Close();
	}

void CObexRfcommConnector::ConnectL()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iConnector, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iConnector->ConnectL();
	}

void CObexRfcommConnector::CancelConnect()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iConnector, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iConnector->Cancel();
	}

void CObexRfcommConnector::AcceptL()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iReceiver, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iReceiver->AcceptL();
	}

void CObexRfcommConnector::CancelAccept()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iReceiver, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iReceiver->Cancel();
	}

TBool CObexRfcommConnector::BringTransportDown()
	{
	LOG_FUNC

	// There's a nasty contract going on here- at most, only one of these AOs 
	// should be active at any time. They each use this object's iSocket, 
	// opening and closing it, and asserting its SubSessionHandle at various 
	// points. They may be better designed as a single AO state machine.
	__ASSERT_DEBUG(iConnector, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iConnector->Cancel();
	__ASSERT_DEBUG(iReceiver, PANIC(KPanicCat, ERfcommConnectorInternalError));
	iReceiver->Cancel();

	// For cases where the underlying transport has gone we'll
	// do an Immediate Shutdown to prevent Close() from blocking.
	// This should stop dialogs from "freezing". 
	//returns true as this transport proceeds with taking down the transport
	if ( iTransportUp )
		{
		TRequestStatus status;
		iSocket.Shutdown(RSocket::EImmediate, status);
		User::WaitForRequest(status);
		iTransportUp = EFalse;
		}

	iSocket.Close();
	return ETrue;
	}

/** Used to set any link layer specific error condition flags. Bluetooth has 
none, so the function is a stub. */
void CObexRfcommConnector::SignalTransportError()
	{
	LOG_FUNC
	}
	
/**
Returns the socket instance iSocket
@return RSocket
*/
RSocket& CObexRfcommConnector::GetSocket()
	{
	LOG_LINE
	LOG_FUNC

	return iSocket;
	}
	
/**
This method is used to field completed connections from both the active 
connector and the listen/acceptor.
*/
void CObexRfcommConnector::ConnectComplete(TInt aError, TObexConnectionInfo& aSockinfo)
	{
	LOG_FUNC
	LOG1(_L8("\taError = %d"), aError);

	if ( aError == KErrNone )
		{
		Observer().TransportUp(aSockinfo);
		iTransportUp = ETrue;
		}
	else
		{
		Observer().Error(aError);
		__ASSERT_DEBUG(!iTransportUp, PANIC(KPanicCat, ERfcommConnectorInternalError));
		}
	}
