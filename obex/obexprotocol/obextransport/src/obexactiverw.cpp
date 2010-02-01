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

#include <obex/internal/obexactiverw.h>
#include <obex/internal/obexpacket.h>
#include <obex/transport/mobextransportnotify.h>
#include <obex/transport/obexconnector.h>
#include "ObexTransportUtil.h"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

/**
Constructor

@param aPriority a priority
@param aOwner the owner 
@param aInfo connection information
*/
CObexActiveRW::CObexActiveRW(TPriority aPriority,
							 MObexTransportNotify& aOwner,
							 TObexConnectionInfo& aInfo)
 :	CActive (aPriority), 
	iOwner(aOwner), 
	iLocation (NULL, 0)
	{
	LOG_LINE
	LOG_FUNC

	CActiveScheduler::Add (this);
	iPacketBased = aInfo.iSocketType==TObexConnectionInfo::ESocketPacket ? ETrue : EFalse;
	}

/**
Destructor
*/
CObexActiveRW::~CObexActiveRW()
	{
	LOG_LINE
	LOG_FUNC

	Cancel();
	}

/**
Start a new write or read
Reset the pointer descriptor to the start of the buffer and make the call to 
start the transfer

@param aPacket the obex packet to read or write
*/
void CObexActiveRW::NewRequest(CObexPacket& aPacket)
	{
	LOG_FUNC

	iPacket = &aPacket;
	iCount = 0;
	// Set descriptor to start of buffer
	// Size pointer according to either the number of bytes remaining in packet
	// OR maximum number of bytes we can receive (see Remaining())
	iLocation.Set(iPacket->iBuffer, 0, Remaining ());
	DoTransfer();
	}

/*
Non-virtual method calling virtual method StartTransfer. BC protection.

@param aPacket  the obex packet to transfer
*/
void CObexActiveRW::Transfer(CObexPacket& aPacket)
	{
	LOG_FUNC

	StartTransfer(aPacket);
	}

/**
RunL called after a read/write has completed
Check to see if we need to issue another request, otherwise process the 
complete packet
*/
EXPORT_C void CObexActiveRW::RunL()
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());

/*
	TBuf<256> hexBuf;
	FTRACE(FPrint(_L("CObexActiveRW::RunL iLocation.Length() = %d"), iLocation.Length()));
	for (TInt i = 0; i < iLocation.Length(); i++)
		{
		hexBuf.AppendFormat(_L("%02X "), iLocation[i]);
		if ((i % 16) == 15)
			{
			FLOG(hexBuf);
			hexBuf = KNullDesC;
			}
		}
	FLOG(hexBuf);
*/

	// Check the request completed OK
	if ( iStatus != KErrNone )
		{
		iOwner.Error(iStatus.Int());
		return;
		}

	// Update the byte count and check to see if the transfer is complete
	iCount += iLocation.Size();
	if ( CompleteTransfer() )
		{
		// Inform interested parties
		OnCompleteTransfer();
		return;
		}
	// Otherwise update the buffer pointer and start another transfer
	LOG1(_L8("CObexActiveRW::RunL Setting location, remaining=%d"), Remaining());
	iLocation.Set (&iPacket->iBuffer[iCount], 0, Remaining());
	if ( iLocation.MaxLength() > 0 )
		{
		DoTransfer();
		}
	}
