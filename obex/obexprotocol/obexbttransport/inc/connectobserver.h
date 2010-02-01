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
// Interface for notification of the completion of a connect request.
// 
//

/**
 @file
*/

#ifndef CONNECTOBSERVER_H
#define CONNECTOBSERVER_H

#include <e32base.h>

class TObexConnectionInfo;

class MConnectObserver
	{
public:
	/** A connect request has completed with the given error. If this is 
	KErrNone, aSockinfo will hold connection information. */
	virtual void ConnectComplete(TInt aError, TObexConnectionInfo& aSockinfo) = 0;
	};

#endif // CONNECTOBSERVER_H
