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
#include "ObexActiveRfcommConnector.h"
#include "connectobserver.h"
#include "logger.h"
#include "obexbtfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "RFCOMM");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ActiveRfcommConn");
#endif

CObexActiveRfcommConnector::CObexActiveRfcommConnector(MConnectObserver& aParent,
													   RSocketServ& aSocketServ,
													   RSocket& aSocket,
													   const TProtocolDesc& aProtocolDesc,
													   const TBTSockAddr& aAddr)
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

CObexActiveRfcommConnector::~CObexActiveRfcommConnector()
	{
	LOG_FUNC

	Cancel();
	}

void CObexActiveRfcommConnector::RunL()
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());

	// This exists purely as a sanity-check on our internal state.
	__ASSERT_DEBUG(iSocket.SubSessionHandle(), PANIC(KPanicCat, EActiveRfcommConnInternalError));

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

	iParent.ConnectComplete(iStatus.Int(), sockinfo);
	}

void CObexActiveRfcommConnector::DoCancel()
	{
	LOG_FUNC

	// This exists purely as a sanity-check on our internal state.
	__ASSERT_DEBUG(iSocket.SubSessionHandle(), PANIC(KPanicCat, EActiveRfcommConnInternalError));
	iSocket.CancelConnect();
	iSocket.Close();
	}

void CObexActiveRfcommConnector::ConnectL()
	{
	LOG_FUNC

	__ASSERT_DEBUG(!iSocket.SubSessionHandle(), PANIC(KPanicCat, EActiveRfcommConnInternalError));
	LEAVEIFERRORL(iSocket.Open(iSocketServ, 
							   iProtocolDesc.iAddrFamily,
							   iProtocolDesc.iSockType, 
							   iProtocolDesc.iProtocol));

	// iAddr is logically const, and needs constness casting away for this 
	// API.
	iSocket.Connect(const_cast<TBTSockAddr&>(iAddr), iStatus);
	SetActive();
	}
