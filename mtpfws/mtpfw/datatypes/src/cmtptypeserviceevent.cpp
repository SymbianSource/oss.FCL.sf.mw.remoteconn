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
#include <mtp/cmtptypeserviceevent.h>

#include "mtpdatatypespanic.h"

// Dataset constants
const TUint KMTPChunk0Size = 18;


const CMTPTypeCompoundBase::TElementInfo CMTPTypeServiceEventElement::iElementMetaData[CMTPTypeServiceEventElement::ENumElements] = 
    {
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EMTPServiceEventCode
        {EIdFlatChunk,                      EMTPTypeFlat,   {EMTPTypeUINT128,   2,                  KMTPTypeUINT128Size}},  // EMTPServiceEventGUID
        {EIdServiceEventNameChunk,          EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPServiceEventName
   };

EXPORT_C CMTPTypeServiceEventList* CMTPTypeServiceEventList::NewL()
    {
    CMTPTypeServiceEventList* self = CMTPTypeServiceEventList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceEventList* CMTPTypeServiceEventList::NewLC()
    {
    CMTPTypeServiceEventList* self = new(ELeave) CMTPTypeServiceEventList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceEventList::~CMTPTypeServiceEventList()
    {
    }

EXPORT_C void CMTPTypeServiceEventList::AppendL(CMTPTypeServiceEventElement* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }
EXPORT_C CMTPTypeServiceEventElement& CMTPTypeServiceEventList::ElementL(TUint aIndex) const
    {
    return static_cast<CMTPTypeServiceEventElement&>( CMTPTypeList::ElementL(aIndex) );
    }

CMTPTypeServiceEventList::CMTPTypeServiceEventList() :
CMTPTypeList(EMTPTypeServiceEventListDataset,EMTPTypeServiceEventElementDataset)
    {
    }

void CMTPTypeServiceEventList::ConstructL()
    {
    InitListL();
    }
    


EXPORT_C CMTPTypeServiceEventElement* CMTPTypeServiceEventElement::NewL()
    {
    CMTPTypeServiceEventElement* self = NewLC( );
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceEventElement* CMTPTypeServiceEventElement::NewLC()
    {
    CMTPTypeServiceEventElement* self = new(ELeave) CMTPTypeServiceEventElement();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceEventElement* CMTPTypeServiceEventElement::NewL(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName)
    {
    CMTPTypeServiceEventElement* self = NewLC( aEventCode, aEventGUID, aEventName);
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceEventElement* CMTPTypeServiceEventElement::NewLC(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName)
    {
    CMTPTypeServiceEventElement* self = new (ELeave) CMTPTypeServiceEventElement(); 
	CleanupStack::PushL(self); 
    self->ConstructL(aEventCode, aEventGUID, aEventName);
	return self;    
    }

/**
Destructor.
*/    
EXPORT_C CMTPTypeServiceEventElement::~CMTPTypeServiceEventElement()
    {    
    iChunkFlat.Close();
    delete iChunkString;
    }


EXPORT_C TUint CMTPTypeServiceEventElement::Type() const
    {
    return EMTPTypeServiceEventElementDataset;
    }


const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServiceEventElement::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeServiceEventElement::CMTPTypeServiceEventElement() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat(KMTPChunk0Size, *this)
    {
    
    }

void CMTPTypeServiceEventElement::ConstructL()
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


MMTPType* CMTPTypeServiceEventElement::NewFlatChunkL()
    {
    MMTPType* chunk(NULL);

    iChunkFlat.OpenL();
    chunk = &iChunkFlat;
        
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
    }

void CMTPTypeServiceEventElement::ConstructL(const TUint16 aEventCode,const TMTPTypeGuid& aEventGUID,const TDesC& aEventName)
    {

    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EEventCode, aEventCode);
    SetL(EEventGUID, aEventGUID);
    SetStringL(EEventName,aEventName);
    }
