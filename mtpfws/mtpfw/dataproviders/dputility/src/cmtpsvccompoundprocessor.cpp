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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvccompoundprocessor.cpp

#include <mtp/mmtpdataproviderframework.h>

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypefile.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h> 
#include <mtp/tmtptypeint128.h>

#include "cmtpsvccompoundprocessor.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

#include "cmtpconnection.h"
#include "cmtpconnectionmgr.h"
#include "mtpsvcdpconst.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsvccompoundprocessorTraces.h"
#endif


// Class constants.

EXPORT_C MMTPRequestProcessor* CMTPSvcCompoundProcessor::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcCompoundProcessor* self = new (ELeave) CMTPSvcCompoundProcessor(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcCompoundProcessor::~CMTPSvcCompoundProcessor()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CMTPSVCCOMPOUNDPROCESSOR_DES_ENTRY );
	delete iReceivedObjectMetaData;
	delete iObjectInfo;
	delete iObjectPropList;
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CMTPSVCCOMPOUNDPROCESSOR_DES_EXIT );
	}

CMTPSvcCompoundProcessor::CMTPSvcCompoundProcessor(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider), iState(EIdle), iIsCommited(EFalse), iIsRollBackHandlerObject(EFalse)
	{
	}

void CMTPSvcCompoundProcessor::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CONSTRUCTL_ENTRY );
	iExpectedSendObjectRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, EMTPOpCodeSendObject);
	iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CONSTRUCTL_EXIT );
	}

