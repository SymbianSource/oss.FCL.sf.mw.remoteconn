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

#include "mtpusbdatatypeconstants.h"
#include "tmtpusbcontrolrequestcanceldata.h"
    
// Dataset element metadata.
const TMTPUsbControlRequestCancelData::TElementInfo TMTPUsbControlRequestCancelData::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT16,    0,  KMTPTypeUINT16Size},    // ECancellationCode
        {EMTPTypeUINT32,    2,  KMTPTypeUINT32Size},    // ETransactionID
    }; 

/** 
Constructor
*/
EXPORT_C TMTPUsbControlRequestCancelData::TMTPUsbControlRequestCancelData() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }     

EXPORT_C TUint TMTPUsbControlRequestCancelData::Type() const
	{
	return EMTPUsbTypeControlRequestCancelData;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPUsbControlRequestCancelData::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
