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
#include <mtp/tmtptypeuint8.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeUint8::TMTPTypeUint8() :
    TMTPTypeUintBase(0, KMTPTypeUINT8Size, EMTPTypeUINT8)
    {
    
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeUint8::TMTPTypeUint8(TUint8 aData) : 
    TMTPTypeUintBase(aData, KMTPTypeUINT8Size, EMTPTypeUINT8)
    {
    
    }

/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeUint8::Set(TUint8 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TUint8 TMTPTypeUint8::Value() const
	{
	return static_cast<TUint8>(iData);
	}

