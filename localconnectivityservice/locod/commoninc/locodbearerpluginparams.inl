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


// -----------------------------------------------------------------------------
// Constructor method, just saves the arguments into member variables
// -----------------------------------------------------------------------------
//
inline TLocodBearerPluginParams::TLocodBearerPluginParams(const TUid aUid, MLocodBearerPluginObserver& aObserver)
    : iImplementationUid(aUid),
      iObserver(aObserver)
    {
    }

// -----------------------------------------------------------------------------
// returns the implementation UID
// -----------------------------------------------------------------------------
//
inline TUid TLocodBearerPluginParams::ImplementationUid() const
    {
    return iImplementationUid;
    }

// -----------------------------------------------------------------------------
// returns the  observer class
// -----------------------------------------------------------------------------
//
inline MLocodBearerPluginObserver& TLocodBearerPluginParams::Observer() const
    {
    return iObserver;
    }
