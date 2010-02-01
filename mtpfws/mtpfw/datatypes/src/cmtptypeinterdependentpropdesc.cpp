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
#include <mtp/cmtptypeinterdependentpropdesc.h>
#include <mtp/mtpdatatypeconstants.h>

#include "mtpdatatypespanic.h"

// Dataset constants    
const TUint CMTPTypeInterdependentPropDesc::KNumberOfInterdependenciesChunk(0);
const TUint CMTPTypeInterdependentPropDesc::KInterdependentPropertiesChunks(1);

const TUint CMTPTypeInterdependentProperties::KNumberOfPropDescsChunk(0);
const TUint CMTPTypeInterdependentProperties::KObjectPropDescChunks(1);

/**
MTP InterdependentPropDesc dataset factory method. This method is used to create an 
empty MTP InterdependentPropDesc dataset. 
@return A pointer to the MTP InterdependentPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeInterdependentPropDesc* CMTPTypeInterdependentPropDesc::NewL()
    {
    CMTPTypeInterdependentPropDesc* self = CMTPTypeInterdependentPropDesc::NewLC(); 
    CleanupStack::Pop(self); 
    return self;  
    }

/**
MTP InterdependentPropDesc dataset factory method. This method is used to create an 
empty MTP InterdependentPropDesc dataset. A pointer to the data type is placed on the
cleanup stack.
@return A pointer to the MTP InterdependentPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeInterdependentPropDesc* CMTPTypeInterdependentPropDesc::NewLC()
    {
    CMTPTypeInterdependentPropDesc* self = new(ELeave) CMTPTypeInterdependentPropDesc();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeInterdependentPropDesc::~CMTPTypeInterdependentPropDesc()
    {
    iChunksInterdependentProperties.ResetAndDestroy();
    }

/**
Appends the specified InterdependentPropDesc value quadruple to the InterdependentPropDesc. 
Ownership of the specified InterdependentPropDesc value quadruple is passed to the 
InterdependentPropDesc.
@param aElement The InterdependentPropDesc value quadruple to append. Ownership IS
transferred.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C void CMTPTypeInterdependentPropDesc::AppendL(CMTPTypeInterdependentProperties* aInterdependentProperties)
    {
    // Increment NumberOfElements.
    const TUint index(iChunkNumberOfInterdependencies.Value());
    iChunkNumberOfInterdependencies.Set(index + 1);
    
    // Append the element.
    AppendElementChunkL(aInterdependentProperties);
    }

/**
Provides the NumberOfElements element value.
*/
EXPORT_C TUint32 CMTPTypeInterdependentPropDesc::NumberOfInterdependencies() const
    {
    return iChunkNumberOfInterdependencies.Value();
    }

