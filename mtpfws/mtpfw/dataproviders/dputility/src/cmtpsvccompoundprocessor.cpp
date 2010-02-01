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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcCompound");)

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
	__FLOG(_L8("~CMTPSvcCompoundProcessor - Entry"));
	delete iReceivedObjectMetaData;
	delete iObjectInfo;
	delete iObjectPropList;
	__FLOG(_L8("~CMTPSvcCompoundProcessor - Exit"));
	__FLOG_CLOSE; 
	}

CMTPSvcCompoundProcessor::CMTPSvcCompoundProcessor(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider), iState(EIdle), iIsCommited(EFalse), iIsRollBackHandlerObject(EFalse)
	{
	}

void CMTPSvcCompoundProcessor::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iExpectedSendObjectRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, EMTPOpCodeSendObject);
	iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
	__FLOG(_L8("ConstructL - Exit"));
	}

/**
Override to match both the SendObjectInfo/SendObjectPropList/UpdateObjectPropList and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/
TBool CMTPSvcCompoundProcessor::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
	{
	__FLOG(_L8("Match - Entry"));
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
	__FLOG(_L8("Match - Exit"));
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
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		responseCode = CheckSendingStateL();
		if (EMTPRespCodeOK == responseCode)
			{
			responseCode = CheckRequestParametersL();
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with code: 0x%04X"), responseCode));
	return responseCode;
	}

/**
Verify if the SendObject request comes after SendObjectInfo/SendObjectPropList request
@return EMTPRespCodeOK if SendObject request comes after a valid SendObjectInfo request, otherwise
EMTPRespCodeNoValidObjectInfo
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckSendingStateL()
	{
	__FLOG(_L8("CheckSendingStateL - Entry"));
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
				__FLOG(_L8("EIdle: Received an orphan SendObject request"));
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
				__FLOG(_L8("EObjectInfoSucceed: Receive send obj info request again, return to EIdle"));
				}
			break;
		default:
			User::Leave(KErrGeneral);
		}
	__FLOG_VA((_L8("CheckSendingStateL - Exit with code: 0x%04X, state: %u"), responseCode, iState));
	return responseCode;
	}

/**
Validates the data type for a given property code.
@return EMTPRespCodeOK if the parent handle matches the store id, or another MTP response code if not
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckRequestParametersL()
	{
	__FLOG(_L8("CheckRequestParametersL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	switch (iOperationCode)
		{
		case EMTPOpCodeSendObject:
			{
			__FLOG(_L8("Check SendObject request parameters"));
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
			__FLOG(_L8("Check SendObjectInfo request parameters"));
			responseCode = CheckStoreAndParent();
			break;
			}
			
		case EMTPOpCodeSendObjectPropList:
			{
			__FLOG(_L8("Check SendObjectPropList request parameters"));
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
			__FLOG(_L8("Check UpdateObjectPropList request parameters"));
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
	__FLOG_VA((_L8("CheckRequestParametersL exit with code: 0x%x"), responseCode));
	return responseCode;
	}

/**
Validates the data type for a given property code.
@return EMTPRespCodeOK if the parent handle matches the store id, or another MTP response code if not
*/
TMTPResponseCode CMTPSvcCompoundProcessor::CheckStoreAndParent()
	{
	__FLOG(_L8("CheckStoreAndParent - Entry"));
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
	
	__FLOG_VA((_L8("CheckStoreAndParent - Exit with code: 0x%x"), responseCode));
	return responseCode;
	}

/**
SendObjectInfo/SendObjectPropList/UpdateObjectPropList/SendObject request handler
To maintain the state information between the two requests, the two requests are 
combined together in one request processor.
*/
void CMTPSvcCompoundProcessor::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	switch (iState)
		{
		case EIdle:
			ServiceObjectPropertiesL();
			break;
		case EObjectInfoSucceed:
			ServiceSendObjectL();
			break;
		default:
			__FLOG(_L8("Wrong state in ServiceL"));
			break;
		}
	__FLOG(_L8("ServiceL - Exit"));
	}

void CMTPSvcCompoundProcessor::ServiceObjectPropertiesL()
	{
	__FLOG(_L8("ServiceObjectPropertiesL - Entry"));
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
	__FLOG(_L8("ServiceObjectPropertiesL - Exit"));
	}

/**
SendObject request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectL()
	{
	__FLOG(_L8("ServiceSendObjectL - Entry"));
	MMTPSvcObjectHandler* pHandler = iDataProvider.ObjectHandler(iFormatCode);
	if (pHandler)
		{
		pHandler->GetBufferForSendObjectL(*iReceivedObjectMetaData, &iObjectContent);
		}
	else
		{
		User::Leave(KErrGeneral);
		}
	ReceiveDataL(*iObjectContent);
	iState = EObjectSendProcessing;
	__FLOG(_L8("ServiceSendObjectL - Exit"));
	}

/**
SendObjectInfo request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectInfoL()
	{
	__FLOG(_L8("ServiceSendObjectInfoL - Entry"));
	delete iObjectInfo;
	iObjectInfo = NULL;
	iObjectInfo = CMTPTypeObjectInfo::NewL();
	ReceiveDataL(*iObjectInfo);
	iState = EObjectInfoProcessing;
	__FLOG(_L8("ServiceSendObjectInfoL - Exit"));
	}

/**
SendObjectPropList request handler
*/
void CMTPSvcCompoundProcessor::ServiceSendObjectPropListL()
	{
	__FLOG(_L8("ServiceSendObjectPropListL - Entry"));
	delete iObjectPropList;
	iObjectPropList = NULL;
	iObjectPropList = CMTPTypeObjectPropList::NewL();
	ReceiveDataL(*iObjectPropList);
	iState = EObjectInfoProcessing;
	__FLOG(_L8("ServiceSendObjectPropListL - Exit"));
	}

