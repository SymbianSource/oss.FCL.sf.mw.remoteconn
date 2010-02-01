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
#include <mtp/tmtptyperequest.h>

// Dataset element metadata.
const TMTPTypeRequest::TElementInfo TMTPTypeRequest::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT16, 0,  KMTPTypeUINT16Size},   // ERequestOperationCode
        {EMTPTypeUINT32, 2,  KMTPTypeUINT32Size},   // ERequestSessionID
        {EMTPTypeUINT32, 6,  KMTPTypeUINT32Size},   // ERequestTransactionID
        {EMTPTypeUINT32, 10, KMTPTypeUINT32Size},   // ERequestParameter1  
        {EMTPTypeUINT32, 14, KMTPTypeUINT32Size},   // ERequestParameter2
        {EMTPTypeUINT32, 18, KMTPTypeUINT32Size},   // ERequestParameter3
        {EMTPTypeUINT32, 22, KMTPTypeUINT32Size},   // ERequestParameter4
        {EMTPTypeUINT32, 26, KMTPTypeUINT32Size},   // ERequestParameter5
    };

/**
Constructor.
*/
EXPORT_C TMTPTypeRequest::TMTPTypeRequest() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }	
    
/**
Equality operator.
@param aRequest The request to compare against this TMTPTypeRequest.
@return ETrue if the two TMTPTypeRequests are equal, otherwise EFalse. 
*/	
EXPORT_C TBool TMTPTypeRequest::operator==(const TMTPTypeRequest& aRequest) const
	{
	return iBuffer == aRequest.iBuffer;
	}
    
/**
Inequality operator.
@param aRequest The request to compare against this TMTPTypeRequest.
@return ETrue if the two TMTPTypeRequests are different, otherwise EFalse. 
*/	
EXPORT_C TBool TMTPTypeRequest::operator!=(const TMTPTypeRequest& aRequest) const
	{
	return !(*this == aRequest);
	}

EXPORT_C TMTPTypeRequest::TMTPTypeRequest(const TMTPTypeRequest& aRequest):
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    iBuffer = aRequest.iBuffer;
    SetBuffer(iBuffer);
    }

EXPORT_C TUint TMTPTypeRequest::Type() const
    {
    return EMTPTypeRequestDataset;
    }
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TMTPTypeRequest::ElementInfo(TInt aElementId) const
    {
    return iElementInfo[aElementId];
    }

