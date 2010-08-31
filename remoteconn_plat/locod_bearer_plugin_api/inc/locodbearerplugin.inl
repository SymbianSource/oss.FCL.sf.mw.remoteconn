/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  This is the LCD Bearer Plugin construction parameters
*                declaration
*
*/


#include <ecom.h>

// -----------------------------------------------------------------------------
// Creates the bearer plug ins and return it to caller
// -----------------------------------------------------------------------------
//
inline CLocodBearerPlugin* CLocodBearerPlugin::NewL(TLocodBearerPluginParams& aParams)
    {
	CLocodBearerPlugin* self = reinterpret_cast<CLocodBearerPlugin*>(
		REComSession::CreateImplementationL(
			aParams.ImplementationUid(),
			_FOFF(CLocodBearerPlugin, iInstanceUid),
			(TAny*)&aParams)
		);

	return self;
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
inline CLocodBearerPlugin::~CLocodBearerPlugin()
    {
    REComSession::DestroyedImplementation(iInstanceUid);
    }

// -----------------------------------------------------------------------------
// Return the implemtation uid
// -----------------------------------------------------------------------------
//
inline TUid CLocodBearerPlugin::ImplementationUid() const
    {
    return iImplementationUid;
    }

// -----------------------------------------------------------------------------
// Constructor method, just saves the arguments into member variables
// -----------------------------------------------------------------------------
//
inline CLocodBearerPlugin::CLocodBearerPlugin(TLocodBearerPluginParams& aParams)
    : iImplementationUid(aParams.ImplementationUid()),
      iObserver(aParams.Observer())
    {
    }

// -----------------------------------------------------------------------------
// Return the observer class
// -----------------------------------------------------------------------------
//
inline MLocodBearerPluginObserver& CLocodBearerPlugin::Observer() const
    {
    return iObserver;
    }
