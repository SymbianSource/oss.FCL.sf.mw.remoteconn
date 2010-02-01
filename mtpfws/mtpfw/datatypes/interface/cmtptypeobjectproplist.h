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

#ifndef CMTPTYPEOBJECTPROPLIST_H
#define CMTPTYPEOBJECTPROPLIST_H

#include <e32std.h>
#include <e32base.h>
#include <mtp/rmtptype.h>
//#include <mtp/cmtptypecompoundbase.h>

class CMTPTypeObjectPropListElement;
class CMTPTypeArray;
/** 
Defines the MTP ObjectPropList dataset. The ObjectPropList dataset provides
optimised access to object properties without needing to individually query 
each object/property pair. It also provides a more flexible querying mechanism 
for object properties in general. This dataset is used in the 
GetObjectPropList, SetObjectPropList, and SendObjectPropList requests.
@publishedPartner
@released 
*/

class CMTPTypeObjectPropList : public CBase, public MMTPType 
    {
public:    
    IMPORT_C static CMTPTypeObjectPropList* NewL();
    IMPORT_C static CMTPTypeObjectPropList* NewLC();
    IMPORT_C virtual ~CMTPTypeObjectPropList();  

    /**
    Reserve an object property element in buffer page. After setting the element's values, the element should 
    be committed, otherwise the new element will be discarded without commitment.
    @param aHandle The object handle of the object property element.
    @param aPropCode The property code of the object property element.
    */
    IMPORT_C CMTPTypeObjectPropListElement& ReservePropElemL(TUint32 aHandle, TUint16 aPropCode);
    
    /**
    Commit the reverved object property element in buffer page.
    @param aElem The object property element to be committed.
    */
    IMPORT_C void CommitPropElemL(CMTPTypeObjectPropListElement& aElem);
    
    /**
    Reset the element cursor, so GetNextElementL() will return the first object property element
    in the object property list.
    */
    IMPORT_C void ResetCursor() const;  
    
    /**
    Get the next object property element in the object property list.
    */
    IMPORT_C CMTPTypeObjectPropListElement& GetNextElementL() const;
    
    /**
    Get the size of the object property list.
    */
    IMPORT_C TUint64 Size() const;
    
    /**
    Get the number of element in the object property list.
    */
    IMPORT_C TUint32 NumberOfElements() const;
    
    /**
    Append all object property elements from another object property list.
    */
    IMPORT_C void AppendObjectPropListL(const CMTPTypeObjectPropList& aSource);
     
public: // From MMTPType
    IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt NextReadChunk(TPtrC8& aChunk) const;
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk);
    IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk, TUint aDataLength);
    IMPORT_C TInt NextWriteChunk(TPtr8& aChunk, TUint aDataLength);
    
    IMPORT_C TUint Type() const;
    IMPORT_C TBool CommitRequired() const;
    IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);
    IMPORT_C TBool ReserveTransportHeader(TUint aHeaderLength, TPtr8& aHeader);
    IMPORT_C TInt Validate() const;
    
private:
    friend class CMTPTypeObjectPropListElement;
    CMTPTypeObjectPropList();
    void  ConstructL();
    TUint ArrayElemWidth(TUint16& aDataType) const;

    TBool ValueBufferConsistent(TUint aPageIdx, TUint aBufferIdx, TUint aValueSize) const;
    TBool ValueAddressAligned(TUint aPageIdx, TUint aBufferIdx) const;
    
    void ReserveNewPage();
    
    void IncreaseIndexL(TUint& aPageIndex, TUint& aBufIndex, TUint aLength, TBool aReserveNewPage) const;   
    
    /**
    Copy data from Element to buffer page.
    */
    void MemoryCopyL(TAny* aTrg, TUint aSrcPageIndex, TUint aSrcBufIndex, TUint aLength) const;
    
    /**
    Copy data from buffer page to Element.
    */
    void MemoryCopyL(TUint aTrgPageIndex, TUint aTrgBufIndex, const TAny* aSrc, TUint aLength);

    

