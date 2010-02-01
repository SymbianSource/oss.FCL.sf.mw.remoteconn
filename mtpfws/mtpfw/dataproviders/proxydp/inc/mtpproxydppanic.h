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
// mmtpproxydppanic.h
// proxy data provider panic code  
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __MTPPROXYDPPANIC_H__
#define __MTPPROXYDPPANIC_H__

#include <e32std.h>

enum TMTPProxyDpPanic
	{
	EMTPNoMatchingProcessor = 0,
	EMTPNotSameRequestProxy = 1,
	EMTPWrongRequestPhase = 2,
	EMTPElementIndexOutOfBound = 3,
	EMTPInvalidDataType = 4,
	EMTPRoutingError = 5,
	};

_LIT(KMTProxyDpPanic, "MTP Proxy Data Provider");
inline void Panic(TMTPProxyDpPanic aCode)
	{
	User::Panic(KMTProxyDpPanic, aCode);
	};
	

#endif // __MTPPROXYDPPANIC_H__

