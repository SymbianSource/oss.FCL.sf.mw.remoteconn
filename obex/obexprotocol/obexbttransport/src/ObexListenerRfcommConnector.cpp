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

#include <obex/transport/obexconnector.h>
#include "ObexListenerRfcommConnector.h"
#include "connectobserver.h"
#include "logger.h"
#include "obexbtfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "RFCOMM");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ListRfcomm");
#endif

CObexListenerRfcommConnector::CObexListenerRfcommConnector(MConnectObserver& aParent,
														   RSocketServ& aSocketServ,
														   RSocket& aSocket,
														   const TProtocolDesc& aProtocolDesc,
														   TBTSockAddr& aAddr)
 :	CActive(EPriorityStandard),
	iParent(aParent),
	iSocketServ(aSocketServ),
	iSocket(aSocket),
	iProtocolDesc(aProtocolDesc),
	iAddr(aAddr)
	{
	LOG_FUNC
	LOG(_L8("\taAddr:"));
	LOGHEXDESC(aAddr);

	CActiveScheduler::Add(this);
	}

CObexListenerRfcommConnector::~CObexListenerRfcommConnector()
	{
	LOG_FUNC

	Cancel();

	// iListeningSocket will only have been open if there was a request 
	// outstanding, in which case it will have been closed in DoCancel, so no 
	// need to do it here.
	__ASSERT_DEBUG(!iListeningSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));
	}

void CObexListenerRfcommConnector::RunL()
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());

	// These exists purely as sanity-checks on our internal state.
	__ASSERT_DEBUG(iSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));
	__ASSERT_DEBUG(iListeningSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));

	TObexConnectionInfo sockinfo;
	if ( iStatus == KErrNone )
		{
		sockinfo.iSocketType = TObexConnectionInfo::ESocketStream;

		TPckgBuf<TUint> optBuf(65535); //If all else fails we should get a max 64K buffer
		iSocket.GetOpt(KRFCOMMMaximumMTU, KSolBtRFCOMM, optBuf);

		// Set socket buffer sizes based on negotiated MTU
		iSocket.SetOpt(KSOSendBuf, KSOLSocket, optBuf);
		iSocket.SetOpt(KSORecvBuf, KSOLSocket, optBuf);		
		}
	else
		{
		iSocket.Close();
		}

	iListeningSocket.Close();
	iParent.ConnectComplete(iStatus.Int(), sockinfo);
	}

void CObexListenerRfcommConnector::DoCancel()
	{
	LOG_FUNC

	// These exist purely as sanity-checks on our internal state.
	__ASSERT_DEBUG(iSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));
	__ASSERT_DEBUG(iListeningSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));
	iListeningSocket.CancelAccept();
	iListeningSocket.Close();
	iSocket.Close();
	}

void CObexListenerRfcommConnector::AcceptL()
	{
	LOG_FUNC

	__ASSERT_DEBUG(!iSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));

	__ASSERT_DEBUG(!iListeningSocket.SubSessionHandle(), PANIC(KPanicCat, EListRfcommInternalError));
	LEAVEIFERRORL(iListeningSocket.Open(iSocketServ,
										iProtocolDesc.iAddrFamily,
										iProtocolDesc.iSockType,
										iProtocolDesc.iProtocol));
	CleanupClosePushL(iListeningSocket);

	LEAVEIFERRORL(iListeningSocket.Bind(iAddr));
	if ( iAddr.Port() == KRfcommPassiveAutoBind )
		{
		const TUint port = iListeningSocket.LocalPort();
		LOG1(_L8("\tKRfcommPassiveAutoBind was specified- really using port %d"), port);
		// This updates the concrete transport controller's iTransportInfo member (by reference).
		iAddr.SetPort(port);
		}
	// We try to set the Object Transfer bit in our service class.  If this fails we
	// ignore it, as it's better to carry on without it than to fail to start listening.
	(void)iListeningSocket.SetOpt(KBTRegisterCodService, KSolBtRFCOMM, EMajorServiceObjectTransfer);
	LEAVEIFERRORL(iListeningSocket.Listen(4)); // queue size of 4

	// NB Client does SDP & Sec registration.

	LEAVEIFERRORL(iSocket.Open(iSocketServ));

	CleanupStack::Pop(&iListeningSocket);

	iListeningSocket.Accept(iSocket, iStatus);
	SetActive();
	}
