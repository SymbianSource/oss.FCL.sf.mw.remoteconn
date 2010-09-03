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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpabstractdatacodemgrTraces.h"
#endif


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
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_CMTPFULLENUMDATACODEMGR_DES_ENTRY );
	delete iKnowledgeFormat;
	iServiceProperties.Close();
	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_CMTPFULLENUMDATACODEMGR_DES_EXIT );
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
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_CONSTRUCTL_ENTRY );
	BuildServiceIDL();
	BuildFormatL();
	BuildServicePropertyL();
	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_CONSTRUCTL_EXIT );
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
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_BUILDSERVICEIDL_ENTRY );
	//Allocate abstract service ID
	LEAVEIFERROR(iFramework.DataCodeGenerator().AllocateServiceID(
						   iPersistentServiceGUID,
						   EMTPServiceTypeAbstract,
						   iServiceID),
                           OstTrace0( TRACE_ERROR, CMTPFULLENUMDATACODEMGR_BUILDSERVICEIDL, 
                                   "allocate abstract service ID faled!" ));
						   
	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_BUILDSERVICEIDL_EXIT );
	}

void CMTPFullEnumDataCodeMgr::BuildFormatL()
	{
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_BUILDFORMATL_ENTRY );


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
	LEAVEIFERROR(iFramework.DataCodeGenerator().AllocateServiceFormatCode(
						   iPersistentServiceGUID,
						   iKnowledgeFormat->iFormatGUID,
						   iKnowledgeFormat->iFormatCode),
						   OstTrace0( TRACE_ERROR, CMTPFULLENUMDATACODEMGR_BUILDFORMATL, 
						           "allocate service format code failed!" ));
						   
	TUint propertyCount = sizeof(KMTPFullEnumSyncKnowledgeObjectProperties) / sizeof(KMTPFullEnumSyncKnowledgeObjectProperties[0]);
	for (TUint j = 0; j < propertyCount; j++)
		{
		iKnowledgeFormat->iProps.AppendL(KMTPFullEnumSyncKnowledgeObjectProperties[j]);
		}

	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_BUILDFORMATL_EXIT );
	}

void CMTPFullEnumDataCodeMgr::BuildServicePropertyL()
	{
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_BUILDSERVICEPROPERTYL_ENTRY );

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
		LEAVEIFERROR(iFramework.DataCodeGenerator().AllocateServicePropertyCode(iPersistentServiceGUID,
						   servicePropertyInfo.iServicePropPKeyNamespace, servicePropertyInfo.iServicePropPKeyID, servicePropertyInfo.iServicePropCode),
						   OstTrace0( TRACE_ERROR, CMTPFULLENUMDATACODEMGR_BUILDSERVICEPROPERTYL, "allocate service property code failed!" ));
		iServiceProperties.InsertInOrder(servicePropertyInfo, TMTPServicePropertyInfo::LinearOrderServicePropOrder);
		}

	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_BUILDSERVICEPROPERTYL_EXIT );
	}

EXPORT_C void CMTPFullEnumDataCodeMgr::GetSevicePropCodesL(RArray<TUint32>& aArray) const
	{
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_GETSEVICEPROPCODESL_ENTRY );
	TInt count = iServiceProperties.Count();
	for (TInt i = 0; i < count; i++)
		{
		aArray.AppendL(iServiceProperties[i].iServicePropCode);
		}
	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_GETSEVICEPROPCODESL_EXIT );
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
	OstTraceFunctionEntry0( CMTPFULLENUMDATACODEMGR_SERVICEPROPERTYINFO_ENTRY );
	const TMTPServicePropertyInfo* pPropInfo = NULL;
	TInt index = iServiceProperties.FindInOrder(aPropCode, TMTPServicePropertyInfo::LinearOrderServicePropOrder);
	if (KErrNotFound != index)
		{
		pPropInfo = &(iServiceProperties[index]);
		}
	OstTraceFunctionExit0( CMTPFULLENUMDATACODEMGR_SERVICEPROPERTYINFO_EXIT );
	return pPropInfo;
	}


