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

__FLOG_STMT (_LIT8(KComponent, "SvcDelSvcPList");)

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
	__FLOG(_L8("~CMTPSvcDeleteServicePropList - Entry"));
	delete iDeleteServicePropList;
	__FLOG(_L8("~CMTPSvcDeleteServicePropList - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcDeleteServicePropList::CMTPSvcDeleteServicePropList(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{
	}

void CMTPSvcDeleteServicePropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry")); 
	iDeleteServicePropList = CMTPTypeDeleteServicePropList::NewL();
	__FLOG(_L8("ConstructL - Exit")); 
	}

TMTPResponseCode CMTPSvcDeleteServicePropList::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		if (serviceId != iDataProvider.ServiceID())
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcDeleteServicePropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	//Recieve the data from the property list
	ReceiveDataL(*iDeleteServicePropList);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcDeleteServicePropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
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
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit responseCode = 0x%04X, failed index = %u"), responseCode, parameter));
	return EFalse;
	}

TBool CMTPSvcDeleteServicePropList::HasDataphase() const
	{
	return ETrue;
	}
