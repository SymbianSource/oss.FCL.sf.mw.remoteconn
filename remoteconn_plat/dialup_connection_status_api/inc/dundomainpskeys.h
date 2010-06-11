/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     DUN Server P&S key definitions
*
*/


#ifndef DUN_DOMAIN_PS_KEYS_H
#define DUN_DOMAIN_PS_KEYS_H

/**
* PubSub Uid of dialup connection status
*/
const TUid KPSUidDialupConnStatus = {0x0100387d};  // UID of DUN server

/**
* Dialup connection status
*
* Possible integer values:
* 0 (EDunUndefined)  Value is not set
* 1 (EDunInactive)   Dun is inactive  (no listening or active plugins)
* 2 (EDunActive)     Dun is active    (at least one active plugin)
*/

const TUint KDialupConnStatus = 0x00000001;

// Enumeration for DUN connection status
enum TDialupConnStatus
    {
    EDialupUndefined,
    EDialupInactive,
    EDialupActive
    };

#endif // DUN_DOMAIN_PS_KEYS_H
