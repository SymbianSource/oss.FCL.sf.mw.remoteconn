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


// -----------------------------------------------------------------------------
// Constructor method, just saves the arguments into member variables
// -----------------------------------------------------------------------------
//
inline TLocodServicePluginParams::TLocodServicePluginParams(TUid aUid, MLocodServicePluginObserver& aObserver)
    : iImplementationUid(aUid),
      iObserver(aObserver)
    {
    }

// -----------------------------------------------------------------------------
// Returns the implementation UID
// -----------------------------------------------------------------------------
//
inline TUid TLocodServicePluginParams::ImplementationUid() const
    {
    return iImplementationUid;
    }

// -----------------------------------------------------------------------------
// return the observer class
// -----------------------------------------------------------------------------
//
inline MLocodServicePluginObserver& TLocodServicePluginParams::Observer() const
    {
    return iObserver;
    }
