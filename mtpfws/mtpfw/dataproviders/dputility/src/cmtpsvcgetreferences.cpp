// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetreferences.cpp

#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpsvcgetreferences.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

__FLOG_STMT(_LIT8(KComponent,"SvcGetRef");)

EXPORT_C MMTPRequestProcessor* CMTPSvcGetReferences::NewL(MMTPDataProviderFramework& aFramework, 
														MMTPConnection& aConnection, 
														MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetReferences* self = new (ELeave) CMTPSvcGetReferences(aFramework, aConnection, aDataProvider);
	return self;
	}

/**
Destructor
*/
EXPORT_C CMTPSvcGetReferences::~CMTPSvcGetReferences()
	{
	__FLOG(_L8("~CMTPSvcGetReferences - Entry"));
	delete iReferences;
	delete iReceivedObjectMetaData;
	__FLOG(_L8("~CMTPSvcGetReferences - Exit"));
	__FLOG_CLOSE; 
	}

/**
Standard c++ constructor
*/
CMTPSvcGetReferences::CMTPSvcGetReferences(MMTPDataProviderFramework& aFramework, 
										MMTPConnection& aConnection, 
										MMTPServiceDataProvider& aDataProvider)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL), 
	iDataProvider(aDataProvider)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("Constructed"));
	}

TMTPResponseCode CMTPSvcGetReferences::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));

	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		// Check object handle
		MMTPObjectMgr& objMgr(iFramework.ObjectMgr());
		// Check whether object handle is valid
		if (objMgr.ObjectL(objectHandle, *iReceivedObjectMetaData))
			{
			// Check whether the owner of this object is correct data provider
			if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId())
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				__FLOG(_L8("CheckRequestL - DataProviderId dismatch"));
				}
			else
				{
				// Check format code
				TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				if (!iDataProvider.ObjectHandler(formatCode))
					{
					responseCode = EMTPRespCodeInvalidObjectHandle;
					}
				}
			}
		else
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with code: 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcGetReferences::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
	delete iReferences;
	iReferences = NULL;
	iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	iDataProvider.ObjectHandler(formatCode)->GetObjectReferenceL(*iReceivedObjectMetaData, *iReferences);
	SendDataL(*iReferences);
	__FLOG(_L8("ServiceL - Exit"));
	}
