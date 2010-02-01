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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcdeleteobject.cpp

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpsvcdeleteobject.h"
#include "mtpdpconst.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcDeleteObject");)

EXPORT_C MMTPRequestProcessor* CMTPSvcDeleteObject::NewL(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcDeleteObject* self = new (ELeave) CMTPSvcDeleteObject(aFramework, aConnection, aDataProvider);
	return self;
	}

EXPORT_C CMTPSvcDeleteObject::~CMTPSvcDeleteObject()
	{
	__FLOG(_L8("~CMTPSvcDeleteObject - Entry"));
	iObjectHandles.Close();
	delete iReceivedObjectMetaData;
	__FLOG(_L8("~CMTPSvcDeleteObject - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcDeleteObject::CMTPSvcDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iDeleteError(KErrNone)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("CMTPSvcDeleteObject - Constructed"));
	}

/**
DeleteObject request handler
*/
void CMTPSvcDeleteObject::LoadAllObjHandlesL(TUint32 aParentHandle)
	{
	__FLOG(_L8("LoadAllObjHandlesL - Entry"));
	const TUint32 KFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	RMTPObjectMgrQueryContext context;
	CleanupClosePushL(context);

	do
		{ // Append all object handles with service dp's storage id.
		TUint32 storageId = iDataProvider.StorageId();
		TMTPObjectMgrQueryParams  params(storageId, KFormatCode, aParentHandle);
		iFramework.ObjectMgr().GetObjectHandlesL(params, context, iObjectHandles);
		}
	while (!context.QueryComplete());
	CleanupStack::PopAndDestroy(&context);
	__FLOG(_L8("LoadAllObjHandlesL - Exit"));
	}

void CMTPSvcDeleteObject::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	if (iFormatCode == EMTPFormatCodeAssociation)
		{
		// Framework may send deleteobject for a directory, allow framework do this.
		SendResponseL(EMTPRespCodeOK);
		return;
		}
	
	const TUint32 KHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	
	if (KHandle == KMTPHandleAll)
		{
		LoadAllObjHandlesL(KMTPHandleNoParent);
		iDeleteIndex = 0;
		iSuccessDeletion = EFalse;
		CompleteSelf(KErrNone);
		}
	else
		{
		TMTPResponseCode responseCode = iObjectHandler->DeleteObjectL(*iReceivedObjectMetaData);
		// Remove from framework.
		iFramework.ObjectMgr().RemoveObjectL(iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid));
		SendResponseL(responseCode);
		__FLOG_VA((_L8("Delete single object exit with response code = 0x%04X"), responseCode));
		}
	__FLOG(_L8("ServiceL - Exit"));
	}

void CMTPSvcDeleteObject::RunL()
	{
	__FLOG(_L8("RunL - Entry"));
	__FLOG_VA((_L8("the number of objects to be deleted is %d, iDeleteIndex is %d"), iObjectHandles.Count(), iDeleteIndex));

	if (iStatus != KErrNone)
		{
		iDeleteError = iStatus.Int();
		}

	CDesCArray* objectDeleted = new (ELeave) CDesCArrayFlat(KMTPDriveGranularity);
	CleanupStack::PushL(objectDeleted);
	
	TInt errCount = 0;
	TInt count = iObjectHandles.Count();
	const TUint32 granularity = iDataProvider.OperationGranularity();
	for (TInt i = 0; iDeleteIndex < count && i < granularity; ++i)
		{
		TUint handle = iObjectHandles[iDeleteIndex++];
		iFramework.ObjectMgr().ObjectL(handle, *iReceivedObjectMetaData);
		if (DeleteObjectL(*iReceivedObjectMetaData) == EMTPRespCodeOK)
			{
			objectDeleted->AppendL(iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid));
			iSuccessDeletion = ETrue;
			}
		else
			{
			++errCount;
			__FLOG_VA((_L8("Delete object failed, SUID:%S"), &(iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid))));
			}
		}

	// Remove object from framework
	iFramework.ObjectMgr().RemoveObjectsL(*objectDeleted);
	CleanupStack::PopAndDestroy(objectDeleted);

	if (iDeleteIndex >= count)
		{
		ProcessFinalPhaseL();
		}
	else 
		{
		TInt err = (errCount > 0) ? KErrGeneral : KErrNone;
		CompleteSelf(err);
		}
	__FLOG(_L8("RunL - Exit")); 
	}

