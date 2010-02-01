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
 @publishedPartner
*/

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypeint32.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeInt32::TMTPTypeInt32() :
    TMTPTypeIntBase(0, KMTPTypeINT32Size, EMTPTypeINT32)
    {
    
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeInt32::TMTPTypeInt32(TInt32 aData) : 
    TMTPTypeIntBase(aData, KMTPTypeINT32Size, EMTPTypeINT32)
    {
    
    }

/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeInt32::Set(TInt32 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TInt32 TMTPTypeInt32::Value() const
	{
	return static_cast<TInt32>(iData);
	}

