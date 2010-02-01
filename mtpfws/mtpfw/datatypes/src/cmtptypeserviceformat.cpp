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
#include <mtp/cmtptypeserviceformat.h>

#include "mtpdatatypespanic.h"

// Dataset constants

const TUint CMTPTypeServiceFormatElement::KFlat1ChunkSize(18);
const TUint CMTPTypeServiceFormatElement::KFlat2ChunkSize(2);


const CMTPTypeCompoundBase::TElementInfo CMTPTypeServiceFormatElement::iElementMetaData[CMTPTypeServiceFormatElement::ENumElements] = 
    {
        {EIdFlat1Chunk,                      EMTPTypeFlat,   {EMTPTypeUINT16,    0,                  KMTPTypeUINT16Size}},   // EMTPServiceFormatCode
        {EIdFlat1Chunk,                      EMTPTypeFlat,   {EMTPTypeUINT128,   2,                  KMTPTypeUINT128Size}},  // EMTPServiceFormatGUID
        {EIdFormatNameChunk,				EMTPTypeString,	{EMTPTypeString,	KMTPNotApplicable,	KMTPNotApplicable}},	// EMTPServiceFormatName        
        {EIdFlat2Chunk,						EMTPTypeFlat,	{EMTPTypeUINT16,	0,					KMTPTypeUINT16Size}},	// EMTPServiceFormatBase
        {EIdFormatMIMETypeChunk,   	        EMTPTypeString, {EMTPTypeString,    KMTPNotApplicable,  KMTPNotApplicable}},    // EMTPServiceFormatMIMTType
   };

EXPORT_C CMTPTypeServiceFormatList* CMTPTypeServiceFormatList::NewL()
    {
    CMTPTypeServiceFormatList* self = CMTPTypeServiceFormatList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeServiceFormatList* CMTPTypeServiceFormatList::NewLC()
    {
    CMTPTypeServiceFormatList* self = new(ELeave) CMTPTypeServiceFormatList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceFormatList::~CMTPTypeServiceFormatList()
    {

    }

EXPORT_C void CMTPTypeServiceFormatList::AppendL(CMTPTypeServiceFormatElement* aElement)
    {
    CMTPTypeList::AppendL(aElement);
    }

CMTPTypeServiceFormatList::CMTPTypeServiceFormatList() :
CMTPTypeList(EMTPTypeServiceFormatListDataset,EMTPTypeServiceFormatElementDataset)
    {
    }

void CMTPTypeServiceFormatList::ConstructL()
    {
    InitListL();
    }
    
EXPORT_C CMTPTypeServiceFormatElement& CMTPTypeServiceFormatList::ElementL(TUint aIndex) const
    {
    return static_cast<CMTPTypeServiceFormatElement&>( CMTPTypeList::ElementL(aIndex) );
    }

EXPORT_C CMTPTypeServiceFormatElement* CMTPTypeServiceFormatElement::NewL()
    {
    CMTPTypeServiceFormatElement* self = NewLC();
    CleanupStack::Pop(self);
    return self; 
    }

EXPORT_C CMTPTypeServiceFormatElement* CMTPTypeServiceFormatElement::NewLC()
    {
    CMTPTypeServiceFormatElement* self = new(ELeave) CMTPTypeServiceFormatElement();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeServiceFormatElement* CMTPTypeServiceFormatElement::NewL(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType)
    {
    CMTPTypeServiceFormatElement* self = NewLC( aFormatCode, aFormatGUID, aFormatName, aFormatBase, aMIMEType);
    CleanupStack::Pop(self);
    return self; 
    }

EXPORT_C CMTPTypeServiceFormatElement* CMTPTypeServiceFormatElement::NewLC(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType)
    {
    CMTPTypeServiceFormatElement* self = new (ELeave) CMTPTypeServiceFormatElement(); 
	CleanupStack::PushL(self); 
    self->ConstructL(aFormatCode, aFormatGUID, aFormatName, aFormatBase, aMIMEType);
	return self;    
    }

/**
Destructor.
*/    
EXPORT_C CMTPTypeServiceFormatElement::~CMTPTypeServiceFormatElement()
    {    
    iChunkFlat1.Close();
    iChunkFlat2.Close();
    delete iChunkFormatNameString;
    delete iChunkMIMETypeString;
    }



EXPORT_C TUint CMTPTypeServiceFormatElement::Type() const
    {
    return EMTPTypeServiceFormatElementDataset;
    }

const CMTPTypeCompoundBase::TElementInfo& CMTPTypeServiceFormatElement::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

CMTPTypeServiceFormatElement::CMTPTypeServiceFormatElement() : 
    CMTPTypeCompoundBase((!KJustInTimeConstruction), EIdNumChunks), 
    iElementInfo(iElementMetaData, ENumElements),
    iChunkFlat1(KFlat1ChunkSize, *this),
    iChunkFlat2(KFlat2ChunkSize, *this)
    {
    
    }

void CMTPTypeServiceFormatElement::ConstructL()
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

MMTPType* CMTPTypeServiceFormatElement::NewFlatChunkL(const TElementInfo& aElementInfo)
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

MMTPType* CMTPTypeServiceFormatElement::NewStringChunkL(const TElementInfo& aElementInfo)
	{
    MMTPType* chunk(NULL);
    if (aElementInfo.iChunkId == EIdFormatNameChunk)
        {
        iChunkFormatNameString = CMTPTypeString::NewL();
        chunk = iChunkFormatNameString ;
        }
    else if ( aElementInfo.iChunkId == EIdFormatMIMETypeChunk )
    	{
    	iChunkMIMETypeString = CMTPTypeString::NewL();
    	chunk = iChunkMIMETypeString ;
    	}
    else
    	{}
    __ASSERT_DEBUG(chunk, User::Invariant());
    return chunk;
	}

void CMTPTypeServiceFormatElement::ConstructL(const TUint16 aFormatCode,const TMTPTypeGuid& aFormatGUID,const TDesC& aFormatName,const TUint16 aFormatBase,const TDesC& aMIMEType)
    {

    ConstructL();

    // Set mandatory construction values.
    SetUint16L(EFormatCode, aFormatCode);
    SetL(EFormatGUID, aFormatGUID);
    SetStringL(EFormatName,aFormatName);
    SetUint16L(EFormatBase,aFormatBase);
    
    SetStringL(EFormatMIMEType,aMIMEType);

    }
