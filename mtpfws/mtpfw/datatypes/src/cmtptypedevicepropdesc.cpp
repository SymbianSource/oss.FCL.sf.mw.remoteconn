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

#include <mtp/cmtptypedevicepropdesc.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint CMTPTypeDevicePropDesc::KFlat1ChunkSize(5);
const TUint CMTPTypeDevicePropDesc::KNumChunksWithoutForm(EIdFormFlagChunk + 1);
const TUint CMTPTypeDevicePropDesc::KNumChunksWithForm(EIdFormChunk + 1);

const TUint CMTPTypeDevicePropDescEnumerationForm::KNumberOfValuesChunk(0);
const TUint CMTPTypeDevicePropDescEnumerationForm::KValueChunks(1);

// Dataset element metadata    
const CMTPTypeCompoundBase::TElementInfo CMTPTypeDevicePropDesc::iElementMetaData[CMTPTypeDevicePropDesc::ENumElements] = 
    {
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EDevicePropertyCode
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT16,    2,                  KMTPTypeUINT16Size}},   // EDatatype
        {EIdFlat1Chunk,         EMTPTypeFlat,       {EMTPTypeUINT8,     4,                  KMTPTypeUINT8Size}},    // EGetSet
        {EIdDefaultValueChunk,  EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EFactoryDefaultValue
        {EIdCurrentValueChunk,  EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // ECurrentValue
        {EIdFormFlagChunk,      EMTPTypeUINT8,      {EMTPTypeUINT8,     KMTPNotApplicable,  KMTPNotApplicable}},    // EFormFlag
        {EIdFormChunk,          EMTPTypeReference,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}},    // EForm
    };

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
empty MTP DevicePropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode. 
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewL(TUint16 aPropertyCode)
    {
    CMTPTypeDevicePropDesc* self = NewLC(aPropertyCode);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
empty MTP DevicePropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode. A pointer to the data type is placed on
the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC(TUint16 aPropertyCode)
    {    
    CMTPTypeDevicePropDesc* self = new (ELeave) CMTPTypeDevicePropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, NULL);
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
MTP DevicePropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode, and the specified FORM. 
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewL(TUint16 aPropertyCode, const MMTPType& aForm)
    {
    CMTPTypeDevicePropDesc* self = NewLC(aPropertyCode, aForm);
    CleanupStack::Pop(self); 
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
MTP DevicePropDesc dataset type with the default Datatype and Form Flag 
defined for the specified PropertyCode, and the specified FORM. A pointer 
to the data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC(TUint16 aPropertyCode, const MMTPType& aForm)
    {    
    CMTPTypeDevicePropDesc* self = new (ELeave) CMTPTypeDevicePropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, &aForm);
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
MTP DevicePropDesc dataset type with the default Datatype defined for the 
specified PropertyCode, and the specified Form Flag and FORM.  
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aGetSet The Get/Set flag of the DevicePropDesc.
@param aFormFlag The required Form Flag of the DevicePropDesc.
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewL(TUint16 aPropertyCode, TUint8 aGetSet, TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeDevicePropDesc* self = NewLC(aPropertyCode, aGetSet, aFormFlag, aForm);
    CleanupStack::Pop(self);
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an 
MTP DevicePropDesc dataset type with the default Datatype defined for the 
specified PropertyCode, and the specified Form Flag and FORM. A pointer 
to the data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aGetSet The Get/Set flag of the DevicePropDesc.
@param aFormFlag The required Form Flag of the DevicePropDesc.
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC(TUint16 aPropertyCode, TUint8 aGetSet, TUint8 aFormFlag, const MMTPType* aForm)
    {
    CMTPTypeDevicePropDesc* self = new (ELeave) CMTPTypeDevicePropDesc(); 
    CleanupStack::PushL(self); 
    TPropertyInfo info;
    PropertyInfoL(aPropertyCode, info);
    info.iGetSet    = aGetSet;
    info.iFormFlag  = aFormFlag;
    self->ConstructL(aPropertyCode, info, aForm);
    return self;    
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an  
MTP DevicePropDesc dataset type with the specified values.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aInfo The MTP DevicePropDesc dataset characteristics (meta data).
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/    
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {
    CMTPTypeDevicePropDesc* self = CMTPTypeDevicePropDesc::NewLC(aPropertyCode, aInfo, aForm); 
    CleanupStack::Pop(self);
    return self; 
    }

/**
MTP DevicePropDesc dataset factory method. This method is used to create an  
MTP DevicePropDesc dataset type with the specified values. A pointer to the 
data type is placed on the cleanup stack.
@param aPropertyCode The ObjectPropertyCode of the property being described by 
the DevicePropDesc dataset.
@param aInfo The MTP DevicePropDesc dataset characteristics (meta data).
@param aForm The optional DevicePropDesc FORM dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave KErrNotSupported, if a FORM dataset is not defined for the property.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {
    CMTPTypeDevicePropDesc* self = new (ELeave) CMTPTypeDevicePropDesc(); 
    CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, aInfo, aForm);
    return self;    
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeDevicePropDesc::~CMTPTypeDevicePropDesc()
    {
    iChunkFlat1.Close();
    iChunkDefaultValue.Close();
    iChunkCurrentValue.Close();
    iChunkForm.Close();
    }

EXPORT_C TInt CMTPTypeDevicePropDesc::FirstWriteChunk(TPtr8& aChunk)
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
        iWriteSequenceState = EWriteFlat1Chunk; 
        
        /* 
        Set the write sequence completion state. Initially assume that the 
        DevicePropDesc has a FORM field. If no FORM field is subsequently
        detected in the write data stream then the completion state is 
        adjusted accordingly.
        */
        iWriteSequenceCompletionState = EWriteFormChunk;
        break;
        
    default:
        break;
        }
        
    return  err;
    }
    
EXPORT_C TInt CMTPTypeDevicePropDesc::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(KMTPChunkSequenceCompletion);
    
    if (iWriteSequenceState != EWriteIdle)
        {
        err = UpdateWriteSequenceErr(CMTPTypeCompoundBase::NextWriteChunk(aChunk));
        if ((iWriteSequenceErr == KMTPChunkSequenceCompletion) && (iWriteSequenceState != EWriteIdle))
            {
            err = KErrNone;
            }   
        }    
    return err;
    }
 
EXPORT_C TUint CMTPTypeDevicePropDesc::Type() const
    {
    return EMTPTypeDevicePropDescDataset;
    }
    
EXPORT_C TBool CMTPTypeDevicePropDesc::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeDevicePropDesc::CommitChunkL(TPtr8& aChunk)
    {
    if (iWriteSequenceErr == KMTPChunkSequenceCompletion)
        {
        switch (iWriteSequenceState)
            {
        case EWriteFlat1Chunk:     
            /* 
            Datatype is available, allocate sufficient chunk storage for 
            and append the EFactoryDefaultValue and ECurrentValue chunks.
            */
            iChunkDefaultValue.Close();
            iChunkDefaultValue.OpenL(Uint16L(EDatatype));
            ChunkAppendL(iChunkDefaultValue);
            break;
            
        case EWriteDefaultValueChunk:
            iChunkCurrentValue.Close();
            iChunkCurrentValue.OpenL(Uint16L(EDatatype));
            ChunkAppendL(iChunkCurrentValue);                        
            break;
            
        case EWriteCurrentValueChunk:
        	ChunkAppendL(iChunkFormFlag);
        	break;
            
        case EWriteFormFlagChunk: 
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
                iWriteSequenceCompletionState = EWriteFormFlagChunk;
                }   
            }
            break;
            
        case EWriteFormChunk:
        case EWriteIdle:
        default:
            break;
            }
    if (iWriteSequenceState < iWriteSequenceCompletionState)
        {
        iWriteSequenceState++;
        }
    else
        {
        iWriteSequenceState = EWriteIdle;
        }
    }
        
    if (CMTPTypeCompoundBase::CommitRequired())
        {
        CMTPTypeCompoundBase::CommitChunkL(aChunk);
        }
    return NULL;
    }
    
