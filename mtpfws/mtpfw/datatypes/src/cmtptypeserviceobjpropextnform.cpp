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

#include <mtp/cmtptypeserviceobjpropextnform.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypestring.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint CMTPTypeServiceObjPropExtnForm::KFlat1ChunkSize(20);
const TUint CMTPTypeServiceObjPropExtnForm::KFlat2ChunkSize(1);
const TUint CMTPTypeServiceObjPropExtnForm::KNumChunksWithoutForm(EIdFlat2Chunk + 1);
const TUint CMTPTypeServiceObjPropExtnForm::KNumChunksWithForm(EIdFormChunk + 1);

//Dataset element metadata    
const CMTPTypeCompoundBase::TElementInfo CMTPTypeServiceObjPropExtnForm::iElementMetaData[CMTPTypeServiceObjPropExtnForm::ENumElements] = 
    {
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT128,    0,                 KMTPTypeUINT128Size}},   // EPKNamespace
        {EIdFlat1Chunk,       EMTPTypeFlat,       {EMTPTypeUINT32,    16,                 KMTPTypeUINT32Size}},    // EPKeyID
        {EIdNameChunk,        EMTPTypeString,     {EMTPTypeString, KMTPNotApplicable,     KMTPNotApplicable}},     // EPropertyName
        {EIdFlat2Chunk,       EMTPTypeFlat,       {EMTPTypeUINT8,     0,                  KMTPTypeUINT8Size}},     // EFormFlag
        {EIdFormChunk,        EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},     // EForm
    };


/**
MTP Service Object Property Extension FORM dataset factory method. This method is used to 
create an empty MTP Service Object Property Extension FORM  of the specified Datatype. 
@param aDataType The data type identifier datacode of the object property.
@return A pointer to the form type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C  CMTPTypeServiceObjPropExtnForm* CMTPTypeServiceObjPropExtnForm::NewL( const TUint aDataType )
    {
    CMTPTypeServiceObjPropExtnForm* self = CMTPTypeServiceObjPropExtnForm::NewLC( aDataType );
    CleanupStack::Pop(self); 
    return self;
    }
/**
MTP DevicePropDesc  Service Object Property Extension FORM  factory method. This method is used to 
create an empty MTP Service Object Property Extension FORM  of the specified Datatype. 
@param aDataType The data type identifier datacode of the the object property.
@return A pointer to the form type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/
EXPORT_C  CMTPTypeServiceObjPropExtnForm* CMTPTypeServiceObjPropExtnForm::NewLC( const TUint aDataType )
    {
    CMTPTypeServiceObjPropExtnForm* self = new(ELeave) CMTPTypeServiceObjPropExtnForm( aDataType );
    CleanupStack::PushL(self);
    TMTPTypeGuid KUndefinedNamespace(0,0);
    TUint KPKeyID(0);
    TBuf<1> KName;
    self->ConstructL( KUndefinedNamespace, KPKeyID, KName, CMTPTypeObjectPropDesc::ENone, NULL );
    
    return self;
    }

   
EXPORT_C  CMTPTypeServiceObjPropExtnForm* CMTPTypeServiceObjPropExtnForm::NewL( const TUint aDataType, const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeServiceObjPropExtnForm* self = CMTPTypeServiceObjPropExtnForm::NewLC( aDataType, aPKNamespace, aPKID, aName, aFormFlag, aForm );
    CleanupStack::Pop(self); 
    return self;
    }
   
  
EXPORT_C  CMTPTypeServiceObjPropExtnForm* CMTPTypeServiceObjPropExtnForm::NewLC( const TUint aDataType, const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeServiceObjPropExtnForm* self = new(ELeave) CMTPTypeServiceObjPropExtnForm(aDataType);
    CleanupStack::PushL(self);
    self->ConstructL( aPKNamespace, aPKID, aName, aFormFlag, aForm );
    return self;
    }


/**
Destructor.
*/
EXPORT_C CMTPTypeServiceObjPropExtnForm::~CMTPTypeServiceObjPropExtnForm()
    {
    iChunkFlat1.Close();
    delete iChunkName;
    iChunkFlat2.Close();
    iChunkForm.Close();
    }

CMTPTypeServiceObjPropExtnForm::CMTPTypeServiceObjPropExtnForm(const TUint aDataType) : 
    CMTPTypeCompoundBase((KJustInTimeConstruction), KNumChunksWithoutForm),
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this),
    iChunkFlat2(KFlat2ChunkSize, *this),
    iDataType(aDataType),
    iInitialised(EFalse)
    {

    }

