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
#include <mtp/tmtptypeuint32.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeUint32::TMTPTypeUint32() :
    TMTPTypeUintBase(0, KMTPTypeUINT32Size, EMTPTypeUINT32)
    {
    
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeUint32::TMTPTypeUint32(TUint32 aData) : 
    TMTPTypeUintBase(aData, KMTPTypeUINT32Size, EMTPTypeUINT32)
    {
    
    }

/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeUint32::Set(TUint32 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TUint32 TMTPTypeUint32::Value() const
	{
	return static_cast<TUint32>(iData);
	}

