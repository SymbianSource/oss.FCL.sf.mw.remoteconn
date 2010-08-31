/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definitions for server
*
*/


#ifndef DUN_SERVERDEF_H
#define DUN_SERVERDEF_H

//  INCLUDES
#include <e32base.h>
#include <e32svr.h>

// DATA TYPES
enum TDunIpc
    {
    EDunFuncManageService,
    EDunFuncActiveConnection,  // For testing purposes only
    EInvalidIpc
    };

// GLOBAL CONSTANTS

_LIT( KDialupServerName, "dunserver" );

const TUint KDunServerMajorVersionNumber = 2;
const TUint KDunServerMinorVersionNumber = 0;
const TUint KDunServerBuildVersionNumber = 0;

#endif  // DUN_SERVERDEF_H
