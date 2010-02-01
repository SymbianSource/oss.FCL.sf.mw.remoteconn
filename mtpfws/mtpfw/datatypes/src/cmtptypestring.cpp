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

#include <mtp/cmtptypestring.h>
#include <mtp/mtpdatatypeconstants.h>

#include "mtpdatatypespanic.h"

// String type constants
const TUint KMTPAlignmentOffset(0);
const TUint KMTPAlignmentSize(1);
const TUint KMTPNumCharsOffset(KMTPAlignmentOffset + KMTPAlignmentSize);
const TUint KMTPNumCharsSize(KMTPTypeUINT8Size);
const TUint KMTPStringCharactersOffset(KMTPNumCharsOffset + KMTPNumCharsSize);

/**
MTP string data type factory method. This method is used to create a 
zero-length MTP string.
@return A pointer to a zero-length MTP string data type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeString* CMTPTypeString::NewL()
    {
	CMTPTypeString* self = NewLC(); 
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
EXPORT_C CMTPTypeString* CMTPTypeString::NewL(const TDesC& aString)
    {
	CMTPTypeString* self = NewLC(aString);
	CleanupStack::Pop(self);
	return self;
    }

/**
MTP string data type factory method. This method is used to create a 
zero-length MTP string. A pointer to the MTP string data type is placed on the
cleanup stack.
@return A pointer to a zero-length MTP string data type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeString* CMTPTypeString::NewLC()
    {
	CMTPTypeString* self = NewLC(KNullDesC);
	return self;    
    }

/**
MTP string data type factory method. This method is used to create an MTP 
string with the specified value. A pointer to the MTP string data type is 
placed on the cleanup stack.
@param aString The initial string value.
@return A pointer to the MTP string data type. Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/      
EXPORT_C CMTPTypeString* CMTPTypeString::NewLC(const TDesC& aString)
    {
	CMTPTypeString* self = new(ELeave) CMTPTypeString;
	CleanupStack::PushL(self);
    self->ConstructL(aString);
	return self;
    }

/**
Destructor
*/
EXPORT_C CMTPTypeString::~CMTPTypeString()
    {
    iBuffer.Close();
    }
    
/**
Provides the number of unicode characters making up the MTP string.
@return The number of unicode characters making up the MTP string.
*/    
EXPORT_C TUint8 CMTPTypeString::NumChars() const
    {
    return iBuffer[KMTPNumCharsOffset];
    }

/**
Provides a non-modifiable reference to the MTP String Characters data. The data
provided does not include the terminating null character.
@return A non-modifiable MTP string data reference.
*/  
EXPORT_C const TDesC& CMTPTypeString::StringChars() const
    {
    return iStringChars;
    }
    
/**
Sets the MTP string data type to the specified value.
@param aString The type's new value.
@leave One of the system wide error codes, if a processing error occurs.
*/     
EXPORT_C void CMTPTypeString::SetL(const TDesC& aString)
    {
    ConstructL(aString);
    }
	
EXPORT_C TInt CMTPTypeString::FirstReadChunk(TPtrC8& aChunk) const
    {
	aChunk.Set(&iBuffer[KMTPNumCharsOffset], (iBuffer.Length() - KMTPAlignmentSize));
    return KMTPChunkSequenceCompletion;
    }
	
EXPORT_C TInt CMTPTypeString::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
	
EXPORT_C TInt CMTPTypeString::FirstWriteChunk(TPtr8& aChunk)
    {
    TInt ret(KErrNone);
    aChunk.Set(&iBuffer[KMTPNumCharsOffset], 0, KMTPNumCharsSize);
    iWriteSequenceState = ENumChars;        
    return ret;
    }
	
EXPORT_C TInt CMTPTypeString::NextWriteChunk(TPtr8& aChunk)
    {
    TInt ret(KMTPChunkSequenceCompletion);
    
    if (iWriteSequenceState != ENumChars)
        {
        ret = KErrNotReady;
        }
    else
        {
        const TUint numChars(NumChars());
        __ASSERT_ALWAYS((numChars <= KMTPMaxStringLength), Panic(EMTPTypeSizeMismatch));
        if (numChars > 0)
            {
        	aChunk.Set(&iBuffer[KMTPStringCharactersOffset], 0, (iBuffer.MaxLength() - KMTPAlignmentSize - KMTPNumCharsSize));
            }
        else
            {
            aChunk.Set(NULL, 0, 0);
            }
            
        iWriteSequenceState = EStringChars;
        }
        
    return ret;
    }
    
EXPORT_C TUint64 CMTPTypeString::Size() const
    {
	return (iBuffer.Size() - KMTPAlignmentSize);
    }
    
EXPORT_C TUint CMTPTypeString::Type() const
	{
	return EMTPTypeString;
	}

EXPORT_C TBool CMTPTypeString::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeString::CommitChunkL(TPtr8& aChunk)
    {
    switch (iWriteSequenceState)
        {
    case ENumChars:
        /* 
        The first chunk specifies the NumChars field, and is used to re-size 
        the array buffer. 
        */
        ReAllocBufferL(NumChars());  
        break;
        
    case EStringChars:
        // Check that the correct number of characters has been received.
        {
        TUint len(aChunk.Length());
        if ((len % KMTPCharSize) ||
            ((len / KMTPCharSize) != NumChars()))
            {
            iValidationState = KMTPDataTypeInvalid;
            User::Leave(iValidationState);
            }
        iWriteSequenceState = EIdle;
        }
        break;
        
    case EIdle:
    default:
        break;
        }
    return NULL;
    }
    
EXPORT_C TInt CMTPTypeString::Validate() const
    {
    return iValidationState;
    }

CMTPTypeString::CMTPTypeString() :
    iBuffer(),
    iStringChars(NULL, 0)
    {

    }

void CMTPTypeString::ConstructL(const TDesC& aString)
    {
    if (aString.Length() > KMTPMaxStringCharactersLength)
        {
        User::Leave(KErrArgument);            
        }
    
    // Allocate string storage.
    TUint numChars(aString.Length());
    if (numChars)
        {
        numChars += KMTPNullCharLen;   
        }
    ReAllocBufferL(numChars);
    
    // Append the String Characters.
    if (numChars > 0)
        {
        iStringChars.Copy(aString);
        }
    }

void CMTPTypeString::ReAllocBufferL(TUint aNumNullTerminatedChars)
    {
    if (aNumNullTerminatedChars > KMTPMaxStringLength)
        {
        User::Leave(KErrOverflow);            
        }
        
    // Delete the current buffer
    iBuffer.Close();
    iStringChars.Set(NULL, 0, 0);
    
    // Allocate a new buffer.
    if (aNumNullTerminatedChars > 0)
        {  
        /*
        Allocate storage for both the NumChars and String Characters fields. A single 
        alignment byte is pre-pended to force String Characters to be 16-bit aligned.
        */
        iBuffer.CreateMaxL(KMTPAlignmentSize + KMTPNumCharsSize + (aNumNullTerminatedChars * KMTPCharSize));
        
        // Insert the terminating NULL character.
        TUint numChars(aNumNullTerminatedChars - KMTPNullCharLen);
        memcpy(&iBuffer[KMTPStringCharactersOffset + (numChars * KMTPCharSize)], &KMTPNullChar, KMTPCharSize);
        
        // Set the String Characters pointer.
        iStringChars.Set(reinterpret_cast<TUint16*>(&iBuffer[KMTPStringCharactersOffset]), numChars, numChars);
        }
    else
        {       
        /*
        Allocate storage for the NumChars field only. A single alignment byte 
        is pre-pended to force String Characters to be 16-bit aligned.
        */
        iBuffer.CreateMaxL(KMTPNumCharsSize + KMTPAlignmentSize);
        }
        
    // Insert the NumChars value
    iBuffer[KMTPNumCharsOffset] = aNumNullTerminatedChars;
    }
