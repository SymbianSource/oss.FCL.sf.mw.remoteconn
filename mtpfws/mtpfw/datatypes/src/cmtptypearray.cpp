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

#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>
#include "mtpdatatypespanic.h"

// Array type constants.
const TUint KMTPNumElementsLen(sizeof(TUint32));
const TUint KMTPNumElementsOffset(0);
const TUint KMTPFirstElementOffset(KMTPNumElementsLen);
const TUint KMTPGranularity(8);

/**
MTP array data type factory method. This method is used to create an empty MTP
array.
@param aElementType The MTP type identifier of the elements contained in the 
array.
@param aElementSize The size (in bytes) of the elements contained in the array.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/   
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(TInt aElementType, TUint aElementSize)
    {
    return NewL(EMTPTypeArray, aElementType, aElementSize);
    }

/**
MTP array data type factory method. This method is used to create an empty MTP
array. A pointer to the MTP array data type is placed on the cleanup stack.
@param aElementType The MTP type identifier of the elements contained in the 
array.
@param aElementSize The size (in bytes) of the elements which contained in 
array.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/  
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(TInt aElementType, TUint aElementSize)
    {
    return CMTPTypeArray::NewLC(EMTPTypeArray, aElementType, aElementSize);
    }

/**
MTP array data type factory method. This method is used to create an empty MTP
array of the specified MTP array type.
@param aArrayType The MTP type identifier to be assigned to the array.
@param aElementType The MTP type identifier of the elements contained in the 
array.
@param aElementSize The size (in bytes) of the elements contained in the array.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/   
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(TInt aArrayType, TInt aElementType, TUint aElementSize)
    {
    CMTPTypeArray* self = NewLC(aArrayType, aElementType, aElementSize);
    CleanupStack::Pop(self);
    return self;
    }

/**
MTP array data type factory method. This method is used to create an empty MTP
array of the specified MTP array type. A pointer to the MTP array data type is 
placed on the cleanup stack.
@param aArrayType The MTP type identifier to be assigned to the array.
@param aElementType The MTP type identifier of the elements contained in the 
array.
@param aElementSize The size (in bytes) of the elements which contained in 
array.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/  
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(TInt aArrayType, TInt aElementType, TUint aElementSize)
    {
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(aArrayType, aElementType, aElementSize);
    CleanupStack::PushL(self);
    self->ConstructL(KMTPGranularity);
    return self;
    }
    
/**
MTP simple data type array factory method. This method is used to create
an empty MTP array of the specified MTP simple data type.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(TInt aArrayType)
    {
    CMTPTypeArray* self = NewLC(aArrayType);
    CleanupStack::Pop(self);
    return self;   
    } 

/**
MTP simple data type array factory method. This method is used to create
an MTP array of the specified MTP simple data type with the specified element 
content.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(TInt aArrayType, const RArray<TInt>& aElements)
    {
    CMTPTypeArray* self = NewLC(aArrayType, aElements);
    CleanupStack::Pop(self);
    return self; 
    }

/**
MTP simple data type array factory method. This method is used to create
an MTP array of the specified MTP simple data type with the specified element 
content.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(TInt aArrayType, const RArray<TUint>& aElements)
    {
    CMTPTypeArray* self = NewLC(aArrayType, aElements);
    CleanupStack::Pop(self);
    return self; 
    }

/**
MTP AINT64 array factory method. This method is used to create an MTP AINT64 
array with the specified element content.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(const RArray<TInt64>& aElements)
    {
    CMTPTypeArray* self = NewLC(aElements);
    CleanupStack::Pop(self); 
    return self; 
    }

/**
MTP AUINT64 array factory method. This method is used to create an MTP AINT64 
array with the specified element content.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewL(const RArray<TUint64>& aElements)
    {
    CMTPTypeArray* self = NewLC(aElements);
    CleanupStack::Pop(self);
    return self; 
    }
    
/**
MTP simple data type array factory method. This method is used to create
an empty MTP array of the specified MTP simple data type. A pointer to the MTP 
array data type is placed on the cleanup stack.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(TInt aArrayType)
    {
    TInt    type(aArrayType);
    TUint   size(0);
    SimpleArrayTypeMetaDataL(aArrayType, type, size);
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(aArrayType, type, size);
    CleanupStack::PushL(self);
    self->ConstructL(KMTPGranularity);
    return self;   
    }

/**
MTP simple data type array factory method. This method is used to create
an MTP array of the specified MTP simple data type with the specified element 
content. A pointer to the MTP array data type is placed on the cleanup stack.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(TInt aArrayType, const RArray<TInt>& aElements)
    {
    TInt    type(aArrayType);
    TUint   size(0);
    SimpleArrayTypeMetaDataL(aArrayType, type, size);
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(aArrayType, type, size);
    CleanupStack::PushL(self);
    
    TUint count(aElements.Count());
    self->ConstructL(count);
    
    for (TUint i(0); (i < count); i++)
        {
        self->AppendIntL(aElements[i]);
        }
    
    return self; 
    }

/**
MTP simple data type array factory method. This method is used to create
an MTP array of the specified MTP simple data type with the specified element 
content. A pointer to the MTP array data type is placed on the cleanup stack.
@param aArrayType The array data type indentifier datacode. This must be in 
the range EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(TInt aArrayType, const RArray<TUint>& aElements)
    {
    TInt    type(aArrayType);
    TUint   size(0);
    SimpleArrayTypeMetaDataL(aArrayType, type, size);
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(aArrayType, type, size);
    CleanupStack::PushL(self);
    
    TUint count(aElements.Count());
    self->ConstructL(count);
    
    for (TUint i(0); (i < count); i++)
        {
        self->AppendUintL(aElements[i]);
        }
    
    return self; 
    }

/**
MTP AINT64 array factory method. This method is used to create an MTP AINT64 
array with the specified element content. A pointer to the MTP array data type 
is placed on the cleanup stack.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(const RArray<TInt64>& aElements)
    {
    TInt    type(EMTPTypeINT64);
    TUint   size(0);
    SimpleArrayTypeMetaDataL(EMTPTypeAINT64, type, size);
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(EMTPTypeAINT64, type, size);
    CleanupStack::PushL(self);
    
    TUint count(aElements.Count());
    self->ConstructL(count);
    
    for (TUint i(0); (i < count); i++)
        {
        self->AppendInt64L(aElements[i]);
        }
    
    return self; 
    }

/**
MTP AUINT64 array factory method. This method is used to create an MTP AUINT64 
array with the specified element content. A pointer to the MTP array data type 
is placed on the cleanup stack.
@param aElements The initial set of element values.
@return A pointer to an empty MTP array data type. Ownership IS transfered.
@leave KErrArgument, if aArrayType is not in the range 
EMTPTypeFirstSimpleArrayType ... EMTPTypeLastSimpleArrayType.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
EXPORT_C CMTPTypeArray* CMTPTypeArray::NewLC(const RArray<TUint64>& aElements)
    {
    TInt    type(EMTPTypeUINT64);
    TUint   size(0);
    SimpleArrayTypeMetaDataL(EMTPTypeAUINT64, type, size);
    CMTPTypeArray* self = new(ELeave) CMTPTypeArray(EMTPTypeAUINT64, type, size);
    CleanupStack::PushL(self);
    
    TUint count(aElements.Count());
    self->ConstructL(count);
    
    for (TUint i(0); (i < count); i++)
        {
        self->AppendUint64L(aElements[i]);
        }
    
    return self; 
    }

/**
Destructor
*/
EXPORT_C CMTPTypeArray::~CMTPTypeArray()
    {
    iBuffer.Close();    
    }
    
