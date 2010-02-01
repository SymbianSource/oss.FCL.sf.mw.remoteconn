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
// mmtpfiledppanic.h
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __MTPFILEDPPANIC_H__
#define __MTPFILEDPPANIC_H__

#include <e32std.h>

/**
define the file data provider panic code
@internalTechnology

*/
enum TMTPFileDpPanic
	{
	EMTPFileDpNoMatchingProcessor = 0,
	EMTPFileDpObjectNull = 1,
	EMTPFileDpFormatNotSupported = 2,
	EMTPFileDpSendObjectStateInvalid = 3,
	EMTPFileDpRequestCheckNull = 4,
	EMTPFileDpHandlesNull = 5,
	EMTPFileDpUnsupportedProperty = 6,
	EMTPFileDpObjectPropertyNull = 7,
	EMTPFileDpCannotRenameDir = 8,
	};

_LIT(KMTPFileDpPanic, "MTP File Data Provider");
inline void Panic(TMTPFileDpPanic aCode)
	{
	User::Panic(KMTPFileDpPanic, aCode);
	};
	

#endif // __MTPDEVDPPANIC_H__

