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

#ifndef OBEXRFCOMMCONNECTOR_H
#define OBEXRFCOMMCONNECTOR_H

#include <bt_sock.h>
#include <obex/transport/obexconnector.h>
#include "connectobserver.h"

class CObexActiveRfcommConnector;
class CObexListenerRfcommConnector;
class MObexTransportNotify;
class TObexBtTransportInfo;

NONSHARABLE_CLASS(CObexRfcommConnector) : public CObexConnector,
											public MConnectObserver
	{
public:
	static CObexRfcommConnector* NewL(MObexTransportNotify& aOwner, 
						 TBTSockAddr& aAddr);
	~CObexRfcommConnector();

public:
	RSocket& GetSocket();

private: 
	CObexRfcommConnector(MObexTransportNotify& aOwner, 
						 TBTSockAddr& aAddr);
	void ConstructL();
	
private: // from CObexConnector
	void ConnectL();
	void CancelConnect();
	void AcceptL();
	void CancelAccept();
	TBool BringTransportDown();
	void SignalTransportError();
	
private: // from MConnectObserver
	void ConnectComplete(TInt aError, TObexConnectionInfo& aSockinfo);

private: // owned
	CObexActiveRfcommConnector* iConnector;
	CObexListenerRfcommConnector* iReceiver;
	TBool iTransportUp;
	RSocket iSocket;
	RSocketServ iSocketServ;
	TBTSockAddr& iAddr;
	TProtocolDesc iProtocolDesc;
	};

#endif // OBEXRFCOMMCONNECTOR_H