/**
Provides the MTP identifier of the elements contained in the array.
@return The MTP identifier of the elements contained in the array.
*/ 
EXPORT_C TInt CMTPTypeArray::ElementType() const
    {
    return iElementType;    
    }
    
/**
Provides the number of elements contained in the MTP array.
@return The number of elements contained in the MTP array.
*/ 
EXPORT_C TUint32 CMTPTypeArray::NumElements() const
    {
    return *(reinterpret_cast<const TUint32*>(&iBuffer[KMTPNumElementsOffset]));
    }

/**
Appends the specified element to the end of the MTP array.
@param aElement The element to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP type of the elements
contained in the array.
*/
EXPORT_C void CMTPTypeArray::AppendL(const MMTPType& aElement)
    {    
    __ASSERT_ALWAYS((aElement.Type() == iElementType), Panic(EMTPTypeIdMismatch));
    const TUint index(NumElements());
    TUint32 numElements(index + 1);
    
    if (iBuffer.MaxLength() < BufferSize(numElements))
        {
        ReAllocBufferL(numElements);
        }
        
    TBool complete(EFalse);
    TUint offset(Offset(index));
    TPtrC8 src;
    const TUint startOffset(offset);
    
    TInt err(aElement.FirstReadChunk(src));
    while (((err == KMTPChunkSequenceCompletion) || (err == KErrNone)) && (!complete))
        {
        __ASSERT_ALWAYS(((offset - startOffset) <= iElementSize), Panic(EMTPTypeSizeMismatch));
        memcpy(&iBuffer[offset], src.Ptr(), src.Length());
        offset += src.Length();
        
        complete = (err == KMTPChunkSequenceCompletion);
        if (!complete)
            {
            err = aElement.NextReadChunk(src);                
            } 
        }
        
    if (err != KMTPChunkSequenceCompletion)
        {
        User::Leave(err);            
        }
        
    SetNumElements(numElements);
    }

