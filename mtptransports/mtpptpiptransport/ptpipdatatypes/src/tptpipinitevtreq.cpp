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
// tptpipinitevetreq.cpp
// 
//

/**
 @internalComponent
*/


#include "tptpipinitevtreq.h"
#include "ptpipdatatypes.h"     
// Dataset element metadata.
const TPTPIPInitEvtRequest::TElementInfo TPTPIPInitEvtRequest::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32,	0,	KMTPTypeUINT32Size},     // EbmRequestType
        {EMTPTypeUINT32,	4,	KMTPTypeUINT32Size},     // EbRequest
        {EMTPTypeUINT32,	8,	KMTPTypeUINT32Size},    // EwValue
    }; 

/** 
Constructor
*/
EXPORT_C TPTPIPInitEvtRequest::TPTPIPInitEvtRequest() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    SetBuffer(iBuffer);
    }     

EXPORT_C TUint TPTPIPInitEvtRequest::Type() const
	{
	return EPTPIPTypeInitEvtRequest;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPInitEvtRequest::ElementInfo(TInt aElementId) const
    {
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    return iElementInfo[aElementId];
    }

