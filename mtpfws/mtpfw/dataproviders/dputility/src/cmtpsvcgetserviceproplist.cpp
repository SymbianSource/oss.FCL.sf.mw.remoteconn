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

__FLOG_STMT (_LIT8(KComponent,"SvcGetSvcPList");)

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
	__FLOG(_L8("~CMTPSvcGetServicePropList - Entry"));
	delete iServicePropList;
	__FLOG(_L8("~CMTPSvcGetServicePropList - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetServicePropList::CMTPSvcGetServicePropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iResponseCode(EMTPRespCodeOK)
	{	
	}

void CMTPSvcGetServicePropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry")); 
	iServicePropList = CMTPTypeServicePropList::NewL();
	__FLOG(_L8("ConstructL - Exit")); 
	}

void CMTPSvcGetServicePropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
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
	__FLOG(_L8("ServiceL - Exit"));
	}

TMTPResponseCode CMTPSvcGetServicePropList::CheckRequestL()
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
		
		if (EMTPRespCodeOK == responseCode)
			{
			TUint32 propcode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
			if (KMTPServicePropsAll != propcode && !iDataProvider.IsValidServicePropCodeL(propcode))
				{
				responseCode = EMTPRespCodeInvalidServicePropCode;
				}
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
}

TBool CMTPSvcGetServicePropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with Response Code: 0x%x"), iResponseCode));
	return EFalse;
	}
