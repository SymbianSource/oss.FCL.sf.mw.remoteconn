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

#ifndef CPTPIPGENERICCONTAINER_H_
#define CPTPIPGENERICCONTAINER_H_

#include "ptpipdatatypes.h"
#include <mtp/cmtptypecompoundbase.h>

/**
 Defines the PTPIP  device class generic bulk container dataset. This is used to hold all the PTPIP packets except data packets
 by changing the payload. It holds the packet length and packet type as its first two elements, after which it holds a payload, 
 which will change, depending on the kind of PTPIPpacket. For eg this could be a request packet, a cancel packet or a start
 data packet. 

 Eg :  the structure of the class while getting a request can be as follows
 1. packet length	4 bytes
 2. packet size		4 bytes
 3. payload = request packet.
 */
class CPTPIPGenericContainer : public CMTPTypeCompoundBase
	{
public:

	/**
	 PTPIP operation dataset element identifiers.
	 */
	enum TElements
		{
		EPacketLength,
		EPacketType,
		EPayload,
		ENumElements
		};

public:
	IMPORT_C static CPTPIPGenericContainer* NewL();
	IMPORT_C ~CPTPIPGenericContainer();

	IMPORT_C MMTPType* Payload() const;
	IMPORT_C void SetPayloadL(MMTPType* aPayload);

	// from MTP
	IMPORT_C TUint Type() const;

private:
	// from CMTPTypeCompoundBase
	const TElementInfo& ElementInfo(TInt aElementId ) const;

private:
	CPTPIPGenericContainer( );
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
	 The flat data chunk comprising the header elements EPacketLength & EPacketType
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
	// Not owned

	MMTPType* iPayload;
	};

#endif /*CPTPIPGENERICCONTAINER_H_*/