/**
Appends the specified elements to the end of the MTP array. The array 
element type MUST be one of the MTP INT8, INT16, or INT32 types.
@param aElements The elements to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/   
EXPORT_C void CMTPTypeArray::AppendL(const RArray<TInt>& aElements)
    {
    TUint count(aElements.Count());
    for (TUint i(0); (i < count); i++)
        {
        AppendIntL(aElements[i]);
        }
    }

/**
Appends the specified elements to the end of the MTP array. The array 
element type MUST be one of the MTP UINT8, UINT16, or UINT32 types.
@param aElements The elements to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/    
EXPORT_C void CMTPTypeArray::AppendL(const RArray<TUint>& aElements)
    {
    TUint count(aElements.Count());
    for (TUint i(0); (i < count); i++)
        {
        AppendUintL(aElements[i]);
        }
    }

/**
Appends the specified elements to the end of the MTP array. The array 
element type MUST be of the MTP INT64 type.
@param aElements The elements to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/   
EXPORT_C void CMTPTypeArray::AppendL(const RArray<TInt64>& aElements)
    {
    TUint count(aElements.Count());
    for (TUint i(0); (i < count); i++)
        {
        AppendInt64L(aElements[i]);
        }
    }

/**
Appends the specified elements to the end of the MTP array. The array 
element type MUST be of the MTP UINT64 type.
@param aElements The elements to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::AppendL(const RArray<TUint64>& aElements)
    {
    TUint count(aElements.Count());
    for (TUint i(0); (i < count); i++)
        {
        AppendUint64L(aElements[i]);
        }
    }

/**
Appends the specified element to the end of the MTP array. The array 
element type MUST be one of the MTP INT8, INT16, or INT32 types.
@param aElement The element to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::AppendIntL(TInt aElement)
    {
    __ASSERT_ALWAYS(((iElementType == EMTPTypeINT8) || (iElementType == EMTPTypeINT16) || (iElementType == EMTPTypeINT32)), Panic(EMTPTypeIdMismatch));
    // Relies on Symbian OS little-endianess.
    AppendL(&aElement);
    }

/**
Appends the specified element to the end of the MTP array. The array 
element type MUST be one of the MTP UINT8, UINT16, or UINT32 types.
@param aElement The element to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::AppendUintL(TUint aElement)
    {  
    __ASSERT_ALWAYS(((iElementType == EMTPTypeUINT8) || (iElementType == EMTPTypeUINT16) || (iElementType == EMTPTypeUINT32)), Panic(EMTPTypeIdMismatch));
    // Relies on Symbian OS little-endianess.
    AppendL(&aElement);  
    }

/**
Appends the specified element to the end of the MTP array. The array 
element type MUST be of the MTP INT64 type.
@param aElement The element to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::AppendInt64L(TInt64 aElement)
    {    
    __ASSERT_ALWAYS((iElementType == EMTPTypeINT64) ,Panic(EMTPTypeIdMismatch));
    AppendL(&aElement);
    }

/**
Appends the specified element to the end of the MTP array. The array 
element type MUST be of the MTP UINT64 type.
@param aElement The element to be appended to the MTP array.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::AppendUint64L(TUint64 aElement)
    {    
    __ASSERT_ALWAYS((iElementType == EMTPTypeUINT64) ,Panic(EMTPTypeIdMismatch));
    AppendL(&aElement);
    }

/**
Provides a copy of the MTP element at the specified array index.
@param aIndex The index in the MTP array of the required element.
@param aElement On completion, a copy of the required element.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPDataTypes 2, if aIndex is greater than or equal to the number of 
objects currently contained in the array.
@panic MTPDataTypes 3, if aElement does not match the MTP type of the elements
contained in the array.
@panic MTPDataTypes 4, if aElement's size does not match that of the elements
contained in the array.
*/
EXPORT_C void CMTPTypeArray::ElementL(TUint aIndex, MMTPType& aElement) const
    {
    __ASSERT_ALWAYS((aIndex < NumElements()), Panic(EMTPTypeBoundsError));
    __ASSERT_ALWAYS((aElement.Type() == iElementType), Panic(EMTPTypeIdMismatch));
    __ASSERT_ALWAYS((aElement.Size() == iElementSize), Panic(EMTPTypeSizeMismatch));
    
    TBool commit(aElement.CommitRequired());
    TBool complete(EFalse);
    TPtr8 dest(NULL, 0);
    TInt err(aElement.FirstWriteChunk(dest));
    TUint offset(Offset(aIndex));
    
    while (((err == KMTPChunkSequenceCompletion) || (err == KErrNone)) && (!complete))
        {
        dest.Copy(&iBuffer[offset], dest.MaxLength());
        offset += dest.MaxLength();
        
        if (commit)
            {
            aElement.CommitChunkL(dest);
            }
        
        complete = (err == KMTPChunkSequenceCompletion);
        if (!complete)
            {
            err = aElement.NextWriteChunk(dest);
            }
        }
        
    if (err != KMTPChunkSequenceCompletion)
        {
        User::Leave(err);            
        }
    }

