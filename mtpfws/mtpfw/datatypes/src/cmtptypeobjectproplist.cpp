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

#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>
#include <e32debug.h> 
#include "mtpdatatypespanic.h"
#include "e32cmn.h" 
#include <utf.h>
// Dataset constants

const TInt KMTPPropListBufferPageSize(0x00010000); // 64kB
const TInt KReservedTransportHeaderSize(32);
const TInt KMaxStringSize(255);
const TUint8 KPropElemHeaderSize(sizeof(TUint32) + sizeof(TUint16) + sizeof(TUint16));
_LIT8(KMtpStringTerminator, "\0\0");

/**
MTP ObjectPropList dataset factory method. This method is used to create an 
empty MTP ObjectPropList dataset. 
@return A pointer to the MTP ObjectPropList dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropList* CMTPTypeObjectPropList::NewL()
    {
    CMTPTypeObjectPropList* self = CMTPTypeObjectPropList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

/**
MTP ObjectPropList dataset factory method. This method is used to create an 
empty MTP ObjectPropList dataset. A pointer to the data type is placed on the
cleanup stack.
@return A pointer to the MTP ObjectPropList dataset type. Ownership IS 
transfered.
@leave One of the system wide error codes, if unsuccessful.
*/ 
EXPORT_C CMTPTypeObjectPropList* CMTPTypeObjectPropList::NewLC()
    {
    CMTPTypeObjectPropList* self = new(ELeave) CMTPTypeObjectPropList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CMTPTypeObjectPropList::CMTPTypeObjectPropList()
    {
	
    }

/**
Destructor.
*/
EXPORT_C CMTPTypeObjectPropList::~CMTPTypeObjectPropList()
    {
    TInt pageCount = iIOPages.Count();
    for(TInt i =0; i< pageCount; i++)
    	{
		iIOPages[i].Close();    	
    	}
	iIOPages.Close();
	delete iCurrentElement;
	delete iReservedElement;
    }

void CMTPTypeObjectPropList::ConstructL()
{
    ReserveNewPage();
	iNumberOfElements = 0;
	memcpy(&(iIOPages[0][KReservedTransportHeaderSize]), &iNumberOfElements, sizeof(TUint32));
	iTransportHeaderIndex = KReservedTransportHeaderSize;
	iRevPageEnd = 0;
	iRevDataEnd = KReservedTransportHeaderSize + 4;
	iReservedElemCommitted = ETrue;
}

EXPORT_C CMTPTypeObjectPropListElement& CMTPTypeObjectPropList::ReservePropElemL(TUint32 aHandle, TUint16 aPropCode)
    {
    if (NULL == iReservedElement)
        {
        iReservedElement = CMTPTypeObjectPropListElement::NewL(this);
        }
    
    iReservedElement->iObjectHandle = aHandle;
    iReservedElement->iPropertyCode = aPropCode;
    iReservedElement->iDataType = 0x0000;
    iReservedElement->iPageIndex = iRevPageEnd;
    iReservedElement->iBufIndex = iRevDataEnd;
    iReservedElement->iArrayBuffered = EFalse;
    
    TUint pageIndex = iReservedElement->iPageIndex;
    TUint bufIndex = iReservedElement->iBufIndex;
    
    MemoryCopyL(pageIndex, bufIndex, &(iReservedElement->iObjectHandle), sizeof(TUint32));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), ETrue);
    MemoryCopyL(pageIndex, bufIndex, &(iReservedElement->iPropertyCode), sizeof(TUint16));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint16), ETrue);
    MemoryCopyL(pageIndex, bufIndex, &(iReservedElement->iDataType), sizeof(TUint16));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint16), ETrue);
    
    iReservedElement->iValueSize = 0;
    
    iReservedElemCommitted = EFalse;
    
    return *iReservedElement;
    }

