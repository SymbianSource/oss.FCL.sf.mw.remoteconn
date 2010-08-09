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


#include <mtp/tmtptypeflatbase.h>
#include <mtp/cmtptypestorageinfo.h>
#include <mtp/cmtptypestring.h>
   
// Dataset constants
const TUint CMTPTypeStorageInfo::KFlatChunkSize(26);

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CMTPTypeStorageInfo::iElementMetaData[CMTPTypeStorageInfo::ENumElements] = 
    {
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EStorageType
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT16,    2,                  KMTPTypeUINT16Size}},   // EFileSystemType
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT16,    4,                  KMTPTypeUINT16Size}},   // EAccessCapability
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT64,    6,                  KMTPTypeUINT64Size}},   // EMaxCapacity
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT64,    14,                 KMTPTypeUINT64Size}},   // EFreeSpaceInBytes
        {EIdFlatChunk,                  EMTPTypeFlat,   {EMTPTypeUINT32,    22,                 KMTPTypeUINT32Size}},   // EFreeSpaceInObjects
        {EIdStorageDescriptionChunk,    EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EStorageDescription
        {EIdVolumeIdentifierChunk,      EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EVolumeIdentifier
    };
    
/**
MTP StorageInfo dataset factory method. This method is used to create an empty 
MTP StorageInfo dataset type. A pointer to the MTP array data type is placed on 
the cleanup stack.
@return A pointer to an empty MTP StorageInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/    
EXPORT_C CMTPTypeStorageInfo* CMTPTypeStorageInfo::NewL()
    {
    CMTPTypeStorageInfo* self=CMTPTypeStorageInfo::NewLC(); 
    CleanupStack::Pop(self);
    return self; 
    }
    

/**
MTP StorageInfo dataset factory method. This method is used to create an empty 
MTP StorageInfo dataset type.
@return A pointer to an empty MTP StorageInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/   
EXPORT_C CMTPTypeStorageInfo* CMTPTypeStorageInfo::NewLC()
    {
    CMTPTypeStorageInfo* self=new (ELeave) CMTPTypeStorageInfo(); 
    CleanupStack::PushL(self); 
    self->ConstructL();
    return self;    
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeStorageInfo::~CMTPTypeStorageInfo()
    {
    iChunkFlat.Close();
    iStringChunks.ResetAndDestroy();
    } 

EXPORT_C TUint CMTPTypeStorageInfo::Type() const
    {
    return EMTPTypeStorageInfoDataset;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeStorageInfo::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }
    
CMTPTypeStorageInfo::CMTPTypeStorageInfo() :
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks),
    iElementInfo(iElementMetaData, ENumElements), 
    iChunkFlat(KFlatChunkSize, *this)
    {
    
    }
        
void CMTPTypeStorageInfo::ConstructL()
    {
    for (TUint i(0); (i < ENumElements); i++)
        {
        const TElementInfo& info(iElementInfo[i]);
        if (ChunkCount() <= info.iChunkId)
            {
            MMTPType* chunk(NULL);
            
            switch (info.iType)
                {
            case EMTPTypeFlat:
                chunk = NewFlatChunkL(info);
                break;
                
            case EMTPTypeString:
                chunk = NewStringChunkL(info);
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
    
MMTPType* CMTPTypeStorageInfo::NewFlatChunkL(const TElementInfo& aElementInfo)
    {
    MMTPType* chunk(NULL);
    switch (aElementInfo.iChunkId)
        {
    case 0:
        iChunkFlat.OpenL();
        chunk = &iChunkFlat;
        break;
        
    default:
        break;
        }        
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

MMTPType* CMTPTypeStorageInfo::NewStringChunkL(const TElementInfo& /*aElementInfo*/)
    {
    CMTPTypeString* chunk(CMTPTypeString::NewLC());
    iStringChunks.AppendL(chunk);
    CleanupStack::Pop(chunk);
    return chunk;
    }

