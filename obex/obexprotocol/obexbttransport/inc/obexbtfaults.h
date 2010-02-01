// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __OBEXBTFAULTS_H
#define __OBEXBTFAULTS_H

/**
@file

This file contains bt specific panics internal to obex
@internalComponent
*/

enum TObexBtFaultCode
	{
	EActiveRfcommConnInternalError = 0,
	EListRfcommInternalError = 1,
	ERfcommConnectorInternalError = 2,
	ERfcommTcInternalError = 3,
	};

#endif // __OBEXBTFAULTS_H
