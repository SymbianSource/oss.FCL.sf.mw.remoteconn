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
#include <mtp/tmtptypenull.h>

#include "mtpdatatypespanic.h"

/**
Default constructor.
*/
EXPORT_C TMTPTypeNull::TMTPTypeNull() :
	iNullBuffer(NULL, 0, 0)
    {
    }

/**
Sets the data sink buffer to be used in write data stream operations.
@param aBuffer The data sink buffer.
*/
    
EXPORT_C void TMTPTypeNull::SetBuffer(const TDes8& aBuffer)
	{
	iNullBuffer.Set(aBuffer.MidTPtr(0));
	}
    
EXPORT_C TInt TMTPTypeNull::FirstReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(KNullDesC8);
    return KMTPChunkSequenceCompletion;
    }
    
EXPORT_C TInt TMTPTypeNull::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(KNullDesC8);
    return KErrNotReady;
    }
    
EXPORT_C TInt TMTPTypeNull::FirstWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(&iNullBuffer[0], 0, iNullBuffer.MaxLength());
    return KErrNone;
    }
    
EXPORT_C TInt TMTPTypeNull::NextWriteChunk(TPtr8& aChunk)
	{
    aChunk.Set(&iNullBuffer[0], 0, iNullBuffer.MaxLength());
    return KErrNone;
    }
    
EXPORT_C TUint64 TMTPTypeNull::Size() const
    {
    return 0;
    }

EXPORT_C TUint TMTPTypeNull::Type() const
    {
    return EMTPTypeUndefined;        
    }
