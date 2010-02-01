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

#ifndef CMTPTYPEARRAY_H
#define CMTPTYPEARRAY_H

#include <e32std.h>
#include <e32base.h>
#include <mtp/mmtptype.h>

/**
Defines the MTP array data type. MTP arrays consist of a contiguous sequence
of objects of the same fixed length type, and are zero based. MTP does not 
define variable length arrays, e.g. of string objects. Arrays are prepended 
with a single 32-bit unsigned integer that identifies the number of elements 
to follow. An empty array would consist of a single 32-bit integer containing
the value 0x00000000. 

MTP arrays are stored in a flat buffer to optimize streaming performance 
to/from the MTP transport connection. 
@publishedPartner
@released 
*/
class CMTPTypeArray : public CBase, public MMTPType 
    {   
public:

    IMPORT_C static CMTPTypeArray* NewL(TInt aElementType, TUint aElementSize);
    IMPORT_C static CMTPTypeArray* NewLC(TInt aElementType, TUint aElementSize); 
    IMPORT_C static CMTPTypeArray* NewL(TInt aArrayType, TInt aElementType, TUint aElementSize);
    IMPORT_C static CMTPTypeArray* NewLC(TInt aArrayType, TInt aElementType, TUint aElementSize); 
    IMPORT_C static CMTPTypeArray* NewL(TInt aArrayType);
    IMPORT_C static CMTPTypeArray* NewL(TInt aArrayType, const RArray<TInt>& aElements);
    IMPORT_C static CMTPTypeArray* NewL(TInt aArrayType, const RArray<TUint>& aElements);
    IMPORT_C static CMTPTypeArray* NewL(const RArray<TInt64>& aElements);
    IMPORT_C static CMTPTypeArray* NewL(const RArray<TUint64>& aElements);
    IMPORT_C static CMTPTypeArray* NewLC(TInt aArrayType);
    IMPORT_C static CMTPTypeArray* NewLC(TInt aArrayType, const RArray<TInt>& aElements);
    IMPORT_C static CMTPTypeArray* NewLC(TInt aArrayType, const RArray<TUint>& aElements);
    IMPORT_C static CMTPTypeArray* NewLC(const RArray<TInt64>& aElements);
    IMPORT_C static CMTPTypeArray* NewLC(const RArray<TUint64>& aElements);
    IMPORT_C ~CMTPTypeArray();
    
    IMPORT_C TInt ElementType() const;
    IMPORT_C TUint32 NumElements() const;
    
    IMPORT_C void AppendL(const MMTPType& aElement);
    IMPORT_C void AppendL(const RArray<TInt>& aElements);
    IMPORT_C void AppendL(const RArray<TUint>& aElements);
    IMPORT_C void AppendL(const RArray<TInt64>& aElements);
    IMPORT_C void AppendL(const RArray<TUint64>& aElements);
    
    IMPORT_C void AppendIntL(TInt aElement);
    IMPORT_C void AppendUintL(TUint aElement);
    IMPORT_C void AppendInt64L(TInt64 aElement);
    IMPORT_C void AppendUint64L(TUint64 aElement);
    
    IMPORT_C void ElementL(TUint aIndex, MMTPType& aElement) const;
    
    IMPORT_C TInt ElementInt(TUint aIndex) const;
    IMPORT_C TUint ElementUint(TUint aIndex) const;
    IMPORT_C TInt64 ElementInt64(TUint aIndex) const;
    IMPORT_C TUint64 ElementUint64(TUint aIndex) const;
    
    IMPORT_C void Array(RArray<TInt>& aElements) const;
    IMPORT_C void Array(RArray<TUint>& aElements) const;
    IMPORT_C void Array(RArray<TInt64>& aElements) const;
    IMPORT_C void Array(RArray<TUint64>& aElements) const;
    IMPORT_C void ToDes( TDes8& aRetDes ) const;
    IMPORT_C void SetByDesL( const TDesC8& aDesc );
    
public: // From MMTPType

    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint64 Size() const;
    IMPORT_C virtual TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C TInt Validate() const;
    
private:

    CMTPTypeArray(TInt aArrayType, TInt aElementType, TUint aElementSize);
    void ConstructL(TUint32 aNumElements);
    void ConstructL(const RPointerArray<MMTPType>& aElements);
    
    void AppendL(const TAny* aElement);
    void Element(TUint aIndex, TAny* aElement) const;
    void ReAllocBufferL(TUint32 aNumElements);
    void SetNumElements(TUint32 aNumElements);
    TUint64 BufferSize(TUint32 aNumElements) const;
    TUint Offset(TUint aIndex) const;  
    void SetInvalidL();
     
    static void SimpleArrayTypeMetaDataL(TInt aArrayType, TInt& aElementType, TUint& aElementSize); 
    
private:

    /**
    The write data stream states.
    */
    enum TWriteSequenceState
        {
        /**
        Write data stream is inactive.
        */
        EIdle,
        
        /**
        Streaming the NumElements data chunk.
        */
        ENumElements,
        
        /**
        Streaming the array Element chunks.
        */
        EElements            
        };
        
	/**
	The array type's MTP datatype code.
	*/
    TInt    iArrayType;
    
    /**
    The data buffer.
    */
    RBuf8   iBuffer;
        
	/**
	The array element type's size in bytes.
	*/
    TUint64 iElementSize;
        
	/**
	The array element type's MTP datatype code.
	*/
    TInt    iElementType;
    
    /**
    The write data stream state variable.
    */
    TUint   iWriteSequenceState;
    
    /**
    The array type's validation state.
    */
    TInt    iValidationState;
    };

#endif // CMTPTYPEARRAY_H
