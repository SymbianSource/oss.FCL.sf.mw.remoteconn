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
#include <mtp/tmtptypeint128.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeInt128::TMTPTypeInt128()
    {
    iData.FillZ(iData.MaxLength());
    }

/**
Conversion constructor.
*/
EXPORT_C TMTPTypeInt128::TMTPTypeInt128(const TPtrC8& aData) : 
    iData(aData)
    {
    }
    
/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeInt128::Set(TInt64 aUpperValue, TUint64 aLowerValue)
	{
    memcpy(&iData[KMTPTypeInt128OffsetMS], &aUpperValue, sizeof(aUpperValue));
    memcpy(&iData[KMTPTypeInt128OffsetLS], &aLowerValue, sizeof(aLowerValue));
	}

/**
Provides the least significant portion of the data type.
@return The least significant 64 bits of the value.
*/    
EXPORT_C TUint64 TMTPTypeInt128::LowerValue() const
    {    
    // memcpy the data to avoid alignment errors.
    TInt64 ret;
    memcpy(&ret, &iData[KMTPTypeInt128OffsetLS], sizeof(ret));
    return ret;
    }

/**
Provides the most significant portion of type's value.
@return The most significant 64 bits of the value.
*/    
EXPORT_C TInt64 TMTPTypeInt128::UpperValue() const
    {
    // memcpy the data to avoid alignment errors.
    TInt64 ret;
    memcpy(&ret, &iData[KMTPTypeInt128OffsetMS], sizeof(ret));
    return ret;
    }  
	
EXPORT_C TInt TMTPTypeInt128::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(iData);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeInt128::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeInt128::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(&iData[0], 0, iData.MaxLength());
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeInt128::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }

EXPORT_C TUint64 TMTPTypeInt128::Size() const
    {
    return iData.MaxLength();
    }
    
EXPORT_C TUint TMTPTypeInt128::Type() const
    {
    return EMTPTypeINT128;
    }


