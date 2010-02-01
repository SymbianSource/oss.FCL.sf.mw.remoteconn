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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcGetSvcCap");)

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
	__FLOG(_L8("~CMTPSvcGetServiceCapabilities - Entry"));
	delete iServiceCapabilityList;
	__FLOG(_L8("~CMTPSvcGetServiceCapabilities - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcGetServiceCapabilities::CMTPSvcGetServiceCapabilities(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider)
:CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iResponseCode(EMTPRespCodeOK)
	{	
	}

void CMTPSvcGetServiceCapabilities::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry")); 
	iServiceCapabilityList = CMTPTypeServiceCapabilityList::NewL();
	__FLOG(_L8("ConstructL - Exit")); 
	}

TMTPResponseCode CMTPSvcGetServiceCapabilities::CheckRequestL()
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
		
		if (EMTPRespCodeOK == responseCode)
			{
			TUint16 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2); 
			if (KMTPFormatsAll != formatCode && !iDataProvider.IsValidFormatCodeL(formatCode))
				{
				responseCode = EMTPRespCodeInvalidObjectFormatCode;
				}
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
}

void CMTPSvcGetServiceCapabilities::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
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
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcGetServiceCapabilities::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with Response Code: 0x%x"), iResponseCode));
	return EFalse;
	}