EXPORT_C void CMTPTypeObjectPropList::CommitPropElemL(CMTPTypeObjectPropListElement& aElem)
    {
    if (&aElem != iReservedElement || iReservedElemCommitted 
            || iReservedElement->iValueSize == 0)
        {
        __ASSERT_DEBUG(EFalse, User::Invariant());
        }
    TUint elemSize = iReservedElement->Size();
    // Update iRevDataEnd, reallocate new buffer page if need
    IncreaseIndexL(iRevPageEnd, iRevDataEnd, elemSize, ETrue);
    ++(iNumberOfElements);
    memcpy(&(iIOPages[0][KReservedTransportHeaderSize]), &iNumberOfElements, sizeof(TUint32));
    
    iReservedElemCommitted = ETrue;
    }


EXPORT_C CMTPTypeObjectPropListElement& CMTPTypeObjectPropList::GetNextElementL() const
    {
    // Update the page index and buffer index of element
    if (NULL == iCurrentElement)
        {
        ResetCursor();
        }
    
    if (iResetCursor)
        {
        iResetCursor = EFalse;
        }
    else
        {
        TUint elemSize = iCurrentElement->Size();
        IncreaseIndexL(iCurrentElement->iPageIndex, iCurrentElement->iBufIndex, elemSize, EFalse);
        }

    iCurrentElement->iArrayBuffered = EFalse;
    
    // Copy value of handle/propcode/datatype from buffer to element
    TUint pageIndex = iCurrentElement->iPageIndex;
    TUint bufIndex = iCurrentElement->iBufIndex;
    MemoryCopyL(&(iCurrentElement->iObjectHandle), pageIndex, bufIndex, sizeof(TUint32));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), EFalse);
    MemoryCopyL(&(iCurrentElement->iPropertyCode), pageIndex, bufIndex, sizeof(TUint16));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint16), EFalse);
    MemoryCopyL(&(iCurrentElement->iDataType), pageIndex, bufIndex, sizeof(TUint16));
    IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint16), EFalse);

    // Calculate value length
    switch(iCurrentElement->iDataType)
        {
        case EMTPTypeUINT8:
            iCurrentElement->iValueSize = sizeof(TUint8);
            break;
        case EMTPTypeUINT16:
            iCurrentElement->iValueSize = sizeof(TUint16);
            break;
        case EMTPTypeUINT32:
            iCurrentElement->iValueSize = sizeof(TUint32);
            break;
        case EMTPTypeUINT64:
            iCurrentElement->iValueSize = sizeof(TUint64);
            break;
        case EMTPTypeUINT128:
            iCurrentElement->iValueSize = 2 * sizeof(TUint64);
            break;
        case EMTPTypeString:
            {
            TUint8 arrayLen = 0;
            MemoryCopyL(&arrayLen, pageIndex, bufIndex, sizeof(TUint8));
            iCurrentElement->iValueSize = arrayLen * sizeof(TUint16) + sizeof(TUint8);
            break;
            }
        case EMTPTypeAUINT16:
        case EMTPTypeAUINT8:
        case EMTPTypeAUINT32:
        case EMTPTypeAUINT64:
        case EMTPTypeAUINT128:
            {
            TUint32 arrayLen = 0;
            TUint32 arrayWidth = ArrayElemWidth(iCurrentElement->iDataType);
            MemoryCopyL(&arrayLen, pageIndex, bufIndex, sizeof(TUint32));
            iCurrentElement->iValueSize = arrayLen * arrayWidth + sizeof(TUint32);
            break;
            }
        default:
            break;
        }
    
    return *iCurrentElement;
    }

TBool CMTPTypeObjectPropList::ValueBufferConsistent(TUint /* aPageIdx */, TUint aBufferIdx, TUint aValueSize) const
    {
    if (aBufferIdx + aValueSize >= KMTPPropListBufferPageSize)
        {
        return EFalse;
        }
    
    return ETrue;
    }

TBool CMTPTypeObjectPropList::ValueAddressAligned(TUint aPageIdx, TUint aBufferIdx) const
    {
    const TUint8* bufPtr = &(iIOPages[aPageIdx][aBufferIdx]);
    if (reinterpret_cast<TUint32>(bufPtr) & 1)
        {
        return EFalse;
        }
    
    return ETrue;
    }

EXPORT_C TUint64 CMTPTypeObjectPropList::Size() const
{
    TInt pageCount = iIOPages.Count();
    TUint64 totalSize = 0;
    totalSize = KMTPPropListBufferPageSize - KReservedTransportHeaderSize  
                + KMTPPropListBufferPageSize * (iRevPageEnd - 1) + iRevDataEnd;

    return totalSize;
}


