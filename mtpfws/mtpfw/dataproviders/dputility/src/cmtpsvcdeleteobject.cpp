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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcdeleteobjectTraces.h"
#endif


// Class constants.

EXPORT_C MMTPRequestProcessor* CMTPSvcDeleteObject::NewL(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcDeleteObject* self = new (ELeave) CMTPSvcDeleteObject(aFramework, aConnection, aDataProvider);
	return self;
	}

EXPORT_C CMTPSvcDeleteObject::~CMTPSvcDeleteObject()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_CMTPSVCDELETEOBJECT_DES_ENTRY );
	iObjectHandles.Close();
	delete iReceivedObjectMetaData;
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_CMTPSVCDELETEOBJECT_DES_EXIT );
	}

CMTPSvcDeleteObject::CMTPSvcDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL), iDataProvider(aDataProvider), iDeleteError(KErrNone)
	{
	OstTrace0( TRACE_NORMAL, CMTPSVCDELETEOBJECT_CMTPSVCDELETEOBJECT, "CMTPSvcDeleteObject - Constructed" );
	}

/**
DeleteObject request handler
*/
void CMTPSvcDeleteObject::LoadAllObjHandlesL(TUint32 aParentHandle)
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_LOADALLOBJHANDLESL_ENTRY );
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
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_LOADALLOBJHANDLESL_EXIT );
	}

void CMTPSvcDeleteObject::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_SERVICEL_ENTRY );
	if (iFormatCode == EMTPFormatCodeAssociation)
		{
		// Framework may send deleteobject for a directory, allow framework do this.
		SendResponseL(EMTPRespCodeOK);
		OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_SERVICEL_EXIT );
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
		OstTrace1( TRACE_NORMAL, CMTPSVCDELETEOBJECT_SERVICEL, 
		        "Delete single object exit with response code = 0x%04X", responseCode );
		}
	OstTraceFunctionExit0( DUP1_CMTPSVCDELETEOBJECT_SERVICEL_EXIT );
	}

void CMTPSvcDeleteObject::RunL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_RUNL_ENTRY );
	OstTraceExt2( TRACE_NORMAL, CMTPSVCDELETEOBJECT_RUNL, 
	        "the number of objects to be deleted is %d, iDeleteIndex is %d", iObjectHandles.Count(), iDeleteIndex );

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
			OstTraceExt1( TRACE_NORMAL, DUP1_CMTPSVCDELETEOBJECT_RUNL, 
			        "Delete object failed, SUID:%S", iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid));
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
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_RUNL_EXIT );
	}

/**
Handle an error in the delete loop by storing the error code and continuing deleting.
*/
TInt CMTPSvcDeleteObject::RunError(TInt aError)
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_RUNERROR_ENTRY );
	CompleteSelf(aError);
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_RUNERROR_EXIT );
	return KErrNone;
	}

/**
Complete myself
*/
void CMTPSvcDeleteObject::CompleteSelf(TInt aError)
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_COMPLETESELF_ENTRY );
	SetActive();
	TRequestStatus* status = &iStatus;
	*status = KRequestPending;
	User::RequestComplete(status, aError);
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_COMPLETESELF_EXIT );
	}

TMTPResponseCode CMTPSvcDeleteObject::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_CHECKREQUESTL_ENTRY );
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
					OstTrace0( TRACE_WARNING, DUP1_CMTPSVCDELETEOBJECT_CHECKREQUESTL, "DataProviderId dismatch" );
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

	OstTrace1( TRACE_NORMAL, CMTPSVCDELETEOBJECT_CHECKREQUESTL, "Exit with response code = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_CHECKREQUESTL_EXIT );
	return responseCode;
	}

void CMTPSvcDeleteObject::ProcessFinalPhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_PROCESSFINALPHASEL_ENTRY );
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
	OstTrace1( TRACE_NORMAL, CMTPSVCDELETEOBJECT_PROCESSFINALPHASEL, "Exit with response code = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_PROCESSFINALPHASEL_EXIT );
	}

TMTPResponseCode CMTPSvcDeleteObject::CheckFmtAndSetHandler(TUint32 aFormatCode)
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_CHECKFMTANDSETHANDLER_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	iObjectHandler = iDataProvider.ObjectHandler(aFormatCode);
	if (!iObjectHandler)
		{
		responseCode = EMTPRespCodeInvalidObjectFormatCode;
		}
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_CHECKFMTANDSETHANDLER_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPSvcDeleteObject::DeleteObjectL(const CMTPObjectMetaData& aObjectMetaData)
	{
	OstTraceFunctionEntry0( CMTPSVCDELETEOBJECT_DELETEOBJECTL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint16 formatCode = aObjectMetaData.Uint(CMTPObjectMetaData::EFormatCode);
	responseCode = CheckFmtAndSetHandler(formatCode);
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = iObjectHandler->DeleteObjectL(aObjectMetaData);
		}
	OstTraceFunctionExit0( CMTPSVCDELETEOBJECT_DELETEOBJECTL_EXIT );
	return responseCode;
	}
