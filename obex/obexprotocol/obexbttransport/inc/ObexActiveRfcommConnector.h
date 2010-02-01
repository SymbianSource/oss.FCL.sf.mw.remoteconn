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

#ifndef OBEXACTIVERFCOMMCONNECTOR_H
#define OBEXACTIVERFCOMMCONNECTOR_H

#include <e32base.h>
#include <bt_sock.h>

class MConnectObserver;

/**
Simple active object to call Connect on a socket. Only one Connect operation 
may be outstanding at any time. 
*/
NONSHARABLE_CLASS(CObexActiveRfcommConnector) : public CActive
	{
public:
	CObexActiveRfcommConnector(MConnectObserver& aParent,
		RSocketServ& aSocketServ,
		RSocket& aSocket,
		const TProtocolDesc& aProtocolDesc,
		const TBTSockAddr& aAddr);
	~CObexActiveRfcommConnector();

public:
	void ConnectL();

private: // from CActive
	void RunL();
	void DoCancel();

private: // unowned
	MConnectObserver& iParent;
	RSocketServ& iSocketServ;
	RSocket& iSocket;
	const TProtocolDesc& iProtocolDesc;
	const TBTSockAddr& iAddr;
	};

#endif // OBEXACTIVERFCOMMCONNECTOR_H