EXPORT_C TUint32 CMTPTypeObjectPropList::NumberOfElements() const
	{
    return iNumberOfElements;
	}

EXPORT_C void CMTPTypeObjectPropList::ResetCursor() const
    {
    if (NULL == iCurrentElement)
        {
        CMTPTypeObjectPropList* propList = const_cast<CMTPTypeObjectPropList*>(this);
        iCurrentElement = CMTPTypeObjectPropListElement::NewL(propList);
        }
    iCurrentElement->iPageIndex = 0;
    iCurrentElement->iBufIndex = KReservedTransportHeaderSize + sizeof(TUint32);
    iResetCursor = ETrue;
    }

EXPORT_C void CMTPTypeObjectPropList::AppendObjectPropListL(const CMTPTypeObjectPropList& aSource)
    {
    TInt chunkStatus = 0;
    TBool copyFirstChunk = ETrue;
    TUint srcElemNum = aSource.NumberOfElements();
    
    while (chunkStatus != KMTPChunkSequenceCompletion)
        {
        TPtrC8 tmpPtrRawData;
        if (copyFirstChunk)
            {
            chunkStatus = aSource.FirstReadChunk(tmpPtrRawData);
            tmpPtrRawData.Set(tmpPtrRawData.Ptr() + sizeof(TUint32), tmpPtrRawData.Length() - sizeof(TUint32));
            copyFirstChunk = EFalse;
            }
        else
            {
            chunkStatus = aSource.NextReadChunk(tmpPtrRawData);
            }
        
        const TUint8* srcPtr = tmpPtrRawData.Ptr();
        MemoryCopyL(iRevPageEnd, iRevDataEnd, srcPtr, tmpPtrRawData.Length());
        IncreaseIndexL(iRevPageEnd, iRevDataEnd, tmpPtrRawData.Length(), ETrue);
        }
    
    iNumberOfElements += srcElemNum;
    memcpy(&(iIOPages[0][KReservedTransportHeaderSize]), &iNumberOfElements, sizeof(TUint32));
    }

EXPORT_C TInt CMTPTypeObjectPropList::FirstReadChunk(TPtrC8& aChunk) const
    {
    TInt ret = KMTPDataTypeInvalid;
    iChunkIndex = 0;
    if (iRevPageEnd == 0)
        {
        aChunk.Set(&(iIOPages[0][iTransportHeaderIndex]), iRevDataEnd - iTransportHeaderIndex);
        ret = KMTPChunkSequenceCompletion;
        }
    else
        {
        aChunk.Set(&(iIOPages[0][iTransportHeaderIndex]), KMTPPropListBufferPageSize - iTransportHeaderIndex);
        ret = KErrNone;
        }
    return ret;
    }

EXPORT_C TInt CMTPTypeObjectPropList::NextReadChunk(TPtrC8& aChunk) const
	{
	TInt ret = KMTPDataTypeInvalid;
	++iChunkIndex;
	if (iChunkIndex < iRevPageEnd)
	    {
        aChunk.Set(&(iIOPages[iChunkIndex][0]), KMTPPropListBufferPageSize);
        ret = KErrNone;
	    }
	else if (iChunkIndex == iRevPageEnd)
	    {
        aChunk.Set(&(iIOPages[iChunkIndex][0]), iRevDataEnd);
        ret = KMTPChunkSequenceCompletion;
	    }
	
	
	return ret;
	}

EXPORT_C TInt CMTPTypeObjectPropList::FirstWriteChunk(TPtr8& aChunk)
	{
    aChunk.Set(&(iIOPages[0][iTransportHeaderIndex]), 0, KMTPPropListBufferPageSize - iTransportHeaderIndex);
    return KErrNone;
	}

EXPORT_C TInt CMTPTypeObjectPropList::NextWriteChunk(TPtr8& aChunk)
	{
	ReserveNewPage();
	aChunk.Set(&(iIOPages[iIOPages.Count()-1][0]), 0, KMTPPropListBufferPageSize);
    return KErrNone;
	}

