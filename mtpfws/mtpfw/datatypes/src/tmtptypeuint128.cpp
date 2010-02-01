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
#include <mtp/tmtptypeuint128.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeUint128::TMTPTypeUint128()
    {
    iData.FillZ(iData.MaxLength());
    }

/**
Conversion constructor.
@param aData The initial data value.
*/
EXPORT_C TMTPTypeUint128::TMTPTypeUint128(const TPtrC8& aData) : 
    iData(aData)
    {
    }
    
/**
Conversion constructor.
@param aUpperGUID the most significant 64 bits: TUint32 and TUint16[2]
@param aLowerGUID the lowest  64 bits : TUint8[8]
*/
EXPORT_C TMTPTypeUint128::TMTPTypeUint128(const TUint64 aUpperValue, const TUint64 aLowerValue) 
    {  
    iData.FillZ(iData.MaxLength());
    memcpy(&iData[KMTPTypeUint128OffsetMS], &aUpperValue, sizeof(aUpperValue));
    memcpy(&iData[KMTPTypeUint128OffsetLS], &aLowerValue, sizeof(aLowerValue));
    }


/**
Sets the data type to the specified value.
*/  
EXPORT_C void TMTPTypeUint128::Set(TUint64 aUpperValue, TUint64 aLowerValue)
	{   
    memcpy(&iData[KMTPTypeUint128OffsetMS], &aUpperValue, sizeof(aUpperValue));
    memcpy(&iData[KMTPTypeUint128OffsetLS], &aLowerValue, sizeof(aLowerValue));
	}

/**
Provides the least significant portion of the data type.
@return The least significant 64 bits of the value.
*/    
EXPORT_C TUint64 TMTPTypeUint128::LowerValue() const
    {    
    // memcpy the data to avoid alignment errors.
    TUint64 ret;
    memcpy(&ret, &iData[KMTPTypeUint128OffsetLS], sizeof(ret));
    return ret;
    }

/**
Provides the most significant portion of type's value.
@return The most significant 64 bits of the value.
*/    
EXPORT_C TUint64 TMTPTypeUint128::UpperValue() const
    {
    // memcpy the data to avoid alignment errors.
    TUint64 ret;
    memcpy(&ret, &iData[KMTPTypeUint128OffsetMS], sizeof(ret));
    return ret;
    }

	
EXPORT_C TInt TMTPTypeUint128::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(iData);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUint128::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeUint128::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(&iData[0], 0, iData.MaxLength());
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUint128::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }

EXPORT_C TUint64 TMTPTypeUint128::Size() const
    {
    return iData.MaxLength();
    }
    
EXPORT_C TUint TMTPTypeUint128::Type() const
    {
    return EMTPTypeUINT128;
    }

EXPORT_C TBool TMTPTypeUint128::Equal(const TMTPTypeUint128& aR) const
    {
    return ( (this->UpperValue() == aR.UpperValue()) && (this->LowerValue() == aR.LowerValue()) );
    }

EXPORT_C TInt TMTPTypeUint128::Compare(const TMTPTypeUint128& aR) const
    {  
    return this->Compare(aR.UpperValue(),aR.LowerValue());
    }

EXPORT_C TInt TMTPTypeUint128::Compare(const TUint64 aRUpper, const TUint64 aRLower) const
    {
    TUint64 tmp = this->UpperValue();
    if( tmp != aRUpper)
        {
        return ( tmp > aRUpper ) ? 1 : -1;
        }
    
    tmp = this->LowerValue();
    if ( tmp != aRLower )
        {
        return ( tmp > aRLower ) ? 1 : -1;
        }
 
    return 0;
    }


