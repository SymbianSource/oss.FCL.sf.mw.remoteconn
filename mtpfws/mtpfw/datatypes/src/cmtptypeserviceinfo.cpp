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
#include <mtp/tmtptypeflatbase.h>
//#include <mtp/cmtptypeservicesection.h>
#include <mtp/cmtptypeserviceinfo.h>
#include <mtp/cmtptypeserviceprop.h>
#include <mtp/cmtptypeserviceformat.h>
#include <mtp/cmtptypeservicemethod.h>
#include <mtp/cmtptypeserviceevent.h>
#include <mtp/cmtptypestring.h>

#include "mtpdatatypespanic.h"

const TUint CMTPTypeServiceInfo::KFlat1ChunkSize(44);
const TUint CMTPTypeServiceInfo::KFlat2ChunkSize(8);

const CMTPTypeCompoundBase::TElementInfo CMTPTypeServiceInfo::iElementMetaData[CMTPTypeServiceInfo::ENumElements] = 
    {
        {EIdFlat1Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EMTPServiceID
        {EIdFlat1Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT32,    4,                  KMTPTypeUINT32Size}},   // EMTPServiceStorageID
        {EIdFlat1Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT128,   8,                  KMTPTypeUINT128Size}},  // EMTPServicePGUID
        {EIdFlat1Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT32,    24,                 KMTPTypeUINT32Size}},   // EMTPServiceVersion
        {EIdFlat1Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT128,   28,                 KMTPTypeUINT128Size}},  // EMTPServiceGUID
        {EIdServiceNameChunk,                 EMTPTypeString,    {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPServiceName        
        {EIdFlat2Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT32,    0,                  KMTPTypeUINT32Size}},   // EMTPServiceType
        {EIdFlat2Chunk,                       EMTPTypeFlat,      {EMTPTypeUINT32,    4,                  KMTPTypeUINT32Size}},   // EMTPBaseServiceID
   };

EXPORT_C CMTPTypeServiceInfo* CMTPTypeServiceInfo::NewL()
    {
    CMTPTypeServiceInfo* self = CMTPTypeServiceInfo::NewLC(); 
    CleanupStack::Pop(self);
    return self; 
    }

EXPORT_C CMTPTypeServiceInfo* CMTPTypeServiceInfo::NewLC()
    {
    CMTPTypeServiceInfo* self = new (ELeave) CMTPTypeServiceInfo(); 
    CleanupStack::PushL(self); 
    self->ConstructL();
    return self;    
    }

EXPORT_C CMTPTypeServiceInfo::~CMTPTypeServiceInfo()
    {   
    iChunkFlat1.Close();
    iChunkFlat2.Close();
    delete iChunkString;
    
    delete iChunkUsedServices;
    delete iChunkServiceDataBlock;
    
    delete iServicePropList;
    delete iServiceFormatList;
    delete iServiceMethodList;
    delete iServiceEventList;
    }

EXPORT_C TUint32 CMTPTypeServiceInfo::NumberOfUsedServices() const
    {
    return iChunkUsedServices->NumElements();
    }

EXPORT_C void CMTPTypeServiceInfo::UsedServiceL( const TUint aIndex, TMTPTypeGuid& aUsedServiceGUID ) const
    {
    iChunkUsedServices->ElementL(aIndex, aUsedServiceGUID);
    }

EXPORT_C void CMTPTypeServiceInfo::AppendUsedServiceL( const TMTPTypeGuid& aElement )
    {
    iChunkUsedServices->AppendL( aElement );
    }

EXPORT_C TUint32 CMTPTypeServiceInfo::NumberOfServiceDataBlock() const
    {
    return iChunkServiceDataBlock->NumElements();
    }

EXPORT_C void CMTPTypeServiceInfo::SerivceDataBlockL( const TUint aIndex, TMTPTypeGuid& aGUID ) const
    {
    iChunkServiceDataBlock->ElementL(aIndex, aGUID);
    }

EXPORT_C void CMTPTypeServiceInfo::AppendServiceDataBlockL( const TMTPTypeGuid& aElement )
    {
    iChunkServiceDataBlock->AppendL( aElement );
    }

EXPORT_C TUint CMTPTypeServiceInfo::Type() const
    {
    return EMTPTypeServiceInfoDataset;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServiceInfo::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

TBool CMTPTypeServiceInfo::WriteableElementL(TInt aElementId) const
    {
    TBool ret(EFalse);
    if(aElementId <= EBaseServiceID )
        ret = ETrue;
    
    return ret;
    }

TBool CMTPTypeServiceInfo::ReadableElementL(TInt aElementId) const
    {
    TBool ret(EFalse);
        if(aElementId <= EBaseServiceID )
            ret = ETrue;
        
        return ret;
    }

CMTPTypeServiceInfo::CMTPTypeServiceInfo() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this),
    iChunkFlat2(KFlat2ChunkSize, *this)
    {
    
    }

void CMTPTypeServiceInfo::ConstructL()
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
                iChunkString = CMTPTypeString::NewL();
                chunk = iChunkString ;
                break;
            default:
                break;
                }
                
            __ASSERT_DEBUG(chunk, User::Invariant());
            ChunkAppendL(*chunk);
            }
        }
    
    iChunkUsedServices = CMTPTypeArray::NewL(EMTPTypeAUINT128, EMTPTypeUINT128,  KMTPTypeUINT128Size );
    ChunkAppendL( *iChunkUsedServices );
    
    iServicePropList = CMTPTypeServicePropertyList::NewL();
    iServiceFormatList = CMTPTypeServiceFormatList::NewL();
    iServiceMethodList = CMTPTypeServiceMethodList::NewL();
    iServiceEventList = CMTPTypeServiceEventList::NewL();
    
    ChunkAppendL(*iServicePropList);
    ChunkAppendL(*iServiceFormatList);
    ChunkAppendL(*iServiceMethodList);
    ChunkAppendL(*iServiceEventList);
    
    iChunkServiceDataBlock = CMTPTypeArray::NewL(EMTPTypeAUINT128, EMTPTypeUINT128,  KMTPTypeUINT128Size );
    ChunkAppendL( *iChunkServiceDataBlock );
    
    }


MMTPType* CMTPTypeServiceInfo::NewFlatChunkL(const TElementInfo& aElementInfo)
    {
    MMTPType* chunk(NULL);
    if (aElementInfo.iChunkId == EIdFlat1Chunk)
        {
        iChunkFlat1.OpenL();
        chunk = &iChunkFlat1;
        }
    else if ( aElementInfo.iChunkId == EIdFlat2Chunk )
        {
        iChunkFlat2.OpenL();
        chunk = &iChunkFlat2;
        }
    else
        {}
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

EXPORT_C CMTPTypeServicePropertyList& CMTPTypeServiceInfo::ServicePropList()
    {
    return *iServicePropList;
    }

EXPORT_C CMTPTypeServiceFormatList& CMTPTypeServiceInfo::ServiceFormatList()
    {
    return *iServiceFormatList;
    }

EXPORT_C CMTPTypeServiceMethodList& CMTPTypeServiceInfo::ServiceMethodList()
    {
    return *iServiceMethodList;
    }

EXPORT_C CMTPTypeServiceEventList& CMTPTypeServiceInfo::ServiceEventList()
    {
    return *iServiceEventList;
    }

