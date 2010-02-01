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
#include <mtp/tmtptypeuint16.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeUint16::TMTPTypeUint16() :
    TMTPTypeUintBase(0, KMTPTypeUINT16Size, EMTPTypeUINT16)
    {
    
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeUint16::TMTPTypeUint16(TUint16 aData) : 
    TMTPTypeUintBase(aData, KMTPTypeUINT16Size, EMTPTypeUINT16)
    {
    
    }

/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeUint16::Set(TUint16 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TUint16 TMTPTypeUint16::Value() const
	{
	return static_cast<TUint16>(iData);
	}

