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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcgetobjectproplist.cpp

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpsvcgetobjectproplist.h"
#include "mtpsvcdpconst.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcGetObjPL");)

EXPORT_C MMTPRequestProcessor* CMTPSvcGetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcGetObjectPropList* self = new (ELeave) CMTPSvcGetObjectPropList(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcGetObjectPropList::~CMTPSvcGetObjectPropList()
	{
	__FLOG(_L8("~CMTPSvcGetObjectPropList - Entry"));
	delete iPropertyList;
	delete iReceivedObjectMetaData;
	iObjectHandles.Close();
	__FLOG(_L8("~CMTPSvcGetObjectPropList - Exit"));
	__FLOG_CLOSE; 
	}

CMTPSvcGetObjectPropList::CMTPSvcGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iError(KErrNone),
	iDataProvider(aDataProvider),
	iResponseCode(EMTPRespCodeOK)
	{
	}

void CMTPSvcGetObjectPropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iPropertyList = CMTPTypeObjectPropList::NewL();
	__FLOG(_L8("ConstructL - Exit"));
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckObjectHandleAndFormatL();
		}
	
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckDepth();
		}
	
	__FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckObjectHandleAndFormatL()
	{
	__FLOG(_L8("CheckObjectHandleAndFormatL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK; 
	
	TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	iPropCode = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
	iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
	// Object is a specified handle
	if (objectHandle != KMTPHandleAll && objectHandle != KMTPHandleAllRootLevel)
		{
		MMTPObjectMgr& objects(iFramework.ObjectMgr());
		//Check whether object handle is valid
		if (objects.ObjectL(objectHandle, *iReceivedObjectMetaData))
			{
			if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId())
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				__FLOG(_L8("CheckRequestL - DataProviderId dismatch"));
				}
			else
				{
				// If handle is ok, ignore format code parameter
				formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				iObjectHandler = iDataProvider.ObjectHandler(formatCode);
				if (!iObjectHandler)
					{
					responseCode = EMTPRespCodeInvalidObjectHandle;
					}
				else
					{
					responseCode = CheckPropertyCodeForFormatL(formatCode);
					}
				}
			}
		else
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		}
	// If object handle is 0x0000000 or 0xFFFFFFFF
	else
		{
		// A formatCode value of 0x00000000 indicates that this parameter is not being used and properties 
		// of all Object Formats are desired. 
		if (formatCode != KMTPNotSpecified32)
			{
			iObjectHandler = iDataProvider.ObjectHandler(formatCode);
			if (!iObjectHandler)
				{
				responseCode = EMTPRespCodeSpecificationByFormatUnsupported;
				}
			else
				{
				responseCode = CheckPropertyCodeForFormatL(formatCode);
				}
			}
		}
	__FLOG_VA((_L8("CheckObjectHandleAndFormatL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckPropertyCodeForFormatL(TUint32 aFormatCode) const
	{
	__FLOG(_L8("CheckPropertyCodeForFormatL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (iPropCode != KMTPObjectPropCodeAll && iPropCode != KMTPNotSpecified32)
		{
		if (!iDataProvider.IsValidObjectPropCodeL(aFormatCode, iPropCode))
			{
			responseCode = EMTPRespCodeInvalidObjectPropCode;
			}
		}
	__FLOG_VA((_L8("CheckPropertyCodeForFormatL - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

/**
Ensures that the requested object depth is one we support.
@return EMTPRespCodeOK, or EMTPRespCodeSpecificationByDepthUnsupported if the depth is unsupported
*/
TMTPResponseCode CMTPSvcGetObjectPropList::CheckDepth() const
	{
	__FLOG(_L8("CheckDepth - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeSpecificationByDepthUnsupported;
	// Support  depth 0 or 1 or 0xFFFFFFFF
	TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	TUint32 depth = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
	if (depth == 0 || depth == 1 || depth == KMTPHandleNoParent)
		{
		responseCode = EMTPRespCodeOK; 
		}
	__FLOG_VA((_L8("CheckDepth - Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcGetObjectPropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
	TUint32 formatCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
	TUint32 depth(Request().Uint32(TMTPTypeRequest::ERequestParameter5));
	iGroupId = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
	// Get all objects or root objects
	if (handle == KMTPHandleAll || handle == KMTPHandleAllRootLevel)
		{
		// For service DP, the two cases are the same, need all handles with format code.
		RMTPObjectMgrQueryContext   context;
		CleanupClosePushL(context);
		do
			{
			TUint32 storageId = iDataProvider.StorageId();
			TMTPObjectMgrQueryParams params(storageId, formatCode, KMTPHandleNoParent);
			iFramework.ObjectMgr().GetObjectHandlesL(params, context, iObjectHandles);
			}
		while (!context.QueryComplete());
		CleanupStack::PopAndDestroy(&context);
		iHandleIndex = 0;
		CompleteSelf(KErrNone);
		}
	else
		{
		GetObjectPropertyHelperL();
		SendDataL(*iPropertyList);
		}
	__FLOG(_L8("ServiceL - Exit"));
	}

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPSvcGetObjectPropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
	__FLOG_VA((_L8("DoHandleResponsePhaseL - Exit with response code = 0x%04X"), responseCode));
	return EFalse;
	}

void CMTPSvcGetObjectPropList::RunL()
	{
	__FLOG(_L8("RunL - Entry"));
	__FLOG_VA((_L8("the number of objects to be queried is %d, iHandleIndex is %d"), iObjectHandles.Count(), iHandleIndex));
	
	TInt count = iObjectHandles.Count();
	const TUint32 granularity = iDataProvider.OperationGranularity();
	for (TInt i = 0; iHandleIndex < count && i < granularity; i++)
		{
		TUint handle = iObjectHandles[iHandleIndex++];
		iFramework.ObjectMgr().ObjectL(handle, *iReceivedObjectMetaData);
		// Process for each object
		TUint16 formatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
		iObjectHandler = iDataProvider.ObjectHandler(formatCode);
		if (iObjectHandler)
			{
			GetObjectPropertyHelperL();
			}
		else
			{
			// An unsupport object got from object mgr.
			iResponseCode = EMTPRespCodeInvalidObjectHandle;
			}
		__FLOG_VA((_L8("Get a object property list, SUID:%S, response code: 0x%4x"), &(iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid)), iResponseCode));
		}
	
	if (iHandleIndex >= count)
		{
		ProcessFinalPhaseL();
		}
	else 
		{
		CompleteSelf(KErrNone);
		}
	__FLOG(_L8("RunL - Exit")); 
	}

TInt CMTPSvcGetObjectPropList::RunError(TInt aError)
	{
	__FLOG(_L8("RunError - Entry"));
	CompleteSelf(aError);
	__FLOG(_L8("RunError - Exit")); 
	return KErrNone;
	}

/**
Complete myself
*/
void CMTPSvcGetObjectPropList::CompleteSelf(TInt aError)
	{
	__FLOG(_L8("CompleteSelf - Entry"));
	SetActive();
	TRequestStatus* status = &iStatus;
	*status = KRequestPending;
	User::RequestComplete(status, aError);
	__FLOG(_L8("CompleteSelf - Exit"));
	}

/**
Signal to the initiator that the deletion operation has finished with or without error
*/
void CMTPSvcGetObjectPropList::ProcessFinalPhaseL()
	{
	__FLOG(_L8("ProcessFinalPhaseL - Entry"));
	SendDataL(*iPropertyList);
	__FLOG(_L8("ProcessFinalPhaseL - Exit"));
	}

void CMTPSvcGetObjectPropList::GetObjectPropertyHelperL()
	{
	if (iPropCode == KMTPObjectPropCodeAll)
		{
		// A value of 0xFFFFFFFF indicates that all properties are requested except those 
		// with a group code of 0xFFFFFFFF
		iGroupId = 0; // Get all object prop codes
		}
	if (iPropCode == KMTPNotSpecified32 || iPropCode == KMTPObjectPropCodeAll)
		{
		// A value of 0x00000000 in the third parameter indicates that the fourth 
		// parameter should be used
		RArray<TUint32> objectPropCodes;
		CleanupClosePushL(objectPropCodes);
		iResponseCode = iObjectHandler->GetAllObjectPropCodeByGroupL(iGroupId, objectPropCodes);
		if (iResponseCode == EMTPRespCodeOK)
			{
			TInt count = objectPropCodes.Count();
			for (TInt i = 0; i < count && iResponseCode == EMTPRespCodeOK; i++)
				{
				iResponseCode = iObjectHandler->GetObjectPropertyL(*iReceivedObjectMetaData, objectPropCodes[i], *iPropertyList);
				}
			}
		CleanupStack::PopAndDestroy(&objectPropCodes);
		}
	else
		{
		// Get one prop info into objectproplist.
		iResponseCode = iObjectHandler->GetObjectPropertyL(*iReceivedObjectMetaData, iPropCode, *iPropertyList);
		}
	}