/**
Override to match both the SendObjectInfo/SendObjectPropList/UpdateObjectPropList and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/
TBool CMTPSvcCompoundProcessor::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_MATCH_ENTRY );
	TBool result = EFalse;
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	if ((&iConnection == &aConnection) && 
		(operationCode == EMTPOpCodeSendObjectInfo || 
		operationCode == EMTPOpCodeSendObject ||
		operationCode == EMTPOpCodeUpdateObjectPropList ||
		operationCode == EMTPOpCodeSendObjectPropList))
		{
		result = ETrue;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_MATCH_EXIT );
	return result;
	}

TBool CMTPSvcCompoundProcessor::HasDataphase() const
	{
	return ETrue;
	}

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTL_ENTRY );
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckSendingStateL();
		if (EMTPRespCodeOK == responseCode)
			{
			responseCode = CheckRequestParametersL();
			}
		}

	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTL, "Exit with code: 0x%04X", responseCode);
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTL_EXIT );
	return responseCode;
	}

/**
Verify if the SendObject request comes after SendObjectInfo/SendObjectPropList request
@return EMTPRespCodeOK if SendObject request comes after a valid SendObjectInfo request, otherwise
EMTPRespCodeNoValidObjectInfo
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckSendingStateL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	iOperationCode = Request().Uint16(TMTPTypeRequest::ERequestOperationCode);
	
	if (iOperationCode == EMTPOpCodeSendObject)
		{
		//In ParseRouter everytime SendObject gets resolved then will be removed from Registry
		//Right away therefore we need reRegister it here again in case possible cancelRequest
		//Against this SendObject being raised.
		iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
		iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
		}

	switch (iState)
		{
		case EIdle:
			// Received an orphan SendObject
			if (iOperationCode == EMTPOpCodeSendObject)
				{
				responseCode = EMTPRespCodeNoValidObjectInfo;
				OstTrace0( TRACE_NORMAL, DUP1_CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL, "EIdle: Received an orphan SendObject request" );
				}
			break;
		case EObjectInfoSucceed:
			// If another SendObjectInfo or SendObjectPropList operation occurs before a SendObject
			// operation, the new ObjectInfo or ObjectPropList shall replace the previously held one. 
			// If this occurs, any storage or memory space reserved for the object described in the 
			// overwritten ObjectInfo or ObjectPropList dataset should be freed before overwriting and 
			// allocating the resources for the new data. 
			
			// Here is for the processor received another SendObjectInfo or SendObjectPropList 
			// before a SendObject or process SendObject failed,
			if (iOperationCode == EMTPOpCodeSendObjectInfo ||
				iOperationCode == EMTPOpCodeSendObjectPropList ||
				iOperationCode == EMTPOpCodeUpdateObjectPropList)
				{
				iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
				if (!iIsCommited)
					{
					// Object Size != 0, need roll back all resource for the new SendInfo request
					CMTPSvcCompoundProcessor::RollBackObject(this);
					iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObjectMetaData);
					}
				delete iObjectInfo;
				iObjectInfo = NULL;
				delete iObjectPropList;
				iObjectPropList = NULL;
				delete iReceivedObjectMetaData;
				iReceivedObjectMetaData = NULL;
				iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
				iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
				iObjectHandler = NULL;
				iState = EIdle;
				// Reset commit state to false
				iIsCommited = EFalse;
				OstTrace0( TRACE_NORMAL, DUP2_CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL, 
				        "EObjectInfoSucceed: Receive send obj info request again, return to EIdle" );
				}
			break;
		default:
		    OstTrace1( TRACE_ERROR, DUP3_CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL, "wrong iState %d", iState);
			User::Leave(KErrGeneral);
		}
	OstTraceExt2( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL, "Exit with code: 0x%04X, state: %u", (TUint32)responseCode, iState );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CHECKSENDINGSTATEL_EXIT );
	return responseCode;
	}

/**
Validates the data type for a given property code.
@return EMTPRespCodeOK if the parent handle matches the store id, or another MTP response code if not
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckRequestParametersL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	switch (iOperationCode)
		{
		case EMTPOpCodeSendObject:
			{
			OstTrace0( TRACE_NORMAL, DUP1_CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL, 
			        "Check SendObject request parameters" );
			// Check SendObject's session ID
			if (iSessionId != iLastSessionID)
				{
				responseCode = EMTPRespCodeNoValidObjectInfo;
				}
			else if ((iLastTransactionID + 1) != iTransactionCode)
				{
				// Check SendObject's transaction ID
				responseCode = EMTPRespCodeInvalidTransactionID;
				}
			break;
			}

		case EMTPOpCodeSendObjectInfo:
			{
			OstTrace0( TRACE_NORMAL, DUP2_CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL, 
			        "Check SendObjectInfo request parameters" );
			responseCode = CheckStoreAndParent();
			break;
			}
			
		case EMTPOpCodeSendObjectPropList:
			{
			OstTrace0( TRACE_NORMAL, DUP3_CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL, 
			        "Check SendObjectPropList request parameters" );
			responseCode = CheckStoreAndParent();
			if (EMTPRespCodeOK == responseCode)
				{
				// SendObjectPropList need check format code and size in the request
				TUint32 objectSizeHigh = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
				TUint32 objectSizeLow  = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
				iObjectSize = MAKE_TUINT64(objectSizeHigh, objectSizeLow);
				
				iFormatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
				responseCode = CheckFmtAndSetHandler(iFormatCode);
				iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EFormatCode, iFormatCode);
				}
			break;
			}

		case EMTPOpCodeUpdateObjectPropList:
			{
			OstTrace0( TRACE_NORMAL, DUP4_CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL, 
			        "Check UpdateObjectPropList request parameters" );
			TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
			if (objectHandle != KMTPHandleNone)
				{
				// Find the updating object information
				MMTPObjectMgr& objects(iFramework.ObjectMgr());
				if (objects.ObjectL(objectHandle, *iReceivedObjectMetaData))
					{
					iFormatCode = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
					if (iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EDataProviderId) != iFramework.DataProviderId())
						{
						responseCode = EMTPRespCodeInvalidObjectHandle;
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
				responseCode = EMTPRespCodeInvalidObjectHandle;
				}
			break;
			}
			
		default:
			// Unexpected operation code
			responseCode = EMTPRespCodeOperationNotSupported;
			break;
		}
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL, "exit with code: 0x%x", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CHECKREQUESTPARAMETERSL_EXIT );
	return responseCode;
	}

/**
Validates the data type for a given property code.
@return EMTPRespCodeOK if the parent handle matches the store id, or another MTP response code if not
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckStoreAndParent()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CHECKSTOREANDPARENT_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	iParentHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	// If the first parameter is unused, it should be set to 0x00000000, and the responder should decide
	// in which store to place the object
	if (iStorageId == KMTPStorageDefault)
		{
		// If the second parameter is used, the first parameter must also be used.
		// If the second parameter is unused, it should be set to 0x00000000
		if (iParentHandle != KMTPHandleNone)
			{
			responseCode = EMTPRespCodeInvalidParentObject;
			}
		else
			{
			// Set storage id as service dp's logical storage id.
			iStorageId = iDataProvider.StorageId();
			}
		}
	else
		{
		// Check logical storage id.
		if (iStorageId != iDataProvider.StorageId())
			{
			responseCode = EMTPRespCodeInvalidStorageID;
			}
		}
	
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_CHECKSTOREANDPARENT, "Exit with code: 0x%x", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CHECKSTOREANDPARENT_EXIT );
	return responseCode;
	}

/**
SendObjectInfo/SendObjectPropList/UpdateObjectPropList/SendObject request handler
To maintain the state information between the two requests, the two requests are 
combined together in one request processor.
*/
void CMTPSvcCompoundProcessor::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SERVICEL_ENTRY );
	switch (iState)
		{
		case EIdle:
			ServiceObjectPropertiesL();
			break;
		case EObjectInfoSucceed:
			ServiceSendObjectL();
			break;
		default:
			OstTrace0( TRACE_WARNING, CMTPSVCCOMPOUNDPROCESSOR_SERVICEL, "Wrong state in ServiceL" );
			break;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SERVICEL_EXIT );
	}

