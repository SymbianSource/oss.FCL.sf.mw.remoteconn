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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetserviceinfoTraces.h"
#endif



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
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEINFO_CMTPSVCGETSERVICEINFO_DES_ENTRY );
	delete iServiceInfo;
	OstTraceFunctionExit0( CMTPSVCGETSERVICEINFO_CMTPSVCGETSERVICEINFO_DES_EXIT );
	}

CMTPSvcGetServiceInfo::CMTPSvcGetServiceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, 
											MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcGetServiceInfo::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEINFO_CONSTRUCTL_ENTRY );
	iServiceInfo = CMTPTypeServiceInfo::NewL();
	OstTraceFunctionExit0( CMTPSVCGETSERVICEINFO_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetServiceInfo::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEINFO_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode  = EMTPRespCodeInvalidServiceID;
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEINFO_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );   
	OstTraceFunctionExit0( CMTPSVCGETSERVICEINFO_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcGetServiceInfo::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEINFO_SERVICEL_ENTRY );
	iResponseCode = (iDataProvider.ServiceHandler())->GetServiceInfoL(*iServiceInfo);
	SendDataL(*iServiceInfo);
	OstTraceFunctionExit0( CMTPSVCGETSERVICEINFO_SERVICEL_EXIT );
	}

TBool CMTPSvcGetServiceInfo::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEINFO_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEINFO_DOHANDLERESPONSEPHASEL, "Exit with response code = 0x%04X", iResponseCode );  	
	OstTraceFunctionExit0( CMTPSVCGETSERVICEINFO_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}
