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

#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypeserviceproplist.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint CMTPTypeServicePropListElement::KFlatChunkSize(8);
    
const TUint CMTPTypeServicePropList::KNumberOfElementsChunk(0);
const TUint CMTPTypeServicePropList::KElementChunks(1);

// Dataset element metadata    
const CMTPTypeCompoundBase::TElementInfo CMTPTypeServicePropListElement::iElementMetaData[CMTPTypeServicePropListElement::ENumElements] = 
    {
        {EIdFlatChunk,  EMTPTypeFlat,       {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EObjectHandle
        {EIdFlatChunk,  EMTPTypeFlat,       {EMTPTypeUINT16,    4,                  KMTPTypeUINT16Size}},   // EPropertyCode
        {EIdFlatChunk,  EMTPTypeFlat,       {EMTPTypeUINT16,    6,                  KMTPTypeUINT16Size}},   // EDatatype
        {EIdValueChunk, EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EValue
    };

/**
MTP ObjectPropList dataset factory method. This method is used to create an 
empty MTP ObjectPropList dataset. 
@return A pointer to the MTP ObjectPropList dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropList* CMTPTypeServicePropList::NewL()
    {
    CMTPTypeServicePropList* self = CMTPTypeServicePropList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

/**
MTP ObjectPropList dataset factory method. This method is used to create an 
empty MTP ObjectPropList dataset. A pointer to the data type is placed on the
cleanup stack.
@return A pointer to the MTP ObjectPropList dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropList* CMTPTypeServicePropList::NewLC()
    {
    CMTPTypeServicePropList* self = new(ELeave) CMTPTypeServicePropList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeServicePropList::~CMTPTypeServicePropList()
    {
    iChunksElement.ResetAndDestroy();
    }

/**
Appends the specified ObjectPropList value quadruple to the ObjectPropList. 
Ownership of the specified ObjectPropList value quadruple is passed to the 
ObjectPropList.
@param aElement The ObjectPropList value quadruple to append. Ownership IS
transferred.
@leave KMTPDataTypeInvalid, If an MTP data object is attached to the 
ObjectPropList dataset.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C void CMTPTypeServicePropList::AppendL(CMTPTypeServicePropListElement* aElement)
    {
    if (iChunkDataObject)
        {
        User::Leave(KMTPDataTypeInvalid);
        }
        
    // Increment NumberOfElements.
    const TUint index(iChunkNumberOfElements.Value());
    iChunkNumberOfElements.Set(index + 1);
    
    // Append the element.
    AppendElementChunkL(aElement);
    }

/**
Appends the specified MTP data object to the ObjectPropList dataset.
@leave KMTPDataTypeInvalid, If an MTP data object is already attached to the 
ObjectPropList dataset.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C void CMTPTypeServicePropList::AppendDataObjectL(MMTPType& aDataObject)
    {
    if (iChunkDataObject)
        {
        User::Leave(KMTPDataTypeInvalid);
        }
    else
        {
        iChunkDataObject = &aDataObject;
        ChunkAppendL(*iChunkDataObject);
        }
    }
    
/**
Provides the NumberOfElements element value.
*/
EXPORT_C TUint32 CMTPTypeServicePropList::NumberOfElements() const
    {
    return iChunkNumberOfElements.Value();
    }

/**
Provides a non-const reference to the ObjectPropList value quadruple element at
the specified index.
@param aIndex The index of the required element.
@return A non-const reference to the requested element.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C CMTPTypeServicePropListElement& CMTPTypeServicePropList::Element(TUint aIndex) const
    {
    return *iChunksElement[aIndex];
    }

/**
Provides a non-const reference to the optional MTP data object attached to the
ObjectPropList dataset.
@leave KErrNotFound, If there is no MTP data object attched to the 
ObjectPropList dataset.
@leave One of the system wide error codes, if unsuccessful.
*/  
EXPORT_C MMTPType& CMTPTypeServicePropList::DataObjectL() const
    {
    return *iChunkDataObject;
    }

EXPORT_C TInt CMTPTypeServicePropList::FirstWriteChunk(TPtr8& aChunk)
    {  
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); (i >= (KNumberOfElementsChunk + 1)); i--)
        {
        ChunkRemove(i);
        }
    iChunksElement.ResetAndDestroy();
    
    // Setup the write chunk pointer.
    TInt err(UpdateWriteSequenceErr(CMTPTypeCompoundBase::FirstWriteChunk(aChunk)));
    switch (err)
        {
    case KMTPChunkSequenceCompletion:
        err = KErrNone;
        // Don't break, fall through to set the write sequence state.
        
    case KErrNone:
        // Set the write sequence state.
        iWriteSequenceState = EElementChunks;
        break;
        
    default:
        break;
        }
        
    return  err;
    }
    
EXPORT_C TInt CMTPTypeServicePropList::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(KMTPChunkSequenceCompletion);
    if (iWriteSequenceState != EIdle)
        {
        err = UpdateWriteSequenceErr(CMTPTypeCompoundBase::NextWriteChunk(aChunk));
        if ((iWriteSequenceErr == KMTPChunkSequenceCompletion) && (iWriteSequenceState != EIdle))
            {
            err = KErrNone;
            }   
        }    
    return err;
    }