void CMTPSvcCompoundProcessor::ServiceObjectPropertiesL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SERVICEOBJECTPROPERTIESL_ENTRY );
	switch (iOperationCode)
		{
		case EMTPOpCodeSendObjectInfo:
			ServiceSendObjectInfoL();
			break;
		
		case EMTPOpCodeSendObjectPropList:
		case EMTPOpCodeUpdateObjectPropList:
			ServiceSendObjectPropListL();
			break;
		default:
			break;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SERVICEOBJECTPROPERTIESL_EXIT );
	}

/**
SendObject request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTL_ENTRY );
	MMTPSvcObjectHandler* pHandler = iDataProvider.ObjectHandler(iFormatCode);
	if (pHandler)
		{
		pHandler->GetBufferForSendObjectL(*iReceivedObjectMetaData, &iObjectContent);
		}
	else
		{
        OstTrace1( TRACE_ERROR, CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTL, 
                "can't get object handler for format code %d", iFormatCode);
		User::Leave(KErrGeneral);
		}
	ReceiveDataL(*iObjectContent);
	iState = EObjectSendProcessing;
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTL_EXIT );
	}

/**
SendObjectInfo request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectInfoL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTINFOL_ENTRY );
	delete iObjectInfo;
	iObjectInfo = NULL;
	iObjectInfo = CMTPTypeObjectInfo::NewL();
	ReceiveDataL(*iObjectInfo);
	iState = EObjectInfoProcessing;
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTINFOL_EXIT );
	}

/**
SendObjectPropList request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectPropListL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTPROPLISTL_ENTRY );
	delete iObjectPropList;
	iObjectPropList = NULL;
	iObjectPropList = CMTPTypeObjectPropList::NewL();
	ReceiveDataL(*iObjectPropList);
	iState = EObjectInfoProcessing;
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SERVICESENDOBJECTPROPLISTL_EXIT );
	}

/**
Override to handle the response phase of SendObjectInfo/SendObjectPropList and SendObject requests
@return EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEPHASEL_ENTRY );
	TBool successful = !iCancelled;
	switch (iState)
		{
		case EObjectInfoProcessing:
			{
			if (iOperationCode == EMTPOpCodeSendObjectInfo)
				{
				successful = DoHandleResponseSendObjectInfoL();
				}
			else if (iOperationCode == EMTPOpCodeSendObjectPropList)
				{
				successful = DoHandleResponseSendObjectPropListL();
				}
			else if (iOperationCode == EMTPOpCodeUpdateObjectPropList)
				{
				successful = DoHandleResponseUpdateObjectPropListL();
				}
			iState = (successful ? EObjectInfoSucceed : EIdle);
			break;
			}
		case EObjectSendProcessing:
			{
			successful = DoHandleResponseSendObjectL();
			iState = (successful ? EObjectSendSucceed : EObjectSendFail);
			break;
			}
		default:
			// Wrong State value.
			OstTrace1( TRACE_WARNING, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEPHASEL, 
			        "DoHandleResponsePhaseL enter an abnormal state %d", iState );
			break;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}

/**
Override to handle the completing phase of SendObjectInfo/SendObjectPropList and SendObject requests
@return ETrue if succesfully received the object content, otherwise EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleCompletingPhaseL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLECOMPLETINGPHASEL_ENTRY );
	TBool result = ETrue;
	CMTPRequestProcessor::DoHandleCompletingPhaseL();

	OstTrace1( TRACE_WARNING, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLECOMPLETINGPHASEL, 
	        "DoHandleCompletingPhaseL - Progress State: %u", iState );
	switch (iState)
		{
		case EObjectInfoSucceed:
			{
			// Two cases will come here:
			// 1. SendObjInfo OK, Store ID for next SendObject checking;
			// 2. SendObject check request fail, such as wrong transaction id or wrong session id.
			//    needn't change transaction id.
			if (iOperationCode == EMTPOpCodeSendObjectInfo || 
				iOperationCode == EMTPOpCodeUpdateObjectPropList ||
				iOperationCode == EMTPOpCodeSendObjectPropList)
				{
				// Reset transaction id for new SendObjInfo request, but ignore wrong SendObject.
				iLastTransactionID = iTransactionCode;
				iLastSessionID = iSessionId;
				iLastInfoOperationCode = iOperationCode;
				}
			result = EFalse;
			OstTraceExt2( TRACE_NORMAL, DUP1_CMTPSVCCOMPOUNDPROCESSOR_DOHANDLECOMPLETINGPHASEL, 
			        "EObjectInfoSucceed: Save send info transaction id: %u, operation: 0x%x", iLastTransactionID, iOperationCode );
			
			break;
			}
		case EObjectSendFail:
			{
			// When process SendObject fail, such as received size is wrong.
			iLastTransactionID++;
			iState = EObjectInfoSucceed;
			result = EFalse;
			break;
			}
		default:
			// The other cases will delete the processor:
			// 1. SendObject OK
			// 2. Framework error and call complete with error state.
			// 3. SendObjInfo fail
			// 4. First request is orphan SendObject, state is Idle
			break;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLECOMPLETINGPHASEL_EXIT );
	return result;
	}

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectInfoL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTINFOL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TBool result(ETrue);
	iFormatCode = iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat);
	// Check format code and set object handler
	responseCode = CheckFmtAndSetHandler(iFormatCode);
	if (responseCode != EMTPRespCodeOK)
		{
		SendResponseL(responseCode);
		}
	else
		{
		iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EFormatCode, iFormatCode);
		iObjectSize = iObjectInfo->Uint32L(CMTPTypeObjectInfo::EObjectCompressedSize);
		
		TBuf<KMaxSUIDLength> suid;
		// Object mgr process dataset and create a temp object.
		responseCode = iObjectHandler->SendObjectInfoL(*iObjectInfo, iParentHandle, suid);
		if (responseCode != EMTPRespCodeOK)
			{
			SendResponseL(responseCode);
			}
		else
			{
			//if object size is zero, then directly store object without waiting for sendobject operation.
			if (iObjectSize == 0)
				{
				OstTrace0( TRACE_NORMAL, DUP1_CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTINFOL, 
				        "CommitReservedObject because object size is 0 and register for SendObject" );
				// Commit new temp object to object mgr, if leave, CleanupStack will rollback new temp object. 
				TCleanupItem rollBackTempObject(RollBackObject, this);
				CleanupStack::PushL(rollBackTempObject);
				// Commit prop to obj mgr
				iObjectHandler->CommitForNewObjectL(suid);
				CleanupStack::Pop(this);
	
				// Prepare to store the created object to framework
				iIsRollBackHandlerObject = ETrue;
				TCleanupItem rollBackTempObjectAndSuid(RollBackObject, this);
				CleanupStack::PushL(rollBackTempObjectAndSuid);
				// Set the created suid to meta
				iReceivedObjectMetaData->SetDesCL(CMTPObjectMetaData::ESuid, suid);
				// An object handle issued during a successful SendObjectInfo or SendObjectPropList operation should 
				// be reserved for the duration of the MTP session
				ReserveObjectL();
				// Commit the created object to framework, if leave, then framework will return General Error
				// CleanupStack will rollback the new created object via delete object operation.
				iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObjectMetaData);
				CleanupStack::Pop(this);
				iIsRollBackHandlerObject = EFalse;
				iIsCommited = ETrue;
				RegisterRequestAndSendResponseL(responseCode);
				}
			else
				{
				// An object handle issued during a successful SendObjectInfo or SendObjectPropList operation should 
				// be reserved for the duration of the MTP session
				ReserveObjectL();
				RegisterRequestAndSendResponseL(responseCode);
				}
			}
		}
	result = (responseCode == EMTPRespCodeOK) ? ETrue : EFalse;
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTINFOL, "exit with code: 0x%x", responseCode);
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTINFOL_EXIT );
	return result;
	}

/**
Handling the completing phase of SendObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectPropListL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTPROPLISTL_ENTRY );
	TBool result = ETrue;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	TBuf<KMaxSUIDLength> suid;
	TUint32 parameter = 0;
	responseCode = SendObjectPropListL(*iObjectPropList, iParentHandle, parameter, suid, iObjectSize);
	if (responseCode != EMTPRespCodeOK)
		{
		SendResponseL(responseCode, 4, &parameter);
		}
	else
		{
		//if object size is zero, then directly store object without waiting for sendobject operation.
		if (iObjectSize == 0)
			{
			OstTrace0( TRACE_NORMAL, DUP1_CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTPROPLISTL, 
			        "CommitReservedObject because object size is 0 and register for SendObject" );
			// Commit new temp object to object mgr, if leave, CleanupStack will rollback new temp object. 
			TCleanupItem rollBackTempObject(RollBackObject, this);
			CleanupStack::PushL(rollBackTempObject);
			// Commit prop to obj mgr
			iObjectHandler->CommitForNewObjectL(suid);
			CleanupStack::Pop(this);

			// Prepare to store the created object to framework
			iIsRollBackHandlerObject = ETrue;
			TCleanupItem rollBackTempObjectAndSuid(RollBackObject, this);
			CleanupStack::PushL(rollBackTempObjectAndSuid);
			// Set the created suid to meta
			iReceivedObjectMetaData->SetDesCL(CMTPObjectMetaData::ESuid, suid);
			// An object handle issued during a successful SendObjectInfo or SendObjectPropList operation should 
			// be reserved for the duration of the MTP session
			ReserveObjectL();
			// Commit the created object to framework, if leave, then framework will return General Error
			// CleanupStack will rollback the new created object via delete object operation.
			iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObjectMetaData);
			CleanupStack::Pop(this);
			iIsRollBackHandlerObject = EFalse;
			iIsCommited = ETrue;
			RegisterRequestAndSendResponseL(responseCode);
			}
		else
			{
			// An object handle issued during a successful SendObjectInfo or SendObjectPropList operation should 
			// be reserved for the duration of the MTP session
			ReserveObjectL();
			RegisterRequestAndSendResponseL(responseCode);
			}
		}

	result = (responseCode == EMTPRespCodeOK) ? ETrue : EFalse;
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTPROPLISTL, "exit with code = 0x%x", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTPROPLISTL_EXIT );
	return result;
	}

/**
Handling the completing phase of UpdateObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseUpdateObjectPropListL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEUPDATEOBJECTPROPLISTL_ENTRY );
	TBool result = ETrue;
	TUint32 parameter = 0;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	// Check object size property with format
	responseCode = ExtractObjectSizeL();
	if (responseCode == EMTPRespCodeOK)
		{
		responseCode = UpdateObjectPropListL(*iReceivedObjectMetaData, *iObjectPropList, parameter);
		}

	if (responseCode == EMTPRespCodeOK)
		{
		if (iObjectSize == 0)
			{
			// If commit leave, roll back the temp object.
			TCleanupItem rollBackTempObject(RollBackObject, this);
			CleanupStack::PushL(rollBackTempObject);
			// Commit prop to obj mgr
			iObjectHandler->CommitL();
			CleanupStack::Pop(this);
			// Commit to obj mgr is ok
			iIsCommited = ETrue;
			// Update operation needn't change framework property so far.
			iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
			iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
			}
		else
			{
			iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
			iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
			}
		}
	SendResponseL(responseCode, 1, &parameter);
	result = (responseCode == EMTPRespCodeOK) ? ETrue: EFalse;
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEUPDATEOBJECTPROPLISTL, "exit with code: 0x%x", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSEUPDATEOBJECTPROPLISTL_EXIT );
	return result;
	}

/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TBool result = ETrue;

	if (iCancelled)
		{
		iObjectHandler->RollBack();
		responseCode = EMTPRespCodeTransactionCancelled;
		}
	else if (iObjectSize != 0)
		{
		// For 0 sized object, ignore the object content verify
		TUint64 receiveSize = iObjectContent->Size();
		if (iObjectSize < receiveSize)
			{
			// If the object sent in the data phase of this operation is larger than 
			// the size indicated in the ObjectInfo dataset sent in the SendObjectInfo 
			// which precedes this operation, this operation should fail and a response 
			// code of Store_Full should be returned.
			responseCode = EMTPRespCodeStoreFull;
			}
		else if (iObjectSize > receiveSize)
			{
			responseCode = EMTPRespCodeIncompleteTransfer;
			}
		// If size is ok, then just need commit the object to data store.
		}
	
	// Commit or Unreserver from framework if object size is not 0.
	if (responseCode == EMTPRespCodeOK && iObjectSize != 0)
		{
		// For create new object, need commit the reserved handle to framework, but update needn't do that
		if (iLastInfoOperationCode != EMTPOpCodeUpdateObjectPropList)
			{
			TBuf<KMaxSUIDLength> suid;
			// Commit new temp object to object mgr, if leave, CleanupStack will rollback new temp object. 
			TCleanupItem rollBackTempObject(RollBackObject, this);
			CleanupStack::PushL(rollBackTempObject);
			// Commit prop to obj mgr
			iObjectHandler->CommitForNewObjectL(suid);
			CleanupStack::Pop(this);

			// Prepare to store the created object to framework
			iIsRollBackHandlerObject = ETrue;
			TCleanupItem rollBackTempObjectAndSuid(RollBackObject, this);
			CleanupStack::PushL(rollBackTempObjectAndSuid);
			// Set the created suid to meta
			iReceivedObjectMetaData->SetDesCL(CMTPObjectMetaData::ESuid, suid);
			// Commit the created object to framework, if leave, then framework will return General Error
			// CleanupStack will rollback the new created object via delete object operation.
			iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObjectMetaData);
			CleanupStack::Pop(this);
			iIsRollBackHandlerObject = EFalse;
			iIsCommited = ETrue;
			}
		else
			{
			// If commit leave, roll back the temp object.
			TCleanupItem rollBackNewObject(RollBackObject, this);
			CleanupStack::PushL(rollBackNewObject);
			// Commit prop to obj mgr
			iObjectHandler->CommitL();
			CleanupStack::Pop(this);
			// Commit to obj mgr is ok
			iIsCommited = ETrue;
			}
		}

	SendResponseL(responseCode);
	// Release the processor when SendObject or Transaction Canceled and unregister SendObject.
	result = (responseCode == EMTPRespCodeOK || responseCode == EMTPRespCodeTransactionCancelled) ? ETrue : EFalse;
	if (result)
		{
		iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
		}
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTL, "exit with code = 0x%x", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_DOHANDLERESPONSESENDOBJECTL_EXIT );
	
	return result;
	}

TMTPResponseCode CMTPSvcCompoundProcessor::ExtractObjectSizeL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_EXTRACTOBJECTSIZEL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TBool foundSizeProp  = EFalse;
	const TUint KCount(iObjectPropList->NumberOfElements());
	iObjectPropList->ResetCursor();
	for (TUint i = 0; i < KCount; i++)
		{
		const CMTPTypeObjectPropListElement& KElement = iObjectPropList->GetNextElementL();
		if (EMTPObjectPropCodeObjectSize == KElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode))
			{
			iObjectSize = KElement.Uint64L(CMTPTypeObjectPropListElement::EValue);
			foundSizeProp = ETrue;
			break;
			}
		}
	
	if (!foundSizeProp)
		{
		// Object size in data set is not available, get the corresponding object's current size property.
		const TDesC& suid = iReceivedObjectMetaData->DesC(CMTPObjectMetaData::ESuid);
		responseCode = iObjectHandler->GetObjectSizeL(suid, iObjectSize);
		if (iObjectSize == KObjectSizeNotAvaiable)
			{
			responseCode = EMTPRespCodeGeneralError;
			}
		}

	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_EXTRACTOBJECTSIZEL_EXIT );
	return responseCode;
	}

/**
Reserves space for and assigns an object handle to the received object, then
sends a success response.
*/
void CMTPSvcCompoundProcessor::ReserveObjectL()
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_RESERVEOBJECTL_ENTRY );
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EParentHandle, iParentHandle);
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EFormatCode, iFormatCode);
	iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObjectMetaData, iObjectSize);
	OstTraceExt4( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_RESERVEOBJECTL, "Exit Storage:%u, ParentHandle:%u, FormatCode:%u, Size:%u", iStorageId, iParentHandle, iFormatCode, iObjectSize );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_RESERVEOBJECTL_EXIT );
	}

