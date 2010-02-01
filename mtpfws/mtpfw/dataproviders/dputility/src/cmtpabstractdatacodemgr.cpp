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
// mw/remoteconn/mtpdataproviders/mtpcontactdp/src/CMTPFullEnumDataCodeMgr.cpp

#include <mtp/mmtpstoragemgr.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpdatacodegenerator.h>

#include "cmtpabstractdatacodemgr.h"

__FLOG_STMT(_LIT8(KComponent, "FullEnumDataCodeMgr");)

EXPORT_C RMTPServiceFormat::~RMTPServiceFormat()
	{
	iProps.Close();
	}

EXPORT_C TInt TMTPServicePropertyInfo::LinearOrderServicePropOrder(const TMTPServicePropertyInfo& aLhs, const TMTPServicePropertyInfo& aRhs)
	{
	return aLhs.iServicePropCode - aRhs.iServicePropCode;
	}

EXPORT_C TInt TMTPServicePropertyInfo::LinearOrderServicePropOrder(const TUint16* aServicePropCode, const TMTPServicePropertyInfo& aObject)
	{
	return (*aServicePropCode - aObject.iServicePropCode);
	}

EXPORT_C TBool RMTPServiceFormat::FormatRelation(const TUint16* aFormatCode, const RMTPServiceFormat& aObject)
	{
	return *aFormatCode == aObject.iFormatCode;
	}

EXPORT_C CMTPFullEnumDataCodeMgr* CMTPFullEnumDataCodeMgr::NewL(MMTPDataProviderFramework& aFramework)
	{
	CMTPFullEnumDataCodeMgr* self = new(ELeave) CMTPFullEnumDataCodeMgr(aFramework);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


EXPORT_C CMTPFullEnumDataCodeMgr::~CMTPFullEnumDataCodeMgr()
	{
	__FLOG(_L8("~CMTPFullEnumDataCodeMgr - Entry"));
	delete iKnowledgeFormat;
	iServiceProperties.Close();
	__FLOG(_L8("~CMTPFullEnumDataCodeMgr - Exit"));
	__FLOG_CLOSE;
	}

CMTPFullEnumDataCodeMgr::CMTPFullEnumDataCodeMgr(MMTPDataProviderFramework& aFramework) :
		iFramework(aFramework),
		iServiceGUID(MAKE_TUINT64(KMTPFullEnumServiceGUID[0], KMTPFullEnumServiceGUID[1]),
					 MAKE_TUINT64(KMTPFullEnumServiceGUID[2], KMTPFullEnumServiceGUID[3])),
		iPersistentServiceGUID(MAKE_TUINT64(KMTPFullEnumServicePSGUID[0], KMTPFullEnumServicePSGUID[1]),
							   MAKE_TUINT64(KMTPFullEnumServicePSGUID[2], KMTPFullEnumServicePSGUID[3]))
	{
	}

void CMTPFullEnumDataCodeMgr::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	BuildServiceIDL();
	BuildFormatL();
	BuildServicePropertyL();
	__FLOG(_L8("ConstructL - Exit"));
	}

EXPORT_C TUint CMTPFullEnumDataCodeMgr::ServiceID() const
	{
	return iServiceID;
	}

EXPORT_C const TMTPTypeGuid& CMTPFullEnumDataCodeMgr::ServiceGUID() const
	{
	return iServiceGUID;
	}

void CMTPFullEnumDataCodeMgr::BuildServiceIDL()
	{
	__FLOG(_L8("BuildServiceIDL - Entry"));
	//Allocate abstract service ID
	User::LeaveIfError(iFramework.DataCodeGenerator().AllocateServiceID(
						   iPersistentServiceGUID,
						   EMTPServiceTypeAbstract,
						   iServiceID));
	__FLOG(_L8("BuildServiceIDL - Exit"));
	}

void CMTPFullEnumDataCodeMgr::BuildFormatL()
	{
	__FLOG(_L8("BuildFormatL - Entry"));


	iKnowledgeFormat = new(ELeave) RMTPServiceFormat;
	iKnowledgeFormat->iIndex = EMTPFormatTypeFullEnumSyncKnowledge;
	iKnowledgeFormat->iFormatCode = 0;
	const TMTPTypeGuid KMTPKnowledgeFormatGUID(
		MAKE_TUINT64(KMTPFullEnumSyncKnowledgeFormatGUID[0], KMTPFullEnumSyncKnowledgeFormatGUID[1]),
		MAKE_TUINT64(KMTPFullEnumSyncKnowledgeFormatGUID[2], KMTPFullEnumSyncKnowledgeFormatGUID[3]));
	iKnowledgeFormat->iFormatGUID = KMTPKnowledgeFormatGUID;
	iKnowledgeFormat->iBaseFormatCode = KBaseFormatCode;
	iKnowledgeFormat->iFormatName.Set(KNameFullEnumSyncKnowledege());
	iKnowledgeFormat->iMIMEType.Set(KNameFullEnumSyncKnowledegeMIMEType());
	User::LeaveIfError(iFramework.DataCodeGenerator().AllocateServiceFormatCode(
						   iPersistentServiceGUID,
						   iKnowledgeFormat->iFormatGUID,
						   iKnowledgeFormat->iFormatCode));
	TUint propertyCount = sizeof(KMTPFullEnumSyncKnowledgeObjectProperties) / sizeof(KMTPFullEnumSyncKnowledgeObjectProperties[0]);
	for (TUint j = 0; j < propertyCount; j++)
		{
		iKnowledgeFormat->iProps.AppendL(KMTPFullEnumSyncKnowledgeObjectProperties[j]);
		}

	__FLOG(_L8("BuildFormatL - Exit"));
	}

