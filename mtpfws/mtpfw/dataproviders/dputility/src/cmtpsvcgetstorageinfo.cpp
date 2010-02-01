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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetstorageinfo.cpp

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypestorageinfo.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpstoragemgr.h>

#include "cmtpsvcgetstorageinfo.h"
#include "mmtpservicedataprovider.h"

__FLOG_STMT(_LIT8(KComponent,"SvcGetStgInfo");)

EXPORT_C MMTPRequestProcessor* CMTPSvcGetStorageInfo::NewL(
											MMTPDataProviderFramework& aFramework, 
											MMTPConnection& aConnection, 
											MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetStorageInfo* self = new (ELeave) CMTPSvcGetStorageInfo(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetStorageInfo::~CMTPSvcGetStorageInfo()
	{
	__FLOG(_L8("~CMTPSvcGetStorageInfo - Entry"));
	delete iStorageInfo;
	__FLOG(_L8("~CMTPSvcGetStorageInfo - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetStorageInfo::CMTPSvcGetStorageInfo(
									MMTPDataProviderFramework& aFramework, 
									MMTPConnection& aConnection, 
									MMTPServiceDataProvider& aDataProvider)
	: CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{
	}

/**
GetStorageInfo request handler
Build storage info data set and send the data to the initiator
*/		
void CMTPSvcGetStorageInfo::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iStorageInfo = CMTPTypeStorageInfo::NewL();
	__FLOG(_L8("ConstructL - Exit"));
	}

void CMTPSvcGetStorageInfo::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	iDataProvider.GetStorageInfoL(*iStorageInfo);
	SendDataL(*iStorageInfo);
	__FLOG(_L8("ServiceL - Exit"));
	}

TMTPResponseCode CMTPSvcGetStorageInfo::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 storageID = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (storageID != iDataProvider.StorageId())
			{
			responseCode = EMTPRespCodeInvalidStorageID;
			}
		}
	__FLOG_VA((_L8("CheckRequestL Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}
