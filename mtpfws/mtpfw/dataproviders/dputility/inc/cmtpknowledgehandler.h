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

#include "mtpsvcdpconst.h"
#include "mmtpsvcobjecthandler.h"

class CMTPTypeFile;
class CRepository;

NONSHARABLE_CLASS(CKnowledgeObject) : public CBase
	{
public:
	static CKnowledgeObject* NewL(CRepository& aRepository);
	~CKnowledgeObject();
	
	/**
	Get the value from the central repository
	@leave One of the system wide error codes, if repository get value fail
	*/
	void LoadL();
	/**
	Set the cached value to the central repository
	@leave One of the system wide error codes, if repository set value fail
	*/
	void CommitL();
	/** 
	Clear the cached knowledge object property values 
	*/
	void Clear();
	/**
	Update cached DateModified to current Device time
	*/
	void RefreshDateModifed();
	/**
	Check if the cached object property values are aligned with repository
	*/
	TBool IsDirty() {return iDirty;};
	
	//Get method for all prop
	TUint64 Size(){return iKnowledgeObjectSize;};
	TDesC& DateModified(){return iDateModified;};
	TDesC& Name(){return iName;};
	const TMTPTypeUint128& LastAuthorProxyID() {return iLastAuthorProxyID;};
	
	//Set method for all prop
	void SetSize(TUint64 aSize);
	void SetDateModified(const TDesC& aDateModified);
	void SetName(const TDesC& aName);
	void SetLastAuthorProxyID(TUint64 aHigh, TUint64 aLow);
	
private:
	//key of central repository
	enum TMTPKnowledgeStoreKeyNum
		{
		ESize 				= 0x10001, 
		EDateModified 		= 0x10002,
		EName 				= 0x10003,
		ELastAuthorProxyID 	= 0x10004
		};

	// Bit flag of the property
	enum TMTPKnowledgeBitFlag
		{
		EBitFlagNone 				= 0x00,
		EBitFlagSize 				= 0x01, 
		EBitFlagDateModified 		= 0x02,
		EBitFlagName 				= 0x04,
		EBitFlagLastAuthorProxyID 	= 0x08,
		EBitFlagAll 				= 0x0F
		};

private:
	CKnowledgeObject(CRepository& aRepository);
	
	void ConstructL();
	
	TMTPResponseCode SetColumnType128Value(TMTPKnowledgeStoreKeyNum aColumnNum, TMTPTypeUint128& aNewData);
	
	CRepository&     	iRepository;

	// The bit wise value of TMTPKnowledgeBitFlag 
	// to state which property is not aligned with repository
	TUint				iDirty;
	
	TUint64          	iKnowledgeObjectSize; 
	RBuf             	iDateModified;
	RBuf             	iName;
	TMTPTypeUint128  	iLastAuthorProxyID;
	};

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

	
private:
	CMTPKnowledgeHandler(MMTPDataProviderFramework& aFramework,TUint16 aFormatCode, CRepository& aReposotry, const TDesC& aKwgSuid);
	void ConstructL();
	
	/**
	Helper for GetObjectInfo request handling
	*/
	void BuildObjectInfoL(CMTPTypeObjectInfo& aObjectInfo) const;
	
private:
	MMTPDataProviderFramework&  iFramework;
	CRepository&                iRepository;
	TUint32                     iStorageID;
	TUint16                     iKnowledgeFormatCode;
	
	CKnowledgeObject*			iCachedKnowledgeObject;
	
	TFileName                   iKnowObjFileName;
	TFileName                   iKnowObjSwpFileName;
	// Knowledge object content file
	CMTPTypeFile*               iKnowledgeObj;
	// Knowledge object swap file
	CMTPTypeFile*               iKnowledgeSwpBuffer;
	const TDesC&               iSuid;
	};

#endif // __CMTPKNOWLEDGEHANDLER_H__