/**
Provides a copy of the MTP element at the specified array index. The array 
element type MUST be one of the MTP INT8, INT16, or INT32 types.
@param aIndex The index in the MTP array of the required element.
@panic MTPDataTypes 2, if aIndex is greater than or equal to the number of 
objects currently contained in the array.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
@return aElement On completion, a copy of the required element.
*/
EXPORT_C TInt CMTPTypeArray::ElementInt(TUint aIndex) const
    {
    __ASSERT_ALWAYS(((iElementType == EMTPTypeINT8) || (iElementType == EMTPTypeINT16) || (iElementType == EMTPTypeINT32)), Panic(EMTPTypeIdMismatch));
    // Relies on Symbian OS little-endianess.
    TInt ret(0);
    Element(aIndex, &ret);
    return ret;
    }

/**
Provides a copy of the MTP element at the specified array index. The array 
element type MUST be one of the MTP UINT8, UINT16, or UINT32 types.
@param aIndex The index in the MTP array of the required element.
@panic MTPDataTypes 2, if aIndex is greater than or equal to the number of 
objects currently contained in the array.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
@return aElement On completion, a copy of the required element.
*/
EXPORT_C TUint CMTPTypeArray::ElementUint(TUint aIndex) const
    {
    __ASSERT_ALWAYS(((iElementType == EMTPTypeUINT8) || (iElementType == EMTPTypeUINT16) || (iElementType == EMTPTypeUINT32)), Panic(EMTPTypeIdMismatch));
    // Relies on Symbian OS little-endianess.
    TUint ret(0);
    Element(aIndex, &ret);
    return ret;
    }

