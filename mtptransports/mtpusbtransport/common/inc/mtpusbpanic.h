// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef MTPUSBPANIC_H
#define MTPUSBPANIC_H

#include <e32std.h>

enum TMTPUsbPanic
    {
    EMTPUsbReserved					= 0,
    
    EMTPUsbBadState    	 			= 1,
    EMTPUsbNotSupported 			= 2,
    EMTPUsbConnectionAlreadyExist 	= 3
    };

_LIT(KMTPUsbPanic, "MTPUsb");
inline void Panic(TMTPUsbPanic aCode)
    {
    User::Panic(KMTPUsbPanic, aCode);
    };

#endif // MTPUSBPANIC_H