void CMTPSvcCompoundProcessor::RegisterRequestAndSendResponseL(TMTPResponseCode aResponseCode)
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_REGISTERREQUESTANDSENDRESPONSEL_ENTRY );
	// Register to framework for handle the next sendobj request
	iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
	iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
	TUint32 parameters[3];
	parameters[0] = iStorageId;
	parameters[1] = iParentHandle;
	// Responder’s reserved ObjectHandle for the incoming object
	parameters[2] = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EHandle);
	SendResponseL(aResponseCode, 3, parameters);
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_REGISTERREQUESTANDSENDRESPONSEL_EXIT );
	}

void CMTPSvcCompoundProcessor::RollBackObject(TAny* aObject)
	{
	reinterpret_cast<CMTPSvcCompoundProcessor*>(aObject)->RollBack();
	}

void CMTPSvcCompoundProcessor::RollBack()
	{
	iObjectHandler->RollBack();
	if (iIsRollBackHandlerObject)
		{
		TRAP_IGNORE(iObjectHandler->DeleteObjectL(*iReceivedObjectMetaData));
		iIsRollBackHandlerObject = EFalse;
		}
	}

TMTPResponseCode CMTPSvcCompoundProcessor::CheckFmtAndSetHandler(TUint32 aFormatCode)
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_CHECKFMTANDSETHANDLER_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	iObjectHandler = iDataProvider.ObjectHandler(aFormatCode);
	if (!iObjectHandler)
		{
		responseCode = EMTPRespCodeInvalidObjectFormatCode;
		}
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_CHECKFMTANDSETHANDLER_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPSvcCompoundProcessor::SendObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParentHandle, 
														TUint32& aParameter, TDes& aSuid, TUint64 aObjectSize)
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_SENDOBJECTPROPLISTL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	aParameter = 0;

	responseCode = iObjectHandler->SendObjectPropListL(aObjectSize, aObjectPropList, aParentHandle, aSuid);
	// If handler cache an entry in SendObjectPropList, then it should never return error code. Processor will 
	// not rollback in this case
	if (EMTPRespCodeOK == responseCode)
		{
		// Parse elements and set property for the object.
		const TUint count(aObjectPropList.NumberOfElements());
		aObjectPropList.ResetCursor();
		for (TUint i = 0; i < count && responseCode == EMTPRespCodeOK; i++)
			{
			CMTPTypeObjectPropListElement& element = aObjectPropList.GetNextElementL();
			TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);
			// All ObjectHandle fields must contain the value 0x00000000, and all properties that are defined in 
			// this operation will be applied to the object, need check every handle value and keep all properties is atomic.
			if (handle != KMTPHandleNone)
				{
				responseCode = EMTPRespCodeInvalidDataset;
				aParameter = i;
				break;
				}
			else
				{
				// Create a new object, don't commit, it will be done in processor.
				responseCode = iObjectHandler->SetObjectPropertyL(aSuid, element, EMTPOpCodeSendObjectPropList);
				}
			if (responseCode != EMTPRespCodeOK)
				{
				aParameter = i;
				break;
				}
			}
		// Roll back the temp object
		if (EMTPRespCodeOK != responseCode)
			{
			iObjectHandler->RollBack();
			}
		}
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_SENDOBJECTPROPLISTL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_SENDOBJECTPROPLISTL_EXIT );
	return responseCode;
	}

