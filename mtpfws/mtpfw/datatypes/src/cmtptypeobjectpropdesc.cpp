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
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypeserviceobjpropextnform.h>
#include <mtp/cmtptypeservicemethodparamextnform.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint CMTPTypeObjectPropDesc::KFlat1ChunkSize(5);
const TUint CMTPTypeObjectPropDesc::KFlat2ChunkSize(5);

const TUint CMTPTypeObjectPropDesc::KNumChunksWithoutForm(EIdFlat2Chunk + 1);
const TUint CMTPTypeObjectPropDesc::KNumChunksWithForm(EIdFormChunk + 1);

const TUint CMTPTypeObjectPropDescEnumerationForm::KNumberOfValuesChunk(0);
const TUint CMTPTypeObjectPropDescEnumerationForm::KValueChunks(1);

// Dataset element metadata    
const CMTPTypeCompoundBase::TElementInfo CMTPTypeObjectPropDesc::iElementMetaData[CMTPTypeObjectPropDesc::ENumElements] = 
    {
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EPropertyCode
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT16,    2,                  KMTPTypeUINT16Size}},   // EDatatype
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT8,     4,                  KMTPTypeUINT8Size}},    // EGetSet
        {EIdDefaultValueChunk,  EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EDefaultValue
        {EIdFlat2Chunk,         EMTPTypeFlat,       {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EGroupCode
        {EIdFlat2Chunk,         EMTPTypeFlat,       {EMTPTypeUINT8,     4,                  KMTPTypeUINT8Size}},    // EFormFlag
        {EIdFormChunk,          EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EForm
    };


EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewL()
    {
    CMTPTypeObjectPropDesc* self = NewLC();
    CleanupStack::Pop(self);
    return self;    
    }


/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
empty MTP ObjectPropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode. 
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewL(TUint16 aPropertyCode)
    {
    CMTPTypeObjectPropDesc* self = NewLC(aPropertyCode);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
empty MTP ObjectPropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode. A pointer to the data type is placed on
the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewLC(TUint16 aPropertyCode)
    {    
    CMTPTypeObjectPropDesc* self = new (ELeave) CMTPTypeObjectPropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, NULL);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
MTP ObjectPropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode, and the specified FORM. 
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewL(TUint16 aPropertyCode, const MMTPType& aForm)
    {
    CMTPTypeObjectPropDesc* self = NewLC(aPropertyCode, aForm);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
MTP ObjectPropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode, and the specified FORM. A pointer 
to the data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewLC(TUint16 aPropertyCode, const MMTPType& aForm)
    {    
    CMTPTypeObjectPropDesc* self = new (ELeave) CMTPTypeObjectPropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, &aForm);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
MTP ObjectPropDesc dataset type with the default Datatype defined for the 
specified PropertyCode, and the specified Form Flag and FORM.  
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aFormFlag The required Form Flag of the ObjectPropDesc.
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewL(TUint16 aPropertyCode, TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeObjectPropDesc* self = NewLC(aPropertyCode, aFormFlag, aForm);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an 
MTP ObjectPropDesc dataset type with the default Datatype defined for the 
specified PropertyCode, and the specified Form Flag and FORM. A pointer 
to the data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aFormFlag The required Form Flag of the ObjectPropDesc.
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewLC(TUint16 aPropertyCode, TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeObjectPropDesc* self = new (ELeave) CMTPTypeObjectPropDesc(); 
    CleanupStack::PushL(self); 
    TPropertyInfo info;
    PropertyInfoL(aPropertyCode, info);
    info.iFormFlag = aFormFlag;
    self->ConstructL(aPropertyCode, info, aForm);
    return self;    
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an  
MTP ObjectPropDesc dataset type with the specified values.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aInfo The MTP ObjectPropDesc dataset characteristics (meta data).
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/    
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {
    CMTPTypeObjectPropDesc* self = CMTPTypeObjectPropDesc::NewLC(aPropertyCode, aInfo, aForm); 
    CleanupStack::Pop(self);
    return self; 
    }

/**
MTP ObjectPropDesc dataset factory method. This method is used to create an  
MTP ObjectPropDesc dataset type with the specified values. A pointer to the 
data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the ObjectPropDesc dataset.
@param aInfo The MTP ObjectPropDesc dataset characteristics (meta data).
@param aForm The optional ObjectPropDesc FORM dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave KMTPDataTypeInvalid, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewLC(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {
    CMTPTypeObjectPropDesc* self = new (ELeave) CMTPTypeObjectPropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, aInfo, aForm);
    return self;    
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeObjectPropDesc::~CMTPTypeObjectPropDesc()
    {
    iChunkFlat1.Close();
    iChunkDefaultValue.Close();
    iChunkFlat2.Close();
    iChunkForm.Close();
    }

EXPORT_C TInt CMTPTypeObjectPropDesc::FirstWriteChunk(TPtr8& aChunk)
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
        ObjectPropDesc has a FORM field. If no FORM field is subsequently
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
    
EXPORT_C TInt CMTPTypeObjectPropDesc::NextWriteChunk(TPtr8& aChunk)
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
 
EXPORT_C TUint CMTPTypeObjectPropDesc::Type() const
    {
    return EMTPTypeObjectPropDescDataset;
    }
    
EXPORT_C TBool CMTPTypeObjectPropDesc::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeObjectPropDesc::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EFlat1Chunk:     
            /* 
            Datatype is available, allocate sufficient chunk storage for 
            and append the DefaultValue chunk.
            */
            iChunkDefaultValue.Close();
            iChunkDefaultValue.OpenL(Uint16L(EDatatype));
            ChunkAppendL(iChunkDefaultValue);
            break;
            
        case EDefaultValueChunk:
            // Append the next chunk.
            ChunkAppendL(iChunkFlat2);
            break;
            
        case EFlat2Chunk: 
            /* 
            FormFlag and Datatype are available, allocate sufficient storage for 
            and append the FORM chunk.
            */
            {
            iChunkForm.Close();
            TUint8 flag(Uint8L(EFormFlag));
            iChunkForm.SetMeta(flag, Uint16L(EDatatype));
            if (HasFormField(flag))
                {
                iChunkForm.OpenL(iElementInfo[EIdFormChunk].iType);
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
    if ((iWriteSequenceState != EIdle ) && (iWriteSequenceState < iWriteSequenceCompletionState))
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
    
void CMTPTypeObjectPropDesc::PropertyInfoL(TUint16 aPropertyCode, TPropertyInfo& aInfo)
    {    
    switch (aPropertyCode)
        {        
    case EMTPObjectPropCodeUndefined:
        aInfo.iDataType     = EMTPTypeUndefined;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
         
    case EMTPObjectPropCodeStorageID:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeObjectFormat:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeProtectionStatus:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeObjectSize:
        aInfo.iDataType     = EMTPTypeUINT64;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeAssociationType:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAssociationDesc:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeObjectFileName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDateCreated:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDateModified:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeKeywords:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeParentObject:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeAllowedFolderContents:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeHidden:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSystemObject:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
        aInfo.iDataType     = EMTPTypeUINT128;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeSyncID:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePropertyBag:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeCreatedBy:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeArtist:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDateAuthored:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDescription:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeURLReference:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeLanguageLocale:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeCopyrightInformation:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSource:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeOriginLocation:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDateAdded:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeNonConsumable:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeCorruptUnplayable:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeProducerSerialNumber:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleFormat:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleSize:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleHeight:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleWidth:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleDuration:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRepresentativeSampleData:
        aInfo.iDataType     = EMTPTypeAUINT8;
        aInfo.iFormFlag     = EByteArrayForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeWidth:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeHeight:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDuration:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRating:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeTrack:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeGenre:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeCredits:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeLyrics:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSubscriptionContentID:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeProducedBy:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeUseCount:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSkipCount:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeLastAccessed:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeParentalRating:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMetaGenre:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeComposer:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEffectiveRating:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSubtitle:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeOriginalReleaseDate:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAlbumName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAlbumArtist:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMood:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDRMStatus:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSubDescription:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeIsCropped:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeIsColourCorrected:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeImageBitDepth:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFnumber:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeExposureTime:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeExposureIndex:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeTotalBitRate:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeBitrateType:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSampleRate:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeNumberOfChannels:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAudioBitDepth:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeScanType:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAudioWAVECodec:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeAudioBitRate:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeVideoFourCCCodec:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeVideoBitRate:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFramesPerThousandSeconds:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeKeyFrameDistance:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeBufferSize:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEncodingQuality:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEncodingProfile:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeDisplayName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeBodyText:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSubject:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePriority:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeGivenName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMiddleNames:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFamilyName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePrefix:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeSuffix:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneticGivenName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneticFamilyName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailPrimary:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailPersonal1:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailPersonal2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailBusiness1:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailBusiness2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeEmailOthers:
        aInfo.iDataType     = EMTPTypeAINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberPrimary:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberPersonal:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberPersonal2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberBusiness:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberBusiness2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberMobile:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberMobile2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFaxNumberPrimary:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFaxNumberPersonal:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeFaxNumberBusiness:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePagerNumber:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneNumberOthers:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePrimaryWebAddress:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePersonalWebAddress:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeBusinessWebAddress:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeInstantMessengerAddress:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeInstantMessengerAddress2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeInstantMessengerAddress3:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ERegularExpressionForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalFull:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalLine1:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalLine2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalCity:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalRegion:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalPostalCode:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressPersonalCountry:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessFull:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessLine1:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessLine2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessCity:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessRegion:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessPostalCode:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressBusinessCountry:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherFull:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherLine1:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherLine2:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherCity:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherRegion:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherPostalCode:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePostalAddressOtherCountry:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeOrganizationName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodePhoneticOrganizationName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeRole:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeBirthdate:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageTo:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageCC:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageBCC:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageRead:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageReceivedTime:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeMessageSender:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityBeginTime:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityEndTime:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityLocation:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityRequiredAttendees:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityOptionalAttendees:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityResources:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityAccepted:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;	
        
    case EMTPObjectPropCodeActivityTentative:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityDeclined:
        aInfo.iDataType     = EMTPTypeAUINT16;
        aInfo.iFormFlag     = ELongStringForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityReminderTime:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = EDateTimeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityOwner:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPObjectPropCodeActivityStatus:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    default:
        if( aPropertyCode <= EMTPCodeUndefined1End )
            {
            aInfo.iDataType     = EMTPTypeUndefined;
            aInfo.iFormFlag     = ENone;
            aInfo.iGetSet       = EReadOnly;
            }
        else
            {
            User::Leave(KErrNotSupported);
            }
        break;
        }
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeObjectPropDesc::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }
    
TBool CMTPTypeObjectPropDesc::ReadableElementL(TInt aElementId) const
    {
    TBool ret(ETrue);
    if (aElementId == EForm)
        {
        ret = HasFormField(Uint8L(EFormFlag));
        }
    return ret;
    }
    
TBool CMTPTypeObjectPropDesc::WriteableElementL(TInt aElementId) const
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
        case EDefaultValue:
        case EGroupCode:
            ret = ETrue;
            break;

        default:
            break;
            }   
        }
        
    return ret;
    }

/*
MTP ObjectPropDesc dataset "pre-production" factory method for datasets
which will be populated from an MTP write data stream. This method is used to 
create a partially constructed MTP ObjectPropDesc element dataset type prior to
the construction characteristics being known. Construction of the element 
dataset is completed on-the-fly as it is written to from an MTP write data 
stream. A pointer to the data type is placed on the cleanup stack.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/
CMTPTypeObjectPropDesc* CMTPTypeObjectPropDesc::NewLC()
    {
    CMTPTypeObjectPropDesc* self = new(ELeave) CMTPTypeObjectPropDesc();
    CleanupStack::PushL(self);
    self->ConstructL(EMTPObjectPropCodeUndefined, NULL); 
    return self;
    }

CMTPTypeObjectPropDesc::CMTPTypeObjectPropDesc() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KNumChunksWithoutForm),
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this),
    iChunkFlat2(KFlat2ChunkSize, *this)
    {
    
    }
    
void CMTPTypeObjectPropDesc::ConstructL(TUint16 aPropertyCode, const MMTPType* aForm)
    { 
    TPropertyInfo info;
    PropertyInfoL(aPropertyCode, info);
    ConstructL(aPropertyCode, info, aForm);
    }
    
void CMTPTypeObjectPropDesc::ConstructL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {    
    // Construct the ObjectPropDesc chunk sequence.
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
                
            case EIdDefaultValueChunk:
                if (aInfo.iDataType != EMTPTypeUndefined)
                    {
                    iChunkDefaultValue.OpenL(aInfo.iDataType);
                    }
                chunk = &iChunkDefaultValue;
                break;
                
            case EIdFlat2Chunk:
                iChunkFlat2.OpenL();
                chunk = &iChunkFlat2;
                break;
                
            case EIdFormChunk:
                iChunkForm.SetMeta(aInfo.iFormFlag, aInfo.iDataType);
                if (HasFormField(aInfo.iFormFlag))
                    {
                    iChunkForm.OpenL(element.iType);
                    chunk = &iChunkForm;
                    }
                break;
                
            default:
                Panic(EMTPTypeBoundsError);
                break;
                }
                    
            // Some chunks (i.e. FORM) are optional.
            if (chunk)
                {
                ChunkAppendL(*chunk);   
                }
            }
        }
        
    // Set the construction values.
    SetUint16L(EPropertyCode, aPropertyCode);
    SetUint16L(EDatatype, aInfo.iDataType);
    SetUint8L(EGetSet, aInfo.iGetSet);
    SetUint8L(EFormFlag, aInfo.iFormFlag);
    
    if (aForm)
        {
        if (!HasFormField(aInfo.iFormFlag))
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
    
TBool CMTPTypeObjectPropDesc::HasFormField(TUint8 aFormFlag) const
    {
    return ((aFormFlag != EDateTimeForm) && (aFormFlag != ENone) && (aFormFlag != EObjectIDForm) );
    }
    
TInt CMTPTypeObjectPropDesc::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }
    
CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm::RMTPTypeObjectPropDescForm() : 
    RMTPType()
    {

    }

void CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm::SetMeta(TUint8 aFormFlag, TUint aDataType)
    {
    iFormFlag = aFormFlag;
    iDataType = aDataType;
    }

MMTPType* CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm::CreateL(TUint aDataType)
    {
    if (aDataType != EMTPTypeReference)
        {
        User::Leave(KMTPDataTypeInvalid);
        }
        
    MMTPType* type(NULL);
    switch (iFormFlag)
        {
    case ERangeForm:
        type = CMTPTypeObjectPropDescRangeForm::NewL(iDataType);
        break;
        
    case EEnumerationForm:
        type = CMTPTypeObjectPropDescEnumerationForm::NewL(iDataType);
        break;
        
    case EFixedLengthArrayForm:
        type = RMTPType::CreateL(EMTPTypeUINT32);
        break;
        
    case ERegularExpressionForm:
        type = RMTPType::CreateL(EMTPTypeString);
        break;
        
    case EByteArrayForm:
    case ELongStringForm:
        type = RMTPType::CreateL(EMTPTypeUINT32);
        break;
    case EServiceObjPropExtnForm:
        {
        type = CMTPTypeServiceObjPropExtnForm::NewL(iDataType);
        }
        break;
    case EServiceMethodParamExtnForm:
        {
        type = CMTPTypeServiceMethodParamExtnForm::NewL(iDataType);
        }
        break;
        
    case EDateTimeForm:
    default:
        // Invalid Form Flag.
        User::Leave(KErrNotSupported);
        break;
        }
        
    return type;
    }

void CMTPTypeObjectPropDesc::RMTPTypeObjectPropDescForm::Destroy(MMTPType* aType)
    {
    switch (iFormFlag)
        {
    case ERangeForm:
        delete static_cast<CMTPTypeObjectPropDescRangeForm*>(aType);
        break;
        
    case EEnumerationForm:
        delete static_cast<CMTPTypeObjectPropDescEnumerationForm*>(aType);
        break;
        
    case EServiceObjPropExtnForm:
        delete static_cast<CMTPTypeServiceObjPropExtnForm*>(aType);
        break;
        
    case EServiceMethodParamExtnForm:
        delete static_cast<CMTPTypeServiceMethodParamExtnForm*>(aType);
        break;
        
    default:
        // All other FORM types are managed by the base class.
        RMTPType::Destroy(aType);
        break;
        }
    }

/**
MTP ObjectPropDesc Enumeration FORM dataset factory method. This method is used to 
create an empty MTP ObjectPropDesc FORM dataset of the specified Datatype. 
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/     
EXPORT_C CMTPTypeObjectPropDescEnumerationForm* CMTPTypeObjectPropDescEnumerationForm::NewL(TUint aDataType)
    {
    CMTPTypeObjectPropDescEnumerationForm* self = CMTPTypeObjectPropDescEnumerationForm::NewLC(aDataType);
    CleanupStack::Pop(self);
    return self;   
    }

/**
MTP ObjectPropDesc Enumeration FORM dataset factory method. This method is used to 
create an empty MTP ObjectPropDesc FORM dataset of the specified Datatype. A 
pointer to the data type is placed on the cleanup stack.
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeObjectPropDescEnumerationForm* CMTPTypeObjectPropDescEnumerationForm::NewLC(TUint aDataType)
    {
    CMTPTypeObjectPropDescEnumerationForm* self = new(ELeave) CMTPTypeObjectPropDescEnumerationForm(aDataType);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeObjectPropDescEnumerationForm::~CMTPTypeObjectPropDescEnumerationForm()
    {
    TInt count(iChunksValue.Count());
    while (count--)
        {
        iChunksValue[count].Close();
        }
    iChunksValue.Close();
    }

/**
Appends the specified value to the set of supported enumeration values.
@param aValue The enumeration value to append.
@leave One of the system wide error codes, if unsuccessful.
@panic MTPDataTypes 3, if the source value type does not match the property 
data type.
*/
EXPORT_C void CMTPTypeObjectPropDescEnumerationForm::AppendSupportedValueL(const MMTPType& aValue)
    {
    __ASSERT_ALWAYS(aValue.Type() == iInfoValue.iType, Panic(EMTPTypeIdMismatch));
    
    // Increment NumberOfValues.
    const TUint index(iChunkNumberOfValues.Value());
    iChunkNumberOfValues.Set(index + 1);
    
    // Create and populate the SupportedValue chunk.
    AppendValueChunkL();
    SetL((KValueChunks + index), aValue);
    }

/**
Provides the NumberOfValues element value.
*/
EXPORT_C TUint16 CMTPTypeObjectPropDescEnumerationForm::NumberOfValues() const
    {
    return iChunkNumberOfValues.Value();
    }

/**
Provides the SupportedValue element at the specified index.
@param aIndex The index of the required value.
@param aValue On successful completion, the requested value.
@leave One of the system wide error codes, if unsuccessful.
@panic MTPDataTypes 3, if the target value type does not match the property 
data type.
*/
EXPORT_C void CMTPTypeObjectPropDescEnumerationForm::SupportedValueL(TUint aIndex, MMTPType& aValue) const
    {
    __ASSERT_ALWAYS(aValue.Type() == iInfoValue.iType, Panic(EMTPTypeIdMismatch));
    GetL((KValueChunks + aIndex), aValue);
    }
    
EXPORT_C TInt CMTPTypeObjectPropDescEnumerationForm::FirstWriteChunk(TPtr8& aChunk)
    {
    /* 
    Reset the type in preparation for the data stream, by deleting all
    except the first chunk.
    */
    for (TUint i(ChunkCount() - 1); (i >= (KNumberOfValuesChunk + 1)); i--)
        {
        ChunkRemove(i);
        }
    iChunksValue.Close();
    
    // Setup the write chunk pointer.
    TInt err(CMTPTypeCompoundBase::FirstWriteChunk(aChunk));
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
    
EXPORT_C TInt CMTPTypeObjectPropDescEnumerationForm::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(CMTPTypeCompoundBase::NextWriteChunk(aChunk));
    if ((err == KMTPChunkSequenceCompletion) &&
        (iChunksValue.Count() < iChunkNumberOfValues.Value()))
        {
        err = KErrNone; 
        }
    return err;
    }
 
EXPORT_C TUint CMTPTypeObjectPropDescEnumerationForm::Type() const
    {
    return EMTPTypeObjectPropDescEnumerationFormDataset;
    }
    
EXPORT_C TBool CMTPTypeObjectPropDescEnumerationForm::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeObjectPropDescEnumerationForm::CommitChunkL(TPtr8& aChunk)
    {
    switch (iWriteSequenceState)
        {
    case EInProgress:
        if (iChunksValue.Count() < iChunkNumberOfValues.Value())
            {
            AppendValueChunkL(); 
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
        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }     
    return NULL;
    }
 
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeObjectPropDescEnumerationForm::ElementInfo(TInt aElementId) const
    {
    if (aElementId == KNumberOfValuesChunk)
        {
        return iInfoNumberOfValues;
        }
    else
        {
        __ASSERT_ALWAYS(((aElementId - KValueChunks) < iChunkNumberOfValues.Value()), Panic(EMTPTypeBoundsError));
        iInfoValue.iChunkId = aElementId;
        return iInfoValue; 
        }
    }

TInt CMTPTypeObjectPropDescEnumerationForm::ValidateChunkCount() const
    {
    TInt ret(KErrNone);
    if (NumberOfValues() != (ChunkCount() - 1))
        {
        ret = KMTPDataTypeInvalid;
        }
    return ret;
    }

CMTPTypeObjectPropDescEnumerationForm::CMTPTypeObjectPropDescEnumerationForm(TUint aDataType) :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfValues         = KDefaultInfo; 
    iInfoNumberOfValues.iType   = EMTPTypeUINT16;
    iInfoValue                  = KDefaultInfo; 
    iInfoValue.iType            = aDataType;
    }

void CMTPTypeObjectPropDescEnumerationForm::ConstructL()
    {
    ChunkAppendL(iChunkNumberOfValues);
    }
    
void CMTPTypeObjectPropDescEnumerationForm::AppendValueChunkL()
    {
    iChunksValue.AppendL(RMTPType());
    
    RMTPType& chunk(iChunksValue[iChunksValue.Count() - 1]);
    chunk.OpenL(iInfoValue.iType);
    ChunkAppendL(chunk);
    }

/**
MTP ObjectPropDesc Range FORM dataset factory method. This method is used to 
create an empty MTP ObjectPropDesc FORM dataset of the specified Datatype. 
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeObjectPropDescRangeForm* CMTPTypeObjectPropDescRangeForm::NewL(TUint aDataType)
    {
    CMTPTypeObjectPropDescRangeForm* self = CMTPTypeObjectPropDescRangeForm::NewLC(aDataType);
    CleanupStack::Pop(self); 
    return self;
    }

/**
MTP ObjectPropDesc Range FORM dataset factory method. This method is used to 
create an empty MTP ObjectPropDesc FORM dataset of the specified Datatype. A 
pointer to the data type is placed on the cleanup stack.
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP ObjectPropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeObjectPropDescRangeForm* CMTPTypeObjectPropDescRangeForm::NewLC(TUint aDataType)
    {
    CMTPTypeObjectPropDescRangeForm* self = new(ELeave) CMTPTypeObjectPropDescRangeForm(aDataType);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeObjectPropDescRangeForm::~CMTPTypeObjectPropDescRangeForm()
    {
    TInt count(iChunks.Count());
    while (count--)
    	{
    	iChunks[count].Close();    	
    	}
    iChunks.Close();
    }

EXPORT_C TUint CMTPTypeObjectPropDescRangeForm::Type() const
    {
    return EMTPTypeObjectPropDescRangeFormDataset;
    } 

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeObjectPropDescRangeForm::ElementInfo(TInt aElementId) const
    {
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    iInfoBuf.iChunkId = aElementId;
    return iInfoBuf;    
    }

CMTPTypeObjectPropDescRangeForm::CMTPTypeObjectPropDescRangeForm(TUint aDataType) : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), ENumElements), 
    iDataType(aDataType)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeReference, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoBuf = KDefaultInfo;
    }

void CMTPTypeObjectPropDescRangeForm::ConstructL()
    {
    for (TUint i(0); (i < ENumElements); i++)   
        {
        iChunks.AppendL(RMTPType());
        iChunks[i].OpenL(iDataType);
        ChunkAppendL(iChunks[i]);
        }
    }
