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

#ifndef __OBEXBTRFCOMMTRANSPORTCONTROLLER_H__
#define __OBEXBTRFCOMMTRANSPORTCONTROLLER_H__

#include <obex/transport/obextransportcontrollerbase.h>

/**
@internalComponent
Transport Controller for the transport BT RFCOMM
*/
NONSHARABLE_CLASS(CObexBtRfcommTransportController) : public CObexTransportControllerBase
	{
public:
	static CObexBtRfcommTransportController* NewL(TObexTransportInfo& aTransportInfo);
	~CObexBtRfcommTransportController();
	
private: // from CObexTransportControllerBase
	void DoRemoteAddr(TSockAddr& aAddr);
	void NewTransportL(TObexConnectionInfo& aInfo);
	TBool DoIsTransportRestartable() const;
	TUint16 GetReceivePacketDataLimit();
	void DoProcess(CObexPacket& aPacket);
	void DoError(TInt aError);
	void DoTransportUp(TObexConnectionInfo& aInfo);
	
private:
	CObexBtRfcommTransportController();
	void ConstructL(TObexTransportInfo& aTransportInfo);
	};

#endif // __OBEXBTRFCOMMTRANSPORTCONTROLLER_H__
