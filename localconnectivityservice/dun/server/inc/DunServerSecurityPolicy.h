/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN server security policy definition
*
*/


#ifndef DUNSERVER_SECURITYPOLICY_H
#define DUNSERVER_SECURITYPOLICY_H

#include "dunserverdef.h"

// First range for KServerRanges[] definition, do not change
const TInt KFirstServerRange  = 0;

// First range for KServerElementsIndex[] definition, do not change
const TInt KFirstElementRange = 0;

// ---------------------------------------------------------------------------
// DunServer's policy
// ---------------------------------------------------------------------------
//

// Definition of the ranges of IPC numbers
const TInt KServerRanges[] =
    {
    KFirstServerRange,  // Range 1: all client APIs
    EInvalidIpc         // Range 2: Invalid IPC opcode
    };

const TUint KServerRangeCount = sizeof( KServerRanges ) / sizeof( TInt );

// Policy to implement for each of the above ranges
const TUint8 KServerElementsIndex[KServerRangeCount] =
    {
    KFirstElementRange,           // applies to 0th range
    CPolicyServer::ENotSupported  // out of range DUN's IPC
    };

//Specific capability checks
const CPolicyServer::TPolicyElement KServerElements[] =
    {
        {_INIT_SECURITY_POLICY_C2(ECapabilityLocalServices, ECapabilityNetworkServices),
         CPolicyServer::EFailClient
        } // Policy 0
    };

// Policy structure for DunServer. This is passed to DunServer's constructor
const CPolicyServer::TPolicy KDunServerPolicy =
        {
        CPolicyServer::EAlwaysPass,  // Indicates that Connect attempts should be processed without any further checks.
        KServerRangeCount,           // Range count
        KServerRanges,               // ranges array
        KServerElementsIndex,        // elements' index
        KServerElements              // array of elements
        };

#endif // DUNSERVER_SECURITYPOLICY_H
