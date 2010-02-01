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

#include <es_sock.h>
#include <obex/internal/obexpacket.h>
#include <obex/internal/obexinternalconstants.h>
#include <obex/internal/obexdata.h>
#include "logger.h"
#include "obextransportfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

_LIT(KPanicCat, "ObexPacket");
CObexPacket::CObexPacket (TUint16 aBufferSize, TUint16 aDataLimit) : iBufferSize ( aBufferSize ), iDataLimit ( aDataLimit )
	{
	LOG_FUNC
 	}

/**
Destructor
*/
EXPORT_C CObexPacket::~CObexPacket ()
	{
	LOG_LINE
	LOG_FUNC

	delete[] iBuffer;
	}

/**
*/
EXPORT_C CObexPacket* CObexPacket::NewL (TUint16 aBufferSize, TUint16 aDataLimit)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY
	LOG2(_L8("\taBufferSize = %d, aDataLimit = %d"), aBufferSize, aDataLimit);

	// Data limit can't be larger than the buffer
	__ASSERT_ALWAYS ( aBufferSize >= aDataLimit, PANIC(KPanicCat, EDataLimitLargerThanBuffer) );

	CObexPacket* self = new (ELeave) CObexPacket ( aBufferSize, aDataLimit );
	CleanupStack::PushL (self);
	self->ConstructL ();
	CleanupStack::Pop (self);
	return (self);
	}

void CObexPacket::ConstructL ()
	{
//	FTRACE(FPrint(_L("CObexPacket::ConstructL buffer %d data limit %d"), iBufferSize, iDataLimit));

	iBuffer = new (ELeave) TUint8 [ iBufferSize ];
	Init ( 0 );
	}
/**
Set up as a fresh packet with the given opcode.
@param aOpcode a opcode
*/
EXPORT_C void CObexPacket::Init (TObexOpcode aOpcode)
	{
	LOG_LINE
	LOG_FUNC

	SetOpcode (aOpcode);
	SetPacketSize (KObexPacketHeaderSize);
	iInsertPoint = Payload ();
	iExtractPoint = Payload ();
	iNotificationEvents = 0;
	}
	
/**
Returns the packet opcode, with the final bit cleared (regardless of its actual value) 
@return TObexOpcode
*/
EXPORT_C TObexOpcode CObexPacket::Opcode () const
	{
	LOG_LINE
	LOG_FUNC

	return (STATIC_CAST(TObexOpcode, iBuffer[0] & ~KObexPacketFinalBit));
	}

/**
Set the packet opcode to the passed value -- final bit will get through too.
@param aOpcode an Opcode
*/
EXPORT_C void CObexPacket::SetOpcode (TObexOpcode aOpcode)
	{
	LOG_LINE
	LOG_FUNC

	iBuffer[0] = aOpcode;
	}

/**
True if the packet's final bit is set 
@return TBool true if this is the final packet
*/
EXPORT_C TBool CObexPacket::IsFinal () const
	{
	LOG_LINE

	const TBool isFinal = (iBuffer[0] & KObexPacketFinalBit);

	LOG1(_L8("CObexPacket::IsFinal returning %d"), isFinal);

	return isFinal;
	}

/**
If aFinal == ETrue (default), the final bit is set, otherwise it is cleared.
@param aFinal whether or not to set or clear the final bit
*/
EXPORT_C void CObexPacket::SetFinal (TBool aFinal)
	{
	LOG_LINE
	LOG_FUNC

	iBuffer[0] = STATIC_CAST(TObexOpcode, aFinal ? (iBuffer[0] | KObexPacketFinalBit) : (iBuffer[0] & ~KObexPacketFinalBit));
	}

/**
Returns the total size of the current packet.
@return TUint16 total size of the packet
*/
EXPORT_C TUint16 CObexPacket::PacketSize () const
	{
	LOG_LINE
	LOG_FUNC

	return (BigEndian::Get16 (&iBuffer[1]));
	}

/**
Sets the crruent packet's size.
@param aSize a packet size
*/
EXPORT_C void CObexPacket::SetPacketSize (TUint16 aSize)
	{
	LOG_LINE
	LOG_FUNC

	BigEndian::Put16 (&iBuffer[1], aSize);
	}

/**
Sets the data limit of the buffer, ensuring it's larger than minimum possible but not larger than the buffer
Parameter passed in is the requested new data limit
Returns size that was set
@param aRequestedSize a Requested size for the data limit
@return TUint16  the data limit size
*/
EXPORT_C TUint16 CObexPacket::SetLegalDataLimit (TUint16 aRequestedSize)
	{
	LOG_LINE
	LOG_FUNC

//	FTRACE(FPrint(_L("CObexPacket::SetLegalDataLimit requested size %d"), aRequestedSize));

	aRequestedSize = Max ( KObexPacketMinSize, aRequestedSize );
	aRequestedSize = Min ( iBufferSize, aRequestedSize );

//	FTRACE(FPrint(_L("CObexPacket::SetLegalDataLimit set size %d"), aRequestedSize));
	iDataLimit = aRequestedSize;
	return iDataLimit;
	}
	
