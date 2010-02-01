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

#ifndef CMTPTYPECOMPOUNDBASE_H
#define CMTPTYPECOMPOUNDBASE_H

#include <e32std.h>
#include <e32base.h>
#include <mtp/mmtptype.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypeflatbase.h>
    
/**
Defines the MTP compound data types base class. Compound data types are used 
to represent MTP datasets comprising mixed sequences of simple, flat, and 
complex data type elements. These elements may be either of fixed or variable 
length. This class implements generic compound data type storage management 
capabilities and is intended for derivation by concrete classes which implement 
specific MTP datasets. Note that this class does NOT assume ownership of any 
of the data elements passed to it.
@publishedPartner
@released 
*/
class CMTPTypeCompoundBase : public CBase, public MMTPType 
    {

public:
    IMPORT_C virtual ~CMTPTypeCompoundBase();

    IMPORT_C virtual void GetL(TInt aElementId, MMTPType& aElement) const;
    IMPORT_C virtual void SetL(TInt aElementId, const MMTPType& aElement);
    
    IMPORT_C virtual void SetInt8L(TInt aElementId, TInt8 aData);    
    IMPORT_C virtual void SetInt16L(TInt aElementId, TInt16 aData);
    IMPORT_C virtual void SetInt32L(TInt aElementId, TInt32 aData);
    IMPORT_C virtual void SetInt64L(TInt aElementId, TInt64 aData);
    
    IMPORT_C virtual void SetUint8L(TInt aElementId, TUint8 aData);
    IMPORT_C virtual void SetUint16L(TInt aElementId, TUint16 aData);
    IMPORT_C virtual void SetUint32L(TInt aElementId, TUint32 aData);    
    IMPORT_C virtual void SetUint64L(TInt aElementId, TUint64 aData);
    
    IMPORT_C virtual void SetStringL(TInt aElementId, const TDesC& aString);

    IMPORT_C virtual TInt8 Int8L(TInt aElementId) const;
    IMPORT_C virtual TInt16 Int16L(TInt aElementId) const;
    IMPORT_C virtual TInt32 Int32L(TInt aElementId) const;
    IMPORT_C virtual TInt64 Int64L(TInt aElementId) const;

    IMPORT_C virtual TUint8 Uint8L(TInt aElementId) const;
    IMPORT_C virtual TUint16 Uint16L(TInt aElementId) const;
    IMPORT_C virtual TUint32 Uint32L(TInt aElementId) const;
    IMPORT_C virtual TUint64 Uint64L(TInt aElementId) const;

    IMPORT_C virtual TUint8 StringNumCharsL(TInt aElementId) const;
    IMPORT_C virtual const TDesC& StringCharsL(TInt aElementId) const;  
	
public: // From MMTPType

    IMPORT_C virtual TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C virtual TInt FirstWriteChunk(TPtr8& aChunk);
    
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C virtual TInt NextWriteChunk(TPtr8& aChunk, TUint aDataLength);
    IMPORT_C virtual TUint64 Size() const;
    IMPORT_C virtual TUint Type() const;
    IMPORT_C virtual TBool CommitRequired() const;
    IMPORT_C virtual MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C virtual TInt Validate() const;
    
public:

    /**
    Defines the MTP compound data type element meta data.
    */   
    struct TElementInfo
        {
        /**
        The element's chunk identifier (sequence number).
        */
        TInt                            iChunkId;
        
        /**
        The element's MTP type identifier.
        */
        TInt                            iType;
        
        /**
        RMTPTypeCompoundFlatChunk element specific meta data.
        */
        TMTPTypeFlatBase::TElementInfo  iFlatChunkInfo;
        };
     
    /**
    Initialisation value for TElementInfo data values that are non applicable
    for an element, e.g. when specifying iFlatChunkInfo values for non-flat
    elements such as arrays and strings.
    */   
    static const TInt KMTPNotApplicable         = -1;     
        
protected:

    /**
    Defines the MTP compound data type flat data chunk class. 
    @publishedPartner
    @released 
    */
    class RMTPTypeCompoundFlatChunk : public TMTPTypeFlatBase
        {    
    public:

        IMPORT_C RMTPTypeCompoundFlatChunk(TUint aSize, CMTPTypeCompoundBase& aParent);
        
        IMPORT_C void Close();
        IMPORT_C void OpenL();
        
    public: // From MMTPType

        IMPORT_C TUint Type() const;

    private: // From TMTPTypeFlatBase

        const TElementInfo& ElementInfo(TInt aElementId) const;
        
    private: // Owned.

        /**
        The data buffer.
        */
        RBuf8                   iBuffer;
        
        /**
        The flat data chunk size.
        */
        TUint                   iSize;
        
    private: // Not owned.
        
        /**
        The parent compound data type instance of which this flat chunk is an 
        element.
        */
        CMTPTypeCompoundBase*   iParent;
        };
        
protected:

    /**
    This flag is used by derived classes to signal the use of just-in-time
    techniques during construction from a write data stream.
    @see CMTPTypeCompoundBase
    */
    static const TBool KJustInTimeConstruction  = ETrue;
    
    /**
    This flag is used by the derived class to indicate that the number of 
    chunks making up the compound type is variable rather than fixed (e.g. a
    list of elements.)
    */
    static const TUint KVariableChunkCount      = 0;

    friend class RMTPTypeCompoundFlatChunk;
    IMPORT_C CMTPTypeCompoundBase(TBool aJustInTimeConstruction, TUint aExpectedChunkCount);
    
    IMPORT_C void ChunkAppendL(const MMTPType& aElement);
    IMPORT_C void ChunkRemove(TUint aChunkId);
    IMPORT_C TUint ChunkCount() const;
    IMPORT_C void Reset();
    IMPORT_C void SetExpectedChunkCount(TUint aExpectedChunkCount);

    /**
    Provides the TElementInfo data for the specified element.
    @param aElementId The identifier of the requested element.
    @return The requested element's meta data.
    */
    virtual const TElementInfo& ElementInfo(TInt aElementId) const = 0;
    
    IMPORT_C virtual TBool ReadableElementL(TInt aElementId) const;
    IMPORT_C virtual TBool WriteableElementL(TInt aElementId) const;
    IMPORT_C virtual TInt ValidateChunkCount() const;

    
protected:

	/**
	Ordered data type element chunk table.
	*/
	RPointerArray<MMTPType> iChunks;

	/**
	The write data stream data chunk table index. 
	*/
	TUint                   iWriteChunk;

	/**
	The write data stream state variable. 
	*/
	TUint                   iWriteSequenceState;
	/**
	The read and write data stream states.
	*/
	enum TReadWriteSequenceState
    {
    /**
    Data stream is inactive.
    */
    EIdle,
    
    /**
    Streaming the first data chunk.
    */
    EInProgressFirst,
    
    /**
    Streaming a subsequent data chunk.
    */
    EInProgressNext,          
    };    

private:

    TInt UpdateReadState(TInt aErr) const;
    
    TInt UpdateWriteState(TInt aErr);
	
    
private:

           
    /**
    This flag indicates the number of chunks which make up the compound type. 
    A value of KVariableChunkCount indicates that the type is of variable 
    rather than fixed length (e.g. a list of elements). If KVariableChunkCount
    is specified then the derived class's @see ValidChunkCount method is invoked 
    by the default @see Valid implementation.
    */
    TUint                   iExpectedChunkCount;
    
    /**
    This flag indicates that the compound data type uses just-in-time 
    techniques when constructing iteself from a write data stream. Typically 
    this is used for types whose final structure is dependant upon data 
    contained within the data stream itself, e.g. a dataset with an element 
    whose data type is determined by meta data which precedes the 
    element in the data stream.
    */
    TBool                   iJustInTimeConstruction;
    
    /**
    The read data stream data chunk table index. Note that this is declared 
    mutable to allow state updates while processing a read data stream.
    */
    mutable TUint           iReadChunk;
    
    /**
    The read data stream state variable. Note that this is declared mutable 
    to allow state updates while processing a read data stream.
    */
    mutable TUint           iReadSequenceState;        
    
    /**
    The write data stream error state. 
    */
    TInt                    iWriteErr;
    };    
    
#endif // CMTPTYPECOMPOUNDBASE_H