EXPORT_C TInt CMTPTypeObjectPropList::FirstWriteChunk(TPtr8& aChunk, TUint aDataLength)
    {
    TInt ret = KMTPDataTypeInvalid;
    if (aDataLength <= KMTPPropListBufferPageSize - iTransportHeaderIndex)
        {
        aChunk.Set(&(iIOPages[0][iTransportHeaderIndex]), 0, aDataLength);
        ret = KMTPChunkSequenceCompletion;
        }
    else
        {
        aChunk.Set(&(iIOPages[0][iTransportHeaderIndex]), 0, KMTPPropListBufferPageSize - iTransportHeaderIndex);
        ret = KErrNone;
        }
    
    return ret;
    }

EXPORT_C TInt CMTPTypeObjectPropList::NextWriteChunk(TPtr8& aChunk, TUint aDataLength)
    {
    TInt ret = KMTPDataTypeInvalid;
    ReserveNewPage();
    if (aDataLength <= KMTPPropListBufferPageSize)
        {
        aChunk.Set(&(iIOPages[iIOPages.Count()-1][0]), 0, aDataLength);
        ret = KMTPChunkSequenceCompletion;
        }
    else
        {
        aChunk.Set(&(iIOPages[iIOPages.Count()-1][0]), 0, KMTPPropListBufferPageSize);
        ret = KErrNone;
        }
    
    return ret;    
    }

EXPORT_C TUint CMTPTypeObjectPropList::Type() const
	{
    return EMTPTypeObjectPropListDataset;
	}

EXPORT_C TBool CMTPTypeObjectPropList::CommitRequired() const
	{
	return ETrue;
	}

EXPORT_C MMTPType* CMTPTypeObjectPropList::CommitChunkL(TPtr8& aChunk)
	{
	iRevPageEnd = iIOPages.Count() - 1;
	if (iRevPageEnd == 0)
	    {
	    iRevDataEnd = aChunk.Length() + KReservedTransportHeaderSize;
	    
	    if (aChunk.Length() >= sizeof(TUint32))
	        {
	        memcpy(&iNumberOfElements, &(iIOPages[0][KReservedTransportHeaderSize]), sizeof(TUint32));
	        }	    
	    }
	else
	    {
	    iRevDataEnd = aChunk.Length();
	    }

	return NULL;
	}

EXPORT_C TBool CMTPTypeObjectPropList::ReserveTransportHeader(TUint aHeaderLength, TPtr8& aHeader)
	{
    if (aHeaderLength > KReservedTransportHeaderSize)
        {
        return EFalse;
        }
    
    iTransportHeaderIndex = KReservedTransportHeaderSize - aHeaderLength;
    
    aHeader.Set(&(iIOPages[0][iTransportHeaderIndex]), aHeaderLength, aHeaderLength);
    return ETrue;
	}

EXPORT_C TInt CMTPTypeObjectPropList::Validate() const
    {
    TInt relValue(KErrNone);
	TUint32 num = NumberOfElements();
	ResetCursor();
	for(TUint32 i = 0; i< num;i++)
		{
		TRAPD(err, GetNextElementL())
		if(KErrNone != err)
			{
			relValue = KMTPDataTypeInvalid;
			break;
			}
		}	
	
	ResetCursor();
	return relValue;
	}

TUint CMTPTypeObjectPropList::ArrayElemWidth(TUint16& aDataType) const
	{
	TUint width(0);
	switch(aDataType)
		{
		case EMTPTypeAUINT8:
			width = 1;
			break;
	    case EMTPTypeAUINT16:
			width = 2;
			break;			
	    case EMTPTypeAUINT32:
			width = 4;
			break;			
	    case EMTPTypeAUINT64:
			width = 8;
			break;			
	    case EMTPTypeAUINT128:
			width = 16;
			break;			
		default:
			break;
		}
	return width;
	}

