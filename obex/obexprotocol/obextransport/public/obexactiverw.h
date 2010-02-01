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

#ifndef __OBEXACTIVERW_H__
#define __OBEXACTIVERW_H__

/**
@file
@internalTechnology
*/

#include <e32def.h>
#include <e32base.h>

class MObexTransportNotify;
struct TObexConnectionInfo;
class CObexPacket;

/**
Base class for a specific transport's active reader and writer classes. 
The derived classes are responsible for sending and receiving data for their 
transport. 

@see CObexReaderBase
@see CObexWriterBase
*/
NONSHARABLE_CLASS(CObexActiveRW) : public CActive
	{
public:
	CObexActiveRW(TPriority aP, MObexTransportNotify& aOwner, TObexConnectionInfo& aInfo);
	~CObexActiveRW();
	void Transfer(CObexPacket& aPacket);

protected: // functions to be implemented by derived classes
	/**
	This method does any required initialisation of data for the transfer, 
	including the received, aPacket, before calling NewRequest.
	
	@see Transfer
	@see NewRequest(CObexPacket& aPacket)
	@param aPacket The packet to read into or written
	*/
	virtual void StartTransfer(CObexPacket& aPacket) =0; //called by non virtual Transfer

	/**
	This method issues either a read or write of data over the relevant 
	transport object. This method is called from NewRequest.
	
	@see NewRequest
	*/
	virtual void DoTransfer () =0;

	/**
	This method compares the number of bytes, iCount, read/ written, against 
	the packet size. In the case of a write, ETrue is returned if all the 
	bytes have been written, otherwise EFalse.
	
	For reads, check if the header is received and if so assign iPacketSize 
	with the packet size indicated in the header of the received packet. Note 
	that the claimed packet size must be greater than the required obex header 
	size and no more than the maximum packet size this transport handles.
	
	@see CObexTransportControllerBase::Process()
	@return TBool return ETrue if the transfer has completed else EFalse
	*/
	virtual TBool CompleteTransfer () =0; // check if packet read/write has completed

	/**
	This method is called when a transfer is complete. Any actions associated 
	with completion of a transfer should be performed here, for instance 
	sending the packet to the owner of the reader to be processed.
	*/
	virtual void OnCompleteTransfer() =0; // perform action associated with read/write completion

	/**
	Return the number bytes left to transfer
	@return TInt the number of bytes left to be transferred
	*/
	virtual TInt Remaining () =0; // how much is left to transfer
	
	//function used by derived classes
	void NewRequest (CObexPacket& aPacket); // start new packet send/receive
	
	//RunL for derived classes
	IMPORT_C void RunL ();
	
// Data
protected:
	/**The owner of the derived reader or writer class. Required at construction.*/
	MObexTransportNotify& iOwner;

	/**The obex packet, received from the owner, that will be sent or written into iLocation.*/
	CObexPacket* iPacket;
	
	/**A pointer to the iPacket buffer at the point where data is to be written 
	from or read into. The maximum length is the remaining bytes to be read or 
	written. Data is read from this buffer into the transport, or written from 
	the transport into this buffer.	*/
	TPtr8 iLocation;
	
	/**The number of bytes that have been read or written*/
	TInt iCount;
	
	/**The size of the obex packet	*/
	TInt iPacketSize;
	
	/**Is true if the transport is a packet based transport*/
	TBool iPacketBased;
	};

#endif // __OBEXACTIVERW_H__