private: 
    /*
    The objectproplist buffer pages, each page is 64k bytes. It will only allocate one page in ContructL
    */
    RArray<RBuf8>                                   iIOPages;

    /*
    The last page number of valid elements
    */
    TUint                                           iRevPageEnd;
    
    /*
    The Reserved data length for recieve
    */
    TUint                                           iRevDataEnd;
    
    /*
    The chunk index used by NextReadChunk
    */
    mutable TUint                                   iChunkIndex;    
    /*
    The index of transport buffer header
    */
    TUint                                           iTransportHeaderIndex;
    /*
    If reserved a element but not committed, iPropElemCommitted will be set to ETrue, otherwise Efalse
    */    
    TBool                                           iReservedElemCommitted;
    /*
    Pointed to the NumberOfelements field of ObjectPropList
    */
    TUint32                                         iNumberOfElements;
    
    /*
    Current element to be read
    */    
    mutable CMTPTypeObjectPropListElement*          iCurrentElement;
    
    /*
    The reverved element
    */
    CMTPTypeObjectPropListElement*                  iReservedElement;
    
    mutable TBool                                   iResetCursor;

	};
/**
Defines the MTP ObjectPropList element dataset. The ObjectPropList element 
dataset describes an object property with a meta data triplet and a value. It
is a  component element of the ObjectPropList dataset.
*/
class CMTPTypeObjectPropListElement : public CBase
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
    IMPORT_C virtual ~CMTPTypeObjectPropListElement(); 
    IMPORT_C TUint32 Size() const;    

    IMPORT_C TUint8 Uint8L(TInt aElementId) const;
    IMPORT_C TUint16 Uint16L(TInt aElementId) const;
    IMPORT_C TUint32 Uint32L(TInt aElementId) const;
    IMPORT_C TUint64 Uint64L(TInt aElementId) const;
    IMPORT_C void    Uint128L(TInt aElementId,TUint64& high, TUint64& low) const;
    IMPORT_C const TDesC& StringL(TInt aElementId) const;
    IMPORT_C const TDesC8& ArrayL(TInt aElementId) const;    
    
    IMPORT_C void SetUint8L(TInt aElementId, TUint8 aData);
    IMPORT_C void SetUint16L(TInt aElementId, TUint16 aData);
    IMPORT_C void SetUint32L(TInt aElementId, TUint32 aData);    
    IMPORT_C void SetUint64L(TInt aElementId, TUint64 aData);
    IMPORT_C void SetUint128L(TInt aElementId, TUint64 high, TUint64 low);
    IMPORT_C void SetStringL(TInt aElementId, const TDesC& aString);
    IMPORT_C void SetArrayL(TInt aElementId, const CMTPTypeArray& aArray);
    IMPORT_C void SetArrayL(TInt aElementId, const TDesC& aString);

    
private:
    friend class CMTPTypeObjectPropList;
    static CMTPTypeObjectPropListElement* NewL(CMTPTypeObjectPropList* propList);
    static CMTPTypeObjectPropListElement* NewLC(CMTPTypeObjectPropList* propList);
    CMTPTypeObjectPropListElement();
    void ConstructL(CMTPTypeObjectPropList* propList);
    void SetDataType(TUint16 aDataType);
    void GetValueL(TAny* aTrg, TUint aLength) const;
    void SetValueL(const TAny* aTrg, TUint aLength);

private:
    mutable CMTPTypeObjectPropList*  iPropList;    
    TUint                    iPageIndex;
    TUint                    iBufIndex;
    
    mutable TUint32          iObjectHandle;
    mutable TUint16          iPropertyCode;
    mutable TUint16          iDataType;
    TUint32 iValueSize;
    // Pointed to string value
    mutable TPtrC            iStringValue;
    // Pointed to array value
    mutable TPtrC8           iArrayValue;
    // If the buffer address of string data is not aligned, the data will be copied to iStringHolder
    mutable RBuf16           iStringHolder;
    // If the buffer address of string data is not aligned, the data will be copied to iStringHolder
    mutable TBool            iArrayBuffered;

    };

#endif // CMTPTYPEOBJECTPROPLIST_H
