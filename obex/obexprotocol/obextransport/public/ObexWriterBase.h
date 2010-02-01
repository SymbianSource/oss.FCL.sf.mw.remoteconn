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

#ifndef __OBEXWRITERBASE_H__
#define __OBEXWRITERBASE_H__

#include <obex/internal/obexactiverw.h>

/**
@file
@publishedPartner
@released
*/

/**
CObexWriterBase provides obex specific writer behaviour for concrete writers to use.  
Derive from this class when providing a writer for a specific transport.  

In a concrete writer the following things must be done:
(a)In DoTransfer a write transfer is issued over the writer's transport
(b)The NewL function must call BaseConstructL().  This is future proofing in the event that the iFuture1 variable is used
and requires initialisation
*/
class CObexWriterBase : public CObexActiveRW
	{
public:
	IMPORT_C virtual ~CObexWriterBase();

protected:
	IMPORT_C CObexWriterBase(TPriority aPriority, MObexTransportNotify& aOwner, TObexConnectionInfo& aInfo);
	IMPORT_C void BaseConstructL();

protected: // from CObexActiveRW
	IMPORT_C virtual TBool CompleteTransfer();
	IMPORT_C virtual void OnCompleteTransfer();
	IMPORT_C virtual void StartTransfer(CObexPacket& aPacket);
	IMPORT_C virtual TInt Remaining();
	/**
	This method issues a write of data over the relevant transport object.
	@see NewRequest
	*/
	virtual void DoTransfer () =0;

	//ExtensionPattern
	IMPORT_C virtual TAny* GetInterface(TUid aUid);

protected: // owned
	// For future data expansion
	TAny* iFuture1;
	};

#endif // __OBEXWRITERBASE_H__
