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
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypeinterdependentpropdesc.h>
#include <mtp/cmtptypeservicecapabilitylist.h>


#include "mtpdatatypespanic.h"

// Dataset constants
const TUint KMTPChunk0Size = 2;

// Dataset element metadata.
const CMTPTypeCompoundBase::TElementInfo CMTPTypeFormatCapability::iElementMetaData[CMTPTypeFormatCapability::ENumElements] = 
    {
        {EIdFlatChunk,                          EMTPTypeFlat,                          {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EFormatCode
   };


EXPORT_C CMTPTypeServiceCapabilityList* CMTPTypeServiceCapabilityList::NewL()
    {
    CMTPTypeServiceCapabilityList* self = CMTPTypeServiceCapabilityList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceCapabilityList* CMTPTypeServiceCapabilityList::NewLC()
    {
    CMTPTypeServiceCapabilityList* self = new(ELeave) CMTPTypeServiceCapabilityList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceCapabilityList::~CMTPTypeServiceCapabilityList()
    {

    }

EXPORT_C void CMTPTypeServiceCapabilityList::AppendL(CMTPTypeFormatCapability* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }

EXPORT_C CMTPTypeFormatCapability& CMTPTypeServiceCapabilityList::ElementL(TUint aIndex) const
	{
	return static_cast<CMTPTypeFormatCapability&>( CMTPTypeList::ElementL(aIndex) );
	}

CMTPTypeServiceCapabilityList::CMTPTypeServiceCapabilityList():
	CMTPTypeList(EMTPTypeServiceCapabilityListDataset,EMTPTypeFormatCapabilityDataset)
    {

    }

void CMTPTypeServiceCapabilityList::ConstructL()
    {
    InitListL();
    }
    

EXPORT_C CMTPTypeFormatCapability* CMTPTypeFormatCapability::NewL()
    {
    CMTPTypeFormatCapability* self = CMTPTypeFormatCapability::NewLC(); 
    CleanupStack::Pop(self);
    return self; 
    }

EXPORT_C CMTPTypeFormatCapability* CMTPTypeFormatCapability::NewLC()
    {
    CMTPTypeFormatCapability* self = new (ELeave) CMTPTypeFormatCapability( NULL ); 
    CleanupStack::PushL(self); 
    self->ConstructL();
    return self;    
    }

EXPORT_C CMTPTypeFormatCapability* CMTPTypeFormatCapability::NewL(const TUint16 aFormatCode, CMTPTypeInterdependentPropDesc* aInterDependentPropDesc )
    {
    CMTPTypeFormatCapability* self = CMTPTypeFormatCapability::NewLC( aFormatCode, aInterDependentPropDesc ); 
    CleanupStack::Pop(self);
    return self; 
    }

EXPORT_C CMTPTypeFormatCapability* CMTPTypeFormatCapability::NewLC(const TUint16 aFormatCode, CMTPTypeInterdependentPropDesc* aInterDependentPropDesc )
    {
    CMTPTypeFormatCapability* self = new (ELeave) CMTPTypeFormatCapability( aInterDependentPropDesc ); 
    CleanupStack::PushL(self); 
    self->ConstructL( aFormatCode );
    return self;    
    }

EXPORT_C CMTPTypeFormatCapability::~CMTPTypeFormatCapability()
    {    
    iChunkFlat.Close();
    delete iObjectPropDescList;
    delete iInterdependentPropDesc;
    }

EXPORT_C void CMTPTypeFormatCapability::AppendL(CMTPTypeObjectPropDesc* aElement)
    {
    __ASSERT_DEBUG(aElement, User::Invariant());
    
    iObjectPropDescList->AppendL( static_cast<MMTPType*>(aElement));
    }

EXPORT_C TUint32 CMTPTypeFormatCapability::NumberOfPropDescs() const
    {
    return iObjectPropDescList->NumberOfElements();
    }

EXPORT_C CMTPTypeObjectPropDesc& CMTPTypeFormatCapability::ObjectPropDescL(TUint aIndex) const
    {
     return static_cast<CMTPTypeObjectPropDesc&>(iObjectPropDescList->ElementL( aIndex ));
    }

EXPORT_C CMTPTypeInterdependentPropDesc& CMTPTypeFormatCapability::InterdependentPropDesc() const
    {
    return *iInterdependentPropDesc;
    }

EXPORT_C TUint CMTPTypeFormatCapability::Type() const
    {
    return EMTPTypeFormatCapabilityDataset;
    }

EXPORT_C TBool CMTPTypeFormatCapability::ReadableElementL(TInt aElementId) const
    {
    if( aElementId == EFormatCode)
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

EXPORT_C TBool CMTPTypeFormatCapability::WriteableElementL(TInt aElementId ) const
    {
    if( aElementId == EFormatCode)
           {
           return ETrue;
           }
       else
           {
           return EFalse;
           } 
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeFormatCapability::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }


CMTPTypeFormatCapability::CMTPTypeFormatCapability( CMTPTypeInterdependentPropDesc* aInterDependentPropDesc) : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KMTPChunk0Size, *this),
    iInterdependentPropDesc(aInterDependentPropDesc)
    {
    
    }

void CMTPTypeFormatCapability::ConstructL()
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
                chunk = NewFlatChunkL();
                break;  
            default:
                break;
                }
                
            __ASSERT_DEBUG(chunk, User::Invariant());
            ChunkAppendL(*chunk);
            }
        }
    
    iObjectPropDescList = CMTPTypeList::NewL(EMTPTypeFormatCapabilityDataset,EMTPTypeObjectPropDescDataset);
    if( NULL == iInterdependentPropDesc )
        {
        iInterdependentPropDesc = CMTPTypeInterdependentPropDesc::NewL();
        }
    ChunkAppendL(*iObjectPropDescList);
    ChunkAppendL(*iInterdependentPropDesc);
    }

MMTPType* CMTPTypeFormatCapability::NewFlatChunkL()
    {
    MMTPType* chunk(NULL);

    iChunkFlat.OpenL();
    chunk = &iChunkFlat;
    
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

void CMTPTypeFormatCapability::ConstructL(const TUint16 aFormatCode)
    {
    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EFormatCode, aFormatCode);
    }
