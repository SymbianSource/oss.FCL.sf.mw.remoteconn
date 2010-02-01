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
@internalComponent
*/

#include <mtp/cmtptypeservicepropdesclist.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpdatatypespanic.h"

// Dataset constants

const TUint CMTPTypeServicePropDesc::KFlat1ChunkSize(6);
const TUint CMTPTypeServicePropDesc::KNumChunksWithoutForm(EIdFlat1Chunk + 1);
const TUint CMTPTypeServicePropDesc::KNumChunksWithForm(EIdFormChunk + 1);

//Dataset element metadata    
const CMTPTypeCompoundBase::TElementInfo CMTPTypeServicePropDesc::iElementMetaData[CMTPTypeServicePropDesc::ENumElements] = 
    {
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EServicePropertyCode
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT16,    2,                  KMTPTypeUINT16Size}},   // EDataType1
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT8,     4,                  KMTPTypeUINT8Size}},   // EGetSet1
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT8,     5,                  KMTPTypeUINT8Size}},    // EFormFlag
        {EIdFormChunk,        EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EForm
    };



EXPORT_C CMTPTypeServicePropDescList* CMTPTypeServicePropDescList::NewL()
    {
    CMTPTypeServicePropDescList* self = CMTPTypeServicePropDescList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }


EXPORT_C CMTPTypeServicePropDescList* CMTPTypeServicePropDescList::NewLC()
    {
    CMTPTypeServicePropDescList* self = new(ELeave) CMTPTypeServicePropDescList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeServicePropDescList::~CMTPTypeServicePropDescList()
    {
    
    }

CMTPTypeServicePropDescList::CMTPTypeServicePropDescList() :
CMTPTypeList(EMTPTypeServicePropDescList,EMTPTypeServicePropDesc)
    {
    }

void CMTPTypeServicePropDescList::ConstructL()
    {
    InitListL();
    }

EXPORT_C void CMTPTypeServicePropDescList::AppendL(CMTPTypeServicePropDesc* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }

EXPORT_C CMTPTypeServicePropDesc& CMTPTypeServicePropDescList::ElementL(TUint aIndex) const
    {
    return static_cast<CMTPTypeServicePropDesc&>( CMTPTypeList::ElementL(aIndex) );
    }

/**
MTP Service Object Property Extension FORM dataset factory method. This method is used to 
create an empty MTP Service Object Property Extension FORM  of the specified Datatype. 
@return A pointer to the form type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C  CMTPTypeServicePropDesc* CMTPTypeServicePropDesc::NewL()
    {
    CMTPTypeServicePropDesc* self = CMTPTypeServicePropDesc::NewLC();
    CleanupStack::Pop(self); 
    return self;
    }
/**
MTP DevicePropDesc  Service Object Property Extension FORM  factory method. This method is used to 
create an empty MTP Service Object Property Extension FORM  of the specified Datatype. 
@return A pointer to the form type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C  CMTPTypeServicePropDesc* CMTPTypeServicePropDesc::NewLC()
    {
    CMTPTypeServicePropDesc* self = new(ELeave) CMTPTypeServicePropDesc();
    CleanupStack::PushL(self);
    const TUint16 KUndefinedCode = 0; 
    self->ConstructL( KUndefinedCode , EMTPTypeUndefined,CMTPTypeObjectPropDesc::EReadOnly , CMTPTypeObjectPropDesc::ENone, NULL );
    return self;
    }
   
EXPORT_C  CMTPTypeServicePropDesc* CMTPTypeServicePropDesc::NewL( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm )
    {
    CMTPTypeServicePropDesc* self = CMTPTypeServicePropDesc::NewLC( aServicePropCode, aDataType, aGetSet, aFormFlag, aForm );
    CleanupStack::Pop(self); 
    return self;
    }
   
  
EXPORT_C  CMTPTypeServicePropDesc* CMTPTypeServicePropDesc::NewLC( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeServicePropDesc* self = new(ELeave) CMTPTypeServicePropDesc();
    CleanupStack::PushL(self);
    self->ConstructL( aServicePropCode, aDataType, aGetSet, aFormFlag, aForm );
    return self;
    }


/**
Destructor.
*/
EXPORT_C CMTPTypeServicePropDesc::~CMTPTypeServicePropDesc()
    {
    iChunkFlat1.Close();
    iChunkForm.Close();
    }

CMTPTypeServicePropDesc::CMTPTypeServicePropDesc() : 
    CMTPTypeCompoundBase((KJustInTimeConstruction), KNumChunksWithoutForm),
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this)
    {

    }

