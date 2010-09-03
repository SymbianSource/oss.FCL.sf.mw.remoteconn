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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "tptpipinitevtreqTraces.h"
#endif
     
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
    OstTraceFunctionEntry0( TPTPIPINITEVTREQUEST_TPTPIPINITEVTREQUEST_ENTRY );
    SetBuffer(iBuffer);
    OstTraceFunctionExit0( TPTPIPINITEVTREQUEST_TPTPIPINITEVTREQUEST_EXIT );
    }     

EXPORT_C TUint TPTPIPInitEvtRequest::Type() const
	{
	OstTraceFunctionEntry0( TPTPIPINITEVTREQUEST_TYPE_ENTRY );
	OstTraceFunctionExit0( TPTPIPINITEVTREQUEST_TYPE_EXIT );
	return EPTPIPTypeInitEvtRequest;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPInitEvtRequest::ElementInfo(TInt aElementId) const
    {
    OstTraceFunctionEntry0( TPTPIPINITEVTREQUEST_ELEMENTINFO_ENTRY );
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    OstTraceFunctionExit0( TPTPIPINITEVTREQUEST_ELEMENTINFO_EXIT );
    return iElementInfo[aElementId];
    }