/**
Provides a copy of the MTP element at the specified array index. The array 
element type MUST be of the MTP INT64 type.
@param aIndex The index in the MTP array of the required element.
@panic MTPDataTypes 2, if aIndex is greater than or equal to the number of 
objects currently contained in the array.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
@return aElement On completion, a copy of the required element.
*/
EXPORT_C TInt64 CMTPTypeArray::ElementInt64(TUint aIndex) const
    {
    __ASSERT_ALWAYS((iElementType == EMTPTypeINT64) ,Panic(EMTPTypeIdMismatch));
    TInt64 ret(0);
    Element(aIndex, &ret);
    return ret;
    }

/**
Provides a copy of the MTP element at the specified array index. The array 
element type MUST be of the MTP UINT64 type.
@param aIndex The index in the MTP array of the required element.
@panic MTPDataTypes 2, if aIndex is greater than or equal to the number of 
objects currently contained in the array.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
@return aElement On completion, a copy of the required element.
*/
EXPORT_C TUint64 CMTPTypeArray::ElementUint64(TUint aIndex) const
    {
    __ASSERT_ALWAYS((iElementType == EMTPTypeUINT64) ,Panic(EMTPTypeIdMismatch));
    TUint64 ret(0);
    Element(aIndex, &ret);
    return ret;
    }

/**
Provides a copy of the MTP array's element content. The array element type MUST
be one of the MTP INT8, INT16, or INT32 types.
@param aElements On completion, a copy of the MTP array's element content.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::Array(RArray<TInt>& aElements) const
    {
    // Clear the array    
    aElements.Close();

    TUint numElements(NumElements());
    for (TUint i(0); (i < numElements); i++)
        {
        aElements.Append(ElementInt(i));
        }
    }

/**
Provides a copy of the MTP array's element content. The array element type MUST
be one of the MTP UINT8, UINT16, or UINT32 types.
@param aElements On completion, a copy of the MTP array's element content.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::Array(RArray<TUint>& aElements) const
    {
    // Clear the array    
    aElements.Close();

    TUint numElements(NumElements());
    for (TUint i(0); (i < numElements); i++)
        {
        aElements.Append(ElementUint(i));
        }
    }

/**
Provides a copy of the MTP array's element content. The array element type MUST
be of the MTP INT64 type.
@param aElements On completion, a copy of the MTP array's element content.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::Array(RArray<TInt64>& aElements) const
    {
    // Clear the array    
    aElements.Close();

    TUint numElements(NumElements());
    for (TUint i(0); (i < numElements); i++)
        {
        aElements.Append(ElementInt64(i));
        }
    }

/**
Provides a copy of the MTP array's element content. The array element type MUST
be of the MTP UINT64 type.
@param aElements On completion, a copy of the MTP array's element content.
@panic MTPDataTypes 3, if aElement does not match the MTP array's element type.
*/
EXPORT_C void CMTPTypeArray::Array(RArray<TUint64>& aElements) const
    {
    // Clear the array    
    aElements.Close();

    TUint numElements(NumElements());
    for (TUint i(0); (i < numElements); i++)
        {
        aElements.Append(ElementUint64(i));
        }
    }

