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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetobjectTraces.h"
#endif


// Class constants.

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
	OstTraceFunctionEntry0( CMTPSVCGETOBJECT_CMTPSVCGETOBJECT_ENTRY );
	if (iReceivedObjectMetaData && iObjectHandler)
		{
		iObjectHandler->ReleaseObjectBuffer();
		}
	delete iReceivedObjectMetaData;

	OstTraceFunctionExit0( CMTPSVCGETOBJECT_CMTPSVCGETOBJECT_EXIT );
	}

CMTPSvcGetObject::CMTPSvcGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) : 
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcGetObject::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECT_CONSTRUCTL_ENTRY );
	OstTraceFunctionExit0( CMTPSVCGETOBJECT_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetObject::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECT_CHECKREQUESTL_ENTRY );
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
				OstTrace0( TRACE_WARNING, DUP1_CMTPSVCGETOBJECT_CHECKREQUESTL, "CheckRequestL - DataProviderId dismatch" );
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
	
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECT_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECT_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcGetObject::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECT_SERVICEL_ENTRY );
	iObjectHandler->GetObjectL(*iReceivedObjectMetaData, &iBuffer);
	SendDataL(*iBuffer);
	OstTraceFunctionExit0( CMTPSVCGETOBJECT_SERVICEL_EXIT );
	}
