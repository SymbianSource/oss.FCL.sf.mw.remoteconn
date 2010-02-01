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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpfullenumservicehandler.h
// FullEnum Service Handler

/**
 @file
 @internalComponent
 */

#ifndef __CMTPFULLENUMSERVICEHANDLER_H__
#define __CMTPFULLENUMSERVICEHANDLER_H__

#include <mtp/cmtptypeserviceproplist.h>
#include <mtp/tmtptypeguid.h>
#include <mtp/cmtptypeobjectpropdesc.h>

#include "mtpdebug.h"
#include "mmtpservicehandler.h"

class CMTPTypeServiceInfo;
class CMTPTypeServiceCapabilityList;
class CMTPTypeDeleteServicePropList;
class CMTPTypeServicePropDescList;
class CRepository;
class CMTPFullEnumDataCodeMgr;

/**
High level interface to MTP request processor
@internalComponent
*/
class CMTPFullEnumServiceHandler: public CBase
//                               , public MMTPServiceHandler
	{
public:
	// Basic function
	static IMPORT_C CMTPFullEnumServiceHandler* NewL(MMTPDataProviderFramework& aFramework,
			const CMTPFullEnumDataCodeMgr& aDataCodeMgr,
			CRepository& aRepository,
			TUint aNormalServiceID,
			const TDesC& aKnowledgeObjectSUID,
			const TMTPTypeGuid& aServiceFormatGUID);
	IMPORT_C ~CMTPFullEnumServiceHandler();

	// MMTPServiceHandler
	IMPORT_C TMTPResponseCode GetServiceCapabilityL(TUint16 aServiceFormatCode, CMTPTypeServiceCapabilityList& aServiceCapabilityList) const;
	IMPORT_C TMTPResponseCode GetServicePropDescL(TUint16 aServicePropertyCode, CMTPTypeServicePropDescList& aPropDescList) const;

	// FullEnum specific APIs
	IMPORT_C TMTPResponseCode GetServicePropetyL(TUint16 aPropertyCode, CMTPTypeServicePropList& aPropList) const;
	IMPORT_C TMTPResponseCode SetServicePropetyL(TUint16 aPropEnumIndex, const CMTPTypeServicePropListElement& aElement);
	IMPORT_C TMTPResponseCode DeleteServiceProperty(TUint16 aPropEnumIndex);

private:
	// Property value of FullEnum Service need store in central repository
	enum TMTPServiceStoreKeyNum
		{
		EVersionProps = 0x1,     // Version Props
		EReplicaID = 0x2,        // Set by synchost at first sync, store in db.
		EKnowledgeObjectID = 0x3,// Use object handle which store in framework.
		ESyncFormat = 0x04,      // Format GUID for the obj that is to be sync.
		ELocalOnlyDelete = 0x05, // Boolean, set by synchost.
		EFilterType = 0x06,      // Set by synchost, device just store it.
		ELastSyncProxyID = 0x07, // a GUID indicating the last sync proxy to perform a sync operation
		ESyncObjectReference = 0x08, //Describing whether object references should be included as part of the sync process or not
		};

	CMTPFullEnumServiceHandler(MMTPDataProviderFramework& aFramework,
							   const CMTPFullEnumDataCodeMgr& aDataCodeMgr,
							   CRepository& iRepository,
							   TUint aNormalServiceID,
							   const TDesC& aKnowledgeObjectSUID,
							   const TMTPTypeGuid& aServiceFormatGUID);
	void ConstructL();

	void LoadServicePropValueL();

private:
	MMTPDataProviderFramework& iFramework;
	const CMTPFullEnumDataCodeMgr&   iDataCodeMgr;

	// Store service property
	CRepository&				iRepository;

	// data from Normal Service
	TUint						iNormalServiceID;
	TMTPTypeGuid				iNormalServiceFormatGUID;
	const TDesC&				iKnowledgeObjectSUID;

	//FullEnum Sync Service propterties which are writable
	TMTPTypeGuid            	iReplicateID;
	TMTPTypeGuid            	iLastSyncProxyID;
	TMTPSyncSvcLocalOnlyDelete 	iLocalOnlyDelete;
	TMTPSyncSvcSyncObjectReferences iSyncObjectReference;
	TMTPSyncSvcFilterType      	iFilterType;

	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPFULLENUMSERVICEHANDLER_H__
