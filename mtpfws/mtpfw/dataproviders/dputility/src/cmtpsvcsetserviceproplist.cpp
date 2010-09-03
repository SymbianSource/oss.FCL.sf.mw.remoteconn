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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsetserviceproplist.cpp

#include "cmtpsvcsetserviceproplist.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcsetserviceproplistTraces.h"
#endif



EXPORT_C MMTPRequestProcessor* CMTPSvcSetServicePropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcSetServicePropList* self = new (ELeave) CMTPSvcSetServicePropList(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcSetServicePropList::~CMTPSvcSetServicePropList()
	{
	OstTraceFunctionEntry0( CMTPSVCSETSERVICEPROPLIST_CMTPSVCSETSERVICEPROPLIST_DES_ENTRY );
	delete iServicePropList;
	OstTraceFunctionExit0( CMTPSVCSETSERVICEPROPLIST_CMTPSVCSETSERVICEPROPLIST_DES_EXIT );
	}
 
CMTPSvcSetServicePropList::CMTPSvcSetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcSetServicePropList::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETSERVICEPROPLIST_CONSTRUCTL_ENTRY );
	iServicePropList = CMTPTypeServicePropList::NewL();
	OstTraceFunctionExit0( CMTPSVCSETSERVICEPROPLIST_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcSetServicePropList::CheckRequestL()
{
	OstTraceFunctionEntry0( CMTPSVCSETSERVICEPROPLIST_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			}
		}

	OstTrace1( TRACE_NORMAL, CMTPSVCSETSERVICEPROPLIST_CHECKREQUESTL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCSETSERVICEPROPLIST_CHECKREQUESTL_EXIT );
	return responseCode;
}

void CMTPSvcSetServicePropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETSERVICEPROPLIST_SERVICEL_ENTRY );
	//Recieve the data from the property list
	ReceiveDataL(*iServicePropList);
	OstTraceFunctionExit0( CMTPSVCSETSERVICEPROPLIST_SERVICEL_EXIT );
	}

TBool CMTPSvcSetServicePropList::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCSETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
	TUint32 parameter = 0;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	const TUint count = iServicePropList->NumberOfElements();
	for (TUint i = 0; i < count && responseCode == EMTPRespCodeOK; i++)
		{
		CMTPTypeServicePropListElement& element = iServicePropList->Element(i);
		
		if(iDataProvider.ServiceID() != element.Uint32L(CMTPTypeServicePropListElement::EObjectHandle))
			{
			parameter = i;
			responseCode = EMTPRespCodeInvalidServiceID;
			break;
			}

		responseCode = (iDataProvider.ServiceHandler())->SetServicePropertyL(element);
		if (EMTPRespCodeOK != responseCode)
			{
			parameter = i;
			break;
			}
		}
	SendResponseL(responseCode, 1, &parameter);
    OstTraceExt2( TRACE_NORMAL, CMTPSVCSETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL, 
            "Exit with responseCode = 0x%04X and failed index: %u", responseCode, parameter );	
	OstTraceFunctionExit0( CMTPSVCSETSERVICEPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
	
	return EFalse;
	}

TBool CMTPSvcSetServicePropList::HasDataphase() const
	{
	return ETrue;
	}