/**
Provides a non-const reference to the InterdependentPropDesc value quadruple element at
the specified index.
@param aIndex The index of the required element.
@param aValue On successful completion, a non-const reference to the requested
element.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C CMTPTypeInterdependentProperties& CMTPTypeInterdependentPropDesc::InterdependentProperties(TUint aIndex) const
    {
    return *iChunksInterdependentProperties[aIndex];
    }

EXPORT_C TInt CMTPTypeInterdependentPropDesc::FirstWriteChunk(TPtr8& aChunk)
    {   
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); (i >= (KNumberOfInterdependenciesChunk + 1)); i--)
        {
        ChunkRemove(i);
        }
    iChunksInterdependentProperties.ResetAndDestroy();
    
    // Setup the write chunk pointer.
    TInt err(UpdateWriteSequenceErr(CMTPTypeCompoundBase::FirstWriteChunk(aChunk)));
    switch (err)
        {
    case KMTPChunkSequenceCompletion:
        err = KErrNone;
        // Don't break, fall through to set the write sequence state.
        
    case KErrNone:
        // Set the write sequence state.
        iWriteSequenceState = EInProgress;
        break;
        
    default:
        break;
        }
        
    return  err;
    }
    
EXPORT_C TInt CMTPTypeInterdependentPropDesc::NextWriteChunk(TPtr8& aChunk)
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

EXPORT_C TUint CMTPTypeInterdependentPropDesc::Type() const
    {
    return EMTPTypeInterdependentPropDescDataset;
    }

EXPORT_C TBool CMTPTypeInterdependentPropDesc::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeInterdependentPropDesc::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EInProgress:
            if ((iChunkNumberOfInterdependencies.Value()) && 
                (iChunksInterdependentProperties.Count() < iChunkNumberOfInterdependencies.Value()))
                {
                // Allocate the next element chunk.
                CMTPTypeInterdependentProperties* element = CMTPTypeInterdependentProperties::NewLC();
                AppendElementChunkL(element);
                CleanupStack::Pop(element);
                }
            else
                {
                iWriteSequenceState = EIdle;
                }
            break;
            
        case EIdle:
            // Completing the last element in the write sequence.
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
 
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeInterdependentPropDesc::ElementInfo(TInt aElementId) const
    {
    if (aElementId == KNumberOfInterdependenciesChunk)
        {
        return iInfoNumberOfInterdependencies;
        }
    else
        {
        __ASSERT_ALWAYS(((aElementId - KInterdependentPropertiesChunks) < iChunkNumberOfInterdependencies.Value()), Panic(EMTPTypeBoundsError));
        iInfoInterdependentProperties.iChunkId = aElementId;
        return iInfoInterdependentProperties; 
        }
    }

TInt CMTPTypeInterdependentPropDesc::ValidateChunkCount() const
    {
    TInt ret(KErrNone);
    if (NumberOfInterdependencies() != (ChunkCount() - 1))
        {
        ret = KMTPDataTypeInvalid;
        }
    return ret;
    }

CMTPTypeInterdependentPropDesc::CMTPTypeInterdependentPropDesc() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfInterdependencies          = KDefaultInfo; 
    iInfoNumberOfInterdependencies.iType    = EMTPTypeUINT32;
    iInfoInterdependentProperties           = KDefaultInfo; 
    iInfoInterdependentProperties.iType     = EMTPTypeInterdependentPropDescElementDataset;
    }

void CMTPTypeInterdependentPropDesc::ConstructL()
    {
    ChunkAppendL(iChunkNumberOfInterdependencies);
    }
    
void CMTPTypeInterdependentPropDesc::AppendElementChunkL(CMTPTypeInterdependentProperties* aElement)
    {
    iChunksInterdependentProperties.AppendL(aElement);
    ChunkAppendL(*aElement);
    }
    
TInt CMTPTypeInterdependentPropDesc::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }

/**
MTP InterdependentPropDesc array element dataset factory method. This method is
used to create an empty MTP InterdependentPropDesc array element dataset. 
@return A pointer to the MTP InterdependentPropDesc array element dataset type.
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeInterdependentProperties* CMTPTypeInterdependentProperties::NewL()
    {
    CMTPTypeInterdependentProperties* self = CMTPTypeInterdependentProperties::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

/**
MTP InterdependentPropDesc array element dataset factory method. This method is
used to create an empty MTP InterdependentPropDesc array element dataset. A 
pointer to the data type is placed on the cleanup stack.
@return A pointer to the MTP InterdependentPropDesc array element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeInterdependentProperties* CMTPTypeInterdependentProperties::NewLC()
    {
    CMTPTypeInterdependentProperties* self = new(ELeave) CMTPTypeInterdependentProperties();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeInterdependentProperties::~CMTPTypeInterdependentProperties()
    {
    iChunksObjectPropDesc.ResetAndDestroy();
    }

/**
Appends the specified ObjectPropDesc dataset to the InterdependentPropDesc 
array element array. Ownership of the specified ObjectPropDesc dataset is 
passed to the InterdependentPropDesc array element array.
@param aElement The ObjectPropDesc dataset to append. Ownership IS transferred.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C void CMTPTypeInterdependentProperties::AppendL(CMTPTypeObjectPropDesc* aElement)
    {
    // Increment NumberOfElements.
    const TUint index(iChunkNumberOfPropDescs.Value());
    iChunkNumberOfPropDescs.Set(index + 1);
    
    // Append the element.
    AppendElementChunkL(aElement);
    }

/**
Provides the NumberOfPropDescs element value.
*/
EXPORT_C TUint32 CMTPTypeInterdependentProperties::NumberOfPropDescs() const
    {
    return iChunkNumberOfPropDescs.Value();
    }

