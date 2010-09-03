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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetstorageinfoTraces.h"
#endif



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
	OstTraceFunctionEntry0( CMTPSVCGETSTORAGEINFO_CMTPSVCGETSTORAGEINFO_ENTRY );
	delete iStorageInfo;
	OstTraceFunctionExit0( CMTPSVCGETSTORAGEINFO_CMTPSVCGETSTORAGEINFO_EXIT );
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
	OstTraceFunctionEntry0( CMTPSVCGETSTORAGEINFO_CONSTRUCTL_ENTRY );
	iStorageInfo = CMTPTypeStorageInfo::NewL();
	OstTraceFunctionExit0( CMTPSVCGETSTORAGEINFO_CONSTRUCTL_EXIT );
	}

void CMTPSvcGetStorageInfo::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSTORAGEINFO_SERVICEL_ENTRY );
	iDataProvider.GetStorageInfoL(*iStorageInfo);
	SendDataL(*iStorageInfo);
	OstTraceFunctionExit0( CMTPSVCGETSTORAGEINFO_SERVICEL_EXIT );
	}

TMTPResponseCode CMTPSvcGetStorageInfo::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSTORAGEINFO_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 storageID = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (storageID != iDataProvider.StorageId())
			{
			responseCode = EMTPRespCodeInvalidStorageID;
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSTORAGEINFO_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode);	
	OstTraceFunctionExit0( CMTPSVCGETSTORAGEINFO_CHECKREQUESTL_EXIT );
	return responseCode;
	}
