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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetobjectinfo.cpp

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtpobjectmetadata.h>


#include "cmtpsvcgetobjectinfo.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetobjectinfoTraces.h"
#endif


EXPORT_C MMTPRequestProcessor* CMTPSvcGetObjectInfo::NewL(MMTPDataProviderFramework& aFramework, 
														MMTPConnection& aConnection, 
														MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetObjectInfo* self = new (ELeave) CMTPSvcGetObjectInfo(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetObjectInfo::~CMTPSvcGetObjectInfo()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTINFO_CMTPSVCGETOBJECTINFO_ENTRY );
	delete iReceivedObjectMetaData;
	delete iObjectInfo;
	OstTraceFunctionExit0( CMTPSVCGETOBJECTINFO_CMTPSVCGETOBJECTINFO_EXIT );
	}

CMTPSvcGetObjectInfo::CMTPSvcGetObjectInfo(MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	: CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{
	}
	
void CMTPSvcGetObjectInfo::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTINFO_CONSTRUCTL_ENTRY );
	
    iObjectInfo = CMTPTypeObjectInfo::NewL();

	OstTraceFunctionExit0( CMTPSVCGETOBJECTINFO_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetObjectInfo::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTINFO_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		MMTPObjectMgr& objectMgr(iFramework.ObjectMgr());
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		if (objectMgr.ObjectL(objectHandle, *iReceivedObjectMetaData))
			{
			//Check whether the owner of this object is correct data provider
			if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId())
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				OstTrace0( TRACE_WARNING, DUP1_CMTPSVCGETOBJECTINFO_CHECKREQUESTL, "CheckRequestL - DataProviderId dismatch" );
				}
			else
				{
				// Check format and set handler
				TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				iObjectHandler = iDataProvider.ObjectHandler(formatCode);
				if (!iObjectHandler)
					{
					responseCode = EMTPRespCodeInvalidObjectFormatCode;
					}
				}
			}
		else
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTINFO_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECTINFO_CHECKREQUESTL_EXIT );
	
	return responseCode;
	}
	
void CMTPSvcGetObjectInfo::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTINFO_SERVICEL_ENTRY );
	__ASSERT_DEBUG(iObjectHandler, User::Invariant());
	TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	iObjectHandler->GetObjectInfoL(*iReceivedObjectMetaData, *iObjectInfo);
	SendDataL(*iObjectInfo);
	OstTraceFunctionExit0( CMTPSVCGETOBJECTINFO_SERVICEL_EXIT );
	}
