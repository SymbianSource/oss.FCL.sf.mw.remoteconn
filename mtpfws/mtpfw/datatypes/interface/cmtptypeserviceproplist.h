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

#ifndef CMTPTYPESERVICEPROPLIST_H_
#define CMTPTYPESERVICEPROPLIST_H_

#include <e32std.h>
#include <mtp/rmtptype.h>
#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeServicePropListElement;

/** 
Defines the MTP ObjectPropList dataset. The ObjectPropList dataset provides
optimised access to object properties without needing to individually query 
each object/property pair. It also provides a more flexible querying mechanism 
for object properties in general. This dataset is used in the 
GetObjectPropList, SetObjectPropList, and SendObjectPropList requests.
@publishedPartner
@released 
*/ 
class CMTPTypeServicePropList : public CMTPTypeCompoundBase
    {
public:
    
    IMPORT_C static CMTPTypeServicePropList* NewL();
    IMPORT_C static CMTPTypeServicePropList* NewLC();

    IMPORT_C virtual ~CMTPTypeServicePropList();  
    
    IMPORT_C void AppendL(CMTPTypeServicePropListElement* aElement);
    IMPORT_C void AppendDataObjectL(MMTPType& aDataObject);
    
    IMPORT_C TUint32 NumberOfElements() const;
    IMPORT_C CMTPTypeServicePropListElement& Element(TUint aIndex) const; 
    IMPORT_C MMTPType& DataObjectL() const;
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk); 
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TInt ValidateChunkCount() const;
    
private:

    CMTPTypeServicePropList();
    void ConstructL();
    
    void AppendElementChunkL(CMTPTypeServicePropListElement* aElement);
    TInt UpdateWriteSequenceErr(TInt aErr);

private: // Owned

    /**
    The write data stream states.
    */
    enum TReadWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Elements data stream is in progress.
        */
        EElementChunks,
        
        /**
        Optional object data stream is in progress.
        */
        EDataObjectChunk            
        };

    /**
    The write data stream error state. 
    */
    TUint                                           iWriteSequenceErr;

    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;
    
    /**
    The NumberOfElements element metadata content.
    */
    CMTPTypeCompoundBase::TElementInfo              iInfoNumberOfElements;
    
    /**
    The Element array metadata content. Note that this is declared mutable 
    to allow state updates while processing a read data stream.
    */
    mutable CMTPTypeCompoundBase::TElementInfo      iInfoElement;
    
    /**
    The NumberOfElements element data chunk ID.
    */
    static const TUint                              KNumberOfElementsChunk;
    
    /**
    The Element array starting data chuck ID.
    */
    static const TUint                              KElementChunks;
    
    /**
    The NumberOfElements element data chunk.
    */
    TMTPTypeUint32                                  iChunkNumberOfElements;
    
    /**
    The Element array data chunks.
    */
    RPointerArray<CMTPTypeServicePropListElement>    iChunksElement;
    
private: // Not owned
    
    /**
    The (optional) MTP data object chunk.
    */
    MMTPType*                                       iChunkDataObject;
    };

/**
Defines the MTP ObjectPropList element dataset. The ObjectPropList element 
dataset describes an object property with a meta data triplet and a value. It
is a  component element of the ObjectPropList dataset.
*/
class CMTPTypeServicePropListElement : public CMTPTypeCompoundBase
    {

public:

    /**
    MTP ObjectPropList property quadruple dataset element identifiers.
    */
    enum TMTPObjectPropListElement
        {
        /**
        The ObjectHandle of the object to which the property applies.
        */
        EObjectHandle,
        
        /**
        The ObjectPropDesc property identifier datacode (read-only).
        */
        EPropertyCode,
        
        /**
        The datatype code of the property (read-only).
        */
        EDatatype,
        
        /**
        The value of the property.
        */
        EValue,

        /**
        The number of dataset elements.
        */
        ENumElements,
        };

public:
    
    IMPORT_C static CMTPTypeServicePropListElement* NewL(TUint16 aPropertyCode);
    IMPORT_C static CMTPTypeServicePropListElement* NewLC(TUint16 aPropertyCode);
    
    IMPORT_C static CMTPTypeServicePropListElement* NewL(TUint32 aObjectHandle, TUint16 aPropertyCode, const MMTPType& aValue);
    IMPORT_C static CMTPTypeServicePropListElement* NewLC(TUint32 aObjectHandle, TUint16 aPropertyCode, const MMTPType& aValue);

    IMPORT_C static CMTPTypeServicePropListElement* NewL(TUint16 aPropertyCode, TUint16 aDataType);
    IMPORT_C static CMTPTypeServicePropListElement* NewLC(TUint16 aPropertyCode, TUint16 aDataType);
    
    IMPORT_C static CMTPTypeServicePropListElement* NewL(TUint32 aObjectHandle, TUint16 aPropertyCode, TUint16 aDataType, const MMTPType& aValue);
    IMPORT_C static CMTPTypeServicePropListElement* NewLC(TUint32 aObjectHandle, TUint16 aPropertyCode, TUint16 aDataType, const MMTPType& aValue);
    
    IMPORT_C virtual ~CMTPTypeServicePropListElement(); 
     
public: // From CMTPTypeCompoundBase

    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk); 
        
private: // From CMTPTypeCompoundBase
    
    const TElementInfo& ElementInfo(TInt aElementId) const;
    TBool WriteableElementL(TInt aElementId) const;
    
private:

    friend MMTPType* CMTPTypeServicePropList::CommitChunkL(TPtr8& aChunk);
    static CMTPTypeServicePropListElement* NewLC();

    CMTPTypeServicePropListElement();
    void ConstructL();
    void ConstructL(TUint16 aPropertyCode, TUint16 aDataType, const TUint32* aObjectHandle, const MMTPType* aValue);

    void DestroyChunks();

private: // Owned

    /**
    The write data stream states.
    */
    enum TWriteSequenceState
        {
        /**
        Data stream is inactive.
        */
        EIdle,
        
        /**
        Streaming the property descriptor triplet dataset data chunk.
        */
        EFlatChunk,
        
        /**
        Streaming the property value data chunk.
        */
        EValueChunk 
        };
        
    /**
    Data chunk identifiers.
    */
    enum TChunkIds
        {
        /**
        The flat data chunk ID.
        */
        EIdFlatChunk,
        
        /**
        The EValue element data chunk ID.
        */
        EIdValueChunk,
        
        /**
        The number of data chunks.
        */
        EIdNumChunks
        };

    /**
    The write data stream state variable. 
    */
    TUint                                           iWriteSequenceState;
    
    /**
    The dataset element metadata table content.
    */
    static const TElementInfo                       iElementMetaData[];
    
    /**
    The dataset element metadata table.
    */
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;

    /**
    The initialised flag indicating if construction is complete and that 
    read-only elements can no longer be modified.
    */
    TBool                                           iInitialised;
    
    /**
    The property meta data triplet flat data chunk size in bytes.
    */
    static const TUint                              KFlatChunkSize;
    
    /**
    The property meta data flat data chunk, comprising elements EObjectHandle 
    to EDatatype.
    */
    RMTPTypeCompoundFlatChunk                       iChunkFlat;
    
    /**
    The EValue data chunk.
    */
    RMTPType                                        iChunkValue;
    };

#endif // CMTPTYPESERVICEPROPLIST_H_
