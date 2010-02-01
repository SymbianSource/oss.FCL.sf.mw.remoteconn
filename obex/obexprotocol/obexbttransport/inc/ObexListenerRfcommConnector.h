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

#ifndef OBEXLISTENERRFCOMMCONNECTOR_H
#define OBEXLISTENERRFCOMMCONNECTOR_H

#include <e32base.h>
#include <bt_sock.h>

class MConnectObserver;

/**
Simple Active Object to call Accept for a socket. Only one accept operation 
may be outstanding at any time. It has its own listening socket. 
*/
NONSHARABLE_CLASS(CObexListenerRfcommConnector) : public CActive
	{
public:
	CObexListenerRfcommConnector(MConnectObserver& aParent,
		RSocketServ& aSocketServ,
		RSocket& aSocket,
		const TProtocolDesc& aProtocolDesc,
		TBTSockAddr& aAddr);
	~CObexListenerRfcommConnector();
	
public:
	void AcceptL();

private: // from CActive
	void RunL();
	void DoCancel();

private: // unowned
	MConnectObserver& iParent;
	RSocketServ& iSocketServ;
	RSocket& iSocket;
	const TProtocolDesc& iProtocolDesc;
	TBTSockAddr& iAddr;
					
private: // owned
	RSocket iListeningSocket;
	};

#endif // OBEXLISTENERRFCOMMCONNECTOR_H