EXPORT_C TUint CMTPTypeServiceObjPropExtnForm::Type() const
    {
    return EMTPTypeServiceObjPropExtnFormDataset;
    } 



EXPORT_C TInt CMTPTypeServiceObjPropExtnForm::FirstWriteChunk(TPtr8& aChunk)
    {
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); i > EIdFlat1Chunk ; i--)
      {
      ChunkRemove(i);
      }
      
    // Setup the write chunk pointer.
    TInt err(UpdateWriteSequenceErr(CMTPTypeCompoundBase::FirstWriteChunk(aChunk)));
    switch (err)
      {
    case KMTPChunkSequenceCompletion:
      err = KErrNone;
      // Don't break, fall through to set the write sequence state.
      
    case KErrNone:
      // Set the write sequence state.
      iWriteSequenceState = EFlat1Chunk; 
      
      /* 
      Set the write sequence completion state. Initially assume that the 
      extension form has a FORM field. If no FORM field is subsequently
      detected in the write data stream then the completion state is 
      adjusted accordingly.
      */
      iWriteSequenceCompletionState = EFormChunk;
      break;
      
    default:
      break;
      }
      
    return  err;
    }

EXPORT_C TInt CMTPTypeServiceObjPropExtnForm::NextWriteChunk(TPtr8& aChunk)
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

EXPORT_C TBool CMTPTypeServiceObjPropExtnForm::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeServiceObjPropExtnForm::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EFlat1Chunk:     
            delete iChunkName;
            iChunkName = NULL;
            iChunkName = CMTPTypeString::NewL();
            ChunkAppendL(*iChunkName);
            break;
            
        case ENameChunk:
            ChunkAppendL(iChunkFlat2);
            break;
            
        case EFlat2Chunk: 
            {
            iChunkForm.Close();
            TUint8 flag(Uint8L(EFormFlag));
            iChunkForm.SetMeta(flag, iDataType);
            if (HasFormField(flag))
                {
                iChunkForm.OpenL(iElementInfo[EForm].iType);
                ChunkAppendL(iChunkForm);
                SetExpectedChunkCount(KNumChunksWithForm);
                }
            else
                {
                // Adjust the write sequence completion state.            
                iWriteSequenceCompletionState = EFlat2Chunk;
                }   
            }
            break;
            
        case EFormChunk:
        case EIdle:
        default:
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
    }
        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }
    return NULL;
    }

TBool CMTPTypeServiceObjPropExtnForm::HasFormField(TUint8 aFormFlag) const
    {
    return ((aFormFlag != CMTPTypeObjectPropDesc::EDateTimeForm) && (aFormFlag != CMTPTypeObjectPropDesc::ENone) && (aFormFlag != CMTPTypeObjectPropDesc::EObjectIDForm) );
    }

TInt CMTPTypeServiceObjPropExtnForm::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }

TBool CMTPTypeServiceObjPropExtnForm::ReadableElementL(TInt aElementId) const
    {
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    
    TBool ret(ETrue);
    if (aElementId == EForm)
        {
        ret = HasFormField(Uint8L(EFormFlag));
        }
    return ret;
    }
    
TBool CMTPTypeServiceObjPropExtnForm::WriteableElementL(TInt aElementId) const
    {  
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    
    if (!iInitialised)
        {
        return ETrue;    
        }
    
    if (aElementId == EForm)
        return EFalse;
    
    return ETrue;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServiceObjPropExtnForm::ElementInfo(TInt aElementId) const
    {
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    
    return iElementInfo[aElementId];  
    }   

void CMTPTypeServiceObjPropExtnForm::ConstructL( const TMTPTypeGuid  aPKNamespace, const TUint aPKID, const TDesC& aName, const TUint8 aFormFlag, const MMTPType* aForm )
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
               
           case EIdNameChunk:
               iChunkName = CMTPTypeString::NewL();
               chunk = iChunkName;
               break;
               
           case EIdFlat2Chunk:
               iChunkFlat2.OpenL();
               chunk = &iChunkFlat2;
               break;
               
           case EIdFormChunk:
               iChunkForm.SetMeta(aFormFlag, iDataType);
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
    SetL( EPKeyNamespace, aPKNamespace );
    SetUint32L( EPKeyID, aPKID );
    SetStringL( EPropertyName, aName );
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
    
    iInitialised = ETrue;
    }