void CMTPFullEnumDataCodeMgr::BuildServicePropertyL()
	{
	__FLOG(_L8("BuildServicePropertyL - Entry"));

	const TMTPTypeGuid KMTPFullEnumSyncServiceNamespace(
		MAKE_TUINT64(KMTPFullEnumSyncServiceNSGUID[0], KMTPFullEnumSyncServiceNSGUID[1]),
		MAKE_TUINT64(KMTPFullEnumSyncServiceNSGUID[2], KMTPFullEnumSyncServiceNSGUID[3]));

	const TMTPTypeGuid KMTPSyncSvcServiceNamespace(
		MAKE_TUINT64(KMTPSyncSvcServiceNSGUID[0], KMTPSyncSvcServiceNSGUID[1]),
		MAKE_TUINT64(KMTPSyncSvcServiceNSGUID[2], KMTPSyncSvcServiceNSGUID[3]));

	// Filtertype only need be allocate once by framework, so put it into abstract service.
	const TMTPServicePropertyInfo KMTPFullEnumSyncServiceProperties[] =
		{
			{EMTPServicePropertyVersionProps,         0, KMTPFullEnumSyncServiceNamespace, 3, KNameFullEnumVersionProps()},
		{EMTPServicePropertyReplicaID,            0, KMTPFullEnumSyncServiceNamespace, 4, KNameFullEnumReplicaID()},
		{EMTPServicePropertyKnowledgeObjectID,    0, KMTPFullEnumSyncServiceNamespace, 7, KNameFullEnumKnowledgeObjectID()},
		{EMTPServicePropertyLastSyncProxyID,      0, KMTPFullEnumSyncServiceNamespace, 8, KNameFullEnumLastSyncProxyID()},
		{EMTPServicePropertyProviderVersion,      0, KMTPFullEnumSyncServiceNamespace, 9, KNameFullEnumProviderVersion()},
		{EMTPServicePropertySyncFormat,           0, KMTPSyncSvcServiceNamespace,      2, KNameSyncSvcSyncFormat()},
		{EMTPServicePropertyLocalOnlyDelete,      0, KMTPSyncSvcServiceNamespace,      3, KNameSyncSvcLocalOnlyDelete()},
		{EMTPServicePropertyFilterType,           0, KMTPSyncSvcServiceNamespace,      4, KNameSyncSvcFilterType()},
		{EMTPServicePropertySyncObjectReferences, 0, KMTPSyncSvcServiceNamespace,      5, KNameSyncSvcSyncObjectReferences()}
		};

	TUint propCount = sizeof(KMTPFullEnumSyncServiceProperties) / sizeof(KMTPFullEnumSyncServiceProperties[0]);

	for (TUint i = 0; i < propCount; i++)
		{
		TMTPServicePropertyInfo servicePropertyInfo = KMTPFullEnumSyncServiceProperties[i];
		User::LeaveIfError(iFramework.DataCodeGenerator().AllocateServicePropertyCode(iPersistentServiceGUID,
						   servicePropertyInfo.iServicePropPKeyNamespace, servicePropertyInfo.iServicePropPKeyID, servicePropertyInfo.iServicePropCode));
		iServiceProperties.InsertInOrder(servicePropertyInfo, TMTPServicePropertyInfo::LinearOrderServicePropOrder);
		}

	__FLOG(_L8("BuildServicePropertyL - Exit"));
	}

EXPORT_C void CMTPFullEnumDataCodeMgr::GetSevicePropCodesL(RArray<TUint32>& aArray) const
	{
	__FLOG(_L8("GetSevicePropCodesL - Entry"));
	TInt count = iServiceProperties.Count();
	for (TInt i = 0; i < count; i++)
		{
		aArray.AppendL(iServiceProperties[i].iServicePropCode);
		}
	__FLOG(_L8("GetSevicePropCodesL - Exit"));
	}

EXPORT_C const RMTPServiceFormat& CMTPFullEnumDataCodeMgr::KnowledgeFormat() const
	{
	// only Knowledge Format supported in FullEnum Sync Service
	__ASSERT_DEBUG((iKnowledgeFormat != NULL), User::Invariant());
	return *iKnowledgeFormat;
	}

/**
The property code must be valid to call this func
*/
EXPORT_C const TMTPServicePropertyInfo* CMTPFullEnumDataCodeMgr::ServicePropertyInfo(TUint16 aPropCode) const
	{
	__FLOG(_L8("ServicePropertyInfo - Entry"));
	const TMTPServicePropertyInfo* pPropInfo = NULL;
	TInt index = iServiceProperties.FindInOrder(aPropCode, TMTPServicePropertyInfo::LinearOrderServicePropOrder);
	if (KErrNotFound != index)
		{
		pPropInfo = &(iServiceProperties[index]);
		}
	__FLOG(_L8("ServicePropertyInfo - Exit"));
	return pPropInfo;
	}


