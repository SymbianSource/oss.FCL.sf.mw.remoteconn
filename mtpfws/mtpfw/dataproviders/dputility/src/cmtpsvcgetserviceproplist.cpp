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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetserviceproplist.cpp

#include "cmtpsvcgetserviceproplist.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetserviceproplistTraces.h"
#endif


const TUint16 KMTPServicePropsAll(0x0000);

EXPORT_C MMTPRequestProcessor* CMTPSvcGetServicePropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetServicePropList* self = new (ELeave) CMTPSvcGetServicePropList(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetServicePropList::~CMTPSvcGetServicePropList()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPLIST_CMTPSVCGETSERVICEPROPLIST_ENTRY );
	delete iServicePropList;
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPLIST_CMTPSVCGETSERVICEPROPLIST_EXIT );
	}

CMTPSvcGetServicePropList::CMTPSvcGetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iResponseCode(EMTPRespCodeOK)
	{	
	}

void CMTPSvcGetServicePropList::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPLIST_CONSTRUCTL_ENTRY );
	iServicePropList = CMTPTypeServicePropList::NewL();
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPLIST_CONSTRUCTL_EXIT );
	}

void CMTPSvcGetServicePropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPLIST_SERVICEL_ENTRY );
	TUint32 propcode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	if (KMTPServicePropertyAll == propcode)
		{
		RArray<TUint32> servicePropArray;
		CleanupClosePushL(servicePropArray);
		(iDataProvider.ServiceHandler())->GetAllSevicePropCodesL(servicePropArray);
		TInt count = servicePropArray.Count();
		for (TInt i = 0; i < count && iResponseCode == EMTPRespCodeOK; i++)
			{
			iResponseCode = (iDataProvider.ServiceHandler())->GetServicePropertyL(servicePropArray[i], *iServicePropList);
			}
		CleanupStack::PopAndDestroy(&servicePropArray);
		}
	else
		{
		iResponseCode = (iDataProvider.ServiceHandler())->GetServicePropertyL(propcode, *iServicePropList);
		}
	SendDataL(*iServicePropList);
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPLIST_SERVICEL_EXIT );
	}

TMTPResponseCode CMTPSvcGetServicePropList::CheckRequestL()
{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPLIST_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode  = EMTPRespCodeInvalidServiceID;
			}
		
		if (EMTPRespCodeOK == responseCode)
			{
			TUint32 propcode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
			if (KMTPServicePropsAll != propcode && !iDataProvider.IsValidServicePropCodeL(propcode))
				{
				responseCode = EMTPRespCodeInvalidServicePropCode;
				}
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEPROPLIST_CHECKREQUESTL, "Exit with responseCode = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPLIST_CHECKREQUESTL_EXIT );
	
	return responseCode;
}

TBool CMTPSvcGetServicePropList::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
    OstTrace1( TRACE_NORMAL, CMTPSVCGETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL, "Exit with responseCode = 0x%04X", iResponseCode );	
	OstTraceFunctionExit0( CMTPSVCGETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}
