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

#include <mtp/mtpprotocolconstants.h>

#include "mtpusbdatatypeconstants.h"
#include "tmtpusbcontrolrequestdevicestatus.h"
    
// Dataset element metadata.
const TMTPUsbControlRequestDeviceStatus::TElementInfo TMTPUsbControlRequestDeviceStatus::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT16,   0,   KMTPTypeUINT16Size},    // EwLength
        {EMTPTypeUINT16,   2,   KMTPTypeUINT16Size},    // ECode
        {EMTPTypeUINT32,    4,   KMTPTypeUINT32Size},     // EParameter1
        {EMTPTypeUINT32,    8,   KMTPTypeUINT32Size},     // EParameter2
        {EMTPTypeUINT32,    12,  KMTPTypeUINT32Size},     // EParameter3 
        {EMTPTypeUINT32,    16,  KMTPTypeUINT32Size},     // EParameter4 
        {EMTPTypeUINT32,    20,  KMTPTypeUINT32Size},     // EParameter5
        {EMTPTypeUINT32,    24,  KMTPTypeUINT32Size},     // EParameter6
        {EMTPTypeUINT32,    28,  KMTPTypeUINT32Size},     // EParameter7
        {EMTPTypeUINT32,    32,  KMTPTypeUINT32Size},     // EParameter8
        {EMTPTypeUINT32,    36,  KMTPTypeUINT32Size},     // EParameter9
        {EMTPTypeUINT32,    40,  KMTPTypeUINT32Size},     // EParameter10
        {EMTPTypeUINT32,    44,  KMTPTypeUINT32Size},     // EParameter11
        {EMTPTypeUINT32,    48,  KMTPTypeUINT32Size},     // EParameter12
        {EMTPTypeUINT32,    52,  KMTPTypeUINT32Size},     // EParameter13
        {EMTPTypeUINT32,    56,  KMTPTypeUINT32Size},     // EParameter14
        {EMTPTypeUINT32,    60,  KMTPTypeUINT32Size},     // EParameter15
        {EMTPTypeUINT32,    64,  KMTPTypeUINT32Size},     // EParameter16
        {EMTPTypeUINT32,    68,  KMTPTypeUINT32Size},     // EParameter17
        {EMTPTypeUINT32,    72,  KMTPTypeUINT32Size},     // EParameter18
        {EMTPTypeUINT32,    76,  KMTPTypeUINT32Size},     // EParameter19
        {EMTPTypeUINT32,    80,  KMTPTypeUINT32Size},     // EParameter20
        {EMTPTypeUINT32,    84,  KMTPTypeUINT32Size},     // EParameter21
        {EMTPTypeUINT32,    88,  KMTPTypeUINT32Size},     // EParameter22
        {EMTPTypeUINT32,    92,  KMTPTypeUINT32Size},     // EParameter23
        {EMTPTypeUINT32,    96,  KMTPTypeUINT32Size},     // EParameter24
        {EMTPTypeUINT32,    100,  KMTPTypeUINT32Size},     // EParameter25
        {EMTPTypeUINT32,    104,  KMTPTypeUINT32Size},     // EParameter26
        {EMTPTypeUINT32,    108,  KMTPTypeUINT32Size},     // EParameter27
        {EMTPTypeUINT32,    112,  KMTPTypeUINT32Size},     // EParameter28
        {EMTPTypeUINT32,    116,  KMTPTypeUINT32Size},     // EParameter29
        {EMTPTypeUINT32,    120,  KMTPTypeUINT32Size},     // EParameter30
        {EMTPTypeUINT32,    124,  KMTPTypeUINT32Size},     // EParameter31
        {EMTPTypeUINT32,    128,  KMTPTypeUINT32Size},     // EParameter32
    }; 

/** 
Constructor
*/
EXPORT_C TMTPUsbControlRequestDeviceStatus::TMTPUsbControlRequestDeviceStatus() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KMaxSize),
    iNumParameters(0)
    {
    SetBuffer(iBuffer);
    }   
    
/**
Provides the number of parameters in the dataset which are set.
@return the number of set parameters.
*/
EXPORT_C TUint TMTPUsbControlRequestDeviceStatus::NumParameters() const
    {
    return iNumParameters;        
    }  
    
/**
Resets the dataset.
*/
EXPORT_C void TMTPUsbControlRequestDeviceStatus::Reset()
    {
    TMTPTypeFlatBase::Reset();
    iNumParameters = 0;
    }
	
EXPORT_C void TMTPUsbControlRequestDeviceStatus::SetUint32(TInt aElementId, TUint32 aData)
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
    
EXPORT_C TUint32 TMTPUsbControlRequestDeviceStatus::Uint32(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId - EParameter1 < iNumParameters), User::Invariant());
    return TMTPTypeFlatBase::Uint32(aElementId);
    }

EXPORT_C TInt TMTPUsbControlRequestDeviceStatus::FirstReadChunk(TPtrC8& aChunk) const
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

EXPORT_C TUint64 TMTPUsbControlRequestDeviceStatus::Size() const
    {
    const TUint emptyParams(((ENumElements - EParameter1) - iNumParameters) * iElementInfo[EParameter1].iSize);
    return (KMaxSize - emptyParams);
    }

EXPORT_C TUint TMTPUsbControlRequestDeviceStatus::Type() const
	{
	return EMTPUsbTypeControlRequestDeviceStatus;
	}
    
EXPORT_C TBool TMTPUsbControlRequestDeviceStatus::CommitRequired() const
    {
    return ETrue;
    }

EXPORT_C MMTPType* TMTPUsbControlRequestDeviceStatus::CommitChunkL(TPtr8& /*aChunk*/)
    {
    iNumParameters = 0;
    
    // Recalculate iNumParameters.
    for (TUint i(EParameter1); (i <= KMaxParameters); i++)
        {
        if (TMTPTypeFlatBase::Uint32(i) != 0)
            {
            iNumParameters = i - EParameter1 + 1;
            }
        }
    return NULL;
    }
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPUsbControlRequestDeviceStatus::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
