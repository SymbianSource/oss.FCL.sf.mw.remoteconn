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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetobject.cpp

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpprotocolconstants.h>

#include "cmtpsvcgetobject.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcGetObject");)

EXPORT_C MMTPRequestProcessor* CMTPSvcGetObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetObject* self = new (ELeave) CMTPSvcGetObject(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetObject::~CMTPSvcGetObject()
	{
	__FLOG(_L8("~CMTPSvcGetObject - Entry"));
	if (iReceivedObjectMetaData && iObjectHandler)
		{
		iObjectHandler->ReleaseObjectBuffer();
		}
	delete iReceivedObjectMetaData;

	__FLOG(_L8("~CMTPSvcGetObject - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetObject::CMTPSvcGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) : 
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcGetObject::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	__FLOG(_L8("ConstructL - Exit"));
	}

TMTPResponseCode CMTPSvcGetObject::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		
		MMTPObjectMgr& objectmgr(iFramework.ObjectMgr());
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		if (objectmgr.ObjectL(objectHandle, *iReceivedObjectMetaData))
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

void CMTPSvcGetObject::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	iObjectHandler->GetObjectL(*iReceivedObjectMetaData, &iBuffer);
	SendDataL(*iBuffer);
	__FLOG(_L8("ServiceL - Exit"));
	}
