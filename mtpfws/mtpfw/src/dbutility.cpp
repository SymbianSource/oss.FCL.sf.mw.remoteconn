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


#include "dbutility.h"
#include "d32dbms.h"
    
/**
Check if the table exists in the database.
@param aDatabase  the database to use
@param aTableName the table name to check
@return ETrue if exists, otherwise, EFalse
*/
TBool DBUtility::IsTableExistsL(RDbDatabase& aDatabase, const TDesC& aTableName)
	{
	TBool result = EFalse;
	CDbTableNames* tableNames = aDatabase.TableNamesL();
	CleanupStack::PushL(tableNames);
	if(tableNames)
		{
		const TInt count = tableNames->Count();
		for(TInt i = 0; i < count; i++)
			{
			if((*tableNames)[i] == aTableName)
				{
				result = ETrue;
				break;
				}
			}
		}
	CleanupStack::PopAndDestroy(tableNames);
	return result;	
	}

/**
Check if the specified index exists in the table.
@param aDatabase  the database to use
@param aTableName the table name to use
@param aIndexName the index name to check
@return ETrue if the index exists, otherwise, EFalse
*/
TBool DBUtility::IsIndexExistsL(RDbDatabase& aDatabase, const TDesC& aTableName, const TDesC& aIndexName)
	{
	TBool result = EFalse;
	CDbIndexNames* indexNames = aDatabase.IndexNamesL(aTableName);
	CleanupStack::PushL(indexNames);
	if(indexNames)
		{
		const TInt count = indexNames->Count();
		for(TInt i = 0; i < count; i++)
			{
			if((*indexNames)[i] == aIndexName)
				{
				result = ETrue;
				break;
				}
			}
		}
	CleanupStack::PopAndDestroy(indexNames);
	return result;	
	}
