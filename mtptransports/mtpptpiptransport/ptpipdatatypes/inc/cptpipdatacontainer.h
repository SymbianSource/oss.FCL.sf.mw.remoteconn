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

#ifndef CPTPIPDATACONTINER_H_
#define CPTPIPDATACONTINER_H_

#include "ptpipdatatypes.h"

/**
 Defines the MTP PTPIP device class generic container dataset. 
 This will contain a data payload which will have the actual mtp data. 

 The structure would be as follows:
 1. packet length	4 bytes
 2. packet size		4 bytes
 3. payload = mtp data.

 */
class CPTPIPDataContainer : public CMTPTypeCompoundBase
	{
public:

	/**
	 PTPIP operation request dataset element identifiers.
	 */
	enum TElements
		{
		EPacketLength,
		EPacketType,
		ETransactionId,
		EPayload,
		ENumElements
		};
		
public:
	IMPORT_C static CPTPIPDataContainer* NewL();
	IMPORT_C ~CPTPIPDataContainer();

	IMPORT_C MMTPType* Payload() const;
	IMPORT_C void SetPayloadL(MMTPType* aPayload);
	IMPORT_C TInt FirstWriteChunk(TPtr8& aChunk);	
	IMPORT_C MMTPType* CommitChunkL(TPtr8& aChunk);

public:
	// From MMTPType	
	IMPORT_C TUint Type() const;

private:
	// From CMTPTypeCompoundBase

	const TElementInfo& ElementInfo(TInt aElementId ) const;	

private:
	CPTPIPDataContainer( );
	void ConstructL( );

private:

	/**
	 Data chunk identifiers.
	 */
	enum TChunkIds
		{
		/**
		 The flat data chunk ID.
		 */
		EIdFlatChunk,

		/**
		 The EMTPExtensions element data chunk ID.
		 */
		EIdPayloadChunk,

		/**
		 The number of data chunks.
		 */
		EIdNumChunks
		};

	/**
	 The flat data chunk comprising the header elements EPacketLength, EPacketType and the transaction Id
	 */
	RMTPTypeCompoundFlatChunk iChunkHeader;

	/**
	 The dataset element metadata table content.
	 */
	static const CMTPTypeCompoundBase::TElementInfo iElementMetaData[];

	/**
	 The dataset element metadata table.
	 */
	const TFixedArray<TElementInfo, ENumElements> iElementInfo;

	/**
	 The size in bytes of the header data chunk.
	 */
	static const TUint KFlatChunkSize;

private:
	MMTPType* iPayload;  // Not owned
	
	// If the Initiator sends the data in this form,
	// | ptp header| content part1 | ptp header | content part2 |
	// But,mtp framework gives us the payload as | content part1 | content part2 |
	// thus we need to ignore the ptp header that comes second time onwards.
	TInt iIsNextHeader;
	
	

	};

#endif /*CPTPIPCONTINERDATA_H_*/
