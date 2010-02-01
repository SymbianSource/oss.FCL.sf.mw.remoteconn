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
 @internalTechnology
*/

#ifndef __MOBEXCLIENTERRORRESOLVER_H
#define __MOBEXCLIENTERRORRESOLVER_H

#include <obex/extensionapis/obexclienterrorresolver.h>

/**
UID with which applications can request the MObexClientErrorResolver API
through CObexClient::ExtensionInterface().
*/
const TUid KObexClientErrorResolverInterface = {0x10281918};

/**
Definition of the Obex client error extension API.
*/
class MObexClientErrorResolver
	{
public:
	/**
	Function used to retreive an extended error code for the last completed client operation.

	@param aErrorSet The set of errors that the resolved error will lie within.
	@return An extended error code that lies within the error set specified.
	*/
	virtual TUint LastError(TObexClientErrorResolutionSetType aErrorSet) const = 0;
	};

#endif __MOBEXCLIENTERRORRESOLVER_H
