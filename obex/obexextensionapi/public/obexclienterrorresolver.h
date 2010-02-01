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

/**
 @file
 @publishedPartner
 @released
*/

#ifndef __OBEXCLIENTERRORRESOLVER_H
#define __OBEXCLIENTERRORRESOLVER_H

#include <obexclient.h>

class MObexClientErrorResolver;

/**
Types of error set that an Obex client error can be resolved to.
*/
enum TObexClientErrorResolutionSetType
	{
	/**
	The basic error set that errors can be resolved to.
	@see TObexClientBasicErrorResolutionSet
	*/
	EObexClientBasicErrorResolutionSet = 0,
	};

/**
The basic error set that errors can be resolved to.
*/
enum TObexClientBasicErrorResolutionSet
	{
	/**
	The last client operation completed without error.
	This value is unused but remains here for compatibility.
	*/
	EObexNoExtendedError = 0,

	/**
	The last client operation completed without error.
	*/
	EObexRequestAccepted = 1,

	/**
	The last client operation was unauthorised or resulted in
	an error being returned from the server.
	*/
	EObexRequestNotAccepted = 2,

	/**
	The last client operation timed out.
	*/
	EObexRequestTimeout = 3,

	/**
	The last client operation was interrupted by an error
	detected on the client side, which includes the following:
	- Cancellation of the operation (Abort)
	- Obex protocol error
	- Out of memory condition
	*/
	EObexRequestLocalInterruption = 4,

	/**
	The last client operation was interrupted due to the
	transport link going down.  This may be caused by:
	- Failure to find a remote device capable of Obex
	- Blocking of the IrDA beam
	*/
	EObexRequestLinkInterruption = 5,
	};

/**
Extension class for CObexClient that allows Obex client applications to
retrieve an extended error code for the last completed client operation.
*/
NONSHARABLE_CLASS(CObexClientErrorResolver) : public CBase
	{
public:
	IMPORT_C static CObexClientErrorResolver* NewL(CObexClient& aClient);
	IMPORT_C static CObexClientErrorResolver* NewL(CObexClient& aClient, TObexClientErrorResolutionSetType aErrorSet);
	IMPORT_C TUint LastError() const;

private:
	// Private constructor - this class is not intended for derivation.
	CObexClientErrorResolver(CObexClient& aClient, TObexClientErrorResolutionSetType aErrorSet);
	void ConstructL();

private:
	CObexClient& iObexClient;
	MObexClientErrorResolver* iClientErrorResolver;
	TObexClientErrorResolutionSetType iErrorSet;
	};

#endif // __OBEXCLIENTERRORRESOLVER_H