void CMTPTypeDevicePropDesc::PropertyInfoL(TUint16 aPropertyCode, TPropertyInfo& aInfo)
    {    
    switch (aPropertyCode)
        {        
    case EMTPDevicePropCodeUndefined:
        aInfo.iDataType     = EMTPTypeUndefined;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeBatteryLevel:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFunctionalMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeImageSize:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeCompressionSetting:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeWhiteBalance:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeRGBGain:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFNumber:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFocalLength:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFocusDistance:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFocusMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeExposureMeteringMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFlashMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeExposureTime:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeExposureProgramMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeExposureIndex:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeExposureBiasCompensation:
        aInfo.iDataType     = EMTPTypeINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeDateTime:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeCaptureDelay:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeStillCaptureMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeContrast:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeSharpness:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeDigitalZoom:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeEffectMode:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeBurstNumber:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeBurstInterval:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeTimelapseNumber:
        aInfo.iDataType     = EMTPTypeUINT16;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeTimelapseInterval:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFocusMeteringMode:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeUploadURL:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeArtist:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeCopyrightInfo:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeSynchronizationPartner:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodeDeviceFriendlyName:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodeVolume:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ERangeForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodeSupportedFormatsOrdered:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeDeviceIcon:
        aInfo.iDataType     = EMTPTypeAUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodePlaybackRate:
        aInfo.iDataType     = EMTPTypeINT32;
        aInfo.iFormFlag     = EEnumerationForm;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodePlaybackObject:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodePlaybackContainerIndex:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodePlaybackPosition:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodeSessionInitiatorVersionInfo:
        aInfo.iDataType     = EMTPTypeString;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodePerceivedDeviceType:
        aInfo.iDataType     = EMTPTypeUINT32;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeFunctionalID:
        aInfo.iDataType     = EMTPTypeUINT128;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadWrite;
        break;
        
    case EMTPDevicePropCodeModelID:
        aInfo.iDataType     = EMTPTypeUINT128;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    case EMTPDevicePropCodeUseDeviceStage:
        aInfo.iDataType     = EMTPTypeUINT8;
        aInfo.iFormFlag     = ENone;
        aInfo.iGetSet       = EReadOnly;
        break;
        
    default:
        User::Leave(KErrNotSupported);
        break;
        }
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeDevicePropDesc::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }
    
