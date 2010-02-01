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

#include <obex/transport/obexactivewriter.h>
#include <obex/internal/obexpacket.h>
#include <es_sock.h>
#include <obex/transport/mobextransportnotify.h>
#include "ObexTransportUtil.h"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

EXPORT_C CObexActiveWriter* CObexActiveWriter::NewL(MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo)
	{
	CObexActiveWriter* self = new(ELeave) CObexActiveWriter(aOwner, aSocket, aInfo);
	CleanupStack::PushL(self);
	self->BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CObexActiveWriter::CObexActiveWriter (MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo)
 :	CObexWriterBase (EPriorityHigh, aOwner, aInfo),
	iInfo ( aInfo ),
	iSocket(aSocket)
	{
	LOG_LINE
	LOG_FUNC
	}

EXPORT_C CObexActiveWriter::~CObexActiveWriter ()
	{
	LOG_LINE
	LOG_FUNC

	Cancel ();
	}

/**
Start transfer.
Calls into CObexActiveRW, which eventaully queues a write which is limited by 
the smaller of iInfo's max send size and aPacket's data limit

@param aPacket The Obex packet to write
*/
void CObexActiveWriter::StartTransfer (CObexPacket& aPacket)
	{
	LOG_FUNC

	// Find out how much we can write to the remote end per transfer
	iTransportWriteLimit = Min(iInfo.iMaxSendSize, aPacket.DataLimit());
	LOG1(_L8("\tiTransportWriteLimit = %d"), iTransportWriteLimit);
	CObexWriterBase::StartTransfer(aPacket);
	}
	

// Called to actually transfer some data out from iLocation 
void CObexActiveWriter::DoTransfer ()
	{
	LOG_FUNC

	// Send the biggest chunk we can over the transport
	if (iPacketBased && (iLocation.MaxLength () > iTransportWriteLimit))
		{
		iLocation.SetLength (iTransportWriteLimit);
		}
	else
		{
		iLocation.SetMax ();
		}

	iSocket.Write (iLocation, iStatus);
	SetActive ();
	}

void CObexActiveWriter::DoCancel ()
	{
	LOG_FUNC

	iSocket.CancelSend ();
	}
