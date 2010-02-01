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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpknowledgehandler.h

/**
 @file
 @internalComponent
 */

#ifndef __CMTPKNOWLEDGEHANDLER_H__
#define __CMTPKNOWLEDGEHANDLER_H__

#include <mtp/cmtptypeserviceproplist.h>
#include <mtp/mtpprotocolconstants.h>

#include "mtpdebug.h"
#include "mtpsvcdpconst.h"
#include "mmtpsvcobjecthandler.h"

class CMTPTypeFile;
class CRepository;

/** 
Controls access to the knowledge object.
@internalComponent
*/
class CMTPKnowledgeHandler : public CBase, public MMTPSvcObjectHandler
	{
public:
	// Basic function
	IMPORT_C static CMTPKnowledgeHandler* NewL(MMTPDataProviderFramework& aFramework, TUint16 aFormatCode, CRepository& aReposotry, const TDesC& aKwgSuid);
	IMPORT_C ~CMTPKnowledgeHandler();

	
	IMPORT_C void SetStorageId(TUint32 aStorageId);
	IMPORT_C void GetObjectSuidL(TDes& aSuid) const;

protected:
	// MMTPSvcObjectHandler
	TMTPResponseCode SendObjectInfoL(const CMTPTypeObjectInfo& aObjectInfo, TUint32& aParentHandle, TDes& aSuid);
	TMTPResponseCode GetObjectInfoL(const CMTPObjectMetaData& aObjectMetaData, CMTPTypeObjectInfo& aObjectInfo);

	TMTPResponseCode SendObjectPropListL(TUint64 aObjectSize, const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParentHandle, TDes& aSuid);
	TMTPResponseCode SetObjectPropertyL(const TDesC& aSuid, const CMTPTypeObjectPropListElement& aElement, TMTPOperationCode aOperationCode);
	TMTPResponseCode GetObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, TUint16 aPropertyCode, CMTPTypeObjectPropList& aPropList);
	TMTPResponseCode DeleteObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, const TUint16 aPropertyCode);

	TMTPResponseCode GetBufferForSendObjectL(const CMTPObjectMetaData& aObjectMetaData, MMTPType** aBuffer);
	TMTPResponseCode GetObjectL(const CMTPObjectMetaData& aObjectMetaData, MMTPType** aBuffer);
	TMTPResponseCode DeleteObjectL(const CMTPObjectMetaData& aObjectMetaData);

	TMTPResponseCode GetObjectSizeL(const TDesC& aSuid, TUint64& aObjectSize);
	TMTPResponseCode GetAllObjectPropCodeByGroupL(TUint32 aGroupId, RArray<TUint32>& aPropCodes);
	
	void CommitL();
	void CommitForNewObjectL(TDes& aSuid);
	void RollBack();
	void ReleaseObjectBuffer();	

	enum TCacheStatus
		{
		EOK,
		EDirty,
		EDeleted
		};
	//key of central repository
	enum TMTPKnowledgeStoreKeyNum
		{
		ESize = 0x10001, 
		EDateModified = 0x10002,
		EName = 0x10003,
		ELastAuthorProxyID = 0x10004
		};
	
	TMTPResponseCode SetColumnType128Value(TMTPKnowledgeStoreKeyNum aColumnNum, TMTPTypeUint128& aNewData);

private:
	CMTPKnowledgeHandler(MMTPDataProviderFramework& aFramework,TUint16 aFormatCode, CRepository& aReposotry, const TDesC& aKwgSuid);
	void ConstructL();
	
	/**
	Get the value from the central repository
	@leave One of the system wide error codes, if repository get value fail
	*/
	void LoadKnowledgeObjPropertiesL();
	/**
	Cleanup Item operation for drop all knowledge properties
	*/
	static void DropCacheWrapper(TAny* aObject);
	void DropKnowledgeObjPropertiesCache();
	/**
	Helper for GetObjectInfo request handling
	*/
	void BuildObjectInfoL(CMTPTypeObjectInfo& aObjectInfo) const;
	/**
	Delete knowledge object properties and content
	@leave One of the system wide error codes, if repository set value fail
	*/
	void DeleteAllObjectPropertiesL();
	
private:
	MMTPDataProviderFramework&  iFramework;
	CRepository&                iRepository;
	TUint32                     iStorageID;
	TUint16                     iKnowledgeFormatCode;
	TUint64                     iKnowledgeObjectSize; 
	HBufC*                      iDateModified;
	HBufC*                      iName;
	TMTPTypeUint128             iLastAuthorProxyID;
	TCacheStatus                iCacheStatus;
	TCacheStatus                iCacheStatusFlag;
	TFileName                   iKnowObjFileName;
	TFileName                   iKnowObjSwpFileName;
	// Knowledge object content file
	CMTPTypeFile*               iKnowledgeObj;
	// Knowledge object swap file
	CMTPTypeFile*               iKnowledgeSwpBuffer;
	const TDesC&               iSuid;
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};

#endif // __CMTPKNOWLEDGEHANDLER_H__