/**
Provides a non-const reference to the InterdependentPropDesc value quadruple element at
the specified index.
@param aIndex The index of the required element.
@param aValue On successful completion, a non-const reference to the requested
element.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C CMTPTypeObjectPropDesc& CMTPTypeInterdependentProperties::ObjectPropDesc(TUint aIndex) const
    {
    return *iChunksObjectPropDesc[aIndex];
    }

EXPORT_C TInt CMTPTypeInterdependentProperties::FirstWriteChunk(TPtr8& aChunk)
    {   
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); (i >= (KNumberOfPropDescsChunk + 1)); i--)
        {
        ChunkRemove(i);
        }
    iChunksObjectPropDesc.ResetAndDestroy();
    
    // Setup the write chunk pointer.
    TInt err(UpdateWriteSequenceErr(CMTPTypeCompoundBase::FirstWriteChunk(aChunk)));
    switch (err)
        {
    case KMTPChunkSequenceCompletion:
        err = KErrNone;
        // Don't break, fall through to set the write sequence state.
        
    case KErrNone:
        // Set the write sequence state.
        iWriteSequenceState = EInProgress;
        break;
        
    default:
        break;
        }
        
    return  err;
    }
    
EXPORT_C TInt CMTPTypeInterdependentProperties::NextWriteChunk(TPtr8& aChunk)
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

EXPORT_C TUint CMTPTypeInterdependentProperties::Type() const
    {
    return EMTPTypeInterdependentPropDescElementDataset;
    }

EXPORT_C TBool CMTPTypeInterdependentProperties::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeInterdependentProperties::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EInProgress:
            if ( ( iChunkNumberOfPropDescs.Value() ) &&
            (iChunksObjectPropDesc.Count() < iChunkNumberOfPropDescs.Value()))
                {
                // Allocate the next element chunk.
                CMTPTypeObjectPropDesc* element = CMTPTypeObjectPropDesc::NewLC();
                AppendElementChunkL(element);
                CleanupStack::Pop(element); 
                }
            else
                {
                iWriteSequenceState = EIdle;
                }
            break;
            
        case EIdle:
            // Completing the last element in the write sequence.
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
 
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeInterdependentProperties::ElementInfo(TInt aElementId) const
    {
    if (aElementId == KNumberOfPropDescsChunk)
        {
        return iInfoNumberOfPropDescs;
        }
    else
        {
        __ASSERT_ALWAYS(((aElementId - KObjectPropDescChunks) < iChunkNumberOfPropDescs.Value()), Panic(EMTPTypeBoundsError));
        iInfoObjectPropDesc.iChunkId = aElementId;
        return iInfoObjectPropDesc; 
        }
    }

TInt CMTPTypeInterdependentProperties::ValidateChunkCount() const
    {
    TInt ret(KErrNone);
    if (NumberOfPropDescs() != (ChunkCount() - 1))
        {
        ret = KMTPDataTypeInvalid;
        }
    return ret;
    }

CMTPTypeInterdependentProperties::CMTPTypeInterdependentProperties() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfPropDescs          = KDefaultInfo; 
    iInfoNumberOfPropDescs.iType    = EMTPTypeUINT32;
    iInfoObjectPropDesc             = KDefaultInfo; 
    iInfoObjectPropDesc.iType       = EMTPTypeInterdependentPropDescElementDataset;
    }

void CMTPTypeInterdependentProperties::ConstructL()
    {
    ChunkAppendL(iChunkNumberOfPropDescs);
    }
    
void CMTPTypeInterdependentProperties::AppendElementChunkL(CMTPTypeObjectPropDesc* aElement)
    {
    iChunksObjectPropDesc.AppendL(aElement);
    ChunkAppendL(*aElement);
    }
    
TInt CMTPTypeInterdependentProperties::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }
