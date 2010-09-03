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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpsvccompoundprocessor.h


/**
 @file
 @internalComponent
 */

#ifndef __CMTPSVCCOMPOUNDPROCESSOR_H__
#define __CMTPSVCCOMPOUNDPROCESSOR_H__

#include "cmtprequestprocessor.h"

class MMTPServiceDataProvider;
class MMTPSvcObjectHandler;
class CMTPTypeObjectInfo;
class CMTPTypeObjectPropList;
class CMTPObjectMetaData;

/** 
Implements the service data provider's CMTPCompoundProcessor request processor.
The processor is responsible for SendObjectInfo, SendObjectPropList, UpdateObjectPropList 
and SendObject requests handling, it will create new object or update existing object. 
@internalComponent
*/
class CMTPSvcCompoundProcessor : public CMTPRequestProcessor
	{
public:
	IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);    
	IMPORT_C ~CMTPSvcCompoundProcessor();

private:
	CMTPSvcCompoundProcessor(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, MMTPServiceDataProvider& aDataProvider);
	void ConstructL();

	// From CMTPRequestProcessor
	TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
	TBool HasDataphase() const;
	
	TMTPResponseCode CheckRequestL();
	TMTPResponseCode CheckSendingStateL();
	TMTPResponseCode CheckRequestParametersL();
	TMTPResponseCode CheckStoreAndParent();
	
	void ServiceL();
	void ServiceObjectPropertiesL();
	void ServiceSendObjectInfoL();
	void ServiceSendObjectPropListL();
	void ServiceSendObjectL();
	
	TBool DoHandleCompletingPhaseL();
	TBool DoHandleResponsePhaseL();
	TBool DoHandleResponseSendObjectInfoL();
	TBool DoHandleResponseSendObjectPropListL();
	TBool DoHandleResponseUpdateObjectPropListL();
	TBool DoHandleResponseSendObjectL();
	
	/**
	Extract objectsize property value from UpdateObjectPropList dataset
	@return See MTP response code definition
	*/
	TMTPResponseCode ExtractObjectSizeL();
	/**
	Reserve object handle from framework for SendObjectInfo or SendObjectPropList
	*/
	void ReserveObjectL();
	/**
	Register route next SendObject from framework and send response code
	*/
	void RegisterRequestAndSendResponseL(TMTPResponseCode aResponseCode);
	
	// Cleanup Item operation for commit rollback including handle and temp object rollback
	static void RollBackObject(TAny* aObject);
	void RollBack();
	
	// Check object format and set object handler pointer
	TMTPResponseCode CheckFmtAndSetHandler(TUint32 aFormatCode);
	TMTPResponseCode SendObjectPropListL(const CMTPTypeObjectPropList& aObjectPropList, TUint32& aParentHandle, 
															TUint32& aParameter, TDes& aSuid, TUint64 aObjectSize);
	TMTPResponseCode UpdateObjectPropListL(CMTPObjectMetaData& aObjectMetaData, 
															const CMTPTypeObjectPropList& aObjectPropList, 
															TUint32& aParameter);

private:
	enum TMTPSendingObjectState
		{
		EIdle,
		EObjectInfoProcessing,
		EObjectInfoSucceed,
		EObjectSendProcessing,
		EObjectSendSucceed,
		EObjectSendFail
		};

	TUint16                      iOperationCode;   // Current operation code
	TUint16                      iLastInfoOperationCode;   // Last info category operation code
	TUint32                      iFormatCode;
	TUint32                      iStorageId;
	TUint32                      iParentHandle;
	TUint64                      iObjectSize;
	
	MMTPServiceDataProvider&     iDataProvider;
	MMTPSvcObjectHandler*        iObjectHandler; // Not own
	MMTPType*                    iObjectContent; // Not own
	CMTPTypeObjectInfo*          iObjectInfo;
	CMTPTypeObjectPropList*      iObjectPropList;
	CMTPObjectMetaData*          iReceivedObjectMetaData;

	TMTPSendingObjectState       iState; // Compound processor state machine
	TMTPTypeRequest              iExpectedSendObjectRequest; // Use for register next SendObject
	TUint32                      iLastTransactionID; // Last valid transaction id
	TUint32                      iLastSessionID; // Last valid session id
	TBool                        iIsCommited; // Is received ObjectInfo or ObjectPropList commited
	TBool                        iIsRollBackHandlerObject; // Roll back handle which has been insert in handler db
	};

#endif // __CMTPSVCCOMPOUNDPROCESSOR_H__

