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

#ifndef __OBEXACTIVEREADER_H__
#define __OBEXACTIVEREADER_H__

#include <obex/transport/obexreaderbase.h>

class RSocket;

/**
Functionality specific to reading from a socket 
@internalTechnology
*/
NONSHARABLE_CLASS(CObexActiveReader) : public CObexReaderBase
	{
public:
	IMPORT_C static CObexActiveReader* NewL(MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo);
	IMPORT_C ~CObexActiveReader ();

private:
	CObexActiveReader (MObexTransportNotify& aOwner, RSocket& aSocket, TObexConnectionInfo& aInfo);

private: // from CObexReaderBase
	virtual void DoCancel ();
	virtual void DoTransfer ();
	virtual TInt GetMaxPacketSize();
	virtual TInt GetInitialPacketSize();

private:
	RSocket& iSocket;
	};

#endif // __OBEXACTIVEREADER_H__