void CMTPTypeObjectPropList::IncreaseIndexL(TUint& aPageIndex, TUint& aBufIndex, TUint aLength, TBool aReserveNewPage) const
    {
    aBufIndex += aLength;
    while (aBufIndex >= KMTPPropListBufferPageSize)
        {
        aBufIndex -= KMTPPropListBufferPageSize;
        ++aPageIndex;
        if (aPageIndex >= iIOPages.Count())
            {
            if (aReserveNewPage)
                {
                const_cast<CMTPTypeObjectPropList*>(this)->ReserveNewPage();
                }
            else
                {
                User::Leave(KErrOverflow);
                }
            }
        }
    }

void CMTPTypeObjectPropList::ReserveNewPage()
    {
    RBuf8 tmpBuffer;
    iIOPages.AppendL(tmpBuffer);
    iIOPages[iIOPages.Count() - 1].CreateMaxL(KMTPPropListBufferPageSize);
    }

void CMTPTypeObjectPropList::MemoryCopyL(TAny* aTrg, TUint aSrcPageIndex, TUint aSrcBufIndex, TUint aLength) const
    {
    TUint bytesLeft = aLength;
    TUint curPageIdx = aSrcPageIndex;
    TUint curSrcBufIdx = aSrcBufIndex;
    TUint8* copyTo = reinterpret_cast<TUint8*>(aTrg);
    while (bytesLeft > 0)
        {
        const TUint8* srcBuf = &(iIOPages[curPageIdx][curSrcBufIdx]);
        TUint copySize = bytesLeft;
        if (curSrcBufIdx + bytesLeft > KMTPPropListBufferPageSize)
            {
            copySize = KMTPPropListBufferPageSize - curSrcBufIdx;
            ++curPageIdx;
            curSrcBufIdx = 0;
            }
        
        if (curPageIdx >= iIOPages.Count())
            {
            User::Leave(KErrOverflow);
            }
        else if ((curPageIdx == iIOPages.Count() - 1) && curSrcBufIdx + bytesLeft - copySize > iRevDataEnd)
            {
            User::Leave(KErrOverflow);
            }
        
        memcpy(copyTo, srcBuf, copySize);
        bytesLeft -= copySize;
        copyTo += copySize;
        }
    }

void CMTPTypeObjectPropList::MemoryCopyL(TUint aTrgPageIndex, TUint aTrgBufIndex, const TAny* aSrc, TUint aLength)
    {
    TUint bytesLeft = aLength;
    TUint curPageIdx = aTrgPageIndex;
    TUint curSrcBufIdx = aTrgBufIndex;
    const TUint8* copyFrom = reinterpret_cast<const TUint8*>(aSrc);
    while (bytesLeft > 0)
        {
        if (curPageIdx == iIOPages.Count())
            {
            ReserveNewPage();
            }        
        TUint8* trgBuf = &(iIOPages[curPageIdx][curSrcBufIdx]);
        TUint copySize = bytesLeft;
        if (curSrcBufIdx + bytesLeft > KMTPPropListBufferPageSize)
            {
            copySize = KMTPPropListBufferPageSize - curSrcBufIdx;
            ++curPageIdx;
            curSrcBufIdx = 0;
            }
        memcpy(trgBuf, copyFrom, copySize);
        bytesLeft -= copySize;
        copyFrom += copySize;
        }
    }

CMTPTypeObjectPropListElement* CMTPTypeObjectPropListElement::NewL(CMTPTypeObjectPropList* propList)
	{
	CMTPTypeObjectPropListElement* self = NewLC(propList);
	CleanupStack::Pop(self);
	return self;
	}

CMTPTypeObjectPropListElement* CMTPTypeObjectPropListElement::NewLC(CMTPTypeObjectPropList* propList)
    {
    CMTPTypeObjectPropListElement* self = new(ELeave) CMTPTypeObjectPropListElement();
    CleanupStack::PushL(self);
    self->ConstructL(propList);
    return self;
    }

CMTPTypeObjectPropListElement::CMTPTypeObjectPropListElement()
	{

	}

EXPORT_C CMTPTypeObjectPropListElement::~CMTPTypeObjectPropListElement()
	{
	iStringHolder.Close();
	}

void CMTPTypeObjectPropListElement::ConstructL(CMTPTypeObjectPropList* propList)
	{
	iArrayBuffered = EFalse;
    iPropList = propList;
	iStringHolder.CreateL(KMaxStringSize);
	}

