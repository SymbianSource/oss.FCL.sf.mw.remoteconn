// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef MTPIMAGEDPPANIC_H
#define MTPIMAGEDPPANIC_H

#include <e32std.h>

/**
define the file data provider panic code
@internalTechnology

*/
enum TMTPImageDpPanic
	{
	EMTPImageDpNoMatchingProcessor = 0,
	EMTPImageDpObjectNull = 1,
	EMTPImageDpFormatNotSupported = 2,
	EMTPImageDpSendObjectStateInvalid = 3,
	EMTPImageDpRequestCheckNull = 4,
	EMTPImageDpHandlesNull = 5,
	EMTPImageDpUnsupportedProperty = 6,
	EMTPImageDpObjectPropertyNull = 7,
	EMTPImageDpCannotRenameDir = 8,
	EMTPImageDpIPropertyNotSupported = 9,
	EMTPImageDpBadLayout     = 10,
	EMTPImageDpTypeMismatch  = 11,
	};

_LIT(KMTPImageDpPanic, "MTP Image Data Provider");

inline void Panic(TMTPImageDpPanic aCode)
	{
	User::Panic(KMTPImageDpPanic, aCode);
	};
	

#endif // MTPIMAGEDPPANIC_H

