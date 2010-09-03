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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcdeleteserviceproplist.cpp

#include <mtp/cmtptypedeleteserviceproplist.h>
#include <mtp/tmtptypedatapair.h>

#include "cmtpsvcdeleteserviceproplist.h"
#include "mmtpservicedataprovider.h"
#include "mmtpservicehandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcdeleteserviceproplistTraces.h"
#endif


EXPORT_C MMTPRequestProcessor* CMTPSvcDeleteServicePropList::NewL(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcDeleteServicePropList* self = new (ELeave) CMTPSvcDeleteServicePropList(aFramework, aConnection,aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcDeleteServicePropList::~CMTPSvcDeleteServicePropList()
	{    
	OstTraceFunctionEntry0( CMTPSVCDELETESERVICEPROPLIST_CMTPSVCDELETESERVICEPROPLIST_DES_ENTRY );
	delete iDeleteServicePropList;
	OstTraceFunctionExit0( CMTPSVCDELETESERVICEPROPLIST_CMTPSVCDELETESERVICEPROPLIST_DES_EXIT );
	}

CMTPSvcDeleteServicePropList::CMTPSvcDeleteServicePropList(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{
	}

void CMTPSvcDeleteServicePropList::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETESERVICEPROPLIST_CONSTRUCTL_ENTRY );
	iDeleteServicePropList = CMTPTypeDeleteServicePropList::NewL();
	OstTraceFunctionExit0( CMTPSVCDELETESERVICEPROPLIST_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcDeleteServicePropList::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETESERVICEPROPLIST_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			}
		}
	
    OstTrace1( TRACE_NORMAL, CMTPSVCDELETESERVICEPROPLIST_CHECKREQUESTL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCDELETESERVICEPROPLIST_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcDeleteServicePropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETESERVICEPROPLIST_SERVICEL_ENTRY );
	//Recieve the data from the property list
	ReceiveDataL(*iDeleteServicePropList);
	OstTraceFunctionExit0( CMTPSVCDELETESERVICEPROPLIST_SERVICEL_EXIT );
	}

TBool CMTPSvcDeleteServicePropList::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETESERVICEPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint32 parameter = 0;
	const TUint count = iDeleteServicePropList->NumberOfElements();
	for (TUint i = 0; i < count && responseCode == EMTPRespCodeOK; i++)
		{
		TMTPTypeDataPair& element = iDeleteServicePropList->ElementL(i);
		
		if (iDataProvider.ServiceID() != element.Uint32(TMTPTypeDataPair::EOwnerHandle))
			{
			parameter = i;
			responseCode = EMTPRespCodeInvalidServiceID;
			break;
			}
	
		TUint16 propertyCode = element.Uint16(TMTPTypeDataPair::EDataCode);
		responseCode = iDataProvider.ServiceHandler()->DeleteServicePropertyL(propertyCode);
		if (EMTPRespCodeOK != responseCode)
			{
			parameter = i;
			break;
			}
		}
	SendResponseL(responseCode, 1, &parameter);
    OstTraceExt2( TRACE_NORMAL, CMTPSVCDELETESERVICEPROPLIST_DOHANDLERESPONSEPHASEL, 
            "Exit responseCode = 0x%04X, failed index = %u", responseCode, parameter );
	OstTraceFunctionExit0( CMTPSVCDELETESERVICEPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}

TBool CMTPSvcDeleteServicePropList::HasDataphase() const
	{
	return ETrue;
	}
