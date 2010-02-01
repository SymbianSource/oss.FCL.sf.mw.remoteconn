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
	SetBuffer(iBuffer);
	}

/**
 Resets the dataset.
 */
EXPORT_C void TPTPIPTypeStartDataPayload::Reset()
	{
	TMTPTypeFlatBase::Reset();
	}

/**
 Sets the transaction id.
 */
EXPORT_C void TPTPIPTypeStartDataPayload::SetUint32(TInt aElementId, TUint32 aData)
	{
	__ASSERT_DEBUG((aElementId == ETransactionId), User::Invariant());
	TMTPTypeFlatBase::SetUint32(aElementId, aData);
	}

/**
 Gets the transaction id
 */
EXPORT_C TUint32 TPTPIPTypeStartDataPayload::Uint32(TInt aElementId) const
	{
	__ASSERT_DEBUG((aElementId == ETransactionId), User::Invariant());
	return TMTPTypeFlatBase::Uint32(aElementId);
	}

/**
 Sets the 64 bit size of total data in the data phase. 
 */
EXPORT_C void TPTPIPTypeStartDataPayload::SetUint64(TInt aElementId, TUint64 aData)
	{
	__ASSERT_DEBUG((aElementId == ETotalSize), User::Invariant());
	TMTPTypeFlatBase::SetUint64(aElementId, aData);
	}

/**
 Gets the 64 bit size of the total data in the data phase. 
 */
EXPORT_C TUint64 TPTPIPTypeStartDataPayload::Uint64(TInt aElementId) const
	{
	__ASSERT_DEBUG((aElementId == ETotalSize), User::Invariant());
	return TMTPTypeFlatBase::Uint64(aElementId);
	}

/**
 todo
 */
EXPORT_C TInt TPTPIPTypeStartDataPayload::FirstReadChunk(TPtrC8& aChunk) const
	{
	TInt ret(TMTPTypeFlatBase::FirstReadChunk(aChunk));
	return ret;
	}

EXPORT_C TUint64 TPTPIPTypeStartDataPayload::Size() const
	{
	return KSize;
	}

EXPORT_C TUint TPTPIPTypeStartDataPayload::Type() const
	{
	return EPTPIPTypeStartDataPayload;
	}

EXPORT_C TBool TPTPIPTypeStartDataPayload::CommitRequired() const
	{
	return ETrue;
	}

/**
 todo: nothing to do here?
 */
EXPORT_C MMTPType* TPTPIPTypeStartDataPayload::CommitChunkL(TPtr8& /*aChunk*/)
	{
	return NULL;
	}

EXPORT_C const TMTPTypeFlatBase::TElementInfo& TPTPIPTypeStartDataPayload::ElementInfo(TInt aElementId) const
	{
	return iElementInfo[aElementId];
	}

