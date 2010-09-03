// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include "tptpipinitevtack.h"
#include "ptpipdatatypes.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "tptpipinitevtackTraces.h"
#endif

    
// Dataset element metadata.
const TPTPIPInitEvtAck::TElementInfo TPTPIPInitEvtAck::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32,	0,	KMTPTypeUINT32Size}, // ELength
        {EMTPTypeUINT32,	4,	KMTPTypeUINT32Size}, // EType
    }; 

/** 
Constructor
*/
EXPORT_C TPTPIPInitEvtAck::TPTPIPInitEvtAck() :
    iElementInfo(iElementMetaData, ENumElements),
    iBuffer(KSize)
    {
    OstTraceFunctionEntry0( TPTPIPINITEVTACK_TPTPIPINITEVTACK_ENTRY );
    SetBuffer(iBuffer);
    OstTraceFunctionExit0( TPTPIPINITEVTACK_TPTPIPINITEVTACK_EXIT );
    }     

EXPORT_C TUint TPTPIPInitEvtAck::Type() const
	{
	OstTraceFunctionEntry0( TPTPIPINITEVTACK_TYPE_ENTRY );
	OstTraceFunctionExit0( TPTPIPINITEVTACK_TYPE_EXIT );
	return EPTPIPTypeInitEvtAck ;
	}
    
EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPInitEvtAck::ElementInfo(TInt aElementId) const
    {
    OstTraceFunctionEntry0( TPTPIPINITEVTACK_ELEMENTINFO_ENTRY );
    __ASSERT_DEBUG((aElementId < ENumElements), User::Invariant());
    OstTraceFunctionExit0( TPTPIPINITEVTACK_ELEMENTINFO_EXIT );
    return iElementInfo[aElementId];
    }
