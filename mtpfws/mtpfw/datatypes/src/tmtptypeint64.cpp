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
#include <mtp/tmtptypeint64.h>

/**
Default constructor.
*/
EXPORT_C TMTPTypeInt64::TMTPTypeInt64() :
    iData(0)
    {
    
    }

/**
Conversion constructor from native type.
*/
EXPORT_C TMTPTypeInt64::TMTPTypeInt64(TInt64 aData) : 
    iData(aData)
    {
    
    }

/**
Destructor
*/
EXPORT_C TMTPTypeInt64::~TMTPTypeInt64()
    {

    }
    
/**
Sets the data type to the specified value.
*/      
EXPORT_C void TMTPTypeInt64::Set(TInt64 aValue)
	{
	iData = aValue;
	} 
    
/**
Provides data types's value.
@return The value of the data type
*/   
EXPORT_C TInt64 TMTPTypeInt64::Value() const
	{
	return iData;
	} 
	
EXPORT_C TInt TMTPTypeInt64::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(reinterpret_cast<const TUint8*>(&iData), sizeof(iData));
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeInt64::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeInt64::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(reinterpret_cast<TUint8*>(&iData), 0, sizeof(iData));
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeInt64::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }
	
EXPORT_C TUint64 TMTPTypeInt64::Size() const
	{
	return sizeof(iData);
	}
	
EXPORT_C TUint TMTPTypeInt64::Type() const
	{
	return EMTPTypeINT64;
	}