EXPORT_C TUint CMTPTypeServicePropList::Type() const
    {
    return EMTPTypeObjectPropListDataset;
    }

EXPORT_C TBool CMTPTypeServicePropList::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeServicePropList::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EElementChunks:
            if ((iChunkNumberOfElements.Value()) && 
                (iChunksElement.Count() < iChunkNumberOfElements.Value()))
                {
                // Allocate the next element chunk.
                CMTPTypeServicePropListElement* element = CMTPTypeServicePropListElement::NewLC();
                AppendElementChunkL(element);
                CleanupStack::Pop(element); 
                }
            else if (iChunkDataObject)
                {
                iWriteSequenceState = EDataObjectChunk;
                ChunkAppendL(*iChunkDataObject);
                }
            else
                {
                iWriteSequenceState = EIdle;
                }
            break;
            
        case EDataObjectChunk:
        case EIdle:
            // Completing the last chunk in the write sequence.
            break;
            
        default:
            Panic(EMTPTypeBadStorage);
            break;
            }
        }
        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }
	return NULL;
    }
 
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServicePropList::ElementInfo(TInt aElementId) const
    {
    if (aElementId == KNumberOfElementsChunk)
        {
        return iInfoNumberOfElements;
        }
    else
        {
        __ASSERT_ALWAYS(((aElementId - KElementChunks) < iChunkNumberOfElements.Value()), Panic(EMTPTypeBoundsError));
        iInfoElement.iChunkId = aElementId;
        return iInfoElement; 
        }
    }

TInt CMTPTypeServicePropList::ValidateChunkCount() const
    {
    TInt ret(KErrNone);
    TUint expected(iChunkDataObject ? (NumberOfElements() + 1) : NumberOfElements());
    if (expected != (ChunkCount() - 1))
        {
        ret = KMTPDataTypeInvalid;
        }
    return ret;
    }

CMTPTypeServicePropList::CMTPTypeServicePropList() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfElements       = KDefaultInfo; 
    iInfoNumberOfElements.iType = EMTPTypeUINT32;
    iInfoElement                = KDefaultInfo; 
    iInfoElement.iType          = EMTPTypeObjectPropListElementDataset;
    }

void CMTPTypeServicePropList::ConstructL()
    {
    ChunkAppendL(iChunkNumberOfElements);
    }
    
void CMTPTypeServicePropList::AppendElementChunkL(CMTPTypeServicePropListElement* aElement)
    {
    iChunksElement.AppendL(aElement);
    ChunkAppendL(*aElement);
    }
    