EXPORT_C void CMTPTypeArray::ToDes( TDes8& aRetDes ) const
    {  
    TInt retSize = aRetDes.MaxSize();
    if( 0 == retSize )
       {
       return ;
       }
    
    TInt length = NumElements() * iElementSize ;
    if(retSize < length)
        length = retSize;
    
    TPtrC8 ptrL = iBuffer.Left( length + KMTPNumElementsLen );
    TPtrC8 ptr = ptrL.Right(length);
    aRetDes.Copy(ptr);
    }


EXPORT_C void CMTPTypeArray::SetByDesL( const TDesC8& aDesc ) 
    {
    TUint32 num = (aDesc.Length() / iElementSize);
    SetNumElements( num );
    if ( num > 0 )
    	{
    	ReAllocBufferL( num );
    	memcpy(&iBuffer[KMTPFirstElementOffset], aDesc.Ptr(), aDesc.Length());
    	}
    }

EXPORT_C TInt CMTPTypeArray::FirstReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(&iBuffer[KMTPNumElementsOffset], Size());
    return KMTPChunkSequenceCompletion;
    }

EXPORT_C TInt CMTPTypeArray::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }

EXPORT_C TInt CMTPTypeArray::FirstWriteChunk(TPtr8& aChunk)
    {    
    TInt ret(KErrNone);
    aChunk.Set(&iBuffer[KMTPNumElementsOffset], 0, KMTPNumElementsLen);
    iWriteSequenceState = ENumElements;
    return ret;
    }

EXPORT_C TInt CMTPTypeArray::NextWriteChunk(TPtr8& aChunk)
    {
    TInt ret(KMTPChunkSequenceCompletion);
    
    if (iWriteSequenceState != ENumElements)
        {
        ret = KErrNotReady;
        }
    else
        {
        const TUint capacity(iBuffer.MaxLength() - KMTPNumElementsLen);
        if (capacity > 0)
            {
            aChunk.Set(&iBuffer[KMTPFirstElementOffset], 0, capacity);
            }
        else
            {
            aChunk.Set(NULL, 0, 0);
            }
            
        iWriteSequenceState = EElements;
        }
        
    return ret;
    }

EXPORT_C TUint64 CMTPTypeArray::Size() const
    {
    return (KMTPNumElementsLen + (NumElements() * iElementSize));
    }

EXPORT_C TUint CMTPTypeArray::Type() const
    {
    return iArrayType;
    }

EXPORT_C TBool CMTPTypeArray::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeArray::CommitChunkL(TPtr8& aChunk)
    {
    switch (iWriteSequenceState)
        {
    case ENumElements:
        /* 
        The first chunk specifies the NumElements field, and is used to re-size 
        the array buffer.
        */
        ReAllocBufferL(NumElements());
        break;
        
    case EElements:
        // Check that the correct number of array elements has been received.
        {            
        TUint count(aChunk.Length() / iElementSize);
        if (count != NumElements())
            {
            SetInvalidL();
            }
        iWriteSequenceState = EIdle;
        }
        break;
        
    case EIdle:
    default:
        User::Leave(KErrNotReady);
        break;
        }
    return NULL;
    }
    
EXPORT_C TInt CMTPTypeArray::Validate() const
    {
    return iValidationState;
    }
    
CMTPTypeArray::CMTPTypeArray(TInt aArrayType, TInt aElementType, TUint aElementSize) :
    iArrayType(aArrayType),
    iElementSize(aElementSize),
    iElementType(aElementType)
    {
        
    }
    
void CMTPTypeArray::ConstructL(TUint32 aNumElements)
    {
    ReAllocBufferL(aNumElements);
    SetNumElements(0);
    }

void CMTPTypeArray::ConstructL(const RPointerArray<MMTPType>& aElements)
    {
    TUint count(aElements.Count());
    ConstructL(count);
    
    for (TUint i(0); (i < count); i++)
        {
        AppendL(aElements[i]);
        }
    }
    