EXPORT_C TUint8 CMTPTypeObjectPropListElement::Uint8L(TInt aElementId) const
	{
	if(EValue != aElementId || EMTPTypeUINT8 != iDataType)
	    User::Leave(KErrArgument);
	TUint8 retValue = 0;
	GetValueL(&retValue, sizeof(TUint8));

	return retValue;
	}

EXPORT_C TUint16 CMTPTypeObjectPropListElement::Uint16L(TInt aElementId) const
	{
	switch(aElementId)
		{
		case EPropertyCode:
			return iPropertyCode;
		case EDatatype:
			return iDataType;
		case EValue:
            if(EMTPTypeUINT16 != iDataType)
                {
                  User::Leave(KErrArgument);
                }
            TUint16 retValue;
            GetValueL(&retValue, sizeof(TUint16));
			return retValue;
		default:
			User::Leave(KErrArgument);
		}	
	return 0;
	}

EXPORT_C TUint32 CMTPTypeObjectPropListElement::Uint32L(TInt aElementId) const
	{
	switch(aElementId)
		{
		case EObjectHandle:
			return iObjectHandle;
		case EValue:
		    if(EMTPTypeUINT32 != iDataType)
		        {
		          User::Leave(KErrArgument);
		        }
		    TUint32 retValue;
		    GetValueL(&retValue, sizeof(TUint32));
		    return retValue;
		default:
			User::Leave(KErrArgument);	
		}
	return 0;
	}

EXPORT_C TUint64 CMTPTypeObjectPropListElement::Uint64L(TInt aElementId) const
	{
	if(EValue != aElementId || EMTPTypeUINT64 != iDataType)
	    {
	    User::Leave(KErrArgument);
	    }
	
	TUint64 retValue;
	GetValueL(&retValue, sizeof(TUint64));	
	return retValue;
	}

EXPORT_C void CMTPTypeObjectPropListElement::Uint128L(TInt aElementId,TUint64& high, TUint64& low) const
	{
	if(EValue != aElementId || EMTPTypeUINT128 != iDataType)
	    {
	    User::Leave(KErrArgument);
	    }
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, EFalse);
    iPropList->MemoryCopyL(&low, pageIndex, bufIndex, sizeof(TUint64));
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint64), EFalse);
    iPropList->MemoryCopyL(&high, pageIndex, bufIndex, sizeof(TUint64));
	}

EXPORT_C const TDesC& CMTPTypeObjectPropListElement::StringL(TInt aElementId) const
	{
	if(EValue != aElementId || EMTPTypeString != iDataType)
	    {
	    User::Leave(KErrArgument);
	    }
	
	if (!iArrayBuffered)
	    {
        TUint arrayLen = 0;
        TUint pageIndex = iPageIndex;
        TUint bufIndex = iBufIndex;
        iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, EFalse);
        iPropList->MemoryCopyL(&arrayLen, pageIndex, bufIndex, sizeof(TUint8));
        iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint8), EFalse);
        
        if (arrayLen == 0)
            {
            // Empty string
            iStringValue.Set(iStringHolder.Ptr(), 0);
            }
        else if (iPropList->ValueBufferConsistent(pageIndex, bufIndex, (arrayLen - 1) * sizeof(TUint16))
                && iPropList->ValueAddressAligned(pageIndex, bufIndex))
            {
            //No need to copy data
            const TUint8* srcPtr = &(iPropList->iIOPages[pageIndex][bufIndex]);            
            iStringValue.Set(reinterpret_cast<const TUint16*>(srcPtr), arrayLen - 1);
            }
        else
            {
            // The address of srcPtr is not aligned, can't pass the address to TPtr16::Set() 
            // Copy the string data in buffer to iStringHolder
            TUint16* dstPtr = const_cast<TUint16*>(iStringHolder.Ptr());
            iPropList->MemoryCopyL(dstPtr, pageIndex, bufIndex, (arrayLen - 1) * sizeof(TUint16));
            iStringValue.Set(dstPtr, arrayLen - 1);
            }
        
	    iArrayBuffered = ETrue;
	    }

	return iStringValue;
	}

