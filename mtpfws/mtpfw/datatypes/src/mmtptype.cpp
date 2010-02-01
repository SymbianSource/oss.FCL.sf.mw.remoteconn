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

#include <mtp/mmtptype.h>
#include <mtp/mtpdatatypeconstants.h>
#include <e32base.h> 

EXPORT_C TBool MMTPType::CommitRequired() const
    {
    return EFalse;
    }
	
EXPORT_C MMTPType* MMTPType::CommitChunkL(TPtr8& /*aChunk*/)
    {
    User::Leave(KErrNotSupported);
    return NULL;
    }
    
EXPORT_C TInt MMTPType::Validate() const
    {
    return KErrNone;
    }
    
EXPORT_C void MMTPType::CopyL(const MMTPType& aFrom, MMTPType& aTo)
    {
    TBool commit(aTo.CommitRequired());
    TBool complete(EFalse);
    TPtr8 dest(NULL, 0);
    TPtrC8 src;
    TBool useAoWrite = EFalse;
    if(EMTPTypeFile == aTo.Type()) //should change later use API to get the write type
        {
        useAoWrite = ETrue;
        }
    
    TInt readErr(KErrNone);
    TInt writeErr(KErrNone);
    
    // Obtain the initial read and write chunks.
    User::LeaveIfError(readErr = aFrom.FirstReadChunk(src));
    User::LeaveIfError(writeErr = aTo.FirstWriteChunk(dest));
    
    while (!complete)
        {
        TUint dataAvailable(src.Length());
        TUint writeCapacity(dest.MaxLength() - dest.Length());
        
        complete = 
            (((readErr == KMTPChunkSequenceCompletion) && !dataAvailable) || 
            (writeErr == KMTPChunkSequenceCompletion));
            
        if (writeCapacity < dataAvailable)
            {
            if (writeCapacity)
                {
                /* 
                Write chunk capacity is available, but not enough to accomodate
                the available read chunk data. Fill the available write chunk 
                capacity and adjust the read chunk pointer.
                */
                dest.Append(src.Left(writeCapacity));                    
                src.Set(src.Right(dataAvailable - writeCapacity));
                }
                
            if (commit)
                {
                /*
                Always commit write chunks which require it, even if the write
                chunk has no capacity.
                */
                aTo.CommitChunkL(dest);   
                if(useAoWrite)
                    {
                    TInt err = KErrNone;
                    while(CActiveScheduler::RunIfReady(err, CActive::EPriorityIdle))
                        {
                        //let the write ao run
                        }
                    User::LeaveIfError(err);
                    }
                }

            if (writeErr != KMTPChunkSequenceCompletion)
                {
                // Obtain the next write chunk
                User::LeaveIfError(writeErr = aTo.NextWriteChunk(dest));  
                }
            }
        else if (dataAvailable)
            {
            /* 
            Write chunk can accomodate the available read chunk data. Consume
            the available read chunk data, adjust the read chunk pointer, 
            and obtain the next write chunk.
            */
            dest.Append(src);
            if (commit && (dest.Length() == dest.MaxLength() || KMTPChunkSequenceCompletion == readErr)) //doesn't commit a half chunk unless it is the last chunk
                {
                aTo.CommitChunkL(dest);
                if(useAoWrite)
                    {
                    TInt err = KErrNone;
                    while(CActiveScheduler::RunIfReady(err, CActive::EPriorityIdle))
                        {
                        //let the write ao run
                        }
                    User::LeaveIfError(err);
                    }
                }

            if (readErr != KMTPChunkSequenceCompletion)
                {
                User::LeaveIfError(readErr = aFrom.NextReadChunk(src)); 
                }
            else
                {
                src.Set(KNullDesC8);   
                }

            /*
            If the write chunk is full, and more write chunks are available, 
            obtain the next write chunk.
            */
            if ((dest.Length() == dest.MaxLength()) && (writeErr != KMTPChunkSequenceCompletion))
                {
                User::LeaveIfError(writeErr = aTo.NextWriteChunk(dest));  
                }
            }
        }        
    }

EXPORT_C TAny* MMTPType::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    return NULL;
    }

EXPORT_C TInt MMTPType::FirstWriteChunk(TPtr8& aChunk, TUint /*aDataLength*/ )
    {
    return FirstWriteChunk(aChunk);
    }

EXPORT_C TInt MMTPType::NextWriteChunk(TPtr8& aChunk, TUint /*aDataLength*/ )
    {
    return NextWriteChunk(aChunk);
    }

EXPORT_C TBool MMTPType::ReserveTransportHeader(TUint /* aHeaderLength */, TPtr8& /* aHeader */)
    {
    return EFalse;
    }

