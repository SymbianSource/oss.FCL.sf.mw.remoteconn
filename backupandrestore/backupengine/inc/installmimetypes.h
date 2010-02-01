/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/



/**
 @file
 @publishedPartner
 @released
*/

#ifndef __INSTALLMIMETYPES_H__
#define __INSTALLMIMETYPES_H__

#include <e32cmn.h>

namespace conn
	{
	_LIT8(KSisDataType, "x-epoc/x-sisx-app");
	_LIT8(KJavaDataType1, "application/java-archive");
	_LIT8(KJavaDataType2, "application/vnd.symbian.install");
	_LIT8(KJavaDataType3, "text/vnd.sun.j2me.app-descriptor");
	_LIT8(KJavaDataType4, "application/x-java-archive");
	
	_LIT8(KJarXMimeTypeCAF, "x-caf-application/x-java-archive");
	_LIT8(KJarMimeTypeCAF, "x-caf-application/java-archive");
	_LIT8(KJadMimeTypeCAF, "x-caf-text/vnd.sun.j2me.app-descriptor");
	} // conn

#endif // __INSTALLMIMETYPES_H__
