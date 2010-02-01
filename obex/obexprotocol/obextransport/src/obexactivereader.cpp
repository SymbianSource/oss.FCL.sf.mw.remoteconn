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

#include <obex/transport/obexactivereader.h>
#include <es_sock.h>
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

/**
Factory function.
@param  aOwner the owner of this 
@param aSocket  a socket for reading and writing
@param aInfo connection information
@return Ownership of a new active reader.
*/
EXPORT_C CObexActiveReader* CObexActiveReader::NewL(MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo)
	{
	CObexActiveReader* self = new(ELeave) CObexActiveReader(aOwner, aSocket, aInfo);
	CleanupStack::PushL(self);
	self->BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Constructor
@param  aOwner the owner of this 
@param aSocket  a socket for reading and writing
@param aInfo connection information
*/
CObexActiveReader::CObexActiveReader (MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo)
	: CObexReaderBase (EPriorityStandard, aOwner, aInfo),
	iSocket(aSocket)
	{
	LOG_LINE
	LOG_FUNC
	}

/**
Destructor
*/
EXPORT_C CObexActiveReader::~CObexActiveReader ()
	{
	LOG_LINE
	LOG_FUNC

	Cancel ();
	}

/**
Called to actually transfer some data into iLocation 
*/
void CObexActiveReader::DoTransfer ()
	{
	LOG_FUNC

	iSocket.Recv(iLocation, KSockReadContinuation, iStatus);
	SetActive ();
	}

/**
Return the maximum packet size
@return TInt the maximum packet size for this transport
*/
TInt CObexActiveReader::GetMaxPacketSize()
	{
	LOG_FUNC

	// Some slightly dubious logic here...
	// IrDA has a degenerate case when running over a 256 byte transport layer link, we claim
	// to support <255 bytes. The Obex spec says that all stations must be able to handle
	// 255 bytes, so some stacks may ignore our limit.
	// Thus we compare against the actual buffer size, rather than what we told the remote end
	// we could handle, so we *may* be able to handle being sent a few bytes more than expected...

	TInt ret = GetObexPacketBufferSize();
	LOG1(_L8("\tret = %d"), ret);
	return ret;
	}

/**
Returns a initial packet size when the packet size of iPacket is not know.  This is used
when determining the remaining bytes to be read.  
@return TInt the initial packet size
*/
TInt CObexActiveReader::GetInitialPacketSize ()
	{
	LOG_FUNC

	return GetObexPacketHeaderSize();
	}

void CObexActiveReader::DoCancel ()
	{
	LOG_FUNC

	iSocket.CancelRecv ();
	}
