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

#include <mtp/cmtptypecompoundbase.h>
#include <mtp/cmtptypestring.h>
#include <mtp/rmtptype.h>

#include "mtpdatatypespanic.h"

/**
Destructor
*/
EXPORT_C CMTPTypeCompoundBase::~CMTPTypeCompoundBase()
    {
    Reset();    
    }
    
/**
Provides a copy of the specified element.
@param aElementId The identifier of the requested element.
@param aElement The MMTPType target data buffer.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the target buffer type does not match the requested
element.
*/
EXPORT_C void CMTPTypeCompoundBase::GetL(TInt aElementId, MMTPType& aElement) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
    
    const TElementInfo& info(ElementInfo(aElementId));
    if (info.iType == EMTPTypeFlat)
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->GetL(aElementId, aElement);
        }
    else
        {
        CopyL(*iChunks[info.iChunkId], aElement);
        }
    }

/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The MMTPType source data buffer.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/  
EXPORT_C void CMTPTypeCompoundBase::SetL(TInt aElementId, const MMTPType& aElement)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    if (info.iType == EMTPTypeFlat)
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetL(aElementId, aElement);
        }
    else
        {
        CopyL(aElement, *iChunks[info.iChunkId]);
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetInt8L(TInt aElementId, TInt8 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetInt8(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT8, Panic(EMTPTypeIdMismatch));   
        static_cast<TMTPTypeInt8&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeINT8:
        {
        TMTPTypeInt8* chunk(static_cast<TMTPTypeInt8*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetInt16L(TInt aElementId, TInt16 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetInt16(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT16, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeInt16&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeINT16:
        {
        TMTPTypeInt16* chunk(static_cast<TMTPTypeInt16*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetInt32L(TInt aElementId, TInt32 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetInt32(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT32, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeInt32&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeINT32:
        {
        TMTPTypeInt32* chunk(static_cast<TMTPTypeInt32*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetInt64L(TInt aElementId, TInt64 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetInt64(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT64, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeInt64&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeINT64:
        {
        TMTPTypeInt64* chunk(static_cast<TMTPTypeInt64*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetUint8L(TInt aElementId, TUint8 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetUint8(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT8, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeUint8&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeUINT8:
        {
        TMTPTypeUint8* chunk(static_cast<TMTPTypeUint8*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetUint16L(TInt aElementId, TUint16 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetUint16(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT16, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeUint16&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeUINT16:
        {
        TMTPTypeUint16* chunk(static_cast<TMTPTypeUint16*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetUint32L(TInt aElementId, TUint32 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetUint32(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT32, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeUint32&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeUINT32:
        {
        TMTPTypeUint32* chunk(static_cast<TMTPTypeUint32*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aData The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetUint64L(TInt aElementId, TUint64 aData)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        chunk->SetUint64(aElementId, aData);
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT64, Panic(EMTPTypeIdMismatch)); 
        static_cast<TMTPTypeUint64&>(chunk->Data()).Set(aData);
        }
        break;
        
    case EMTPTypeUINT64:
        {
        TMTPTypeUint64* chunk(static_cast<TMTPTypeUint64*>(iChunks[info.iChunkId]));
        chunk->Set(aData);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aString The source data.
@leave KErrNotSupported, if the element is not writeable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void CMTPTypeCompoundBase::SetStringL(TInt aElementId, const TDesC& aString)
    {
    if (!WriteableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeString, Panic(EMTPTypeIdMismatch)); 
        static_cast<CMTPTypeString&>(chunk->Data()).SetL(aString);
        }
        break;
        
    case EMTPTypeString:
        {
        CMTPTypeString* chunk(static_cast<CMTPTypeString*>(iChunks[info.iChunkId]));
        chunk->SetL(aString);
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
    }


/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt8 CMTPTypeCompoundBase::Int8L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TInt8 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Int8(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT8, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeInt8&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeINT8:
        {
        TMTPTypeInt8* chunk(static_cast<TMTPTypeInt8*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt16 CMTPTypeCompoundBase::Int16L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TInt16 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Int16(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT16, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeInt16&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeINT16:
        {
        TMTPTypeInt16* chunk(static_cast<TMTPTypeInt16*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt32 CMTPTypeCompoundBase::Int32L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TInt32 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Int32(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT32, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeInt32&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeINT32:
        {
        TMTPTypeInt32* chunk(static_cast<TMTPTypeInt32*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt64 CMTPTypeCompoundBase::Int64L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TInt64 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Int64(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeINT64, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeInt64&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeINT64:
        {
        TMTPTypeInt64* chunk(static_cast<TMTPTypeInt64*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint8 CMTPTypeCompoundBase::Uint8L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TUint8 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Uint8(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT8, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeUint8&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeUINT8:
        {
        TMTPTypeUint8* chunk(static_cast<TMTPTypeUint8*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint16 CMTPTypeCompoundBase::Uint16L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TUint16 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Uint16(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT16, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeUint16&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeUINT16:
        {
        TMTPTypeUint16* chunk(static_cast<TMTPTypeUint16*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint32 CMTPTypeCompoundBase::Uint32L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
    
    TUint32 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Uint32(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT32, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeUint32&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeUINT32:
        {
        TMTPTypeUint32* chunk(static_cast<TMTPTypeUint32*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    case EMTPTypeUndefined:
    default:
        {
        TMTPTypeUint32 chunk;
        GetL(aElementId, chunk);
        ret = chunk.Value();
        }
        break;
        }
        
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint64 CMTPTypeCompoundBase::Uint64L(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
    
    TUint64 ret(0);
    
    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeFlat:
        {
        RMTPTypeCompoundFlatChunk* chunk(static_cast<RMTPTypeCompoundFlatChunk*>(iChunks[info.iChunkId]));
        ret = chunk->Uint64(aElementId); 
        }
        break;
        
    case EMTPTypeReference:
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeUINT64, Panic(EMTPTypeIdMismatch)); 
        ret = static_cast<TMTPTypeUint64&>(chunk->Data()).Value();
        }
        break;
        
    case EMTPTypeUINT64:
        {
        TMTPTypeUint64* chunk(static_cast<TMTPTypeUint64*>(iChunks[info.iChunkId]));
        ret = chunk->Value();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }
    
/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint8 CMTPTypeCompoundBase::StringNumCharsL(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    TUint8 ret(0);

    const TElementInfo& info(ElementInfo(aElementId));
    switch (info.iType)
        {
    case EMTPTypeReference:
        {
             
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeString, Panic(EMTPTypeIdMismatch));   
        ret = static_cast<CMTPTypeString&>(chunk->Data()).NumChars();
        }
        break;
        
    case EMTPTypeString:
        {
        CMTPTypeString* chunk(static_cast<CMTPTypeString*>(iChunks[info.iChunkId]));
        ret = chunk->NumChars();
        }
        break;
        
    default:
        Panic(EMTPTypeIdMismatch);
        break;
        }
        
    return ret;
    }
   
/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@leave KErrNotSupported, if the element is not readable.
@leave One of the system wide error code, if a processing error occurs.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C const TDesC& CMTPTypeCompoundBase::StringCharsL(TInt aElementId) const
    {
    if (!ReadableElementL(aElementId))
        {
        User::Leave(KErrNotSupported);            
        }
        
    const TElementInfo& info(ElementInfo(aElementId));
    if (info.iType == EMTPTypeString)
        {
        CMTPTypeString* chunk(static_cast<CMTPTypeString*>(iChunks[info.iChunkId]));
        return chunk->StringChars();
        }
    else if (info.iType == EMTPTypeReference)
        {
        RMTPType* chunk(static_cast<RMTPType*>(iChunks[info.iChunkId]));
        __ASSERT_DEBUG(chunk->Type() == EMTPTypeString, Panic(EMTPTypeIdMismatch));   
        return static_cast<CMTPTypeString&>(chunk->Data()).StringChars();
        }
    else
        {
        Panic(EMTPTypeIdMismatch);
        }
        
    return KNullDesC;
    }

EXPORT_C TInt CMTPTypeCompoundBase::FirstReadChunk(TPtrC8& aChunk) const
    {
    TInt err(KErrNone);
    aChunk.Set(NULL, 0);
    
    if (iChunks.Count() == 0)
        {
        err = KErrNotFound;            
        }
    else
        {
        iReadChunk = 0;
        err = UpdateReadState(iChunks[iReadChunk]->FirstReadChunk(aChunk));
        }
    
    return err;
    }

EXPORT_C TInt CMTPTypeCompoundBase::NextReadChunk(TPtrC8& aChunk) const
    {
    TInt err(KErrNotReady);
    aChunk.Set(NULL, 0);
    
    switch (iReadSequenceState)
        {
    case EInProgressFirst:
        err = UpdateReadState(iChunks[iReadChunk]->FirstReadChunk(aChunk));
        break;
        
    case EInProgressNext:
        err = UpdateReadState(iChunks[iReadChunk]->NextReadChunk(aChunk));
        break;
        
    case EIdle:
    default:
        err = KErrNotReady;
        break;
        }
    
    return err;
    }

EXPORT_C TInt CMTPTypeCompoundBase::FirstWriteChunk(TPtr8& aChunk)
    {
    TInt err(KErrNone);
    aChunk.Set(NULL, 0, 0);
    
    if (iChunks.Count() == 0)
        {
        err = KErrNotFound;            
        }
    else
        {
        iWriteChunk = 0;
        err = UpdateWriteState(iChunks[iWriteChunk]->FirstWriteChunk(aChunk));
        }
    
    return err;
    }

EXPORT_C TInt CMTPTypeCompoundBase::NextWriteChunk(TPtr8& aChunk)
    {
    TInt err(KErrNotReady);
    aChunk.Set(NULL, 0, 0);
    
    switch (iWriteSequenceState)
        {
    case EInProgressFirst:
        err = UpdateWriteState(iChunks[iWriteChunk]->FirstWriteChunk(aChunk));
        break;
        
    case EInProgressNext:
        err = UpdateWriteState(iChunks[iWriteChunk]->NextWriteChunk(aChunk));
        break;
        
    case EIdle:
    default:
        err = KErrNotReady;
        break;
        }
    
    return err;
    }

EXPORT_C TInt CMTPTypeCompoundBase::NextWriteChunk(TPtr8& aChunk, TUint aDataLength)
    {
    TInt err(KErrNotReady);
    aChunk.Set(NULL, 0, 0);
    
    switch (iWriteSequenceState)
        {
    case EInProgressFirst:
        err = UpdateWriteState(iChunks[iWriteChunk]->FirstWriteChunk(aChunk, aDataLength));
        break;
        
    case EInProgressNext:
        err = UpdateWriteState(iChunks[iWriteChunk]->NextWriteChunk(aChunk, aDataLength));
        break;
        
    case EIdle:
    default:
        err = KErrNotReady;
        break;
        }
    
    return err;
    }

EXPORT_C TUint64 CMTPTypeCompoundBase::Size() const
    {
    TUint64 size(0);
    TUint numChunks(iChunks.Count());
    for (TUint i(0); (i < numChunks); i++)
        {
        TUint64 chunkSize(iChunks[i]->Size());
        size += chunkSize;
        }
        
    return size;  
    }

EXPORT_C TUint CMTPTypeCompoundBase::Type() const
    {
    return EMTPTypeCompound;        
    }

EXPORT_C TBool CMTPTypeCompoundBase::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* CMTPTypeCompoundBase::CommitChunkL(TPtr8& aChunk)
    {        
    MMTPType *chunk(iChunks[iWriteChunk]);
    MMTPType* res = NULL;
    if (chunk->CommitRequired())
        {
        res = chunk->CommitChunkL(aChunk);
        }
        
    if (iJustInTimeConstruction)
        {
        UpdateWriteState(iWriteErr);   
        }
    
    if (iWriteSequenceState == EInProgressFirst)
        {
        iWriteChunk++;            
        }
    return res;
    }
    
    
EXPORT_C TInt CMTPTypeCompoundBase::Validate() const
    {
    const TUint count(ChunkCount());
    TInt ret(KMTPDataTypeInvalid);
    if (iExpectedChunkCount == KVariableChunkCount)
        {
        ret = ValidateChunkCount();
        }
    else if (count == iExpectedChunkCount)
        {
        /*
        The element count is correct. Verify that the last element was 
        correctly received.
        */
        ret = KErrNone;
        }
        
    if (ret == KErrNone)
        {
        /*
        The element count is correct. Verify that the last element was 
        correctly received.
        */
        ret = iChunks[count - 1]->Validate();
        }
    return ret;
    }

/**
Constructor.
@param aJustInTimeConstruction This flag indicates that the derived class uses 
a just-in-time construction technique when constructing iteself from a write
data stream. Typically this is used for types whose final structure is dependant
upon data contained within the data stream itself, e.g. a dataset with an 
element whose data type is determined by meta data which precedes the element 
in the data stream.
@param aExpectedChunkCount This flag is used by the derived class to indicate 
the number of chunks which make up the compound type. A value of 
KVariableChunkCount indicates that the type is of variable rather than fixed 
length (e.g. a list of elements). If KVariableChunkCount is specified then the
derived class's ValidChunkCount method is invoked when the compound type is
validated (@see ValidateChunkCount).
@see Validate
@see ValidateChunkCount
@see KVariableChunkCount
*/
EXPORT_C CMTPTypeCompoundBase::CMTPTypeCompoundBase(TBool aJustInTimeConstruction, TUint aExpectedChunkCount) :
    iExpectedChunkCount(aExpectedChunkCount),
    iJustInTimeConstruction(aJustInTimeConstruction)
    {
        
    }

/**
Appends the specified data chunk to the compound type's sequence of chunks.
@param aChunk The chunk to be appended.
*/
EXPORT_C void CMTPTypeCompoundBase::ChunkAppendL(const MMTPType& aChunk)
    {
    iChunks.AppendL(&aChunk);
    }

/**
Removes the chunk at the specified position in the chunk sequence.
@param aChunkId The index of the chunk in in the chunk sequence.
*/
EXPORT_C void CMTPTypeCompoundBase::ChunkRemove(TUint aChunkId) 
    {
    iChunks.Remove(aChunkId);
    }
    
/**
Provides the number of discrete data chunks of which the compound type is
composed.
@return The number of discrete data chunks components which make up the 
compound type.
*/
EXPORT_C TUint CMTPTypeCompoundBase::ChunkCount() const
    {
    return iChunks.Count();   
    }
    
/**
Resets the compound base, removing all data chunk references.
*/
EXPORT_C void CMTPTypeCompoundBase::Reset()
    {
    iChunks.Close();    
    }
    
/**
Sets the expected chunk count. This flag is used by the derived class to 
indicate the number of chunks which make up the compound type. A value of 
KVariableChunkCount indicates that the type is of variable rather than fixed 
length (e.g. a list of elements). If KVariableChunkCount is specified then the
derived class's ValidChunkCount method is invoked when the compound type is
validated (@see ValidateChunkCount).
@param aExpectedChunkCount The new expected chunk count.
*/
EXPORT_C void CMTPTypeCompoundBase::SetExpectedChunkCount(TUint aExpectedChunkCount)
    {
    iExpectedChunkCount = aExpectedChunkCount;
    }
    
/**
Indicates if the specified element can be read from.
@param aElementId The identifier of the requested element.
@return ETrue if the element is readable, otherwise EFalse.
*/
EXPORT_C TBool CMTPTypeCompoundBase::ReadableElementL(TInt /*aElementId*/) const
    {
    return ETrue;        
    }
    
/**
Indicates if the specified element can be written to.
@param aElementId The identifier of the requested element.
@return ETrue if the element is writeable, otherwise EFalse.
*/
EXPORT_C TBool CMTPTypeCompoundBase::WriteableElementL(TInt /*aElementId*/) const
    {
    return ETrue;        
    }
    
TInt CMTPTypeCompoundBase::UpdateReadState(TInt aErr) const
    {
    TInt ret(aErr);
    
    switch (aErr)
        {
    case KMTPChunkSequenceCompletion:
        if (++iReadChunk < iChunks.Count())
            {
            iReadSequenceState = EInProgressFirst;
            ret = KErrNone;                    
            }
        else
            {
            iReadSequenceState = EIdle;                 
            }
        break;
        
    case KErrNone:
        iReadSequenceState = EInProgressNext;
        break;
        
    default:
        break;
        }
        
    return ret;
    }
    
/**
Validates that the number of chunks making up the compound type is valid. This
method is invoked on variable length compound types by the default @see Valid 
implementation.
@see Valid
@see KVariableChunkCount
*/
EXPORT_C TInt CMTPTypeCompoundBase::ValidateChunkCount() const
    {
    return KMTPDataTypeInvalid;
    }

TInt CMTPTypeCompoundBase::UpdateWriteState(TInt aErr)
    {
    TInt ret(aErr);
    
    iWriteErr = aErr;
    switch (iWriteErr)
        {
    case KMTPChunkSequenceCompletion:
        if ((iWriteChunk + 1) < iChunks.Count())
            {
            iWriteSequenceState = EInProgressFirst;
            ret = KErrNone;
            }
        else
            {
            iWriteSequenceState = EIdle;                 
            }
        break;
        
    case KErrNone:
        iWriteSequenceState = EInProgressNext;
        break;
        
    default:
        break;
        }
        
    return ret;
    }

/**
Constructor.
@param aSize The size (in bytes) of the flat data chunk.
@param aParent The compound data type of which the flat data chunk is a component.
*/
EXPORT_C CMTPTypeCompoundBase::RMTPTypeCompoundFlatChunk::RMTPTypeCompoundFlatChunk(TUint aSize, CMTPTypeCompoundBase& aParent) :
    iSize(aSize),
    iParent(&aParent)
    {

    }

/**
Releases the storage assigned to the flat data chunk.
*/
EXPORT_C void CMTPTypeCompoundBase::RMTPTypeCompoundFlatChunk::Close()
    {
    iBuffer.Close(); 
    }

/**
Allocates storage for the flat data chunk.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPTypeCompoundBase::RMTPTypeCompoundFlatChunk::OpenL()
    {
    iBuffer.CreateMaxL(iSize);
    iBuffer.FillZ();
    SetBuffer(iBuffer);
    }
    
EXPORT_C TUint CMTPTypeCompoundBase::RMTPTypeCompoundFlatChunk::Type() const
    {
    return EMTPTypeFlat;
    }

const TMTPTypeFlatBase::TElementInfo& CMTPTypeCompoundBase::RMTPTypeCompoundFlatChunk::ElementInfo(TInt aElementId) const
    {
    return iParent->ElementInfo(aElementId).iFlatChunkInfo;
    }
