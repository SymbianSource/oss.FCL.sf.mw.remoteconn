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
#include <mtp/tmtptyperesponse.h>
// Dataset element metadata.
const TMTPTypeResponse::TElementInfo TMTPTypeResponse::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT16,    0,  KMTPTypeUINT16Size},    // EResponseCode
        {EMTPTypeUINT32,    2,  KMTPTypeUINT32Size},    // EResponseSessionID
        {EMTPTypeUINT32,    6,  KMTPTypeUINT32Size},    // EResponseTransactionID
        {EMTPTypeUINT32,    10, KMTPTypeUINT32Size},    // EResponseParameter1
        {EMTPTypeUINT32,    14, KMTPTypeUINT32Size},    // EResponseParameter2
        {EMTPTypeUINT32,    18, KMTPTypeUINT32Size},    // EResponseParameter3
        {EMTPTypeUINT32,    22, KMTPTypeUINT32Size},    // EResponseParameter4
        {EMTPTypeUINT32,    26, KMTPTypeUINT32Size},    // EResponseParameter5
        {EMTPTypeINT32,    30, KMTPTypeUINT32Size},    // ENumValidParam
                
    };

/**
Constructor.
*/
EXPORT_C TMTPTypeResponse::TMTPTypeResponse() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    TMTPTypeFlatBase::Reset();
    }

EXPORT_C TMTPTypeResponse::TMTPTypeResponse(const TMTPTypeResponse& aResponse):
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    iBuffer = aResponse.iBuffer;
    SetBuffer(iBuffer);
    }

EXPORT_C TUint TMTPTypeResponse::Type() const
    {
    return EMTPTypeResponseDataset;
    }
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPTypeResponse::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

EXPORT_C void TMTPTypeResponse::SetUint32(TInt aElementId, TUint32 aData)
    {
    // Recalculate iNumOfValidParams
    if(aElementId - EResponseTransactionID > TMTPTypeFlatBase::Int32(ENumValidParam))
        {
        TMTPTypeFlatBase::SetInt32(ENumValidParam, aElementId - EResponseTransactionID);
        }
    //  Set the element value.
    TMTPTypeFlatBase::SetUint32(aElementId, aData);
    }


EXPORT_C TInt TMTPTypeResponse::GetNumOfValidParams() const
    {
    return TMTPTypeFlatBase::Int32(ENumValidParam);
    }
