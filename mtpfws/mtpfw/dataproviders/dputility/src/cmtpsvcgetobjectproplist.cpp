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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvcgetobjectproplistTraces.h"
#endif



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
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CMTPSVCGETOBJECTPROPLIST_DES_ENTRY );
	delete iPropertyList;
	delete iReceivedObjectMetaData;
	iObjectHandles.Close();
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CMTPSVCGETOBJECTPROPLIST_DES_EXIT );
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
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
	iPropertyList = CMTPTypeObjectPropList::NewL();
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CONSTRUCTL_EXIT );
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckObjectHandleAndFormatL();
		}
	
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckDepth();
		}
	
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_CHECKREQUESTL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CHECKREQUESTL_EXIT );

	return responseCode;
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckObjectHandleAndFormatL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CHECKOBJECTHANDLEANDFORMATL_ENTRY );
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
				OstTrace0( TRACE_WARNING, DUP1_CMTPSVCGETOBJECTPROPLIST_CHECKOBJECTHANDLEANDFORMATL, "CheckRequestL - DataProviderId dismatch" );
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
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_CHECKOBJECTHANDLEANDFORMATL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CHECKOBJECTHANDLEANDFORMATL_EXIT );

	return responseCode;
	}

TMTPResponseCode CMTPSvcGetObjectPropList::CheckPropertyCodeForFormatL(TUint32 aFormatCode) const
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CHECKPROPERTYCODEFORFORMATL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (iPropCode != KMTPObjectPropCodeAll && iPropCode != KMTPNotSpecified32)
		{
		if (!iDataProvider.IsValidObjectPropCodeL(aFormatCode, iPropCode))
			{
			responseCode = EMTPRespCodeInvalidObjectPropCode;
			}
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_CHECKPROPERTYCODEFORFORMATL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CHECKPROPERTYCODEFORFORMATL_EXIT );
	return responseCode;
	}

/**
Ensures that the requested object depth is one we support.
@return EMTPRespCodeOK, or EMTPRespCodeSpecificationByDepthUnsupported if the depth is unsupported
*/
TMTPResponseCode CMTPSvcGetObjectPropList::CheckDepth() const
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_CHECKDEPTH_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeSpecificationByDepthUnsupported;
	// Support  depth 0 or 1 or 0xFFFFFFFF
	TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	TUint32 depth = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
	if (depth == 0 || depth == 1 || depth == KMTPHandleNoParent)
		{
		responseCode = EMTPRespCodeOK; 
		}
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_CHECKDEPTH, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_CHECKDEPTH_EXIT );
	return responseCode;
	}

void CMTPSvcGetObjectPropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_SERVICEL_ENTRY );
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
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_SERVICEL_EXIT );
	}

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPSvcGetObjectPropList::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iResponseCode);
	SendResponseL(responseCode);
    OstTrace1( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL, "Exit with responseCode = 0x%04X", responseCode );	
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}

void CMTPSvcGetObjectPropList::RunL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_RUNL_ENTRY );
	OstTraceExt2( TRACE_NORMAL, CMTPSVCGETOBJECTPROPLIST_RUNL, 
	        "the number of objects to be queried is %d, iHandleIndex is %d", iObjectHandles.Count(), iHandleIndex);
	
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
		OstTraceExt2( TRACE_NORMAL, DUP1_CMTPSVCGETOBJECTPROPLIST_RUNL, 
		        "Get a object property list, SUID:%S, response code: 0x%4x",iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid), (TUint32)iResponseCode );
		}
	
	if (iHandleIndex >= count)
		{
		ProcessFinalPhaseL();
		}
	else 
		{
		CompleteSelf(KErrNone);
		}
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_RUNL_EXIT );
	}

TInt CMTPSvcGetObjectPropList::RunError(TInt aError)
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_RUNERROR_ENTRY );
	CompleteSelf(aError);
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_RUNERROR_EXIT );
	return KErrNone;
	}

/**
Complete myself
*/
void CMTPSvcGetObjectPropList::CompleteSelf(TInt aError)
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_COMPLETESELF_ENTRY );
	SetActive();
	TRequestStatus* status = &iStatus;
	*status = KRequestPending;
	User::RequestComplete(status, aError);
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_COMPLETESELF_EXIT );
	}

/**
Signal to the initiator that the deletion operation has finished with or without error
*/
void CMTPSvcGetObjectPropList::ProcessFinalPhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCGETOBJECTPROPLIST_PROCESSFINALPHASEL_ENTRY );
	SendDataL(*iPropertyList);
	OstTraceFunctionExit0( CMTPSVCGETOBJECTPROPLIST_PROCESSFINALPHASEL_EXIT );
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
