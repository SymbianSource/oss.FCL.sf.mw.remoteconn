// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalTechnology
*/

#ifndef __OBEXACTIVEWRITER_H__
#define __OBEXACTIVEWRITER_H__

#include <obex/transport/obexwriterbase.h>
#include <obex/transport/obexconnector.h>

class RSocket;

/**
Functionality specific to writing to a socket.
@internalTechnology
*/
NONSHARABLE_CLASS(CObexActiveWriter) : public CObexWriterBase
	{
public:
	IMPORT_C static CObexActiveWriter* NewL(MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo);
	IMPORT_C ~CObexActiveWriter();

private:
	CObexActiveWriter(MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo);

private: // from CObexWriterBase
	virtual void StartTransfer(CObexPacket& aPacket);
	virtual void DoCancel();
	virtual void DoTransfer();

private:
	TInt iTransportWriteLimit;
	TObexConnectionInfo iInfo; // used to find out transport limits per transfer
	RSocket& iSocket;
	};

#endif // __OBEXACTIVEWRITER_H__
