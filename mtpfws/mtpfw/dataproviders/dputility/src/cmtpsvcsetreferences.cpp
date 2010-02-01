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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcsetreferences.cpp


#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpreferencemgr.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpsvcsetreferences.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

__FLOG_STMT(_LIT8(KComponent,"SvcSetRef");)

EXPORT_C MMTPRequestProcessor* CMTPSvcSetReferences::NewL(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcSetReferences* self = new (ELeave) CMTPSvcSetReferences(aFramework, aConnection, aDataProvider);
	return self;
	}

EXPORT_C CMTPSvcSetReferences::~CMTPSvcSetReferences()
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
CMTPSvcSetReferences::CMTPSvcSetReferences(MMTPDataProviderFramework& aFramework, 
										MMTPConnection& aConnection, 
										MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), 
	iDataProvider(aDataProvider)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("Constructed"));
	}

/**
SetReferences request handler
start receiving reference data from the initiator
*/
void CMTPSvcSetReferences::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	delete iReferences;
	iReferences = NULL;
	iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	ReceiveDataL(*iReferences);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcSetReferences::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	if(!VerifyReferenceHandlesL())
		{
		SendResponseL(EMTPRespCodeInvalidObjectReference);
		}
	else
		{
		TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
		TMTPResponseCode responseCode = EMTPRespCodeOK;
		responseCode = (iDataProvider.ObjectHandler(formatCode))->SetObjectReferenceL(*iReceivedObjectMetaData, *iReferences);
		SendResponseL(responseCode);
		}
	__FLOG(_L8("DoHandleResponsePhaseL - Exit"));
	return EFalse;
	}

TBool CMTPSvcSetReferences::HasDataphase() const
	{
	return ETrue;
	}

TBool CMTPSvcSetReferences::VerifyReferenceHandlesL() const
	{
	__FLOG(_L8("VerifyReferenceHandlesL - Entry"));
	__ASSERT_DEBUG(iReferences, User::Invariant());
	TBool result = ETrue;
	TInt count = iReferences->NumElements();
	CMTPObjectMetaData* object = CMTPObjectMetaData::NewLC();
	MMTPObjectMgr& objectMgr = iFramework.ObjectMgr();
	for(TInt i = 0; i < count; i++)
		{
		TMTPTypeUint32 handle;
		iReferences->ElementL(i, handle);
		if(!objectMgr.ObjectL(handle, *object))
			{
			result = EFalse;
			break;
			}
		}
	CleanupStack::PopAndDestroy(object);
	__FLOG(_L8("VerifyReferenceHandlesL - Exit"));
	return result;
	}

TMTPResponseCode CMTPSvcSetReferences::CheckRequestL()
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