// All object handlers current don't support partial update, so once update one parameter failed,
// all updated will be reverted.
TMTPResponseCode CMTPSvcCompoundProcessor::UpdateObjectPropListL(CMTPObjectMetaData& aObjectMetaData, 
														const CMTPTypeObjectPropList& aObjectPropList, 
														TUint32& /*aParameter*/)
	{
	OstTraceFunctionEntry0( CMTPSVCCOMPOUNDPROCESSOR_UPDATEOBJECTPROPLISTL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	const TUint count = aObjectPropList.NumberOfElements();
	aObjectPropList.ResetCursor();
	for (TUint i = 0; i < count; i++)
		{
		CMTPTypeObjectPropListElement& element = aObjectPropList.GetNextElementL();
		TUint32 handle = element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle);
		// All object handle in dataset must contain either 0x00000000 or match with the parameter 1
		if (handle != aObjectMetaData.Uint(CMTPObjectMetaData::EHandle) && handle != KMTPHandleNone)
			{
			responseCode = EMTPRespCodeInvalidObjectHandle;
			}
		else
			{
			const TDesC& suid = aObjectMetaData.DesC(CMTPObjectMetaData::ESuid);
			// Update will be treated as adding a new object for RO object property.
			responseCode = iObjectHandler->SetObjectPropertyL(suid, element, EMTPOpCodeUpdateObjectPropList);
			}
		if(EMTPRespCodeOK != responseCode)
			{
			iObjectHandler->RollBack();
			break;
			}
		}
	OstTrace1( TRACE_NORMAL, CMTPSVCCOMPOUNDPROCESSOR_UPDATEOBJECTPROPLISTL, "Exit with responseCode = 0x%04X", responseCode );
	OstTraceFunctionExit0( CMTPSVCCOMPOUNDPROCESSOR_UPDATEOBJECTPROPLISTL_EXIT );
	return responseCode;
	}
