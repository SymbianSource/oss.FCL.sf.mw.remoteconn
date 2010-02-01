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

#ifndef MTPDATATYPESPANIC_H
#define MTPDATATYPESPANIC_H

#include <e32std.h>

enum TMTPDataTypesPanic
	{
	EMTPTypeReserved        = 0,
	
	EMTPTypeBadStorage      = 1,
	EMTPTypeBoundsError     = 2,
	EMTPTypeIdMismatch      = 3,
	EMTPTypeSizeMismatch    = 4,
	EMTPTypeIdUnknown       = 5,
	EMTPTypeNotSupported    = 6,
	EMTPTypeInUse           = 7,
	};

_LIT(KMTPDataTypesPanic, "MTPDataTypes");
inline void Panic(TMTPDataTypesPanic aCode)
	{
	User::Panic(KMTPDataTypesPanic, aCode);
	};
	

#endif // MTPDATATYPESPANIC_H

