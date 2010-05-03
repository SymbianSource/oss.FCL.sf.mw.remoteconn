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
 @internalComponent
*/

#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpusbdatatypeconstants.h"
#include "tmtpusbparameterpayloadblock.h"

// Dataset element metadata.
const TMTPUsbParameterPayloadBlock::TElementInfo TMTPUsbParameterPayloadBlock::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32, 0,     KMTPTypeUINT32Size},    // EParameter1  
        {EMTPTypeUINT32, 4,     KMTPTypeUINT32Size},    // EParameter2
        {EMTPTypeUINT32, 8,     KMTPTypeUINT32Size},    // EParameter3
        {EMTPTypeUINT32, 12,    KMTPTypeUINT32Size},    // EParameter4
        {EMTPTypeUINT32, 16,    KMTPTypeUINT32Size},    // EParameter5
    };

/**
Constructor.
*/
EXPORT_C TMTPUsbParameterPayloadBlock::TMTPUsbParameterPayloadBlock() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KMaxSize),
    iNumParameters(0)
    {
    SetBuffer(iBuffer);
    }
    
/**
Provides the number of parameters in the block which are set.
@return the number of set parameters.
*/
EXPORT_C TUint TMTPUsbParameterPayloadBlock::NumParameters() const
    {
    return iNumParameters;        
    }
    
/**
Copies up to five parameter values into the parameter block dataset 
from the specified dataset. The paramters to be copied should be non null paramter or valid null paramter.
@param aFrom The parameter values source dataset. This dataset should define three or five contiguous 32-bit parameter values starting at the specified offset.
@param aParamStartOffset The element ID of the first in the set of contiguous 
source dataset parameter values.
@param aParamEndOffset The element ID of the last in the set of contiguous 
source dataset parameter values.
@param aIsNullParamValid a boolean value to check if a null parameter is valid. A value of ETrue means a null parameter is valid; EFalse means invalid.
@param aNumOfNullParam the number of valid null parameters to be copied.
*/
EXPORT_C void TMTPUsbParameterPayloadBlock::CopyIn(const TMTPTypeFlatBase& aFrom, TUint aParamStartOffset, TUint aParamEndOffset, TBool /*aIsNullParamValid*/, TUint /*aNumOfNullParam*/)
    {
    __ASSERT_DEBUG((aParamEndOffset >= aParamStartOffset && (aParamEndOffset - aParamStartOffset) <= ENumElements), User::Invariant());
    TUint32 parameter = KMTPNotSpecified32;
    TUint loopCount =  aParamEndOffset - aParamStartOffset;  

    for (TUint i(TMTPUsbParameterPayloadBlock::EParameter1); i < loopCount; i++)
        {
        parameter = aFrom.Uint32(aParamStartOffset + i);
        SetUint32(i, parameter);
        }
    }

/**
Copies the non-null parameter values from the parameter block dataset into the 
specified dataset. Note: the number of parameters to be copied out depends on two conditions:
1. The number of parameters contained in this USB parameter block dataset.
2. The number of parameters to be copied out to the parameter values sink dataset, which is (aParamEndOffset - aParamStartOffset + 1) by the caller.
@param aTo The parameter values sink dataset. This dataset should define three or five contiguous 32-bit parameter values starting at the specified offset.
@param aParamStartOffset the element ID of the first in the set of contiguous sink dataset parameter values.
@param aParamEndOffset the element ID of the last in the set of contiguous sink dataset parameter values.
*/
EXPORT_C void TMTPUsbParameterPayloadBlock::CopyOut(TMTPTypeFlatBase& aTo, TUint aParamStartOffset, TUint aParamEndOffset)
    {
    __ASSERT_DEBUG((aParamEndOffset >= aParamStartOffset && (aParamEndOffset - aParamStartOffset + 1) <= ENumElements), User::Invariant());
    TUint loopCount =  aParamEndOffset - aParamStartOffset; 
    for (TUint i(TMTPUsbParameterPayloadBlock::EParameter1); (i < iNumParameters && i <= loopCount); i++)
        {
        aTo.SetUint32((aParamStartOffset + i), Uint32(i));
        } 
    }
    
/**
Resets the dataset.
*/
EXPORT_C void TMTPUsbParameterPayloadBlock::Reset()
    {
    TMTPTypeFlatBase::Reset();
    iNumParameters = 0;
    }
	
EXPORT_C void TMTPUsbParameterPayloadBlock::SetUint32(TInt aElementId, TUint32 aData)
    {
    // Recalculate iNumParameters.
    TUint num((aElementId - EParameter1) + 1);
    
    if (num > iNumParameters)
        {
        iNumParameters = num;            
        }
        
    //  Set the element value.
    TMTPTypeFlatBase::SetUint32(aElementId, aData);

    }
    
EXPORT_C TUint32 TMTPUsbParameterPayloadBlock::Uint32(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < iNumParameters), User::Invariant());
    return TMTPTypeFlatBase::Uint32(aElementId);
    }

EXPORT_C TInt TMTPUsbParameterPayloadBlock::FirstReadChunk(TPtrC8& aChunk) const
    {
    TInt    ret(TMTPTypeFlatBase::FirstReadChunk(aChunk));
    TUint64 size(Size());
    
    if (size < aChunk.Size())
        {
        // Truncate the buffer at the last set parameter.
        aChunk.Set(aChunk.Left(size));
        }

    return ret;
    }

EXPORT_C TUint64 TMTPUsbParameterPayloadBlock::Size() const
    {
    return (iNumParameters * KMTPTypeUINT32Size); 
    }

EXPORT_C TUint TMTPUsbParameterPayloadBlock::Type() const
    {
    return EMTPUsbTypeParameterPayloadBlock;
    }
    
EXPORT_C TBool TMTPUsbParameterPayloadBlock::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* TMTPUsbParameterPayloadBlock::CommitChunkL(TPtr8& /*aChunk*/)
    {
    iNumParameters = 0;
    
    // Recalculate iNumParameters.
    for (TUint i(EParameter1); (i <= EParameter5); i++)
        {
        if (TMTPTypeFlatBase::Uint32(i) != KMTPNotSpecified32)
            {
            iNumParameters = (i + 1);                
            }
        }
    return NULL;
    }
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPUsbParameterPayloadBlock::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }

