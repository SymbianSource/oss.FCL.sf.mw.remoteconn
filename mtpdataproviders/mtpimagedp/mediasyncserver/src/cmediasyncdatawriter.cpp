// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#include <e32base.h> 

#include "cmediasyncdatawriter.h"

CMediaSyncDataWriter* CMediaSyncDataWriter::NewLC(const RChunk& aChunk)
    {
    CMediaSyncDataWriter* self = new(ELeave) CMediaSyncDataWriter();
    CleanupStack::PushL(self);
    self->ConstructL(aChunk);
    return self;
    }

CMediaSyncDataWriter::~CMediaSyncDataWriter()
    {
    
    }

CMediaSyncDataWriter::CMediaSyncDataWriter()
    {
    
    }

void CMediaSyncDataWriter::ConstructL(const RChunk& aChunk)
    {
    TUint8* base = aChunk.Base();    
    User::LeaveIfNull(base);
    
    iHeaderInfo = reinterpret_cast<TDataHeaderInfo*>(base);
    iHeaderInfo->iCount = 0;
    iWriteBase = base + sizeof(TDataHeaderInfo);
    iMaxSize = aChunk.MaxSize() - sizeof(TDataHeaderInfo);    
    iOffset = 0;
    }

TInt CMediaSyncDataWriter::FreeSpaceBytes()
    {
    return (iMaxSize - iOffset);
    }

inline void CMediaSyncDataWriter::CheckBufferCapacityL(TInt aReqSize)
    {
    if (aReqSize > (iMaxSize - iOffset))
        {
        User::Leave(KErrOverflow);
        }
    }

void CMediaSyncDataWriter::AppendEntryL(TUint32 aObjectId, TUint8 aType, const TDesC& aUri)
    {    
    //copy object id       
    CheckBufferCapacityL(sizeof(TUint32));
    Mem::Copy((iWriteBase + iOffset), &aObjectId, sizeof(TUint32));
    iOffset += sizeof(TUint32);
    
    //copy notification type
    CheckBufferCapacityL(sizeof(TUint8));
    Mem::Copy((iWriteBase + iOffset), &aType, sizeof(TUint8));
    iOffset += sizeof(TUint8);
    
    //copy uri length
    CheckBufferCapacityL(sizeof(TUint8));
    TUint8 uriLen = aUri.Length();
    Mem::Copy((iWriteBase + iOffset), &uriLen, sizeof(TUint8));
    iOffset += sizeof(TUint8);
    if (uriLen > 0)
        {
        //copy uri content
        CheckBufferCapacityL(aUri.Size());
        TPtr8 ptr(reinterpret_cast<TUint8*>(const_cast<TUint16*>(aUri.Ptr())), aUri.Size(), aUri.Size());
        Mem::Copy((iWriteBase + iOffset), ptr.Ptr(), ptr.Size());
        iOffset += ptr.Size();
        }
    
    ++iHeaderInfo->iCount;
    }

    
    



