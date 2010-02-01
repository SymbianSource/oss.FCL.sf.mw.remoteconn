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
 @internalTechnology
*/

#ifndef MTPDPPANIC_H
#define MTPDPPANIC_H

#include <e32std.h>

/**
Defines the MTP Data Provider category panic codes.
@internalTechnology
*/
enum TMTPDpPanic
	{
	EMTPDpNoMatchingProcessor = 0,
	EMTPDpObjectNull = 1,
	EMTPDpFormatNotSupported = 2,
	EMTPDpSendObjectStateInvalid = 3,
	EMTPDpReferencesNull = 4,
	EMTPDpRequestCheckNull = 5,
	EMTPDpHandlesNull = 6,
	EMTPDpUnsupportedProperty = 7,
	EMTPDpObjectPropertyNull = 8,
	EMTPDpCannotRenameDir = 9,
	};

_LIT(KMTPDpPanic, "MTP Data Provider");
inline void Panic(TMTPDpPanic aCode)
	{
	User::Panic(KMTPDpPanic, aCode);
	};
	

#endif // MTPDPPANIC_H