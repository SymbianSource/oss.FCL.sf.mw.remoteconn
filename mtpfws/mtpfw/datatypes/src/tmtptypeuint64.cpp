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
#include <mtp/tmtptypeuint64.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeUint64::TMTPTypeUint64() :
    iData(0)
    {
    
    }

/**
Conversion constructor from native type.
*/
EXPORT_C TMTPTypeUint64::TMTPTypeUint64(TUint64 aData) : 
    iData(aData)
    {
    
    }
    
/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeUint64::Set(TUint64 aValue)
	{
	iData = aValue;
	}
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TUint64 TMTPTypeUint64::Value() const
	{
	return iData;
	}
	
EXPORT_C TInt TMTPTypeUint64::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(reinterpret_cast<const TUint8*>(&iData), sizeof(iData));
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUint64::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeUint64::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(reinterpret_cast<TUint8*>(&iData), 0, sizeof(iData));
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUint64::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }
	
EXPORT_C TUint64 TMTPTypeUint64::Size() const
	{
	return sizeof(iData);
	}
	
EXPORT_C TUint TMTPTypeUint64::Type() const
	{
	return EMTPTypeUINT64;
	}
