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

#ifndef MMTPTYPE_H
#define MMTPTYPE_H

#include <e32std.h>

/**
Defines the generic MTP data type interface. All non-opaque MTP data comprises
either a single atomic value of a simple type, or a set of atomic simple type
values organised according to some dataset structure. Structured datasets may 
take one of the following general forms:
    
    1. Flat - a contiguous collection of values. 
    2. Complex - a contiguous array or string of values.
    3. Compound - a mixed sequence of simple, flat, and complex types.
    
Any time data is passed as a parameter to an MTP operation, response or event; 
or any time data is passed in the data phase of a fully described operation, 
that data must be of a type that implements the generic MTP data type 
interface.
@publishedPartner
@released 
*/
class MMTPType
    {
    
public:

    /**
    Initiates a data read sequence on the type by providing a const pointer 
    to the first in the sequence of data chunks that make up the type's data 
    stream. Subsequent chunks in the sequence can be sequentially retrieved 
    using NextReadChunk.
    @param aChunk The const pointer to be set to point to the start of the 
    first data chunk. This pointer's length is set to zero if no data chunk
    exists or a processing error occurs.
    @return KErrNone, if successful and the read sequence IS NOT complete, i.e.
    there are one or more additional data chunks available in the read 
    sequence.
    @return KMTPChunkSequenceCompletion, if successful and the read sequence 
    IS complete, i.e. there are no more additional data chunks available in 
    the read sequence.
    @return KErrNotFound, if no data chunk is found.
    @return One of the system wide error codes, if a general processing 
    error occurs.
    @see NextReadChunk.
    */
    virtual TInt FirstReadChunk(TPtrC8& aChunk) const = 0;

    /**
    Continues a data read sequence on the type by providing a const pointer 
    to the next in the sequence of data chunks that make up the type's data 
    stream. This data read sequence should have been previously initiated by 
    invoking FirstReadChunk.
    @param aChunk The const pointer to be set to point to the start of the 
    next data chunk. This pointer's length is set to zero if no data chunk
    exists or a processing error occurs.
    @return KErrNone, if successful and the read sequence IS NOT complete, i.e.
    there are one or more additional data chunks available in the read 
    sequence.
    @return KMTPChunkSequenceCompletion, if successful and the read sequence 
    IS complete, i.e. there are no more additional data chunks available in the 
    read sequence.
    @return KErrNotReady, if there is no data read sequence in progress, i.e.
    a read sequence was not previously initiated by invoking FirstReadChunk.
    @return One of the system wide error codes, if a general processing 
    error occurs.
    @see FirstReadChunk.
    */
    virtual TInt NextReadChunk(TPtrC8& aChunk) const = 0;

    /**
    Initiates a data write sequence on the type by providing a non-const 
    pointer to the first in the sequence of data chunks that make up the type's 
    data stream. Subsequent chunks in the sequence can be sequentially 
    retrieved using NextWriteChunk.
    @param aChunk The non-const pointer to be set to point to the start of the 
    first data chunk. If successful, the pointer's Length is set to zero, and 
    MaxLength is set to the available capacity of the data chunk. If not 
    successful, the pointer's MaxLength is set to zero.
    @return KErrNone, if successful and the write sequence IS NOT complete, 
    i.e. there are one or more additional data chunks available in the write 
    sequence.
    @return KMTPChunkSequenceCompletion, if successful and the write sequence 
    IS complete, i.e. there are no more additional data chunks available in the 
    write sequence.
    @return KErrNotFound, if no data chunk is found.
    @return One of the system wide error codes, if a general processing 
    error occurs.
    @see NextWriteChunk.
    */
    virtual TInt FirstWriteChunk(TPtr8& aChunk) = 0;

    /**
    Continues a data write sequence on the type by providing a non-const 
    pointer to the next in the sequence of data chunks that make up the type's 
    data stream. This data write sequence should have been previously initiated 
    by invoking FirstWriteChunk.
    @param aChunk The non-const pointer to be set to point to the start of the 
    next data chunk. If successful, the pointer's Length is set to zero, and 
    MaxLength is set to the available capacity of the data chunk. If not 
    successful, the pointer's MaxLength is set to zero..
    @return KErrNone, if successful and the write sequence IS NOT complete, i.e.
    there are one or more additional data chunks available in the write 
    sequence.
    @return KMTPChunkSequenceCompletion, if successful and the write sequence 
    IS complete, i.e. there are no more additional data chunks available in the 
    write sequence.
    @return KErrNotReady, if there is no data write sequence in progress, i.e.
    a write sequence was not previously initiated by invoking FirstWriteChunk.
    @return One of the system wide error codes, if a general processing 
    error occurs.
    @see FirstReadChunk.
    */
    virtual TInt NextWriteChunk(TPtr8& aChunk) = 0;
    
    /**
    Provides the size in bytes of the MTP data type.
    @return The types size in bytes.
    */
    virtual TUint64 Size() const = 0;
        
    /**
    Provides the MTP data type identification datacode.
    @return The MTP data type identification datacode.
    */
    virtual TUint Type() const = 0;
    
    /**
    Commits the specified write data chunk. This method should only be invoked 
    if the type indicates that committing data is required 
    (@see CommitRequired). Generally this would only be required by types which 
    are performing intermediate buffering, e.g. a file data object.
    @param aChunk The data chunk to be commited.
    @leave KErrNotSupported, if the type does not support data commit.
    @leave KMTPDataTypeInvalid, if the data type is structurally invalid.
    @leave One of the system wide error codes, if a general processing error 
    occurs.
    @see CommitRequired
    */
    IMPORT_C virtual MMTPType* CommitChunkL(TPtr8& aChunk);
    
    /**
    Indicates if the data type requires that data be committed during data
    write sequences. Generally this would only be required by types which 
    perform intermediate buffering (e.g. a file data object), or by data 
    types whose final structure is determined by the value of meta data 
    elements within the data stream itself (e.g an MTP ObjectPropDesc dataset).
    @return ETrue if CommitChunkL should be called after each invocation of 
    ChunkWritePtr, otherwise EFalse.
    @see CommitChunkL
    @see ChunkWritePtr
    */
    IMPORT_C virtual TBool CommitRequired() const;
    
    /**
    Validates the structural integrity of the data type. This method can be
    invoked on completion of a data write sequence to validate that the 
    received data represents a structurally valid instance of the data type.
    @return KErrNone if the data type is structurally valid, otherwise 
    KMTPDataTypeInvalid.
    */
    IMPORT_C virtual TInt Validate() const;

    /**
    Copies the data from one MTP data type object to another. This is 
    accomplished by attaching read and write data streams from the source 
    object to the sink object. The source and sink objects are not 
    required to be of the same type and no type compatibility checking is 
    performed.
    @param aFrom The MTP source data type object.
    @param aTo The MTP source data type object.
    @leave One of the system wide error codes, if a general processing error 
    occurs.
    */
	IMPORT_C static void CopyL(const MMTPType& aFrom, MMTPType& aTo);
    
    /**
    Provides an MTP data type extension interface implementation 
    for the specified interface Uid. 
    @param aInterfaceUid The unique identifier for the extension interface being 
    requested.
    @return Pointer to an interface instance or 0 if the interface is not 
    supported. Ownership is NOT transfered.
    */
    IMPORT_C virtual TAny* GetExtendedInterface(TUid aInterfaceUid);

    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk, TUint aDataLength);
    
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk, TUint aDataLength);
    
    IMPORT_C virtual TBool ReserveTransportHeader(TUint aHeaderLength, TPtr8& aHeader);
    };
    
/**
MTP completion code +17 : Indicates that a data type read or write data 
sequence has successfully completed. This code does not indicate an error 
condition.
@see FirstReadChunk.
@see NextReadChunk.
@see FirstWriteChunk.
@see NextWriteChunk.
*/
const TInt KMTPChunkSequenceCompletion = (+17);
    
/**
MTP completion code -20 : Indicates that a data type is structurally invalid.
@see CommitChunkL
@see Validate.
*/
const TInt KMTPDataTypeInvalid = (-20);
    
#endif // MMTPTYPE_H

