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

#include "ptpipdatatypes.h"
#include "tptpipstartdatapayload.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "tptpipstartdatapayloadTraces.h"
#endif


	
// Dataset element metadata.
const TPTPIPTypeStartDataPayload::TElementInfo TPTPIPTypeStartDataPayload::iElementMetaData[ENumElements] = 
    {
        {EMTPTypeUINT32, 0,  KMTPTypeUINT32Size},  // Transaction Id
        {EMTPTypeUINT64, 4,  KMTPTypeUINT64Size}   // Total Data size
    };

/**
 Constructor.
 */
EXPORT_C TPTPIPTypeStartDataPayload::TPTPIPTypeStartDataPayload() :
	iElementInfo(iElementMetaData, ENumElements),
	iBuffer(KSize)
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_TPTPIPTYPESTARTDATAPAYLOAD_ENTRY );
	SetBuffer(iBuffer);
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_TPTPIPTYPESTARTDATAPAYLOAD_EXIT );
	}

/**
 Resets the dataset.
 */
EXPORT_C void TPTPIPTypeStartDataPayload::Reset()
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_RESET_ENTRY );
	TMTPTypeFlatBase::Reset();
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_RESET_EXIT );
	}

/**
 Sets the transaction id.
 */
EXPORT_C void TPTPIPTypeStartDataPayload::SetUint32(TInt aElementId, TUint32 aData)
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_SETUINT32_ENTRY );
	__ASSERT_DEBUG((aElementId == ETransactionId), User::Invariant());
	TMTPTypeFlatBase::SetUint32(aElementId, aData);
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_SETUINT32_EXIT );
	}

/**
 Gets the transaction id
 */
EXPORT_C TUint32 TPTPIPTypeStartDataPayload::Uint32(TInt aElementId) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_UINT32_ENTRY );
	__ASSERT_DEBUG((aElementId == ETransactionId), User::Invariant());
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_UINT32_EXIT );
	return TMTPTypeFlatBase::Uint32(aElementId);
	}

/**
 Sets the 64 bit size of total data in the data phase. 
 */
EXPORT_C void TPTPIPTypeStartDataPayload::SetUint64(TInt aElementId, TUint64 aData)
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_SETUINT64_ENTRY );
	__ASSERT_DEBUG((aElementId == ETotalSize), User::Invariant());
	TMTPTypeFlatBase::SetUint64(aElementId, aData);
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_SETUINT64_EXIT );
	}

/**
 Gets the 64 bit size of the total data in the data phase. 
 */
EXPORT_C TUint64 TPTPIPTypeStartDataPayload::Uint64(TInt aElementId) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_UINT64_ENTRY );
	__ASSERT_DEBUG((aElementId == ETotalSize), User::Invariant());
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_UINT64_EXIT );
	return TMTPTypeFlatBase::Uint64(aElementId);
	}

/**
 todo
 */
EXPORT_C TInt TPTPIPTypeStartDataPayload::FirstReadChunk(TPtrC8& aChunk) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_FIRSTREADCHUNK_ENTRY );
	TInt ret(TMTPTypeFlatBase::FirstReadChunk(aChunk));
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_FIRSTREADCHUNK_EXIT );
	return ret;
	}

EXPORT_C TUint64 TPTPIPTypeStartDataPayload::Size() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_SIZE_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_SIZE_EXIT );
	return KSize;
	}

EXPORT_C TUint TPTPIPTypeStartDataPayload::Type() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_TYPE_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_TYPE_EXIT );
	return EPTPIPTypeStartDataPayload;
	}

EXPORT_C TBool TPTPIPTypeStartDataPayload::CommitRequired() const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_COMMITREQUIRED_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_COMMITREQUIRED_EXIT );
	return ETrue;
	}

/**
 todo: nothing to do here?
 */
EXPORT_C MMTPType* TPTPIPTypeStartDataPayload::CommitChunkL(TPtr8& /*aChunk*/)
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_COMMITCHUNKL_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_COMMITCHUNKL_EXIT );
	return NULL;
	}

EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPTypeStartDataPayload::ElementInfo(TInt aElementId) const
	{
	OstTraceFunctionEntry0( TPTPIPTYPESTARTDATAPAYLOAD_ELEMENTINFO_ENTRY );
	OstTraceFunctionExit0( TPTPIPTYPESTARTDATAPAYLOAD_ELEMENTINFO_EXIT );
	return iElementInfo[aElementId];
	}

