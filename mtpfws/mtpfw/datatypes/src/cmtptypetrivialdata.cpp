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
#include <mtp/cmtptypetrivialdata.h>


const TInt KMaxSizeOfWriteBuffer = 0x00004000; // 16KB

EXPORT_C CMTPTypeTrivialData* CMTPTypeTrivialData::NewL()
    {
    CMTPTypeTrivialData* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMTPTypeTrivialData* CMTPTypeTrivialData::NewLC()
    {
    CMTPTypeTrivialData* self = new(ELeave) CMTPTypeTrivialData();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CMTPTypeTrivialData::ConstructL()
    {
    
    }

CMTPTypeTrivialData::CMTPTypeTrivialData()
    {

    }

EXPORT_C CMTPTypeTrivialData::~CMTPTypeTrivialData()
    {
    iBuffer.Close();
    }

EXPORT_C TInt CMTPTypeTrivialData::FirstReadChunk(TPtrC8& /* aChunk */) const
    {
    __ASSERT_DEBUG(EFalse, User::Invariant());
    return KMTPChunkSequenceCompletion;
    }
    
EXPORT_C TInt CMTPTypeTrivialData::NextReadChunk(TPtrC8& /* aChunk */) const
    {
    __ASSERT_DEBUG(EFalse, User::Invariant());
    return KErrNotReady;
    }

EXPORT_C TInt CMTPTypeTrivialData::FirstWriteChunk(TPtr8& aChunk)
    {    
    return NextWriteChunk(aChunk);
    }

EXPORT_C TInt CMTPTypeTrivialData::NextWriteChunk(TPtr8& aChunk)
    {    
    if (iBuffer.MaxSize() == 0)
    {
    TInt err = iBuffer.Create(KMaxSizeOfWriteBuffer);
    if(KErrNone != err)
        {
        return err;
        }
    }
    aChunk.Set( const_cast<TUint8*>(iBuffer.Ptr()), 0, KMaxSizeOfWriteBuffer);
    return KErrNone;    
    }

EXPORT_C TInt CMTPTypeTrivialData::FirstWriteChunk(TPtr8& aChunk, TUint aDataLength )
    {
    return NextWriteChunk(aChunk, aDataLength);
    }

EXPORT_C TInt CMTPTypeTrivialData::NextWriteChunk(TPtr8& aChunk, TUint aDataLength )
    {
    TInt ret = KMTPChunkSequenceCompletion;
    TUint len = aDataLength;
    if(aDataLength > KMaxSizeOfWriteBuffer)
        {
        len = KMaxSizeOfWriteBuffer;
        ret = KErrNone;
        }
    
    if (iBuffer.MaxSize() == 0)
    {
    ret = iBuffer.Create(len);
    if(KErrNone != ret)
        {
        return ret;
        }
    }
    
    aChunk.Set( const_cast<TUint8*>(iBuffer.Ptr()), 0, len);

    return ret;
    }

EXPORT_C TUint64 CMTPTypeTrivialData::Size() const
    {
    return 0;
    }

EXPORT_C TUint CMTPTypeTrivialData::Type() const
    {
    return EMTPTypeTrivialData;        
    }

   

