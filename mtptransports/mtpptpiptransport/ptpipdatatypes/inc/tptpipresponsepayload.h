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
// tptpippayloadresponse.h
// 
//

/**
 @internalComponent
*/

#ifndef CPTPIPPAYLOADRESPONSE_H_
#define CPTPIPPAYLOADRESPONSE_H_

#include "ptpipdatatypes.h"

/**
 Defines the PTPIP dataset for the ptpip response operation. This is used 
 as the payload in the generic container when the PTPIP response operation is sent. 
 */
class TPTPIPTypeResponsePayload : public TMTPTypeFlatBase
	{
public:

	/**
	 PTPIP operation request dataset element identifiers.
	 */
	enum TElements
		{
		EResponseCode,
		ETransactionId,
		EParameter1,
		EParameter2,
		EParameter3,
		EParameter4,
		EParameter5,
		ENumElements
		};

public:
	IMPORT_C TPTPIPTypeResponsePayload();
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
	static const TInt KSize = 26;

	/**
	 The size of the 2 header elements - response code and transaction id
	 */
	static const TInt KHeaderElementsSize = 6;

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
	 The number of parameters in the block which are set.
	 */
	//TUint iNumParameters;
	TUint iMaxSetParam;

	};

#endif /*CPTPIPPAYLOADRESPONSE_H_*/
