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
 @internalComponent
*/

//defined for testing purposes only
//#define TEST_CLIENT_CHANGES_ITS_MIND_ABOUT_CHALLENGE

#ifndef __OBEXUTIL_H
#define __OBEXUTIL_H

#include <obexconstants.h>
#include "obexconstantsinternal.h"
#include "obexfaults.h"

class TObexTransportInfo;

//this macro traps the leave and if an error has occured it calls the function defined by _trap
#define OBEX_TRAP(_trap, _stmt)  TRAPD(_code, _stmt);if(_code != KErrNone) _trap (_code)

inline TInt ObexHeaderType(TUint8 aHI)
	{
	return ((aHI & KObexHeaderTypeMask) >> KObexHeaderTypeOffset);
	}

NONSHARABLE_CLASS(IrOBEXUtil) 
	{
public:
 	static void Panic(TObexPanicCode aCode);
 	static void Fault(TObexFaultCode aCode);
	static TInt EpocError(TUint8 aResp);
	static TObexResponse ObexResponse(TInt aErr,TObexResponse aDefault);
	static TObexTransportInfo*  CreateTransportInfoL(TObexProtocolInfo& aObexProtocolInfoPtr, TObexProtocolPolicy& aObexProtocolPolicy);
	};
	
#endif // __OBEXUTIL_H
