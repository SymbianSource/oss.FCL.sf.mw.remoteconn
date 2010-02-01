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

#ifndef __OBEXPACKET_H__
#define __OBEXPACKET_H__

#include <e32base.h>
#include <obex/internal/obexinternalconstants.h>
#include <obex/internal/obextransportconstants.h>
#include <obextypes.h>

class TObexData;

/**	
Packets, which are the basic unit transfered between CObex and the transport
@internalTechnology
*/ 
NONSHARABLE_CLASS(CObexPacket) : public CBase
	{
friend class CObexActiveRW;

public:
	IMPORT_C ~CObexPacket ();
	IMPORT_C static CObexPacket* NewL (TUint16 aBufferSize, TUint16 aDataLimit);
													  
	// Functions to set packet data
	IMPORT_C void Init (TObexOpcode aOpcode);
	
	IMPORT_C TObexOpcode Opcode () const;
	IMPORT_C void SetOpcode (TObexOpcode aOpcode);

	IMPORT_C TBool IsFinal () const;
	IMPORT_C void SetFinal (TBool aFinal = ETrue);
	
	// Get/Set the current packet size
	// These refer size encoded in the OBEX packet itself (not the buffer used to contain the packet)
    	IMPORT_C TUint16 PacketSize () const;
	IMPORT_C void SetPacketSize (TUint16 aSize);

	// Get/Set the buffer size/data limit
	IMPORT_C TUint16 SetLegalDataLimit (TUint16 aSize);
	IMPORT_C TUint16 DataLimit () const; 
	IMPORT_C TUint16 BufferSize () const;

	// Setters & Getters, payload
	IMPORT_C TBool InsertData (const TObexData& aHeader);
	IMPORT_C TBool ExtractData (TObexData& aHeader);
	IMPORT_C TUint16 RemainingInsertSpace () const;
	IMPORT_C TUint16 RemainingExtractSpace () const;
	IMPORT_C void AddPacketProcessEvents(TObexPacketProcessEvents aEvents);
	IMPORT_C void RemovePacketProcessEvents(TObexPacketProcessEvents aEvents);
	IMPORT_C TObexPacketProcessEvents PacketProcessNotificationEvents() const;
	IMPORT_C void Dump() const;

private:
	CObexPacket (TUint16 aBufferSize, TUint16 aDataLimit);
	void ConstructL ();

	inline const TUint8* Payload () const {return (&iBuffer[KObexPacketHeaderSize]);};
	inline TUint8* Payload () {return (&iBuffer[KObexPacketHeaderSize]);};
	inline TUint16 PayloadSize () const {return (STATIC_CAST(TUint16, PacketSize () - KObexPacketHeaderSize));};


// Data
private:
	TUint16 iBufferSize;	// how big the buffer actually is
	TUint16 iDataLimit;		// how large the data can be (usually set per conenction)
	TUint8* iInsertPoint;
	TUint8* iExtractPoint;
	TUint8* iBuffer;
	TObexPacketProcessEvents iNotificationEvents;
	};	// CObexPacket
	
#endif // __OBEXPACKET_H__
