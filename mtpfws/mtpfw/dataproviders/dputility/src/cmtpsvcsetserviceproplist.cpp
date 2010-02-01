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

__FLOG_STMT (_LIT8(KComponent, "SvcSetSvcPList");)

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
	__FLOG(_L8("~CMTPSvcSetServicePropList - Entry"));
	delete iServicePropList;
	__FLOG(_L8("~CMTPSvcSetServicePropList - Exit"));
	__FLOG_CLOSE;
	}
 
CMTPSvcSetServicePropList::CMTPSvcSetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider)
	{	
	}

void CMTPSvcSetServicePropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry")); 
	iServicePropList = CMTPTypeServicePropList::NewL();
	__FLOG(_L8("ConstructL - Exit")); 
	}

TMTPResponseCode CMTPSvcSetServicePropList::CheckRequestL()
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

void CMTPSvcSetServicePropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	//Recieve the data from the property list
	ReceiveDataL(*iServicePropList);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcSetServicePropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry")); 
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
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with responseCode = 0x%04X and failed index: %u"), responseCode, parameter));
	return EFalse;
	}

TBool CMTPSvcSetServicePropList::HasDataphase() const
	{
	return ETrue;
	}
