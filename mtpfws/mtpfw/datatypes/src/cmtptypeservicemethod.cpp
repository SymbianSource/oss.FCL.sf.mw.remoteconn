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
#include <mtp/cmtptypeservicemethod.h>

#include "mtpdatatypespanic.h"

// Dataset constants


const TUint CMTPTypeServiceMethodElement::KFlat1ChunkSize(18);
const TUint CMTPTypeServiceMethodElement::KFlat2ChunkSize(2);


const CMTPTypeCompoundBase::TElementInfo CMTPTypeServiceMethodElement::iElementMetaData[CMTPTypeServiceMethodElement::ENumElements] = 
    {
        {EIdFlat1Chunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EMTPServiceMethodCode
        {EIdFlat1Chunk,                      EMTPTypeFlat,   {EMTPTypeUINT128,   2,                  KMTPTypeUINT128Size}},  // EMTPServiceMethodGUID
        {EIdServiceMethodNameChunk,         EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPServiceMethodName
        {EIdFlat2Chunk,						EMTPTypeFlat,	{EMTPTypeUINT16,	0,					KMTPTypeUINT16Size}},	// EMTPServiceMethodAssocFormat        
   };

EXPORT_C CMTPTypeServiceMethodList* CMTPTypeServiceMethodList::NewL()
    {
    CMTPTypeServiceMethodList* self = CMTPTypeServiceMethodList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceMethodList* CMTPTypeServiceMethodList::NewLC()
    {
    CMTPTypeServiceMethodList* self = new(ELeave) CMTPTypeServiceMethodList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceMethodList::~CMTPTypeServiceMethodList()
    {

    }

EXPORT_C void CMTPTypeServiceMethodList::AppendL(CMTPTypeServiceMethodElement* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }



CMTPTypeServiceMethodList::CMTPTypeServiceMethodList() :
CMTPTypeList(EMTPTypeServiceMethodListDataset,EMTPTypeServiceMethodElementDataset)
    {
    }

void CMTPTypeServiceMethodList::ConstructL()
    {
    InitListL();
    }

EXPORT_C CMTPTypeServiceMethodElement& CMTPTypeServiceMethodList::ElementL(TUint aIndex) const
    {
    return static_cast<CMTPTypeServiceMethodElement&>( CMTPTypeList::ElementL(aIndex) );
    }    

EXPORT_C CMTPTypeServiceMethodElement* CMTPTypeServiceMethodElement::NewL()
    {
    CMTPTypeServiceMethodElement* self = NewLC();
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceMethodElement* CMTPTypeServiceMethodElement::NewLC()
    {
    CMTPTypeServiceMethodElement* self = new(ELeave) CMTPTypeServiceMethodElement();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceMethodElement* CMTPTypeServiceMethodElement::NewL(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate)
    {
    CMTPTypeServiceMethodElement* self = NewLC( aMethodCode, aMethodGUID, aMethodName, aMethodAssociateFormate);
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceMethodElement* CMTPTypeServiceMethodElement::NewLC(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate)
    {
    CMTPTypeServiceMethodElement* self = new (ELeave) CMTPTypeServiceMethodElement(); 
	CleanupStack::PushL(self); 
    self->ConstructL(aMethodCode, aMethodGUID, aMethodName, aMethodAssociateFormate);
	return self;    
    }

/**
Destructor.
*/    
EXPORT_C CMTPTypeServiceMethodElement::~CMTPTypeServiceMethodElement()
    {    
    iChunkFlat1.Close();
    iChunkFlat2.Close();
    delete iChunkString;
    }

EXPORT_C TUint CMTPTypeServiceMethodElement::Type() const
    {
    return EMTPTypeServiceMethodElementDataset;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServiceMethodElement::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeServiceMethodElement::CMTPTypeServiceMethodElement() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this),
    iChunkFlat2(KFlat2ChunkSize, *this)
    {
    
    }

void CMTPTypeServiceMethodElement::ConstructL()
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
    }

MMTPType* CMTPTypeServiceMethodElement::NewFlatChunkL(const TElementInfo& aElementInfo)
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

void CMTPTypeServiceMethodElement::ConstructL(const TUint16 aMethodCode,const TMTPTypeGuid& aMethodGUID,const TDesC& aMethodName,const TUint16 aMethodAssociateFormate)
    {

    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EMethodCode, aMethodCode);
    SetL(EMethodGUID, aMethodGUID);
    SetStringL(EMethodName,aMethodName);
    SetUint16L(EMethodAssociateFormate,aMethodAssociateFormate);
    }
