// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @InternalTechnology
*/

#include "cmtptypecomplexbuffer.h"
#include <mtp/mtpdatatypeconstants.h>

// File type constants.
const TInt KMTPBufferChunkSize(0x00019000); // 100KB

/**
MTP string data type factory method. This method is used to create a 
zero-length MTP string.
@return A pointer to a zero-length MTP string data type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeComplexBuffer* CMTPTypeComplexBuffer::NewL()
    {
	CMTPTypeComplexBuffer* self = NewLC(); 
	CleanupStack::Pop(self);
	return self; 
    }

/**
MTP string data type factory method. This method is used to create an 
MTP string with the specified value.
@param aString The initial string value.
@return A pointer to the MTP string data type. Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/     
EXPORT_C CMTPTypeComplexBuffer* CMTPTypeComplexBuffer::NewL(const TDesC8& aData)
    {
	CMTPTypeComplexBuffer* self = NewLC(aData);
	CleanupStack::Pop(self);
	return self;
    }

/**
MTP file object data type factory method. 
@param aFs The handle of an active file server session.
@param aName The name of the file. Any path components (i.e. drive letter
or directory), which are not specified, are taken from the session path. 
@param aMode The mode in which the file is opened (@see TFileMode).
@return A pointer to the MTP file object data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
@see TFileMode
*/   
EXPORT_C CMTPTypeComplexBuffer* CMTPTypeComplexBuffer::NewLC()
    {
    CMTPTypeComplexBuffer* self = NewLC(KNullDesC8);
    return self;
    }

/**
MTP file object data type factory method. A pointer to the MTP file object data
type is placed on the cleanup stack.
@param aFs The handle of an active file server session.
@param aName The name of the file. Any path components (i.e. drive letter
or directory), which are not specified, are taken from the session path. 
@param aMode The mode in which the file is opened (@see TFileMode).
@return A pointer to the MTP file object data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
@see TFileMode
*/   
EXPORT_C CMTPTypeComplexBuffer* CMTPTypeComplexBuffer::NewLC(const TDesC8& aData)
    {
    CMTPTypeComplexBuffer* self = new(ELeave) CMTPTypeComplexBuffer;
    CleanupStack::PushL(self);
    self->ConstructL(aData);
    return self;
    }

/**
Destructor
*/
EXPORT_C CMTPTypeComplexBuffer::~CMTPTypeComplexBuffer()
    {
    iBuffer.Close();
    }

/**
Provides a reference to the native file object encapsulate by the MTP file 
object data type.
@return The native file object reference.
*/
EXPORT_C const TDesC8& CMTPTypeComplexBuffer::Buffer()
    {
    return iBuffer;
    }    
    
EXPORT_C void CMTPTypeComplexBuffer::SetBuffer(const TDesC8& aData )
	{
    iBuffer.Zero();
    if(aData == KNullDesC8)
        {
    	return;
        }

	iBuffer.Copy(aData);   	
	}

EXPORT_C TInt CMTPTypeComplexBuffer::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(iBuffer.Ptr(), iBuffer.Length());
    return KMTPChunkSequenceCompletion;
    }

EXPORT_C TInt CMTPTypeComplexBuffer::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }

EXPORT_C TInt CMTPTypeComplexBuffer::FirstWriteChunk(TPtr8& aChunk)
    {
    TInt err(KErrNone);
	aChunk.Set(NULL, 0, 0);
    iWriteSequenceState = EIdle;
    aChunk.Set(&iBuffer[0], 0, iBuffer.MaxLength());
    iWriteSequenceState = EInProgress;
    return err;
    }
    
EXPORT_C TInt CMTPTypeComplexBuffer::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(KErrNone);
	aChunk.Set(NULL, 0, 0);
    
    if (iWriteSequenceState != EInProgress)
        {
        err = KErrNotReady;            
        }
    else
        {
    	aChunk.Set(&iBuffer[0], 0, iBuffer.MaxLength());
        }
       
    return err;
    }

EXPORT_C TUint64 CMTPTypeComplexBuffer::Size() const
    {
    return iBuffer.Size();
    }

EXPORT_C TUint CMTPTypeComplexBuffer::Type() const
    {
    return EMTPTypeString;
    }

EXPORT_C TBool CMTPTypeComplexBuffer::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeComplexBuffer::CommitChunkL(TPtr8& aChunk)
    {
    iBuffer.Zero(); 
    iBuffer.Copy(aChunk);
    return NULL;
    }
    
CMTPTypeComplexBuffer::CMTPTypeComplexBuffer()
    {
        
    }

void CMTPTypeComplexBuffer::ConstructL(const TDesC8& aData)
    {    
    iBuffer.CreateMaxL(KMTPBufferChunkSize);
    if(aData != KNullDesC8)
    	{
    	iBuffer.Zero();
    	iBuffer.Copy(aData);
    	}
    }

