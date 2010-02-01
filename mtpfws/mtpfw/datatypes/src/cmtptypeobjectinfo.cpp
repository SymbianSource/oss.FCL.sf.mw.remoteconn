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


#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtptypescomplex.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint CMTPTypeObjectInfo::KFlatChunkSize(52);

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CMTPTypeObjectInfo::iElementMetaData[CMTPTypeObjectInfo::ENumElements] = 
    {
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EStorageID
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT16,    4,                  KMTPTypeUINT16Size}},   // EObjectFormat
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT16,    6,                  KMTPTypeUINT16Size}},   // EProtectionStatus
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    8,                  KMTPTypeUINT32Size}},   // EObjectCompressedSize
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT16,    12,                 KMTPTypeUINT16Size}},   // EThumbFormat
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    14,                 KMTPTypeUINT32Size}},   // EThumbCompressedSize
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    18,                 KMTPTypeUINT32Size}},   // EThumbPixWidth
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    22,                 KMTPTypeUINT32Size}},   // EThumbPixHeight
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    26,                 KMTPTypeUINT32Size}},   // EImagePixWidth
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    30,                 KMTPTypeUINT32Size}},   // EImagePixHeight
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    34,                 KMTPTypeUINT32Size}},   // EImageBitDepth
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    38,                 KMTPTypeUINT32Size}},   // EParentObject  
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT16,    42,                 KMTPTypeUINT16Size}},   // EAssociationType
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    44,                 KMTPTypeUINT32Size}},   // EAssociationDescription
        {EIdFlat1Chunk,         EMTPTypeFlat,   {EMTPTypeUINT32,    48,                 KMTPTypeUINT32Size}},   // ESequenceNumber
        {EIdFilenameChunk,      EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EFilename
        {EIdDateCreatedChunk,   EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EDateCreated
        {EIdDateModifiedChunk,  EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EDateModified
        {EIdKeywordsChunk,      EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EKeywords
    };

/**
MTP ObjectInfo dataset factory method. This method is used to create an empty 
MTP ObjectInfo dataset type. A pointer to the data type is placed on the 
cleanup stack.
@return A pointer to an empty MTP ObjectInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/    
EXPORT_C CMTPTypeObjectInfo* CMTPTypeObjectInfo::NewL()
    {
	CMTPTypeObjectInfo* self = CMTPTypeObjectInfo::NewLC(); 
	CleanupStack::Pop(self);
	return self; 
    }

/**
MTP ObjectInfo dataset factory method. This method is used to create an empty 
MTP ObjectInfo dataset type.
@return A pointer to an empty MTP ObjectInfo dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectInfo* CMTPTypeObjectInfo::NewLC()
    {
	CMTPTypeObjectInfo* self = new (ELeave) CMTPTypeObjectInfo(); 
	CleanupStack::PushL(self); 
    self->ConstructL();
	return self;    
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeObjectInfo::~CMTPTypeObjectInfo()
    {    
    iChunkFlat.Close();
    iChunksString.ResetAndDestroy();
    }
 
EXPORT_C TUint CMTPTypeObjectInfo::Type() const
    {
    return EMTPTypeObjectInfoDataset;
    }
    
const CMTPTypeCompoundBase::TElementInfo& CMTPTypeObjectInfo::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeObjectInfo::CMTPTypeObjectInfo() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KFlatChunkSize, *this)
    {
    
    }
    
void CMTPTypeObjectInfo::ConstructL()
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
            ChunkAppendL(*chunk);
            }
        }
    }

MMTPType* CMTPTypeObjectInfo::NewFlatChunkL(const TElementInfo& aElementInfo)
    {
    MMTPType* chunk(NULL);
    if (aElementInfo.iChunkId == 0)
        {
        iChunkFlat.OpenL();
        chunk = &iChunkFlat;
        }
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

MMTPType* CMTPTypeObjectInfo::NewStringChunkL(const TElementInfo& /*aElementInfo*/)
    {
    CMTPTypeString* chunk(CMTPTypeString::NewLC());
    iChunksString.AppendL(chunk);
    CleanupStack::Pop(chunk); 
    return chunk;
    }
