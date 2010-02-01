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
#include <mtp/cmtptypeopaquedata.h>


const TInt KMaxSizeOfWriteBuffer = 0x00010000; // 64KB

EXPORT_C CMTPTypeOpaqueData* CMTPTypeOpaqueData::NewL()
    {
    CMTPTypeOpaqueData* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMTPTypeOpaqueData* CMTPTypeOpaqueData::NewLC()
    {
    CMTPTypeOpaqueData* self = new(ELeave) CMTPTypeOpaqueData();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeOpaqueData* CMTPTypeOpaqueData::NewL(const TDesC8 &aDes)
    {
    CMTPTypeOpaqueData* self = NewLC(aDes);
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMTPTypeOpaqueData* CMTPTypeOpaqueData::NewLC(const TDesC8 &aDes)
    {
    CMTPTypeOpaqueData* self = new(ELeave) CMTPTypeOpaqueData();
    CleanupStack::PushL(self);
    self->ConstructL(aDes);
    return self;
    }

void CMTPTypeOpaqueData::ConstructL()
    {
    
    }

CMTPTypeOpaqueData::CMTPTypeOpaqueData():
    iPtrBuffer(NULL, 0)
    {
    
    }

EXPORT_C CMTPTypeOpaqueData::~CMTPTypeOpaqueData()
    {
    iBuffer.Close();
    }

void CMTPTypeOpaqueData::ConstructL(const TDesC8 &aDes)
    {
    iPtrBuffer.Set(const_cast<TUint8*>(aDes.Ptr()), aDes.Length(), aDes.Size());
    }

EXPORT_C TInt CMTPTypeOpaqueData::FirstReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(iPtrBuffer);

    return KMTPChunkSequenceCompletion;
    }
    
EXPORT_C TInt CMTPTypeOpaqueData::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }

TInt CMTPTypeOpaqueData::CreateBuffer( const TInt aMaxSize )
    {
    TInt err = iBuffer.Create( aMaxSize );
    if( KErrNone != err)
        return err;
      
    iPtrBuffer.Set(const_cast<TUint8*>(iBuffer.Ptr()), 0, aMaxSize);
    
    return KErrNone;
    }

EXPORT_C TInt CMTPTypeOpaqueData::FirstWriteChunk(TPtr8& aChunk)
    {
    if(iPtrBuffer.MaxSize() == 0)
        {
        TInt err =  CreateBuffer( KMaxSizeOfWriteBuffer );
        if( KErrNone != err)
            return err;
        }
    
    aChunk.Set( const_cast<TUint8*>(iPtrBuffer.Ptr()), 0, iPtrBuffer.MaxSize());
    
    return KMTPChunkSequenceCompletion;
    }

EXPORT_C TInt CMTPTypeOpaqueData::FirstWriteChunk(TPtr8& aChunk, TUint aDataLength )
    {
    if(KMaxSizeOfWriteBuffer < aDataLength )
        return KErrOverflow;
    
    TInt size = aDataLength;
    if( aDataLength <= 0 )
        {
        size = KMaxSizeOfWriteBuffer;
        }
    
    TInt err =  CreateBuffer( size );
    if( KErrNone != err)
        return err;

    
    return FirstWriteChunk(aChunk);
    }

EXPORT_C TInt CMTPTypeOpaqueData::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }

EXPORT_C TBool CMTPTypeOpaqueData::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeOpaqueData::CommitChunkL(TPtr8& aChunk)
    {
    iPtrBuffer.Set(aChunk);
    return NULL;
    }

EXPORT_C TUint64 CMTPTypeOpaqueData::Size() const
    {
    return iPtrBuffer.Size();
    }

EXPORT_C TUint CMTPTypeOpaqueData::Type() const
    {
    return EMTPTypeOpaqueData;        
    }

EXPORT_C TInt CMTPTypeOpaqueData::Read(TPtrC8 &aDes) const
    {
    aDes.Set( iPtrBuffer );
    return KErrNone;
    }

EXPORT_C TInt CMTPTypeOpaqueData::Write( const TPtrC8 &aDes)
    {
    if(iBuffer.MaxSize() != 0)
        {
        iBuffer.Close();
        }
    
    iPtrBuffer.Set(const_cast<TUint8*>(aDes.Ptr()), aDes.Length(), aDes.Size());
    return KErrNone;
    }

   

