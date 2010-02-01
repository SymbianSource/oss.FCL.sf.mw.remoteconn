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
#include <mtp/tmtptypeintbase.h>
    
EXPORT_C TInt TMTPTypeIntBase::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(reinterpret_cast<const TUint8*>(&iData), iSize);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeIntBase::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeIntBase::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(reinterpret_cast<TUint8*>(&iData), 0, iSize);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeIntBase::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }
	
EXPORT_C TUint64 TMTPTypeIntBase::Size() const
	{
	return iSize;
	}
	
EXPORT_C TUint TMTPTypeIntBase::Type() const
	{
	return iType;
	}

/**
Constructor.
@param aData The INT data with which the type is to be initialised.
@param aSize The size of the INT data type.
@param aType The INT data type code.
*/
EXPORT_C TMTPTypeIntBase::TMTPTypeIntBase(TInt aData, TUint aSize, TInt aType) : 
    iData(aData),
    iSize(aSize),
    iType(aType)
    {
    
    }


