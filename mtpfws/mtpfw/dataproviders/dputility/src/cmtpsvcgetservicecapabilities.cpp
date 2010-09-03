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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetservicecapabilities.cpp

#include <mtp/cmtptypeservicecapabilitylist.h>

#include "cmtpsvcgetservicecapabilities.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetservicecapabilitiesTraces.h"
#endif


// Class constants.

EXPORT_C MMTPRequestProcessor* CMTPSvcGetServiceCapabilities::NewL(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetServiceCapabilities* self = new (ELeave) CMTPSvcGetServiceCapabilities(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetServiceCapabilities::~CMTPSvcGetServiceCapabilities()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICECAPABILITIES_CMTPSVCGETSERVICECAPABILITIES_ENTRY );
	delete iServiceCapabilityList;
	OstTraceFunctionExit0( CMTPSVCGETSERVICECAPABILITIES_CMTPSVCGETSERVICECAPABILITIES_EXIT );
	}

CMTPSvcGetServiceCapabilities::CMTPSvcGetServiceCapabilities(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider)
:CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iResponseCode(EMTPRespCodeOK)
	{	
	}

void CMTPSvcGetServiceCapabilities::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICECAPABILITIES_CONSTRUCTL_ENTRY );
	iServiceCapabilityList = CMTPTypeServiceCapabilityList::NewL();
	OstTraceFunctionExit0( CMTPSVCGETSERVICECAPABILITIES_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetServiceCapabilities::CheckRequestL()
{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICECAPABILITIES_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			}
		
		if (EMTPRespCodeOK == responseCode)
			{
			TUint16 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2); 
			if (KMTPFormatsAll != formatCode && !iDataProvider.IsValidFormatCodeL(formatCode))
				{
				responseCode = EMTPRespCodeInvalidObjectFormatCode;
				}
			}
		}
    OstTrace1(TRACE_NORMAL, CMTPSVCGETSERVICECAPABILITIES_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICECAPABILITIES_CHECKREQUESTL_EXIT );
	return responseCode;
}

void CMTPSvcGetServiceCapabilities::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICECAPABILITIES_SERVICEL_ENTRY );
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	if (KMTPFormatsAll == formatCode)
		{
		RArray<TUint32> formatCodes;
		CleanupClosePushL(formatCodes);
		(iDataProvider.ServiceHandler())->GetAllServiceFormatCodeL(formatCodes); 
		TInt count = formatCodes.Count();
		for (TInt i = 0; i < count && iResponseCode == EMTPRespCodeOK; i++)
			{
			iResponseCode = (iDataProvider.ServiceHandler())->GetServiceCapabilityL(formatCodes[i], *iServiceCapabilityList);
			}
		CleanupStack::PopAndDestroy(&formatCodes);
		}
	else
		{
		iResponseCode = (iDataProvider.ServiceHandler())->GetServiceCapabilityL(formatCode, *iServiceCapabilityList);
		}
	SendDataL(*iServiceCapabilityList);
	OstTraceFunctionExit0( CMTPSVCGETSERVICECAPABILITIES_SERVICEL_EXIT );
	}

TBool CMTPSvcGetServiceCapabilities::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICECAPABILITIES_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
    OstTrace1(TRACE_NORMAL, CMTPSVCGETSERVICECAPABILITIES_DOHANDLERESPONSEPHASEL, "Exit with response code = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICECAPABILITIES_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}
