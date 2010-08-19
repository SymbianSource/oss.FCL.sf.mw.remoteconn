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

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtptypescomplex.h>

#include "cmtptypedeviceinfo.h"

// Dataset constants
const TUint KMTPChunk0Size = 8;

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CMTPTypeDeviceInfo::iElementMetaData[CMTPTypeDeviceInfo::ENumElements] = 
    {
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EStandardVersion
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT32,    2,                  KMTPTypeUINT32Size}},   // EMTPVendorExtensionID
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    6,                  KMTPTypeUINT16Size}},   // EMTPVersion
        {EIdMTPExtensionsChunk,             EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPExtensions
        {EIdFunctionalModeChunk,            EMTPTypeUINT16, {EMTPTypeUINT16,    KMTPNotApplicable,  KMTPNotApplicable}},    // EFunctionalMode
        {EIdOperationsSupportedChunk,       EMTPTypeArray,  {EMTPTypeAUINT16,   KMTPNotApplicable,  KMTPNotApplicable}},    // EOperationsSupported
        {EIdEventsSupportedChunk,           EMTPTypeArray,  {EMTPTypeAUINT16,   KMTPNotApplicable,  KMTPNotApplicable}},    // EventsSupported
        {EIdDevicePropertiesSupportedChunk, EMTPTypeArray,  {EMTPTypeAUINT16,   KMTPNotApplicable,  KMTPNotApplicable}},    // EDevicePropertiesSupported
        {EIdCaptureFormatsChunk,            EMTPTypeArray,  {EMTPTypeAUINT16,   KMTPNotApplicable,  KMTPNotApplicable}},    // ECaptureFormats
        {EIdPlaybackFormatsChunk,           EMTPTypeArray,  {EMTPTypeAUINT16,   KMTPNotApplicable,  KMTPNotApplicable}},    // EPlaybackFormats
        {EIdManufacturerChunk,              EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EManufacturer
        {EIdModelChunk,                     EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EModel  
        {EIdDeviceVersionChunk,             EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EDeviceVersion
        {EIdSerialNumberChunk,              EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // ESerialNumber
    };

/**
MTP DeviceInfo dataset factory method. This method is used to create an empty 
MTP DeviceInfo dataset type. A pointer to the MTP array data type is placed on 
the cleanup stack.
@return A pointer to an empty MTP DeviceInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/    
CMTPTypeDeviceInfo* CMTPTypeDeviceInfo::NewL()
    {
	CMTPTypeDeviceInfo* self = CMTPTypeDeviceInfo::NewLC(); 
	CleanupStack::Pop(self);
	return self; 
    }

/**
MTP DeviceInfo dataset factory method. This method is used to create an empty 
MTP DeviceInfo dataset type.
@return A pointer to an empty MTP DeviceInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/    
CMTPTypeDeviceInfo* CMTPTypeDeviceInfo::NewLC()
    {
	CMTPTypeDeviceInfo* self = new (ELeave) CMTPTypeDeviceInfo(); 
	CleanupStack::PushL(self); 
    self->ConstructL();
	return self;    
    }

/**
Destructor.
*/    
CMTPTypeDeviceInfo::~CMTPTypeDeviceInfo()
    {    
    iChunkFlat.Close();
    iArrayChunks.ResetAndDestroy();
    iStringChunks.ResetAndDestroy();
    }
 
TUint CMTPTypeDeviceInfo::Type() const
    {
    return EMTPTypeDeviceInfoDataset;
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeDeviceInfo::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeDeviceInfo::CMTPTypeDeviceInfo() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KMTPChunk0Size, *this)
    {
    
    }
    
void CMTPTypeDeviceInfo::ConstructL()
    {
    for (TUint i(0); (i < ENumElements); i++)
        {
        const TElementInfo& info(iElementInfo[i]);
        if (ChunkCount() <= info.iChunkId)
            {
            MMTPType* chunk(NULL);
            
            switch (info.iType)
                {
            case EMTPTypeArray:
                chunk = NewArrayChunkL(info);
                break;
                
            case EMTPTypeFlat:
                chunk = NewFlatChunkL(info);
                break;
                
            case EMTPTypeString:
                chunk = NewStringChunkL(info);
                break;
                
            case EMTPTypeUINT16:
                chunk = NewUInt16Chunk(info);
                break;
                
            default:
                break;
                }
                
            __ASSERT_DEBUG(chunk, User::Invariant());
            CleanupStack::PushL(chunk);
            ChunkAppendL(*chunk);
            CleanupStack::Pop(chunk);
            }
        }
    }

MMTPType* CMTPTypeDeviceInfo::NewArrayChunkL(const TElementInfo& aElementInfo)
    {
    CMTPTypeArray* chunk(NULL);
    if (aElementInfo.iFlatChunkInfo.iType)
        {
        chunk = CMTPTypeArray::NewLC(EMTPTypeAUINT16);
        }
    __ASSERT_DEBUG(chunk, User::Invariant());
    iArrayChunks.AppendL(chunk);
    CleanupStack::Pop(chunk);
    return chunk;
    }

MMTPType* CMTPTypeDeviceInfo::NewFlatChunkL(const TElementInfo& aElementInfo)
    {
    MMTPType* chunk(NULL);
    if (aElementInfo.iChunkId == EIdFlatChunk)
        {
        iChunkFlat.OpenL();
        chunk = &iChunkFlat;
        }
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

MMTPType* CMTPTypeDeviceInfo::NewStringChunkL(const TElementInfo& /*aElementInfo*/)
    {
    CMTPTypeString* chunk(CMTPTypeString::NewLC());
    iStringChunks.AppendL(chunk);
    CleanupStack::Pop(chunk);
    return chunk;
    }

MMTPType* CMTPTypeDeviceInfo::NewUInt16Chunk(const TElementInfo& aElementInfo)
    {
    MMTPType* chunk(NULL);
    if (aElementInfo.iChunkId == EIdFunctionalModeChunk)
        {
        chunk = &iChunkFunctionalMode;
        }
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk; 
    }
