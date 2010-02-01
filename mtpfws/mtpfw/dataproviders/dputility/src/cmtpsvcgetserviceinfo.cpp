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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetserviceinfo.cpp

#include <mtp/cmtptypeserviceinfo.h>

#include "cmtpsvcgetserviceinfo.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent, "SvcGetSvcInfo");)

EXPORT_C MMTPRequestProcessor* CMTPSvcGetServiceInfo::NewL(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetServiceInfo* self = new (ELeave) CMTPSvcGetServiceInfo(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetServiceInfo::~CMTPSvcGetServiceInfo()
	{
	__FLOG(_L8("~CMTPSvcGetServiceInfo - Entry"));
	delete iServiceInfo;
	__FLOG(_L8("~CMTPSvcGetServiceInfo - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetServiceInfo::CMTPSvcGetServiceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, 
											MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcGetServiceInfo::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry")); 
	iServiceInfo = CMTPTypeServiceInfo::NewL();
	__FLOG(_L8("ConstructL - Exit")); 
	}

TMTPResponseCode CMTPSvcGetServiceInfo::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode  = EMTPRespCodeInvalidServiceID;
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcGetServiceInfo::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	iResponseCode = (iDataProvider.ServiceHandler())->GetServiceInfoL(*iServiceInfo);
	SendDataL(*iServiceInfo);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcGetServiceInfo::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with Response Code: 0x%x"), iResponseCode));
	return EFalse;
	}
