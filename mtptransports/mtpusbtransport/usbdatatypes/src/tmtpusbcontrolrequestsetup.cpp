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

#include "mtpusbdatatypeconstants.h"
#include "tmtpusbcontrolrequestsetup.h"
    
// Dataset element metadata.
const TMTPUsbControlRequestSetup::TElementInfo TMTPUsbControlRequestSetup::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT8,     0,  KMTPTypeUINT8Size},     // EbmRequestType
        {EMTPTypeUINT8,     1,  KMTPTypeUINT8Size},     // EbRequest
        {EMTPTypeUINT16,    2,  KMTPTypeUINT16Size},    // EwValue
        {EMTPTypeUINT16,    4,  KMTPTypeUINT16Size},    // EwIndex
        {EMTPTypeUINT16,    6,  KMTPTypeUINT16Size},    // EwLength 
    }; 

/** 
Constructor
*/
EXPORT_C TMTPUsbControlRequestSetup::TMTPUsbControlRequestSetup() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }     

EXPORT_C TUint TMTPUsbControlRequestSetup::Type() const
	{
	return EMTPUsbTypeControlRequestSetup;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPUsbControlRequestSetup::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
