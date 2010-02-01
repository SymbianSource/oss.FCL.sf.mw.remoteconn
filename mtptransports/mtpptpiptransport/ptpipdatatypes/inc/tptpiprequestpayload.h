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

#ifndef CPTPIPPAYLOADREQUEST_H_
#define CPTPIPPAYLOADREQUEST_H_

#include "ptpipdatatypes.h"

/**
 Defines the PTPIP dataset for the ptpip request operation. This is used 
 as the payload in the generic container when the PTPIP request operation is sent. 
 */
class TPTPIPTypeRequestPayload : public TMTPTypeFlatBase
	{
public:

	/**
	 PTPIP operation request dataset element identifiers.
	 */
	enum TElements
		{
		EDataPhase,
		EOpCode,
		ETransactionId,
		EParameter1,
		EParameter2,
		EParameter3,
		EParameter4,
		EParameter5,
		ENumElements
		};

public:
	IMPORT_C TPTPIPTypeRequestPayload();
	IMPORT_C TUint NumParameters() const;
	IMPORT_C void CopyIn(const TMTPTypeFlatBase& aFrom, TUint aParamStartOffset, TUint aParamEndOffSet, TBool aIsNullParamValid, TUint aNumOfNullParam);
	IMPORT_C void CopyOut(TMTPTypeFlatBase& aTo, TUint aParamStartOffset, TUint aParamEndOffset);

public:
	// From TMTPTypeFlatBase
	IMPORT_C void Reset();
	IMPORT_C void SetUint16(TInt aElementId, TUint16 aData);
	IMPORT_C TUint16 Uint16(TInt aElementId) const;
	IMPORT_C void SetUint32(TInt aElementId, TUint32 aData);
	IMPORT_C TUint32 Uint32(TInt aElementId) const;
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
	static const TInt KSize = 30;

	/**
	 The size of the 3 header elements - data phase, op code and transaction id
	 */
	static const TInt KHeaderElementsSize = 10;

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

	/**
	 The number of parameters(EParameter1 to EParameter5)  in the block which are set.
	 It will be between 1 to 5 and 
	 will never include the count of the first 3 header elements- data phase, op code and tran id.
	 */
//	TUint iNumParameters;
	TUint iMaxSetParam;
	};

#endif /*CPTPIPPAYLOADREQUEST_H_*/
