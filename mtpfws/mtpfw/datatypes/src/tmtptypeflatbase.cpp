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

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtptypessimple.h>
#include <mtp/tmtptypeflatbase.h>

#include "mtpdatatypespanic.h"

/**
Default constructor.
*/
EXPORT_C TMTPTypeFlatBase::TMTPTypeFlatBase() :
    iBuffer(NULL, 0, 0)
    {
    
    }
    
/**
Resets the dataset by zero filling it..
*/
EXPORT_C void TMTPTypeFlatBase::Reset()
    {
    iBuffer.FillZ();
    } 
    
/**
Provides a copy of the specified element.
@param aElementId The identifier of the requested element.
@param aElement The MMTPType target data buffer.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the target buffer type does not match the requested 
element.
*/
EXPORT_C void TMTPTypeFlatBase::GetL(TInt aElementId, MMTPType& aElement) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == aElement.Type()), Panic(EMTPTypeIdMismatch));
    
    TBool commit(aElement.CommitRequired());
    TBool complete(EFalse);
    TPtr8 dest(NULL, 0);
    TInt offset(info.iOffset);
    
    TInt err(aElement.FirstWriteChunk(dest));
    while (((err == KMTPChunkSequenceCompletion) || (err == KErrNone)) && (!complete))
        {
        complete = (err == KMTPChunkSequenceCompletion);
        
        TPtrC8 src(&(iBuffer[offset]), (info.iSize - (offset - info.iOffset)));  
        dest.Copy(src);
        offset += dest.Length();
        
        if (commit)
            {
            aElement.CommitChunkL(dest);   
            }
        
        if (!complete)
            {
            err = aElement.NextWriteChunk(dest);                
            }  
        }
        
    if (err != KMTPChunkSequenceCompletion)
        {
        User::Leave(err);            
        }
    }

/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The MMTPType source data buffer.
@leave One of the system wide error code, if a processing error occurs. 
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
@panic MTPDataTypes 4, if the source buffer size does not match the requested 
element.
*/  
EXPORT_C void TMTPTypeFlatBase::SetL(TInt aElementId, const MMTPType& aElement)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == aElement.Type()), Panic(EMTPTypeIdMismatch));
    __ASSERT_ALWAYS((info.iSize == aElement.Size()), Panic(EMTPTypeSizeMismatch));
    
    TBool complete(EFalse);
    TInt offset(info.iOffset);
    TPtrC8 src;
    
    TInt err(aElement.FirstReadChunk(src));
    while (((err == KMTPChunkSequenceCompletion) || (err == KErrNone)) && (!complete))
        {
        __ASSERT_ALWAYS((((offset + src.Length()) - info.iOffset) <= info.iSize), Panic(EMTPTypeSizeMismatch));
        complete = (err == KMTPChunkSequenceCompletion);
        
        iBuffer.Replace(offset, src.Length(), src);
        offset += src.Length();
        
        if (!complete)
            {
            err = aElement.NextReadChunk(src);                
            } 
        }
        
    if (err != KMTPChunkSequenceCompletion)
        {
        User::Leave(err);            
        }
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetInt8(TInt aElementId, TInt8 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT8), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetInt16(TInt aElementId, TInt16 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT16), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetInt32(TInt aElementId, TInt32 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT32), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetInt64(TInt aElementId, TInt64 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT64), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetUint8(TInt aElementId, TUint8 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT8), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetUint16(TInt aElementId, TUint16 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT16), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetUint32(TInt aElementId, TUint32 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT32), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }
    
