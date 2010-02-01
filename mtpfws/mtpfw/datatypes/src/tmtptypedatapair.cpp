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
#include <mtp/tmtptypedatapair.h>

// data pair element metadata.
const TMTPTypeDataPair::TElementInfo TMTPTypeDataPair::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32,    0,  KMTPTypeUINT32Size},    // EOwnerHandle
        {EMTPTypeUINT16,    4,  KMTPTypeUINT16Size},    // EDataCode
    };

/**
Constructor.
*/
EXPORT_C TMTPTypeDataPair::TMTPTypeDataPair() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }
    
EXPORT_C TMTPTypeDataPair::TMTPTypeDataPair( const TUint32 aHandle, const TUint16 aCode ):
	iElementInfo(iElementMetaData, ENumElements),
	iBuffer(KSize)
	{
    SetBuffer(iBuffer);
	SetUint32( TMTPTypeDataPair::EOwnerHandle, aHandle );
	SetUint16(TMTPTypeDataPair::EDataCode, aCode);
	}

EXPORT_C TMTPTypeDataPair::TMTPTypeDataPair(const TMTPTypeDataPair& aPair):
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    iBuffer = aPair.iBuffer;
    SetBuffer(iBuffer);
    }

EXPORT_C TUint TMTPTypeDataPair::Type() const
    {
    return EMTPTypeDataPair;
    }
    
const TMTPTypeFlatBase::TElementInfo& TMTPTypeDataPair::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }
