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
 @released
*/

#ifndef CMTPTYPESTRING_H
#define CMTPTYPESTRING_H

#include <e32base.h>
#include <mtp/mmtptype.h>

/**
Defines the MTP string data type. MTP strings consist of standard 2-byte 
Unicode characters and are null-terminated. Strings are prepended with a 
single 8-bit unsigned integer that identifies the number of characters (NOT 
bytes) to follow and includes the terminating null value. Strings are limited
to 255 characters (including the null value). An empty string would consist
of a single 8-bit integer containing the value 0x00. 

MTP strings are stored in a flat buffer to optimise streaming performance 
to/from the MTP transport connection.
@publishedPartner
@released 
*/
class CMTPTypeString : public CBase, public MMTPType
    {
public:
    
    IMPORT_C static CMTPTypeString* NewL();
    IMPORT_C static CMTPTypeString* NewL(const TDesC& aString);
    IMPORT_C static CMTPTypeString* NewLC();
    IMPORT_C static CMTPTypeString* NewLC(const TDesC& aString);
    IMPORT_C ~CMTPTypeString();
    
    IMPORT_C TUint8 NumChars() const;
    IMPORT_C const TDesC& StringChars() const;  

    IMPORT_C void SetL(const TDesC& aString);

public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C TInt Validate() const;
    
private:

    CMTPTypeString();
    void ConstructL(const TDesC& aString);
    
    void ReAllocBufferL(TUint aNumNullTerminatedChars);
    
private: // Owned

    /**
    The write data stream states.
    */
    enum TWriteSequenceState
        {
        /**
        Write data stream inactive.
        */
        EIdle,
        
        /**
        Streaming the NumChars data chunk.
        */
        ENumChars,
        
        /**
        Streaming the String Characters data chunk.
        */
        EStringChars            
        };

    /**
    The data buffer.
    */
    RBuf8   iBuffer;
    
    /**
    The String Characters data pointer.
    */
    TPtr    iStringChars;
    
    /**
    The write data stream state variable.
    */
    TUint   iWriteSequenceState;
    
    /**
    The String type's validation state.
    */
    TInt    iValidationState;
    };

#endif // CMTPTYPESTRING_H