/**	
return the data limit
@return TUint16 the data limit
*/
EXPORT_C TUint16 CObexPacket::DataLimit () const	
	{ 
	LOG_LINE
	LOG_FUNC

	return iDataLimit; 
	}
	
/**
return the buffer size	
@return TUint16 the buffer size
*/
EXPORT_C TUint16 CObexPacket::BufferSize() const
  	{
	LOG_LINE
	LOG_FUNC

   	return iBufferSize; 
  	}
  
/**
Insert the passed packet data object at iInsertPoint 
@param aHeader OBEX header to insert
@return True if the number of bytes is not zero and there is enough space to insert data
*/
EXPORT_C TBool CObexPacket::InsertData (const TObexData& aHeader)
	{
	LOG_LINE
	LOG_FUNC

	if (aHeader.TotalSize () > RemainingInsertSpace ()) return EFalse;
	TUint16 bytesInserted = aHeader.WriteOut (iInsertPoint, RemainingInsertSpace ());
	iInsertPoint += bytesInserted;
	SetPacketSize (STATIC_CAST(TUint16, PacketSize () + bytesInserted));
	return (bytesInserted != 0);
	}

/**
Extract into the passed data object from iExtractPoint 
@param aHeader Container for extracted data
@return True if bytes read is not zero
*/
EXPORT_C TBool CObexPacket::ExtractData (TObexData& aHeader)
	{
	LOG_LINE
	LOG_FUNC

	TUint16 bytesRead = aHeader.ParseIn (iExtractPoint, RemainingExtractSpace ());
	iExtractPoint += bytesRead;
	
	// If no bytes have been read two possible conditions have occured
	// 1. There is no remaining extract space in the packet (i.e. at the end)
	// 2. There has been an error when parsing.
	// In either case iExtractPoint will not move and aHeader has not really
	// changed its state (iHI, iHV, iHVSize and iHVRep will change but get
	// reset on the next ParseIn.  So if no bytes are read then from the API we
	// assume that we have finished processing the packet, and so can reset the
	// extract point, in case it needs to be parsed again.
	if (bytesRead == 0)
		{
		iExtractPoint = Payload();
		}
	
	return (bytesRead != 0);
	}

/**
Returns the number of bytes of unused space in the obex packet (on writes)
@return Remaining insert space
*/
EXPORT_C TUint16 CObexPacket::RemainingInsertSpace () const
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (iInsertPoint >= Payload () && iInsertPoint <= &iBuffer[iDataLimit], PANIC(KPanicCat, EPacketOverrun));
	return (TUint16)((&iBuffer[iDataLimit]) - iInsertPoint);
	}

/**
Returns the number of bytes of unread data in the obex packet (on reads) 
@return Remaining extract space
*/
EXPORT_C TUint16 CObexPacket::RemainingExtractSpace () const
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG (iExtractPoint >= Payload () && iExtractPoint <= &iBuffer[PacketSize ()], PANIC (KPanicCat, EPacketOverrun));
	return (TUint16)((&iBuffer[PacketSize ()] - iExtractPoint));
	}

/** Adds events that should be notified.

Add events into the packet process events mask.  When this packet
is processed this mask will be checked and notifications will be
issued only if the appropriate bits are set.

@param aEvents The additional events to notify on.
*/
EXPORT_C void CObexPacket::AddPacketProcessEvents(TObexPacketProcessEvents aEvents)
	{
	LOG_LINE
	LOG_FUNC
	iNotificationEvents |= aEvents;
	}
	
/** Removes events from packet notification.

Remove events from the packet process events mask.  When this packet
is processed this mask will be checked and notifications will be
issued only if the appropriate bits are set.

@param aEvents The events to cease notification of.
*/
EXPORT_C void CObexPacket::RemovePacketProcessEvents(TObexPacketProcessEvents aEvents)
	{
	LOG_LINE
	LOG_FUNC
	iNotificationEvents &= ~aEvents;
	}

/** Gets events that will be notified.

Retrieves the packet process events mask.  When this packet
is processed this mask will be checked and notifications will be
issued only if the appropriate bits are set.

@return The events that will be notified.
*/
EXPORT_C TObexPacketProcessEvents CObexPacket::PacketProcessNotificationEvents() const
	{
	LOG_LINE
	LOG_FUNC
	return iNotificationEvents;
	}

/**
Logs the size of the packet. 
If __LOG_PACKET_DUMP__ is defined (obex/common/logger.h) then also logs the 
packet's contents.
*/
EXPORT_C void CObexPacket::Dump () const
	{
	LOG1(_L8("PacketSize = %d, Packet Dump:"), PacketSize ());
#ifdef __LOG_PACKET_DUMP__
	LOGHEXRAW(iBuffer, iBufferSize);
#endif
	}
