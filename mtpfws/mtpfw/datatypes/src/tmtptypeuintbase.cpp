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
#include <mtp/tmtptypeuintbase.h>
    
EXPORT_C TInt TMTPTypeUintBase::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(reinterpret_cast<const TUint8*>(&iData), iSize);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUintBase::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt TMTPTypeUintBase::FirstWriteChunk(TPtr8& aChunk)
    {
	aChunk.Set(reinterpret_cast<TUint8*>(&iData), 0, iSize);
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt TMTPTypeUintBase::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }
	
EXPORT_C TUint64 TMTPTypeUintBase::Size() const
	{
	return iSize;
	}
	
EXPORT_C TUint TMTPTypeUintBase::Type() const
	{
	return iType;
	}

/**
Constructor.
@param aData The UINT data with which the type is to be initialised.
@param aSize The size of the UINT data type.
@param aType The UINT data type code.
*/
EXPORT_C TMTPTypeUintBase::TMTPTypeUintBase(TUint aData, TUint aSize, TInt aType) : 
    iData(aData),
    iSize(aSize),
    iType(aType)
    {
    
    }

