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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetservicepropdesc.cpp

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/cmtptypeservicepropdesclist.h>

#include "cmtpsvcgetservicepropdesc.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetservicepropdescTraces.h"
#endif


EXPORT_C MMTPRequestProcessor* CMTPSvcGetServicePropDesc::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetServicePropDesc* self = new (ELeave) CMTPSvcGetServicePropDesc(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetServicePropDesc::~CMTPSvcGetServicePropDesc()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPDESC_CMTPSVCGETSERVICEPROPDESC_ENTRY );
	delete iPropDescList;
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPDESC_CMTPSVCGETSERVICEPROPDESC_EXIT );
	}

CMTPSvcGetServicePropDesc::CMTPSvcGetServicePropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider),
	iResponseCode(EMTPRespCodeOK)
	{
	}

void CMTPSvcGetServicePropDesc::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPDESC_CONSTRUCTL_ENTRY );
	iPropDescList = CMTPTypeServicePropDescList::NewL();
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPDESC_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetServicePropDesc::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPDESC_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceID = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		TUint32 propCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
		
		if ((iDataProvider.ServiceID() != serviceID))
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			OstTrace0( TRACE_WARNING, DUP1_CMTPSVCGETSERVICEPROPDESC_CHECKREQUESTL, "Service Id Parameter don't be supported" );
			}
		
		if (EMTPRespCodeOK == responseCode)
			{
			if ((KMTPNotSpecified32 != propCode) && 
				!(iDataProvider.IsValidServicePropCodeL(propCode)))
				{
				responseCode = EMTPRespCodeInvalidServicePropCode;
				OstTrace0( TRACE_WARNING, DUP2_CMTPSVCGETSERVICEPROPDESC_CHECKREQUESTL, "Service Object PropCode Parameter don't be supported" );
				}
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEPROPDESC_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPDESC_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcGetServicePropDesc::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPDESC_SERVICEL_ENTRY );
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	if (KMTPNotSpecified32 != propCode)
		{
		iResponseCode = (iDataProvider.ServiceHandler())->GetServicePropDescL(propCode, *iPropDescList);
		}
	else
		{
		RArray<TUint32> propCodeArray;
		CleanupClosePushL(propCodeArray);
		(iDataProvider.ServiceHandler())->GetAllSevicePropCodesL(propCodeArray); 
		TInt count = propCodeArray.Count();
		for (TInt i = 0; i < count && iResponseCode == EMTPRespCodeOK; i++)
			{
			iResponseCode = (iDataProvider.ServiceHandler())->GetServicePropDescL(propCodeArray[i], *iPropDescList);
			}
		CleanupStack::PopAndDestroy(&propCodeArray);
		}
	SendDataL(*iPropDescList);
    OstTraceExt2( TRACE_NORMAL, CMTPSVCGETSERVICEPROPDESC_SERVICEL, 
            "Exit with Response Code: 0x%x, Service Property Count: %u", iResponseCode, iPropDescList->NumberOfElements());
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPDESC_SERVICEL_EXIT );
	}

TBool CMTPSvcGetServicePropDesc::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPDESC_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEPROPDESC_DOHANDLERESPONSEPHASEL, "Exit with Response Code: 0x%x", iResponseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPDESC_DOHANDLERESPONSEPHASEL_EXIT );
	
	return EFalse;
	}