TBool CMTPTypeDevicePropDesc::ReadableElementL(TInt aElementId) const
    {
    TBool ret(ETrue);
    if (aElementId == EForm)
        {
        ret = HasFormField(Uint8L(EFormFlag));
        }
    return ret;
    }
    
TBool CMTPTypeDevicePropDesc::WriteableElementL(TInt aElementId) const
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
        case EGetSet:
        case EFactoryDefaultValue:
        case ECurrentValue:
            ret = ETrue;
            break;

        default:
            break;
            }   
        }
        
    return ret;
    }

/*
MTP DevicePropDesc dataset "pre-production" factory method for datasets
which will be populated from an MTP write data stream. This method is used to 
create a partially constructed MTP DevicePropDesc element dataset type prior to
the construction characteristics being known. Construction of the element 
dataset is completed on-the-fly as it is written to from an MTP write data 
stream. A pointer to the data type is placed on the cleanup stack.
@return A pointer to the ObjectPropList element dataset type. 
Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/
CMTPTypeDevicePropDesc* CMTPTypeDevicePropDesc::NewLC()
    {
    CMTPTypeDevicePropDesc* self = new(ELeave) CMTPTypeDevicePropDesc();
    CleanupStack::PushL(self);
    TPropertyInfo info;
    PropertyInfoL(EMTPObjectPropCodeUndefined, info);
    self->ConstructL(EMTPObjectPropCodeUndefined, info, NULL);
    
    return self;
    }

CMTPTypeDevicePropDesc::CMTPTypeDevicePropDesc() :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KNumChunksWithoutForm),
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this)
    {
    
    }

void CMTPTypeDevicePropDesc::ConstructL(TUint16 aPropertyCode, const MMTPType* aForm)
    {
    TPropertyInfo info;
    PropertyInfoL(aPropertyCode, info);
    ConstructL(aPropertyCode, info, aForm);
    }
    
void CMTPTypeDevicePropDesc::ConstructL(TUint16 aPropertyCode, const TPropertyInfo& aInfo, const MMTPType* aForm)
    {    
    // Construct the DevicePropDesc chunk sequence.
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
                
            case EIdCurrentValueChunk:
                if (aInfo.iDataType != EMTPTypeUndefined)
                    {
                    iChunkCurrentValue.OpenL(aInfo.iDataType);
                    }
                chunk = &iChunkCurrentValue;
                break;
                
            case EIdFormFlagChunk:
                chunk = &iChunkFormFlag;
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
    SetUint16L(EDevicePropertyCode, aPropertyCode);
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
    
TBool CMTPTypeDevicePropDesc::HasFormField(TUint8 aFormFlag) const
    {
    return (aFormFlag != ENone);
    }
    
TInt CMTPTypeDevicePropDesc::UpdateWriteSequenceErr(TInt aErr)
    {
    iWriteSequenceErr = aErr;
    return iWriteSequenceErr;        
    }
    
CMTPTypeDevicePropDesc::RMTPTypeDevicePropDescForm::RMTPTypeDevicePropDescForm() : 
    RMTPType()
    {

    }

void CMTPTypeDevicePropDesc::RMTPTypeDevicePropDescForm::SetMeta(TUint8 aFormFlag, TUint aDataType)
    {
    iFormFlag = aFormFlag;
    iDataType = aDataType;
    }

MMTPType* CMTPTypeDevicePropDesc::RMTPTypeDevicePropDescForm::CreateL(TUint aDataType)
    {
    if (aDataType != EMTPTypeReference)
        {
        User::Leave(KMTPDataTypeInvalid);
        }
        
    MMTPType* type(NULL);
    switch (iFormFlag)
        {
    case ERangeForm:
        type = CMTPTypeDevicePropDescRangeForm::NewL(iDataType);
        break;
        
    case EEnumerationForm:
        type = CMTPTypeDevicePropDescEnumerationForm::NewL(iDataType);
        break;
        
    default:
        // Invalid Form Flag.
        User::Leave(KMTPDataTypeInvalid);
        break;
        }
        
    return type;
    }

void CMTPTypeDevicePropDesc::RMTPTypeDevicePropDescForm::Destroy(MMTPType* aType)
    {
    switch (iFormFlag)
        {
    case ERangeForm:
        delete static_cast<CMTPTypeDevicePropDescRangeForm*>(aType);
        break;
        
    case EEnumerationForm:
        delete static_cast<CMTPTypeDevicePropDescEnumerationForm*>(aType);
        break;
        
    default:
        // All other FORM types are managed by the base class.
        RMTPType::Destroy(aType);
        break;
        }
    }

/**
MTP DevicePropDesc Enumeration FORM dataset factory method. This method is used to 
create an empty MTP DevicePropDesc FORM dataset of the specified Datatype. 
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/     
EXPORT_C CMTPTypeDevicePropDescEnumerationForm* CMTPTypeDevicePropDescEnumerationForm::NewL(TUint aDataType)
    {
    CMTPTypeDevicePropDescEnumerationForm* self = CMTPTypeDevicePropDescEnumerationForm::NewLC(aDataType);
    CleanupStack::Pop(self);
    return self;   
    }

/**
MTP DevicePropDesc Enumeration FORM dataset factory method. This method is used to 
create an empty MTP DevicePropDesc FORM dataset of the specified Datatype. A 
pointer to the data type is placed on the cleanup stack.
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeDevicePropDescEnumerationForm* CMTPTypeDevicePropDescEnumerationForm::NewLC(TUint aDataType)
    {
    CMTPTypeDevicePropDescEnumerationForm* self = new(ELeave) CMTPTypeDevicePropDescEnumerationForm(aDataType);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeDevicePropDescEnumerationForm::~CMTPTypeDevicePropDescEnumerationForm()
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
EXPORT_C void CMTPTypeDevicePropDescEnumerationForm::AppendSupportedValueL(const MMTPType& aValue)
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
EXPORT_C TUint16 CMTPTypeDevicePropDescEnumerationForm::NumberOfValues() const
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
EXPORT_C void CMTPTypeDevicePropDescEnumerationForm::SupportedValueL(TUint aIndex, MMTPType& aValue) const
    {
    __ASSERT_ALWAYS(aValue.Type() == iInfoValue.iType, Panic(EMTPTypeIdMismatch));
    GetL((KValueChunks + aIndex), aValue);
    }
    
EXPORT_C TInt CMTPTypeDevicePropDescEnumerationForm::FirstWriteChunk(TPtr8& aChunk)
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
    
EXPORT_C TInt CMTPTypeDevicePropDescEnumerationForm::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(CMTPTypeCompoundBase::NextWriteChunk(aChunk));
    if ((err == KMTPChunkSequenceCompletion) &&
        (iChunksValue.Count() < iChunkNumberOfValues.Value()))
        {
        err = KErrNone; 
        }
    return err;
    }
 
EXPORT_C TUint CMTPTypeDevicePropDescEnumerationForm::Type() const
    {
    return EMTPTypeDevicePropDescEnumerationFormDataset;
    }
    
EXPORT_C TBool CMTPTypeDevicePropDescEnumerationForm::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeDevicePropDescEnumerationForm::CommitChunkL(TPtr8& aChunk)
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
 
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeDevicePropDescEnumerationForm::ElementInfo(TInt aElementId) const
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

TInt CMTPTypeDevicePropDescEnumerationForm::ValidateChunkCount() const
    {
    TInt ret(KErrNone);
    if (NumberOfValues() != (ChunkCount() - 1))
        {
        ret = KMTPDataTypeInvalid;
        }
    return ret;
    }

CMTPTypeDevicePropDescEnumerationForm::CMTPTypeDevicePropDescEnumerationForm(TUint aDataType) :
    CMTPTypeCompoundBase(KJustInTimeConstruction, KVariableChunkCount)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeUndefined, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoNumberOfValues         = KDefaultInfo; 
    iInfoNumberOfValues.iType   = EMTPTypeUINT16;
    iInfoValue                  = KDefaultInfo; 
    iInfoValue.iType            = aDataType;
    }

void CMTPTypeDevicePropDescEnumerationForm::ConstructL()
    {
    ChunkAppendL(iChunkNumberOfValues);
    }
    
void CMTPTypeDevicePropDescEnumerationForm::AppendValueChunkL()
    {
    iChunksValue.AppendL(RMTPType());
    
    RMTPType& chunk(iChunksValue[iChunksValue.Count() - 1]);
    chunk.OpenL(iInfoValue.iType);
    ChunkAppendL(chunk);
    }

/**
MTP DevicePropDesc Range FORM dataset factory method. This method is used to 
create an empty MTP DevicePropDesc FORM dataset of the specified Datatype. 
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeDevicePropDescRangeForm* CMTPTypeDevicePropDescRangeForm::NewL(TUint aDataType)
    {
    CMTPTypeDevicePropDescRangeForm* self = CMTPTypeDevicePropDescRangeForm::NewLC(aDataType);
    CleanupStack::Pop(self); 
    return self;
    }

/**
MTP DevicePropDesc Range FORM dataset factory method. This method is used to 
create an empty MTP DevicePropDesc FORM dataset of the specified Datatype. A 
pointer to the data type is placed on the cleanup stack.
@param aDataType The data type identifier datacode of the dataset.
@return A pointer to the MTP DevicePropDesc dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeDevicePropDescRangeForm* CMTPTypeDevicePropDescRangeForm::NewLC(TUint aDataType)
    {
    CMTPTypeDevicePropDescRangeForm* self = new(ELeave) CMTPTypeDevicePropDescRangeForm(aDataType);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeDevicePropDescRangeForm::~CMTPTypeDevicePropDescRangeForm()
    {
    TInt count(iChunks.Count());
    while (count--)
        {
        iChunks[count].Close();
        }
    iChunks.Reset();
    }

EXPORT_C TUint CMTPTypeDevicePropDescRangeForm::Type() const
    {
    return EMTPTypeDevicePropDescRangeFormDataset;
    } 

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeDevicePropDescRangeForm::ElementInfo(TInt aElementId) const
    {
    __ASSERT_ALWAYS((aElementId < ENumElements), Panic(EMTPTypeBoundsError));
    iInfoBuf.iChunkId = aElementId;
    return iInfoBuf;    
    }

CMTPTypeDevicePropDescRangeForm::CMTPTypeDevicePropDescRangeForm(TUint aDataType) : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), ENumElements), 
    iDataType(aDataType)
    {
    const CMTPTypeCompoundBase::TElementInfo KDefaultInfo = {0, EMTPTypeReference, {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}};
    iInfoBuf = KDefaultInfo;
    }

void CMTPTypeDevicePropDescRangeForm::ConstructL()
    {
    for (TUint i(0); (i < ENumElements); i++)   
        {
        iChunks.AppendL(RMTPType());
        iChunks[i].OpenL(iDataType);
        ChunkAppendL(iChunks[i]);
        }
    }
