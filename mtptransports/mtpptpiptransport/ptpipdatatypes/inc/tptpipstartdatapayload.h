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
// ptpipstartdatapayload.h
// 
//

/**
 @internalComponent
*/

#ifndef CPTPIPPAYLOADSTARTDATA_H_
#define CPTPIPPAYLOADSTARTDATA_H_

#include "ptpipdatatypes.h"

/**
 Defines the PTPIP dataset for the start data operation. This is used as the 
 payload in the generic container when the start data PTPIP operation is sent.
 */
class TPTPIPTypeStartDataPayload : public TMTPTypeFlatBase
	{
public:

	/**
	 PTPIP operation startdata's dataset element identifiers.
	 */
	enum TElements
		{
		ETransactionId,
		ETotalSize,
		ENumElements
		};

public:
	IMPORT_C TPTPIPTypeStartDataPayload();

public:
	// From TMTPTypeFlatBase
	IMPORT_C void Reset();
	IMPORT_C void SetUint32(TInt aElementId, TUint32 aData);
	IMPORT_C TUint32 Uint32(TInt aElementId) const;
	IMPORT_C void SetUint64(TInt aElementId, TUint64 aData);
	IMPORT_C TUint64 Uint64(TInt aElementId) const;
	IMPORT_C TInt FirstReadChunk(TPtrC8& aChunk) const;
	IMPORT_C TUint64 Size() const;
	IMPORT_C TUint Type() const;
	IMPORT_C TBool CommitRequired() const;
	IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);

private:
	// From TMTPTypeFlatBase
	IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;

private:

	/**
	 The dataset size in bytes.
	 */
	static const TInt KSize = 12;

	/**
	 The dataset element metadata table content.
	 */
	static const TMTPTypeFlatBase::TElementInfo iElementMetaData[];

	/**
	 The dataset element metadata table.
	 */
	const TFixedArray<TElementInfo, ENumElements> iElementInfo;

	/**
	 The data buffer.
	 */
	TBuf8<KSize> iBuffer;
	};

#endif /*CPTPIPPAYLOADSTARTDATA_H_*/