EXPORT_C TUint CMTPTypeServicePropDesc::Type() const
    {
    return EMTPTypeServicePropDesc;
    } 



EXPORT_C TInt CMTPTypeServicePropDesc::FirstWriteChunk(TPtr8& aChunk)
    {
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); (i >= (EIdFlat1Chunk + 1)); i--)
      {
      ChunkRemove(i);
      }
      
    // Setup the write chunk pointer.
       TInt err(CMTPTypeCompoundBase::FirstWriteChunk(aChunk));
       switch (err)
           {
       case KMTPChunkSequenceCompletion:
           err = KErrNone;
           // Don't break, fall through to set the write sequence state.
           
       case KErrNone:
           // Set the write sequence state.
           iWriteSequenceState = EFormChunk;
           iWriteSequenceCompletionState = EFormChunk;
           break;
           
       default:
           break;
           }
      
    return  err;
    }

EXPORT_C TInt CMTPTypeServicePropDesc::NextWriteChunk(TPtr8& /*aChunk*/)
    {
        return KMTPChunkSequenceCompletion;
    }


EXPORT_C TBool CMTPTypeServicePropDesc::CommitRequired() const
    {
    return ETrue;
    }


EXPORT_C MMTPType* CMTPTypeServicePropDesc::CommitChunkL(TPtr8& aChunk)
    {
        switch (iWriteSequenceState)
            {
        case EFormChunk:
            {
            iChunkForm.Close();
            TUint8 flag(Uint8L(EFormFlag));
            TUint16 type(Uint16L(EDataType1));
            iChunkForm.SetMeta(flag, type);
            if (HasFormField(flag))
                {
                iChunkForm.OpenL(iElementInfo[EForm].iType);
                ChunkAppendL(iChunkForm);
                SetExpectedChunkCount(KNumChunksWithForm);
                }
            else
                {
                // Adjust the write sequence completion state.            
                iWriteSequenceCompletionState = EFormChunk;
                }   
            }
            break;
            
        case EIdle:
            // Completing the last element in the write sequence.
            break;
            
        default:
            Panic(EMTPTypeBadStorage);
            break;
            }
        
    if ( (iWriteSequenceState != EIdle ) && (iWriteSequenceState < iWriteSequenceCompletionState) )
        {
        iWriteSequenceState++;
        }
    else
        {
        iWriteSequenceState = EIdle;
        }

        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }
    return NULL;
    }


TBool CMTPTypeServicePropDesc::HasFormField(TUint8 aFormFlag) const
    {
    return ((aFormFlag != CMTPTypeObjectPropDesc::EDateTimeForm) && (aFormFlag != CMTPTypeObjectPropDesc::ENone)&&(aFormFlag != CMTPTypeObjectPropDesc::EObjectIDForm));
    }


const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServicePropDesc::ElementInfo(TInt aElementId) const
    {
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    
    return iElementInfo[aElementId];  
    }

void CMTPTypeServicePropDesc::ConstructL( const TUint16  aServicePropCode, const TUint16 aDataType, const TUint8 aGetSet, const TUint8 aFormFlag, const MMTPType* aForm )
    {  
    for (TUint i(0); (i < ENumElements); i++)
       {
       const TElementInfo& element(iElementInfo[i]);
       if (ChunkCount() <= element.iChunkId)
           {
           MMTPType* chunk(NULL);
           switch (element.iChunkId)
               {
           case EIdFlat1Chunk:
               iChunkFlat1.OpenL();
               chunk = &iChunkFlat1;
               break;
               
           case EIdFormChunk:
               iChunkForm.SetMeta(aFormFlag, aDataType);
               if (HasFormField(aFormFlag))
                   {
                   iChunkForm.OpenL(element.iType);
                   chunk = &iChunkForm;
                   SetExpectedChunkCount(KNumChunksWithForm);
                   }
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
    
    // Set the construction values.
    SetUint16L( EServicePropertyCode, aServicePropCode );
    SetUint16L( EDataType1, aDataType );
    SetUint8L( EGetSet1, aGetSet );
    SetUint8L(EFormFlag, aFormFlag);
    
    if (aForm)
        {
        if (!HasFormField(aFormFlag))
            { 
            User::Leave(KMTPDataTypeInvalid);
            } 
        else
            {
            SetL(EForm, *aForm); 
            }
        }
    }




