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
* Description:  This is the LCD Service Plugin construction parameters
*                declaration
*
*/


#include <ecom.h>

// -----------------------------------------------------------------------------
// Constructor method, just saves the arguments into member variables
// -----------------------------------------------------------------------------
//
inline CLocodServicePlugin* CLocodServicePlugin::NewL(TLocodServicePluginParams& aParams)
    {
	CLocodServicePlugin* self = reinterpret_cast<CLocodServicePlugin*>(
		REComSession::CreateImplementationL(
			aParams.ImplementationUid(),
			_FOFF(CLocodServicePlugin, iInstanceUid),
			(TAny*)&aParams)
		);

	return self;
    }
    

// -----------------------------------------------------------------------------
// Service plug ins destructor
// -----------------------------------------------------------------------------
//
inline CLocodServicePlugin::~CLocodServicePlugin()
    {
    REComSession::DestroyedImplementation(iInstanceUid);
    }
    
// -----------------------------------------------------------------------------
// returns the implementation UID
// -----------------------------------------------------------------------------
//
inline TUid CLocodServicePlugin::ImplementationUid() const
    {
    return iImplementationUid;
    }

// -----------------------------------------------------------------------------
// CLocodServicePlugin C++ constructor
// -----------------------------------------------------------------------------
//
inline CLocodServicePlugin::CLocodServicePlugin(TLocodServicePluginParams& aParams)
    : iImplementationUid(aParams.ImplementationUid()),
      iObserver(aParams.Observer())
    {
    }

// -----------------------------------------------------------------------------
// return the observer class
// -----------------------------------------------------------------------------
//
inline MLocodServicePluginObserver& CLocodServicePlugin::Observer() const
    {
    return iObserver;
    }

