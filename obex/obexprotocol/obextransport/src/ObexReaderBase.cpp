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

#include <obex/transport/obexreaderbase.h>
#include <obex/internal/obexpacket.h>
#include <obex/transport/mobextransportnotify.h>
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

/**
*Constructor
*/
EXPORT_C CObexReaderBase::CObexReaderBase(TPriority aPriority, 
										  MObexTransportNotify& aOwner, 
										  TObexConnectionInfo& aInfo)
	: CObexActiveRW (aPriority, aOwner, aInfo)
	{
	LOG_LINE
	LOG_FUNC
	}

/**
* Destructor
*/
EXPORT_C CObexReaderBase::~CObexReaderBase()
	{
	LOG_LINE
	LOG_FUNC

	Cancel();
	}

/**
This function is a place holder for future use. If the iFuture1 variable is 
used it will need this function for any allocation required.  

To prevent binary compatiblity breaks if the iFuture1 variable is used, this 
function must be called from the 
NewL of derived classes.  
*/
EXPORT_C void CObexReaderBase::BaseConstructL()
	{
	LOG_LINE
	LOG_FUNC
	}

/**
Start transfer. Calls into CObexActiveRW, which eventually queues a read 

@param aPacket The Obex packet to read into.
*/
EXPORT_C void CObexReaderBase::StartTransfer (CObexPacket& aPacket)
	{
	LOG_LINE
	LOG_FUNC

	// Ensure that we don't try to queue two reads at once (this should probably
	// never happen anyway).
	if (IsActive())
		{
		FLOG(_L("\treturning because already active..."));
		return;
		}
	iGotHeader = EFalse;
	// Initial packet size indicates how much data we should ask for on the 
	// first read
	// Packet & stream based transports do the same thing here due to ESOCK 
	// flexibility.
	// See Remaining() and CompleteTransfer() for a clearer picture
	iPacketSize = 0;
	aPacket.Init(0);
	NewRequest (aPacket);
	}

/**
Check if the packet read is complete 
@return ETrue if the read is complete.  EFalse otherwise.
*/
EXPORT_C TBool CObexReaderBase::CompleteTransfer ()
	{
	LOG_LINE
	LOG_FUNC

	iTransferError = KErrNone;

	// Can't check anything until we have at least the OBEX packet header
	if (iCount >= KObexPacketHeaderSize)
		{// iCount is the number of bytes read thus far
		if (!iGotHeader)
			{
			iPacketSize = iPacket->PacketSize ();

			// Check packet's claimed size is at least as big as the header just sent
			if ( iPacketSize < KObexPacketHeaderSize )
				{
				iTransferError = KErrCommsFrame;
				return ETrue;
				}
				TInt maxPacketSize = GetMaxPacketSize();
				LOG1(_L8("\taMaxPacketSize = %d"), maxPacketSize);
			if (iPacketSize > maxPacketSize)
				{// The peer is sending us a packet thats too large for us
				iTransferError = KErrCommsFrame;
				return (ETrue);
				}
			iGotHeader = ETrue;
			OnReadActivity();
			}


		if (iCount >= iPacketSize)
			{// We've got the whole packet.
			return (ETrue);
			}
		}
	return (EFalse);
		
	}

/** 
	Called when read activity is detected. 
	This method will certainly be called when read activity is 
	first detected on an Obex operation, and it may be called 
	from time to time thereafter until that Obex operation is
	completed.
*/
EXPORT_C void CObexReaderBase::OnReadActivity()
	{
	iOwner.SignalPacketProcessEvent(EObexReadActivityDetected);
	}

/** Performs any actions necessary on completion of a read.
*/
EXPORT_C void CObexReaderBase::OnCompleteTransfer()
	{
	if(iTransferError)
		{
		iOwner.Error(iTransferError);
		}
	else
		{
		iOwner.Process(*iPacket);
		}
	}

/*
Returns the number of bytes remaining to be read based on the default packet 
size if the packet size is unknown, otherwise based on the packet size

@return TInt the number of bytes remaining to be read
*/
EXPORT_C TInt CObexReaderBase::Remaining()
	{
	LOG_LINE
	LOG_FUNC
	

	TInt remaining;
	
	if (iPacketSize == 0)
		{	
		// If we don't know the size of the packet yet, ask for as much as 
		// an initial or default size (-iCount) specifies
		TInt defaultPacketSize = GetInitialPacketSize();
		LOG1(_L8("\taRemainingLimit = %d"), defaultPacketSize);
		remaining = defaultPacketSize - iCount;
		}
	else
		{
		remaining = iPacketSize - iCount;
		}
	
	if (remaining < 0)
		{
		remaining = 0;
		}
	
	return remaining;
	}

/**
This function returns the buffer size of the read packet.  

@return TInt the size of the buffer held by the iPacket member variable
*/
EXPORT_C TInt CObexReaderBase::GetObexPacketBufferSize()
	{
	return iPacket->BufferSize();
	}

/**
This function returns the size of the obex packet's header

@return TInt the size of the obex header
*/
EXPORT_C TInt CObexReaderBase::GetObexPacketHeaderSize()
	{
	return KObexPacketHeaderSize;
	}

/**
This function return the maximum limit the received data can be

@return TInt the data limit of the obex packet
*/
EXPORT_C TInt CObexReaderBase::GetObexPacketDataLimit()
	{
	return iPacket->DataLimit();
	}
		
/**
This function is part of the extension pattern and is implemented by all 
derived instantiable classes that need to extend it's interface.

By default this returns null. Any derived class that is required to extend its 
interface and that of this base class returns its new interface in the form of 
an M class, that it extends, if and only if  the corresponding TUid, aUid, is 
received. Otherwise the function calls the base class implementation, 
returning NULL.

@return TAny* The M Class representing the extension to the interface 
otherwise NULL
@param aUid The uid associated with the M Class that is being implemented
*/
EXPORT_C TAny* CObexReaderBase::GetInterface(TUid /*aUid*/)
	{
	return NULL;
	}