EXPORT_C const TDesC8& CMTPTypeObjectPropListElement::ArrayL(TInt aElementId) const
	{
	if(EValue != aElementId || iDataType < EMTPTypeFirstSimpleArrayType 
	        || iDataType > EMTPTypeLastSimpleArrayType)
	    {
	    User::Leave(KErrArgument);
	    }
	
   if (!iArrayBuffered)
        {
        TUint32 arrayLen = 0;
        TUint32 arrayWidth = iPropList->ArrayElemWidth(iDataType);
        
        TUint pageIndex = iPageIndex;
        TUint bufIndex = iBufIndex;
        iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, EFalse);
        iPropList->MemoryCopyL(&arrayLen, pageIndex, bufIndex, sizeof(TUint32));
        iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), EFalse);
        
        if (arrayLen == 0)
            {
            // Empty array
            iArrayValue.Set(reinterpret_cast<const TUint8*>(iStringHolder.Ptr()), 0);
            }
        // if iDataType == EMTPTypeAINT16 or iDataType == EMTPTypeAUINT16, the array maybe be converted to TDes16 by the user, the address should be aligned
        else if (iPropList->ValueBufferConsistent(pageIndex, bufIndex, arrayLen * arrayWidth)
                && ((iDataType != EMTPTypeAINT16 && iDataType != EMTPTypeAUINT16) || iPropList->ValueAddressAligned(pageIndex, bufIndex)))
            {
            //No need to copy data
            const TUint8* srcPtr = &(iPropList->iIOPages[pageIndex][bufIndex]);
            iArrayValue.Set(srcPtr, arrayLen * arrayWidth);
            }
        else
            {
            // The address of srcPtr is not aligned, can't pass the address to TPtr16::Set() 
            // Copy the array data in buffer to iStringHolder
            if (iStringHolder.MaxLength() * sizeof(TUint16) < arrayLen * arrayWidth)
                {
                iStringHolder.Zero();
                iStringHolder.ReAllocL((arrayLen * arrayWidth + 1) / sizeof(TUint16));
                }
            TUint16* dstPtr = const_cast<TUint16*>(iStringHolder.Ptr());
            iPropList->MemoryCopyL(dstPtr, pageIndex, bufIndex, arrayLen * arrayWidth);
            iArrayValue.Set(reinterpret_cast<const TUint8*>(dstPtr), arrayLen * arrayWidth);
            }
        iArrayBuffered = ETrue;
        }
   
	return iArrayValue;
	}