void CMTPTypeArray::AppendL(const TAny* aElement)
    {    
    const TUint index(NumElements());
    TUint32 numElements(index + 1);
    
    if (iBuffer.MaxLength() < BufferSize(numElements))
        {
        ReAllocBufferL(numElements);
        }

    memcpy(&iBuffer[Offset(index)], aElement, iElementSize);
    SetNumElements(numElements);
    }
    
void CMTPTypeArray::Element(TUint aIndex, TAny* aElement) const
    {
    __ASSERT_ALWAYS((aIndex < NumElements()), Panic(EMTPTypeBoundsError));
    memcpy(aElement, &iBuffer[Offset(aIndex)], iElementSize);
    }
    
void CMTPTypeArray::ReAllocBufferL(TUint32 aNumElements)
    {
    TInt currentSize(iBuffer.MaxLength());
    TUint64 newSize(BufferSize(aNumElements));
    
    if (newSize != currentSize)
        {
        if (newSize > KMaxTInt)
            {
            SetInvalidL();
            }
        else
            {
            if (newSize < currentSize)
                {
                // Buffer is shrinking.
                iBuffer.Close();   
                }
            iBuffer.ReAllocL(newSize);
            iBuffer.SetMax();
            SetNumElements(aNumElements);   
            }
        }
    }

void CMTPTypeArray::SetNumElements(TUint32 aNumElements)
    {
    memcpy(&iBuffer[KMTPNumElementsOffset], &aNumElements, KMTPNumElementsLen);
    }

TUint64 CMTPTypeArray::BufferSize(TUint32 aNumElements) const
    {
    return ((iElementSize * aNumElements) + KMTPNumElementsLen);
    }
    
TUint CMTPTypeArray::Offset(TUint aIndex) const
    {
    return (KMTPFirstElementOffset + (aIndex * iElementSize));
    }
    
void CMTPTypeArray::SetInvalidL()
    {
    iValidationState = KMTPDataTypeInvalid;
    User::Leave(iValidationState);
    }
    
void CMTPTypeArray::SimpleArrayTypeMetaDataL(TInt aArrayType, TInt& aElementType, TUint& aElementSize)
    {            
    switch (aArrayType)
        {
    case EMTPTypeAINT8:
        aElementType    = EMTPTypeINT8;
        aElementSize    = KMTPTypeINT8Size;
        break; 
        
    case EMTPTypeAUINT8:
        aElementType    = EMTPTypeUINT8;
        aElementSize    = KMTPTypeUINT8Size;
        break; 
        
    case EMTPTypeAINT16:
        aElementType    = EMTPTypeINT16;
        aElementSize    = KMTPTypeINT16Size;
        break; 
        
    case EMTPTypeAUINT16:
        aElementType    = EMTPTypeUINT16;
        aElementSize    = KMTPTypeUINT16Size;
        break; 
        
    case EMTPTypeAINT32:
        aElementType    = EMTPTypeINT32;
        aElementSize    = KMTPTypeINT32Size;
        break; 
        
    case EMTPTypeAUINT32:
        aElementType    = EMTPTypeUINT32;
        aElementSize    = KMTPTypeUINT32Size;
        break; 
        
    case EMTPTypeAINT64:
        aElementType    = EMTPTypeINT64;
        aElementSize    = KMTPTypeINT64Size;
        break; 
        
    case EMTPTypeAUINT64:
        aElementType    = EMTPTypeUINT64;
        aElementSize    = KMTPTypeUINT64Size;
        break; 
        
    case EMTPTypeAINT128:
        aElementType    = EMTPTypeINT128;
        aElementSize    = KMTPTypeINT128Size;
        break; 
        
    case EMTPTypeAUINT128:
        aElementType    = EMTPTypeUINT128;
        aElementSize    = KMTPTypeUINT128Size;
        break; 
        
    default:
        User::Leave(KErrArgument);
        } 
    }
