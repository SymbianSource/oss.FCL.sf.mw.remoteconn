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

__FLOG_STMT(_LIT8(KComponent,"SvcGetSvcPDesc");)

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
	__FLOG(_L8("~CMTPSvcGetServicePropDesc - Entry"));
	delete iPropDescList;
	__FLOG(_L8("~CMTPSvcGetServicePropDesc - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetServicePropDesc::CMTPSvcGetServicePropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider),
	iResponseCode(EMTPRespCodeOK)
	{
	}

void CMTPSvcGetServicePropDesc::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iPropDescList = CMTPTypeServicePropDescList::NewL();
	__FLOG(_L8("ConstructL - Exit"));
	}

TMTPResponseCode CMTPSvcGetServicePropDesc::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 serviceID = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		TUint32 propCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
		
		if ((iDataProvider.ServiceID() != serviceID))
			{
			responseCode = EMTPRespCodeInvalidServiceID;
			__FLOG(_L8("Service Id Parameter don't be supported"));
			}
		
		if (EMTPRespCodeOK == responseCode)
			{
			if ((KMTPNotSpecified32 != propCode) && 
				!(iDataProvider.IsValidServicePropCodeL(propCode)))
				{
				responseCode = EMTPRespCodeInvalidServicePropCode;
				__FLOG(_L8("Service Object PropCode Parameter don't be supported"));
				}
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcGetServicePropDesc::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
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
	__FLOG_VA((_L8("ServiceL - Exit with Response Code: 0x%x, Service Property Count: %u"), iResponseCode, iPropDescList->NumberOfElements()));
	}

TBool CMTPSvcGetServicePropDesc::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with Response Code: 0x%x"), iResponseCode));
	return EFalse;
	}