EXPORT_C void CMTPTypeObjectPropListElement::SetUint8L(TInt aElementId, TUint8 aData)
    {
    if(EValue != aElementId)
        {
        User::Leave(KErrArgument);
        }
    
    SetDataType(EMTPTypeUINT8);
    SetValueL(&aData, sizeof(TUint8));
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetUint16L(TInt aElementId, TUint16 aData)
    {
    switch(aElementId)
        {
        case EPropertyCode:
            {
            iPropertyCode = aData;
            // Copy PropertyCode to page buffer
            TUint pageIndex = iPageIndex;
            TUint bufIndex = iBufIndex;
            iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), ETrue);
            iPropList->MemoryCopyL(pageIndex, bufIndex, &aData, sizeof(TUint16));
            }
            break;
        case EDatatype:
            SetDataType(aData);
            break;
        case EValue:
            SetDataType(EMTPTypeUINT16);
            SetValueL(&aData, sizeof(TUint16));
            break;
        default:
            User::Leave(KErrArgument);
        }   
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetUint32L(TInt aElementId, TUint32 aData)
    {
    switch(aElementId)
        {
        case EObjectHandle:
            iObjectHandle = aData;
            iPropList->MemoryCopyL(iPageIndex, iBufIndex, &aData, sizeof(TUint32));
            break;
        case EValue:
            SetDataType(EMTPTypeUINT32);
            SetValueL(&aData, sizeof(TUint32));
            break;
        default:
            User::Leave(KErrArgument);
        }
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetUint64L(TInt aElementId, TUint64 aData)
    {
    if(EValue != aElementId)
        User::Leave(KErrArgument);
    
    SetDataType(EMTPTypeUINT64);
    SetValueL(&aData, sizeof(TUint64));
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetUint128L(TInt aElementId, TUint64 high, TUint64 low)
    {
    if(EValue != aElementId)
        {
        User::Leave(KErrArgument);
        }
    
    SetDataType(EMTPTypeUINT128);
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &low, sizeof(TUint64));
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint64), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &high, sizeof(TUint64));
    iValueSize = sizeof(TUint64) * 2;
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetStringL(TInt aElementId, const TDesC& aString)
    {
    if(EValue != aElementId || aString.Length() > KMaxStringSize)
        {
        User::Leave(KErrArgument);
        }
    
    TUint8 len = aString.Length() + 1;

    // For string which length is 255, truncate the last character to handle the file name of 255
    if (aString.Length() == KMaxStringSize)
        {
        len = KMaxStringSize;
        }
    
    SetDataType(EMTPTypeString);


    iValueSize = 1 + len * sizeof(TUint16);
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    // Copy string length
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &len, sizeof(TUint8));
    // Copy string data
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint8), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, aString.Ptr(), (len - 1) * sizeof(TUint16));
    // Append terminator
    iPropList->IncreaseIndexL(pageIndex, bufIndex, (len - 1) * sizeof(TUint16), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, KMtpStringTerminator().Ptr(), 2);
        
    iArrayBuffered = EFalse;
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetArrayL(TInt aElementId, const CMTPTypeArray& aArray)
    {
    if(EValue != aElementId)
        {
        User::Leave(KErrArgument);
        }
    
    SetDataType(aArray.Type());
    TUint32 num = aArray.NumElements();
    
    
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    
    // Set number of array elements
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &num, sizeof(TUint32));

    TPtrC8 srcPtr;
    aArray.FirstReadChunk(srcPtr);
    srcPtr.Set(srcPtr.Ptr() + sizeof(TUint32), srcPtr.Length() - sizeof(TUint32));
    
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), ETrue);
    TUint arrayWidth = iPropList->ArrayElemWidth(iDataType);
    iValueSize = sizeof(TUint32) + num * arrayWidth;
    iPropList->MemoryCopyL(pageIndex, bufIndex, srcPtr.Ptr(), num * arrayWidth);
    iArrayBuffered = EFalse;
    }

EXPORT_C void CMTPTypeObjectPropListElement::SetArrayL(TInt aElementId, const TDesC& aString)
    {
    if(EValue != aElementId)
        {
        User::Leave(KErrArgument);
        }
    
    TUint32 len = aString.Length() + 1;
    
    SetDataType(EMTPTypeAUINT16);

    iValueSize = sizeof(TUint32) + len * sizeof(TUint16);
    
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    // Copy string length
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &len, sizeof(TUint32));
    // Copy string data
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, aString.Ptr(), (len - 1) * sizeof(TUint16));
    // Append terminator
    iPropList->IncreaseIndexL(pageIndex, bufIndex, (len - 1) * sizeof(TUint16), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, KMtpStringTerminator().Ptr(), 2);
        
    iArrayBuffered = EFalse;
     }

EXPORT_C TUint32 CMTPTypeObjectPropListElement::CMTPTypeObjectPropListElement::Size() const
    {
    return iValueSize + KPropElemHeaderSize;
    }

void CMTPTypeObjectPropListElement::SetDataType(TUint16 aDataType)
    {
    iDataType = aDataType;
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;    
    iPropList->IncreaseIndexL(pageIndex, bufIndex, sizeof(TUint32) + sizeof(TUint16), ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, &iDataType, sizeof(TUint16));
    }

void CMTPTypeObjectPropListElement::GetValueL(TAny* aTrg, TUint aLength) const
    {
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, EFalse);
    iPropList->MemoryCopyL(aTrg, pageIndex, bufIndex, aLength);
    }

void CMTPTypeObjectPropListElement::SetValueL(const TAny* aSrc, TUint aLength)
    {
    TUint pageIndex = iPageIndex;
    TUint bufIndex = iBufIndex;
    iPropList->IncreaseIndexL(pageIndex, bufIndex, KPropElemHeaderSize, ETrue);
    iPropList->MemoryCopyL(pageIndex, bufIndex, aSrc, aLength);
    iValueSize = aLength;
    }