/**
Updates the specified element.
@param aElementId The identifier of the element to update.
@param aElement The source data.
@panic MTPDataTypes 3, if the source data type does not match the requested 
element.
*/    
EXPORT_C void TMTPTypeFlatBase::SetUint64(TInt aElementId, TUint64 aData)
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT64), Panic(EMTPTypeIdMismatch));
        
    TPtrC8 data(reinterpret_cast<TUint8*>(&aData), sizeof(aData));
    iBuffer.Replace(info.iOffset, data.Length(), data);
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt8 TMTPTypeFlatBase::Int8(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT8), Panic(EMTPTypeIdMismatch));
    return (static_cast<TInt8>(iBuffer[info.iOffset]));
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt16 TMTPTypeFlatBase::Int16(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT16), Panic(EMTPTypeIdMismatch));
        
    // memcpy the element data to avoid alignment errors.
    TInt16 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt32 TMTPTypeFlatBase::Int32(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT32), Panic(EMTPTypeIdMismatch));
    
    // memcpy the element data to avoid alignment errors.
    TInt32 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TInt64 TMTPTypeFlatBase::Int64(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeINT64), Panic(EMTPTypeIdMismatch));
    
    // memcpy the element data to avoid alignment errors.
    TInt64 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint8 TMTPTypeFlatBase::Uint8(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT8), Panic(EMTPTypeIdMismatch));
    return (static_cast<TUint8>(iBuffer[info.iOffset]));
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint16 TMTPTypeFlatBase::Uint16(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT16), Panic(EMTPTypeIdMismatch));
    
    // memcpy the element data to avoid alignment errors.
    TUint16 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint32 TMTPTypeFlatBase::Uint32(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT32), Panic(EMTPTypeIdMismatch));
    
    // memcpy the element data to avoid alignment errors.
    TUint32 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }

/**
Provides the value of the specified element.
@param aElementId The identifier of the requested element.
@return The value of the element.
@panic MTPDataTypes 3, if the source buffer type does not match the requested 
element.
*/
EXPORT_C TUint64 TMTPTypeFlatBase::Uint64(TInt aElementId) const
    {
    const TElementInfo& info(ElementInfo(aElementId));
    __ASSERT_ALWAYS((info.iType == EMTPTypeUINT64), Panic(EMTPTypeIdMismatch));
    
    // memcpy the element data to avoid alignment errors.
    TUint64 ret;
    memcpy(&ret, &iBuffer[info.iOffset], sizeof(ret));
    return ret;
    }
    
EXPORT_C TInt TMTPTypeFlatBase::FirstReadChunk(TPtrC8& aChunk) const
    {
    __ASSERT_ALWAYS(iBuffer.MaxLength() > 0, Panic(EMTPTypeBadStorage));
    aChunk.Set(iBuffer);
    return KMTPChunkSequenceCompletion;
    }
    
EXPORT_C TInt TMTPTypeFlatBase::NextReadChunk(TPtrC8& aChunk) const
    {
    aChunk.Set(NULL, 0);
    return KErrNotReady;
    }
    
EXPORT_C TInt TMTPTypeFlatBase::FirstWriteChunk(TPtr8& aChunk)
    {
    __ASSERT_ALWAYS(iBuffer.MaxLength() > 0, Panic(EMTPTypeBadStorage));
    aChunk.Set(&iBuffer[0], 0, iBuffer.MaxLength());
    return KMTPChunkSequenceCompletion;
    }
    
EXPORT_C TInt TMTPTypeFlatBase::NextWriteChunk(TPtr8& aChunk)
    {
    aChunk.Set(NULL, 0, 0);
    return KErrNotReady;
    }
    
EXPORT_C TUint64 TMTPTypeFlatBase::Size() const
    {
    return iBuffer.Size();
    }

EXPORT_C TUint TMTPTypeFlatBase::Type() const
    {
    return EMTPTypeFlat;        
    }

/**
Sets the MTP dataset buffer to be managed.
@param aBuffer The MTP dataset buffer to be managed.
*/
EXPORT_C void TMTPTypeFlatBase::SetBuffer(const TDes8& aBuffer)
    {
    iBuffer.Set(aBuffer.MidTPtr(0));
    iBuffer.SetLength(iBuffer.MaxLength());
    }


