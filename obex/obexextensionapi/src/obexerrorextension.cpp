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
 @internalComponent
*/

#include <obex/extensionapis/obexclienterrorresolver.h>
#include <obex/internal/mobexclienterrorresolver.h>

/**
Factory function for CObexClientErrorResolver.  Will leave if the associated CObexClient
does not support this extension type.

@param aClient Reference to the instance of CObexClient being extended.
@return A pointer to a new CObexClientErrorResolver object.
*/
EXPORT_C CObexClientErrorResolver* CObexClientErrorResolver::NewL(CObexClient& aClient)
	{
	return NewL(aClient, EObexClientBasicErrorResolutionSet);
	}

/**
Factory function for CObexClientErrorResolver.  Will leave if the associated CObexClient
does not support this extension type.

@param aClient Reference to the instance of CObexClient being extended.
@param aErrorSet The set of errors that the resolved error will lie within.
@return A pointer to a new CObexClientErrorResolver object.
*/
EXPORT_C CObexClientErrorResolver* CObexClientErrorResolver::NewL(CObexClient& aClient, TObexClientErrorResolutionSetType aErrorSet)
	{
	CObexClientErrorResolver* self = new (ELeave)CObexClientErrorResolver(aClient, aErrorSet);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
Function used to retreive an extended error code for the last completed client operation.

@return An extended error code that lies within the error set specified at construction time.
*/
EXPORT_C TUint CObexClientErrorResolver::LastError() const
	{
	return iClientErrorResolver->LastError(iErrorSet);
	}

CObexClientErrorResolver::CObexClientErrorResolver(CObexClient& aClient, TObexClientErrorResolutionSetType aErrorSet)
	: iObexClient(aClient), iErrorSet(aErrorSet)
	{
	}

void CObexClientErrorResolver::ConstructL()
	{
	iClientErrorResolver = static_cast<MObexClientErrorResolver*>(iObexClient.ExtensionInterface(KObexClientErrorResolverInterface));
	User::LeaveIfNull(iClientErrorResolver);
	}
