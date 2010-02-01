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
#include "tmtpusbcontrolrequestmodheader.h"
    
// Dataset element metadata.
const TMTPUsbControlRequestMODHeader::TElementInfo TMTPUsbControlRequestMODHeader::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32,    0,  KMTPTypeUINT32Size},    // EdwLength
        {EMTPTypeUINT16,    4,  KMTPTypeUINT16Size},    // EbcdVersion
        {EMTPTypeUINT16,    6,  KMTPTypeUINT16Size},    // EwIndex
        {EMTPTypeUINT8,     8,  KMTPTypeUINT8Size},     // EbCount
        {EMTPTypeUINT8,     9,  KMTPTypeUINT8Size},     // EReserved1 
        {EMTPTypeUINT8,     10, KMTPTypeUINT8Size},     // EReserved2 
        {EMTPTypeUINT8,     11, KMTPTypeUINT8Size},     // EReserved3 
        {EMTPTypeUINT8,     12, KMTPTypeUINT8Size},     // EReserved4 
        {EMTPTypeUINT8,     13, KMTPTypeUINT8Size},     // EReserved5 
        {EMTPTypeUINT8,     14, KMTPTypeUINT8Size},     // EReserved6 
        {EMTPTypeUINT8,     15, KMTPTypeUINT8Size},     // EReserved7 
    }; 

/** 
Constructor
*/
EXPORT_C TMTPUsbControlRequestMODHeader::TMTPUsbControlRequestMODHeader() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }     

EXPORT_C TUint TMTPUsbControlRequestMODHeader::Type() const
	{
	return EMTPUsbTypeControlRequestMODHeader;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPUsbControlRequestMODHeader::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }
