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
#include <mtp/cmtptypestring.h>
#include <mtp/cmtptypeserviceprop.h>

#include "mtpdatatypespanic.h"

// Dataset constants


const TUint KMTPChunk0Size = 22;


const CMTPTypeCompoundBase::TElementInfo CMTPTypeServicePropertyElement::iElementMetaData[CMTPTypeServicePropertyElement::ENumElements] = 
    {
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EMTPServicePropCode
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT128,   2,                  KMTPTypeUINT128Size}},  // EMTPServicePropNamespace
        {EIdFlatChunk,						EMTPTypeFlat,	{EMTPTypeUINT32,	18,					KMTPTypeUINT32Size}},	// EMTPServicePropPKID
        {EIdServicePropNameChunk,         	EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPServicePropName
   };

EXPORT_C CMTPTypeServicePropertyList* CMTPTypeServicePropertyList::NewL()
    {
    CMTPTypeServicePropertyList* self = CMTPTypeServicePropertyList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServicePropertyList* CMTPTypeServicePropertyList::NewLC()
    {
    CMTPTypeServicePropertyList* self = new(ELeave) CMTPTypeServicePropertyList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServicePropertyList::~CMTPTypeServicePropertyList()
    {

    }

EXPORT_C void CMTPTypeServicePropertyList::AppendL(CMTPTypeServicePropertyElement* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }

CMTPTypeServicePropertyList::CMTPTypeServicePropertyList() :
CMTPTypeList(EMTPTypeServicePropertyListDataset,EMTPTypeServicePropertyElementDataset)
    {
    }

void CMTPTypeServicePropertyList::ConstructL()
    {
    InitListL();
    }

EXPORT_C CMTPTypeServicePropertyElement& CMTPTypeServicePropertyList::ElementL(TUint aIndex) const
    {
    return static_cast<CMTPTypeServicePropertyElement&>( CMTPTypeList::ElementL(aIndex) );
    }   

EXPORT_C CMTPTypeServicePropertyElement* CMTPTypeServicePropertyElement::NewL()
    {
    CMTPTypeServicePropertyElement* self = NewLC();
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServicePropertyElement* CMTPTypeServicePropertyElement::NewLC()
    {
    CMTPTypeServicePropertyElement* self = new(ELeave) CMTPTypeServicePropertyElement();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServicePropertyElement* CMTPTypeServicePropertyElement::NewL(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName)
    {
    CMTPTypeServicePropertyElement* self = NewLC(aPropertyCode, aNameSpace,aPKeyID, aPropertyName);
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServicePropertyElement* CMTPTypeServicePropertyElement::NewLC(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName)
    {
    CMTPTypeServicePropertyElement* self = new (ELeave) CMTPTypeServicePropertyElement(); 
	CleanupStack::PushL(self); 
    self->ConstructL(aPropertyCode, aNameSpace,aPKeyID, aPropertyName);
	return self;    
    }

/**
Destructor.
*/    
EXPORT_C CMTPTypeServicePropertyElement::~CMTPTypeServicePropertyElement()
    {    
    iChunkFlat.Close();
    delete iChunkString;
    }

EXPORT_C TUint CMTPTypeServicePropertyElement::Type() const
    {
    return EMTPTypeServicePropertyElementDataset;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServicePropertyElement::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeServicePropertyElement::CMTPTypeServicePropertyElement() : 
    CMTPTypeCompoundBase((KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KMTPChunk0Size, *this)
    {
    
    }

void CMTPTypeServicePropertyElement::ConstructL()
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
    }


MMTPType* CMTPTypeServicePropertyElement::NewFlatChunkL()
    {
    MMTPType* chunk(NULL);

    iChunkFlat.OpenL();
    chunk = &iChunkFlat;

    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

void CMTPTypeServicePropertyElement::ConstructL(const TUint16 aPropertyCode, const TMTPTypeGuid& aNameSpace,const TUint32 aPKeyID, const TDesC& aPropertyName)
    {

    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EPropertyCode, aPropertyCode);
    SetL(ENameSpace, aNameSpace);

    SetUint32L(EPkeyID,aPKeyID);
    SetStringL(EPorpertyName,aPropertyName);

    }
