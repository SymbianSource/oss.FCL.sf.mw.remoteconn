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

__FLOG_STMT(_LIT8(KComponent,"SvcGetObjInfo");)

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
	__FLOG(_L8("~CMTPSvcGetObjectInfo - Destructed"));
	delete iReceivedObjectMetaData;
	delete iObjectInfo;
	__FLOG_CLOSE;
	}

CMTPSvcGetObjectInfo::CMTPSvcGetObjectInfo(MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	: CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{
	}
	
void CMTPSvcGetObjectInfo::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL -  - Entry"));
	
    iObjectInfo = CMTPTypeObjectInfo::NewL();
    
    __FLOG(_L8("ConstructL -  - Exit"));
	}

TMTPResponseCode CMTPSvcGetObjectInfo::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
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
				__FLOG(_L8("CheckRequestL - DataProviderId dismatch"));
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
	__FLOG_VA((_L8("CheckRequestL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}
	
void CMTPSvcGetObjectInfo::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	__ASSERT_DEBUG(iObjectHandler, User::Invariant());
	TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	iObjectHandler->GetObjectInfoL(*iReceivedObjectMetaData, *iObjectInfo);
	SendDataL(*iObjectInfo);
	__FLOG(_L8("ServiceL - Exit"));
	}
