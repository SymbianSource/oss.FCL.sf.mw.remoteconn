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
#ifndef DBUTILITY_H
#define DBUTILITY_H

#include <e32base.h>

class RDbDatabase;

/** 
Uitlity class for database management.

@internalComponent
 
*/
class DBUtility
	{
public:	
    static TBool IsTableExistsL(RDbDatabase&, const TDesC& aTableName);
    static TBool IsIndexExistsL(RDbDatabase&, const TDesC& aTableName, const TDesC& aIndexName);
	};
	
#endif