/**
Handle an error in the delete loop by storing the error code and continuing deleting.
*/
TInt CMTPSvcDeleteObject::RunError(TInt aError)
	{
	__FLOG(_L8("RunError - Entry")); 
	CompleteSelf(aError);
	__FLOG(_L8("RunError - Exit"));
	return KErrNone;
	}

/**
Complete myself
*/
void CMTPSvcDeleteObject::CompleteSelf(TInt aError)
	{
	__FLOG(_L8("CompleteSelf - Entry"));
	SetActive();
	TRequestStatus* status = &iStatus;
	*status = KRequestPending;
	User::RequestComplete(status, aError);
	__FLOG(_L8("CompleteSelf - Exit"));
	}

TMTPResponseCode CMTPSvcDeleteObject::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		// Check object handle with dataprovider id, if handle is valid, then ignore format code in parameter 2.
		if (objectHandle != KMTPHandleAll)
			{
			MMTPObjectMgr& objects = iFramework.ObjectMgr();
			//Check whether object handle is valid
			// Framework may send a handle which is a directory and dp can't check id in this case
			if (objects.ObjectL(objectHandle, *iReceivedObjectMetaData))
				{
				iFormatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId() && (iFormatCode != EMTPFormatCodeAssociation))
					{
					responseCode = EMTPRespCodeInvalidObjectHandle;
					__FLOG(_L8("DataProviderId dismatch"));
					}
				else
					{
					responseCode = CheckFmtAndSetHandler(iFormatCode);
					}
				}
			else
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				}
			}
		else
			{
			// Delete all objects for a format
			iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
			// Check format code
			if (iFormatCode != KMTPFormatsAll)
				{
				responseCode = CheckFmtAndSetHandler(iFormatCode);
				}
			}
		}

	__FLOG_VA((_L8("CheckRequestL Exit with response code = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcDeleteObject::ProcessFinalPhaseL()
	{
	__FLOG(_L8("ProcessFinalPhaseL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (iDeleteError != KErrNone)
		{
		if (iSuccessDeletion)
			{
			responseCode = EMTPRespCodePartialDeletion;
			}
		else
			{
			responseCode = EMTPRespCodeStoreReadOnly;
			}
		}
	SendResponseL(responseCode);
	__FLOG_VA((_L8("ProcessFinalPhaseL - Exit with response code = 0x%04X"), responseCode));
	}

TMTPResponseCode CMTPSvcDeleteObject::CheckFmtAndSetHandler(TUint32 aFormatCode)
	{
	__FLOG(_L8("CheckFmtAndSetHandler - Entry")); 
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	iObjectHandler = iDataProvider.ObjectHandler(aFormatCode);
	if (!iObjectHandler)
		{
		responseCode = EMTPRespCodeInvalidObjectFormatCode;
		}
	__FLOG(_L8("CheckFmtAndSetHandler - Exit"));
	return responseCode;
	}

TMTPResponseCode CMTPSvcDeleteObject::DeleteObjectL(const CMTPObjectMetaData& aObjectMetaData)
	{
	__FLOG(_L8("DeleteObjectL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint16 formatCode = aObjectMetaData.Uint(CMTPObjectMetaData::EFormatCode);
	responseCode = CheckFmtAndSetHandler(formatCode);
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = iObjectHandler->DeleteObjectL(aObjectMetaData);
		}
	__FLOG(_L8("DeleteObjectL - Exit"));
	return responseCode;
	}
