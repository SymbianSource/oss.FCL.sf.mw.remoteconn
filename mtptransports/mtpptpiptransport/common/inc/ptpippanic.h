// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef PTPIPPANIC_H
#define PTPIPPANIC_H

#include <e32std.h>

_LIT(KPTPIPPanic, "PTPIP");

enum TPTPIPPanic
    {
    EPTPIPReserved					= 0,
    EPTPIPBadState    	 			= 1,
    EPTPIPNotSupported 				= 2,
    EPTPIPConnectionAlreadyExist	= 3
    };

inline void Panic(TPTPIPPanic aCode)
    {
    User::Panic(KPTPIPPanic, aCode);
    };

#endif // PTPIPPANIC_H