/**
Override to handle the response phase of SendObjectInfo/SendObjectPropList and SendObject requests
@return EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
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
			__FLOG_VA((_L8("DoHandleResponsePhaseL enter an abnormal state %d"), iState));
			break;
		}
	__FLOG(_L8("DoHandleResponsePhaseL - Exit"));
	return EFalse;
	}

/**
Override to handle the completing phase of SendObjectInfo/SendObjectPropList and SendObject requests
@return ETrue if succesfully received the object content, otherwise EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleCompletingPhaseL()
	{
	__FLOG(_L8("DoHandleCompletingPhaseL - Entry"));
	TBool result = ETrue;
	CMTPRequestProcessor::DoHandleCompletingPhaseL();
	
	__FLOG_VA((_L8("DoHandleCompletingPhaseL - Progress State: %u"), iState));
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
			__FLOG_VA((_L8("EObjectInfoSucceed: Save send info transaction id: %u, operation: 0x%x"), iLastTransactionID, iOperationCode));
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
	__FLOG(_L8("DoHandleCompletingPhaseL - Exit"));
	return result;
	}

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectInfoL()
	{
	__FLOG(_L8("DoHandleResponseSendObjectInfoL - Entry"));
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
				__FLOG(_L8("CommitReservedObject because object size is 0 and register for SendObject"));
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
	__FLOG_VA((_L8("DoHandleResponseSendObjectInfoL exit with code: 0x%x"), responseCode));
	return result;
	}

/**
Handling the completing phase of SendObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectPropListL()
	{
	__FLOG(_L8("DoHandleResponseSendObjectPropListL - Entry"));
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
			__FLOG(_L8("CommitReservedObject because object size is 0 and register for SendObject"));
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
	__FLOG_VA((_L8("DoHandleResponseSendObjectPropListL exit with code = 0x%x"), responseCode));
	return result;
	}

/**
Handling the completing phase of UpdateObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseUpdateObjectPropListL()
	{
	__FLOG(_L8("DoHandleResponseUpdateObjectPropListL - Entry"));
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
	__FLOG_VA((_L8("DoHandleResponseUpdateObjectPropListL exit with code: 0x%x"), responseCode));
	return result;
	}

/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/
TBool CMTPSvcCompoundProcessor::DoHandleResponseSendObjectL()
	{
	__FLOG(_L8("DoHandleResponseSendObjectL - Entry"));
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
	__FLOG_VA((_L8("DoHandleResponseSendObjectL exit with code = 0x%x"), responseCode));
	return result;
	}

TMTPResponseCode CMTPSvcCompoundProcessor::ExtractObjectSizeL()
	{
	__FLOG(_L8("ExtractObjectSizeL - Entry"));
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

	__FLOG(_L8("ExtractObjectSizeL - Exit"));
	return responseCode;
	}

/**
Reserves space for and assigns an object handle to the received object, then
sends a success response.
*/
void CMTPSvcCompoundProcessor::ReserveObjectL()
	{
	__FLOG(_L8("ReserveObjectL - Entry"));
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EParentHandle, iParentHandle);
	iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EFormatCode, iFormatCode);
	iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObjectMetaData, iObjectSize);
	__FLOG_VA((_L8("ReserveObjectL Exit Storage:%u, ParentHandle:%u, FormatCode:%u, Size:%u "), iStorageId, iParentHandle, iFormatCode, iObjectSize));
	}

void CMTPSvcCompoundProcessor::RegisterRequestAndSendResponseL(TMTPResponseCode aResponseCode)
	{
	__FLOG(_L8("RegisterRequestAndSendResponseL - Entry"));
	// Register to framework for handle the next sendobj request
	iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
	iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
	TUint32 parameters[3];
	parameters[0] = iStorageId;
	parameters[1] = iParentHandle;
	// Responder’s reserved ObjectHandle for the incoming object
	parameters[2] = iReceivedObjectMetaData->Uint(CMTPObjectMetaData::EHandle);
	SendResponseL(aResponseCode, 3, parameters);
	__FLOG(_L8("RegisterRequestAndSendResponseL - Exit"));
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

TMTPResponseCode CMTPSvcCompoundProcessor::SendObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParentHandle, 
														TUint32& aParameter, TDes& aSuid, TUint64 aObjectSize)
	{
	__FLOG(_L8("SendObjectPropListL - Entry"));
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
	__FLOG_VA((_L8("SendObjectPropListL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

// All object handlers current don't support partial update, so once update one parameter failed,
// all updated will be reverted.
TMTPResponseCode CMTPSvcCompoundProcessor::UpdateObjectPropListL(CMTPObjectMetaData& aObjectMetaData, 
														const CMTPTypeObjectPropList& aObjectPropList, 
														TUint32& /*aParameter*/)
	{
	__FLOG(_L8("UpdateObjectPropList - Entry")); 
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
	__FLOG_VA((_L8("UpdateObjectPropListL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}
