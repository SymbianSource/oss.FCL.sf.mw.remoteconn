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

#ifndef __OBEXREADERBASE_H__
#define __OBEXREADERBASE_H__

#include <obex/internal/obexactiverw.h>

/**
@file
@publishedPartner
@released
*/

/**
CObexReaderBase provides obex specific reader behaviour for concrete readers to use.  
Derive from this class when providing a reader for a specific transport.  

In a concrete reader the following things must be done:
(a) In DoTransfer a read data transfer is issued over the reader's transport
(b) GetMaxPacketSize must return the maximum packet size that can be received
(c) GetInitialPacketSize must return a packet size that can be used in the event  that the packet 
size of the packet to be read  is not yet known (for example irda and bt use the obexpacketheadersize and usb
uses the buffer size of the packet created to receive the data)
(d)The NewL function must call BaseConstructL().  This is future proofing in the event that the iFuture1 variable is used
and requires initialisation
*/
class CObexReaderBase : public CObexActiveRW
	{
public:
	IMPORT_C virtual ~CObexReaderBase();
	
protected:
	IMPORT_C CObexReaderBase(TPriority aPriority, MObexTransportNotify& aOwner, TObexConnectionInfo& aInfo);
	IMPORT_C void BaseConstructL();

protected: // from CObexActiveRW
	IMPORT_C virtual void StartTransfer (CObexPacket& aPacket);
	IMPORT_C TBool CompleteTransfer ();
	IMPORT_C void OnReadActivity();
	IMPORT_C virtual void OnCompleteTransfer();
	IMPORT_C TInt Remaining();
	IMPORT_C TInt GetObexPacketBufferSize();
	IMPORT_C TInt GetObexPacketHeaderSize();
	IMPORT_C TInt GetObexPacketDataLimit();
	/** This method issues either a read of data over the relevant  transport object.  	
	@see NewRequest*/
	virtual void DoTransfer () =0;
	
	//ExtensionPattern
	IMPORT_C virtual TAny* GetInterface(TUid aUid);
	/** Returns the maximum packet size that can be received */
	virtual TInt GetMaxPacketSize() = 0;
	
	/** Returns a initial packet size when the packet size of iPacket is not know.  This is used
	when determining the remaining bytes to be read. */
	virtual TInt GetInitialPacketSize() = 0;

private:
	TBool iGotHeader;
	TInt iTransferError;

	//For future data expansion
	TAny* iFuture1; 
	};

#endif // __OBEXREADERBASE_H__