TInt CMTPTypeServicePropList::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with the default 
ElementDataType defined for the specified ElementPropertyCode, and the default 
ElementObjectHandle and ElementValue values. 
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewL(TUint16 aPropertyCode)
    {
    CMTPTypeServicePropListElement* self = NewLC(aPropertyCode);
    CleanupStack::Pop(self); 
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with with the default 
ElementDataType defined for the specified ElementPropertyCode, and the default 
ElementObjectHandle and ElementValue values. A pointer to the data type is 
placed on the cleanup stack.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewLC(TUint16 aPropertyCode)
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    CMTPTypeObjectPropDesc::PropertyInfoL(aPropertyCode, info);

    CMTPTypeServicePropListElement* self = new(ELeave) CMTPTypeServicePropListElement();
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, info.iDataType, NULL, NULL);
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with with the default 
ElementDataType defined for the specified ElementPropertyCode, and the 
specified ElementObjectHandle and ElementValue values. 
@param aObjectHandle The ObjectHandle of the object to which the property 
applies.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aValue The value of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewL(TUint32 aObjectHandle, TUint16 aPropertyCode, const MMTPType& aValue)
    {
    CMTPTypeServicePropListElement* self = NewLC(aObjectHandle, aPropertyCode, aValue);
    CleanupStack::Pop(self);
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with with the default 
ElementDataType defined for the specified ElementPropertyCode, and the 
specified ElementObjectHandle and ElementValue values. A pointer to the data 
type is placed on the cleanup stack.
@param aObjectHandle The ObjectHandle of the object to which the property 
applies.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aValue The value of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewLC(TUint32 aObjectHandle, TUint16 aPropertyCode, const MMTPType& aValue)
    {
    CMTPTypeObjectPropDesc::TPropertyInfo info;
    CMTPTypeObjectPropDesc::PropertyInfoL(aPropertyCode, info);

    CMTPTypeServicePropListElement* self = new(ELeave) CMTPTypeServicePropListElement();
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, info.iDataType, &aObjectHandle, &aValue);
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with the specified 
ElementPropertyCode and ElementDataType values, and the default 
ElementObjectHandle and ElementValue values. 
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aDataType The datacode identifying the datatype of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewL(TUint16 aPropertyCode, TUint16 aDataType)
    {
    CMTPTypeServicePropListElement* self = NewLC(aPropertyCode, aDataType);
    CleanupStack::Pop(self);
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with the specified 
ElementPropertyCode and ElementDataType values, and the default 
ElementObjectHandle and ElementValue values. A pointer to the data type is 
placed on the cleanup stack.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aDataType The datacode identifying the datatype of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewLC(TUint16 aPropertyCode, TUint16 aDataType)
    {
    CMTPTypeServicePropListElement* self = new(ELeave) CMTPTypeServicePropListElement();
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, aDataType, NULL, NULL);
    return self;
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with the specified values. 
@param aObjectHandle The ObjectHandle of the object to which the property 
applies.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aDataType The datacode identifying the datatype of the property.
@param aValue The value of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewL(TUint32 aObjectHandle, TUint16 aPropertyCode, TUint16 aDataType, const MMTPType& aValue)
    {
    CMTPTypeServicePropListElement* self = NewLC(aObjectHandle, aPropertyCode, aDataType, aValue);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP ObjectPropList element dataset factory method. This method is used to 
create an MTP ObjectPropList element dataset type with the specified values. A 
pointer to the data type is placed on the cleanup stack.
@param aObjectHandle The ObjectHandle of the object to which the property 
applies.
@param aPropertyCode The datacode identifying the ObjectPropDesc which 
describes the property.
@param aDataType The datacode identifying the datatype of the property.
@param aValue The value of the property.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewLC(TUint32 aObjectHandle, TUint16 aPropertyCode, TUint16 aDataType, const MMTPType& aValue)
    {
    CMTPTypeServicePropListElement* self = new(ELeave) CMTPTypeServicePropListElement();
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, aDataType, &aObjectHandle, &aValue);
    return self;    
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeServicePropListElement::~CMTPTypeServicePropListElement()
    {
    iChunkFlat.Close();
    iChunkValue.Close();
    }

EXPORT_C TInt CMTPTypeServicePropListElement::FirstWriteChunk(TPtr8& aChunk)
    {
    // Setup the write chunk pointer.
    TInt err(CMTPTypeCompoundBase::FirstWriteChunk(aChunk));
    switch (err)
        {
    case KMTPChunkSequenceCompletion:
        err = KErrNone;
        // Don't break, fall through to set the write sequence state.
        
    case KErrNone:
        // Set the write sequence state.
        iWriteSequenceState = EFlatChunk;
        break;
        
    default:
        break;
        }        
    return  err;
    }

EXPORT_C TUint CMTPTypeServicePropListElement::Type() const
    {
    return EMTPTypeObjectPropListElementDataset;
    }

EXPORT_C TBool CMTPTypeServicePropListElement::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeServicePropListElement::CommitChunkL(TPtr8& aChunk)
    {
    switch (iWriteSequenceState)
        {
    case EFlatChunk:
        /* 
        ElementDatatype is available, (re)construct storage for the 
        ElementValue.
        */
        iChunkValue.Close();
        iChunkValue.OpenL(Uint16L(EDatatype));
        
        // Update the write sequence state.
        iWriteSequenceState = EValueChunk;
        break;
        
    case EValueChunk:
        // Update the write sequence state.
        iWriteSequenceState = EIdle;
        break;
        
    default:
        break;
        }
        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }
	return NULL;
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServicePropListElement::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

TBool CMTPTypeServicePropListElement::WriteableElementL(TInt aElementId) const
    {
    TBool ret(EFalse);

    if (!iInitialised)
        {
        ret = ETrue;    
        }
    else
        {
        switch (aElementId)
            {
        case EObjectHandle:
        case EValue:
            ret = ETrue;
            break;

        default:
            break;
            }   
        }
        
    return ret;
    }

/*
MTP ObjectPropList element dataset "pre-production" factory method for datasets
which will be populated from an MTP write data stream. This method is used to 
create a partially constructed MTP ObjectPropList element dataset type prior to
the construction characteristics being known. Construction of the element 
dataset is completed on-the-fly as it is written to from an MTP write data 
stream. A pointer to the data type is placed on the cleanup stack.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/
CMTPTypeServicePropListElement* CMTPTypeServicePropListElement::NewLC()
    {
    CMTPTypeServicePropListElement* self = new(ELeave) CMTPTypeServicePropListElement();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CMTPTypeServicePropListElement::CMTPTypeServicePropListElement() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, EIdNumChunks),
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KFlatChunkSize, *this)
    {

    }

void CMTPTypeServicePropListElement::ConstructL()
    {
    for (TUint i(0); (i < ENumElements); i++)
        {
        const TElementInfo& element(iElementInfo[i]);
        if (ChunkCount() <= element.iChunkId)
            {
            MMTPType* chunk(NULL);
            
            switch (element.iChunkId)
                {
            case EIdFlatChunk:
                iChunkFlat.OpenL();
                chunk = &iChunkFlat;
                break;
                
            case EIdValueChunk:
                /* 
                iChunkValue is always opened elsewhere. CommitChunkL in the  
                just-in-time construction case, or other ConstructL overloads in
                normal construction cases.
                */
                chunk = &iChunkValue;
                break;
                
            default:
                Panic(EMTPTypeBoundsError);
                break;
                }
                
            if (chunk)
                {
                ChunkAppendL(*chunk);
                }
            }
        }
    }

void CMTPTypeServicePropListElement::ConstructL(TUint16 aPropertyCode, TUint16 aDataType, const TUint32* aObjectHandle, const MMTPType* aValue)
    {
    iChunkValue.OpenL(aDataType);
    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EPropertyCode, aPropertyCode);
    SetUint16L(EDatatype, aDataType);

    // Set optional construction values.
    if (aObjectHandle)
        {
        SetUint32L(EObjectHandle, *aObjectHandle);
        }

    if (aValue)
        {
        SetL(EValue, *aValue);
        }
    
    iInitialised = ETrue;
    }

