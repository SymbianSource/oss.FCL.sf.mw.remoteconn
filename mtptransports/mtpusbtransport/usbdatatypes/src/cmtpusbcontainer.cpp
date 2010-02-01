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

#include "cmtpusbcontainer.h"
#include "mtpusbdatatypeconstants.h"

// Dataset constants
const TUint CMTPUsbContainer::KFlatChunkSize(12);

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CMTPUsbContainer::iElementMetaData[CMTPUsbContainer::ENumElements] = 
    {
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EContainerLength
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT16,    4,                  KMTPTypeUINT16Size}},   // EContainerType
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT16,    6,                  KMTPTypeUINT16Size}},   // ECode
        {EIdFlatChunk,      EMTPTypeFlat,       {EMTPTypeUINT32,    8,                  KMTPTypeUINT32Size}},   // ETransactionID
        {EIdPayloadChunk,   EMTPTypeUndefined,  {EMTPTypeUndefined, KMTPNotApplicable,  KMTPNotApplicable}}     // EPayload
    };

/**
MTP USB device class generic bulk container dataset factory method. This method
is used to create an empty MTP USB device class generic bulk container dataset 
type. 
@return A pointer to an empty an empty MTP USB device class generic bulk 
container dataset type. Ownership IS transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPUsbContainer* CMTPUsbContainer::NewL()
    {
    CMTPUsbContainer* self = new (ELeave) CMTPUsbContainer(); 
    CleanupStack::PushL(self); 
    self->ConstructL();   
    CleanupStack::Pop(self);
    return self; 
    }

/**
Destructor.
*/
EXPORT_C CMTPUsbContainer::~CMTPUsbContainer()
    {
    iChunkHeader.Close();
    }

/**
Provides the bulk container payload.
@return The bulk container payload.
*/
EXPORT_C MMTPType* CMTPUsbContainer::Payload() const
    {    
    return iPayload;
    }

/**
Sets the bulk container payload.
@param aPayload The new bulk container payload.
*/
EXPORT_C void CMTPUsbContainer::SetPayloadL(MMTPType* aPayload)
    {    
    if (iPayload)
        {
        // Remove the existing payload from the super class.
        ChunkRemove(iElementMetaData[EPayload].iChunkId);
        }
        
    if (aPayload)
        {
        // Pass the payload to the super class for management.
        ChunkAppendL(*aPayload);  
        }
    iPayload = aPayload; 
    }


EXPORT_C TUint CMTPUsbContainer::Type() const
    {
    return EMTPUsbTypeContainer;
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPUsbContainer::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
       
/**
Constructor.
*/
CMTPUsbContainer::CMTPUsbContainer() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iChunkHeader(KFlatChunkSize, *this),
    iElementInfo(iElementMetaData, ENumElements)
    {
    
    }

/**
Second phase constructor.
*/   
void CMTPUsbContainer::ConstructL()
    {
    iChunkHeader.OpenL();
    ChunkAppendL(iChunkHeader);
    }

