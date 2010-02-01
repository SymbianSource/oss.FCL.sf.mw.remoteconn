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

#include <mtp/cmtptypelist.h>

#include "mtpdatatypespanic.h"
#include <mtp/tmtptypedatapair.h>

const TUint CMTPTypeList::KChunkNumberofElements(0);


const TUint CMTPTypeList::KChunckNumberOfHeader(1);

EXPORT_C CMTPTypeList* CMTPTypeList::NewL(TMTPTypeIds aListType, TMTPTypeIds aElementType)
    {
    CMTPTypeList* self = CMTPTypeList::NewLC(aListType, aElementType); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeList* CMTPTypeList::NewLC(TMTPTypeIds aListType, TMTPTypeIds aElementType)
    {
    CMTPTypeList* self = new(ELeave) CMTPTypeList(aListType, aElementType);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C  CMTPTypeList::~CMTPTypeList()
    {
    ResetAndDestroy();
    }

CMTPTypeList::CMTPTypeList(TInt aListType, TInt aElementType):
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount),
    iListType(aListType),
    iElementType(aElementType),
    iChunkNumberOfElements(KChunkNumberofElements)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfElements       = KDefaultInfo; 
    iInfoNumberOfElements.iType = EMTPTypeUINT32;
    iInfoElement                = KDefaultInfo; 
    iInfoElement.iType          = aElementType;
    }

void CMTPTypeList::InitListL()
    {
    ChunkAppendL(iChunkNumberOfElements);
    }

void CMTPTypeList::ConstructL()
    {
    InitListL();
    }


EXPORT_C void CMTPTypeList::AppendL(const MMTPType* aElement)
    {
    if (aElement == NULL)
        {
        User::Leave(KMTPDataTypeInvalid);
        }
             
    // Append the element.
    AppendElementChunkL(aElement);
    
    // Increment NumberOfElements.
    const TUint index(iChunkNumberOfElements.Value());
    iChunkNumberOfElements.Set(index + 1);
    }

EXPORT_C void CMTPTypeList::Remove(const TInt aIndex)
    {
        
    MMTPType* tmp = iChunksElement[aIndex];
    iChunksElement.Remove(aIndex);
    ChunkRemove(aIndex + 1);
    RMTPType::Destroy(iElementType,tmp );
    
    const TUint index(iChunkNumberOfElements.Value());
    iChunkNumberOfElements.Set(index - 1);
    }

EXPORT_C TInt CMTPTypeList::ElementType() const
    {
    return iElementType;
    }

EXPORT_C TUint32 CMTPTypeList::NumberOfElements() const
    {
    return iChunkNumberOfElements.Value();
    }

EXPORT_C MMTPType& CMTPTypeList::ElementL(const TInt aIndex) const
	{
	__ASSERT_ALWAYS(( aIndex < NumberOfElements() ), User::Leave(EMTPTypeBoundsError));

	return *iChunksElement[aIndex];
	}

EXPORT_C TInt CMTPTypeList::FirstWriteChunk(TPtr8& aChunk)
    {
    ResetAndDestroy();
    
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

EXPORT_C TInt CMTPTypeList::NextWriteChunk(TPtr8& aChunk)
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

EXPORT_C TInt CMTPTypeList::NextWriteChunk(TPtr8& aChunk, TUint /*aDataLength*/)
	{
	return NextWriteChunk(aChunk);
	}

EXPORT_C TUint CMTPTypeList::Type() const
    {
    return iListType;
    }

EXPORT_C TBool CMTPTypeList::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeList::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EElementChunks:
            if ((iChunkNumberOfElements.Value()) && 
                (iChunksElement.Count() < iChunkNumberOfElements.Value()))
                {
                MMTPType* element = RMTPType::AllocL(iElementType);
                CleanupStack::PushL(element);
                AppendElementChunkL(element);
                CleanupStack::Pop(element); 
                }
            else
                {
                iWriteSequenceState = EIdle;
                }
            break;
            
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


const CMTPTypeCompoundBase::TElementInfo& CMTPTypeList::ElementInfo(TInt aElementId) const
    {
    if (aElementId == KChunkNumberofElements)
        {
        return iInfoNumberOfElements;
        }
    else
        {
        __ASSERT_ALWAYS(((aElementId - KChunckNumberOfHeader) < iChunkNumberOfElements.Value()), Panic(EMTPTypeBoundsError));
        iInfoElement.iChunkId = aElementId;
        return iInfoElement; 
        }
    }


TInt CMTPTypeList::ValidateChunkCount() const
    {
    if (NumberOfElements() != (ChunkCount() - 1))
        {
        return KMTPDataTypeInvalid;
        }
    
    return KErrNone;
    }

TInt CMTPTypeList::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }


void CMTPTypeList::AppendElementChunkL(const MMTPType* aElement)
    {
    iChunksElement.AppendL(aElement);
    ChunkAppendL(*aElement);
    }

EXPORT_C void CMTPTypeList::ResetAndDestroy()
    {
    const TUint num = iChunksElement.Count();
    for(TInt i = num -1  ; i  >= 0 ; i-- )
        {
        Remove(i);
        }
    
    iChunksElement.Close();
    }

