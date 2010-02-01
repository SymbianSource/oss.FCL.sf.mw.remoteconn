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
#include <mtp/tmtptypeint8.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeInt8::TMTPTypeInt8() :
    TMTPTypeIntBase(0, KMTPTypeINT8Size, EMTPTypeINT8)
    {
    
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeInt8::TMTPTypeInt8(TInt8 aData) : 
    TMTPTypeIntBase(aData, KMTPTypeINT8Size, EMTPTypeINT8)
    {
    
    }

/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeInt8::Set(TInt8 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TInt8 TMTPTypeInt8::Value() const
	{
	return static_cast<TInt8>(iData);
	}

