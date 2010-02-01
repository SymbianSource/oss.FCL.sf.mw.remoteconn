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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpsvcdeleteobjectproplist.cpp

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypedeleteobjectproplist.h>
#include <mtp/tmtptypedatapair.h>

#include "cmtpsvcdeleteobjectproplist.h"
#include "mmtpservicedataprovider.h"
#include "mmtpsvcobjecthandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SvcDelObjPropList");)

EXPORT_C MMTPRequestProcessor* CMTPSvcDeleteObjectPropList::NewL(MMTPDataProviderFramework& aFramework, 
													MMTPConnection& aConnection, 
													MMTPServiceDataProvider& aDataProvider)
	{
	CMTPSvcDeleteObjectPropList* self = new (ELeave) CMTPSvcDeleteObjectPropList(aFramework, aConnection, aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPSvcDeleteObjectPropList::~CMTPSvcDeleteObjectPropList()
	{
	__FLOG(_L8("~CMTPSvcDeleteObjectPropList - Entry"));
	delete iPropertyList;
	delete iReceivedObjectMetaData;
	__FLOG(_L8("~CMTPSvcDeleteObjectPropList - Exit"));
	__FLOG_CLOSE;
	}

CMTPSvcDeleteObjectPropList::CMTPSvcDeleteObjectPropList(MMTPDataProviderFramework& aFramework, 
												MMTPConnection& aConnection, 
												MMTPServiceDataProvider& aDataProvider) :
	CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider)
	{
	}

void CMTPSvcDeleteObjectPropList::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	iPropertyList = CMTPTypeDeleteObjectPropList::NewL();
	__FLOG(_L8("ConstructL - Exit"));
	}

TMTPResponseCode CMTPSvcDeleteObjectPropList::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK == responseCode)
		{
		TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		iReceivedObjectMetaData = CMTPObjectMetaData::NewL();
		// Check object handle
		if (objectHandle != KMTPHandleAll)
			{
			MMTPObjectMgr& objMgr(iFramework.ObjectMgr());
			// Check whether object handle is valid
			if (objMgr.ObjectL(objectHandle, *iReceivedObjectMetaData))
				{
				// Check whether the owner of this object is correct service data provider
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
		else
			{
			iReceivedObjectMetaData->SetUint(CMTPObjectMetaData::EHandle, KMTPHandleAll);
			}
		}
	__FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

void CMTPSvcDeleteObjectPropList::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	ReceiveDataL(*iPropertyList);
	__FLOG(_L8("ServiceL - Exit"));
	}

TBool CMTPSvcDeleteObjectPropList::DoHandleResponsePhaseL()
	{
	__FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TUint32 parameter(0);
	TMTPResponseCode responseCode = EMTPRespCodeOK;

	responseCode = DeleteObjectPropListL(*iReceivedObjectMetaData, *iPropertyList, parameter);
	
	SendResponseL(responseCode, 1, &parameter);
	__FLOG_VA((_L8("DeleteObjectPropListL responseCode = 0x%x, failed index = %u"), responseCode, parameter));
	return EFalse;
	}

TBool CMTPSvcDeleteObjectPropList::HasDataphase() const
	{
	return ETrue;
	}

TMTPResponseCode CMTPSvcDeleteObjectPropList::DeleteObjectPropListL(CMTPObjectMetaData& aObjectMetaData, 
														const CMTPTypeDeleteObjectPropList& aPropList, 
														TUint32& aParameter)
	{
	__FLOG(_L8("DeleteObjectPropList- Entry")); 
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint handle = aObjectMetaData.Uint(CMTPObjectMetaData::EHandle);
	MMTPSvcObjectHandler* lastHandler = NULL; 
	TUint errIndex = 0;
	
	if (handle == KMTPHandleAll)
		{
		TUint32 lastHandle = 0;
		// In this case, all object handles specified in the dataset, can be multiple objects
		const TUint count(aPropList.NumberOfElements());
		CMTPObjectMetaData* objectMetaData = CMTPObjectMetaData::NewL();
		CleanupStack::PushL(objectMetaData);
		for (TUint i = 0; i < count; i++)
			{
			TMTPTypeDataPair& element = aPropList.ElementL(i);
			TUint32 handle = element.Uint32(TMTPTypeDataPair::EOwnerHandle);
			// Get object handle every time for multiple objects.
			if (iFramework.ObjectMgr().ObjectL(handle, *objectMetaData))
				{
				TUint16 formatCode = objectMetaData->Uint(CMTPObjectMetaData::EFormatCode);
				TUint16 propCode = element.Uint16(TMTPTypeDataPair::EDataCode);
				MMTPSvcObjectHandler* pHandler = iDataProvider.ObjectHandler(formatCode);
				if (pHandler)
					{
					// If the handler is not the last handle, need commit all properties now
					if (lastHandle != 0 && lastHandler && lastHandle != handle)
						{
						// Performance can be improved here
						TRAPD(err, lastHandler->CommitL());
						if (KErrNone != err)
							{
							lastHandler->RollBack();
							responseCode = EMTPRespCodeInvalidObjectHandle;
							aParameter = errIndex;
							break;
							}
						// Record the next segment's first index number
						errIndex = i;
						}
					lastHandler = pHandler;
					lastHandle = handle;
					responseCode = pHandler->DeleteObjectPropertyL(*objectMetaData, propCode);
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
			if (responseCode != EMTPRespCodeOK)
				{
				aParameter = i;
				// All properties prior to the failed property will be updated.
				if (i != errIndex && lastHandler)
					{
					TRAPD(err, lastHandler->CommitL());
					if (KErrNone != err)
						{
						lastHandler->RollBack();
						aParameter = errIndex;
						}
					}
				break;
				}
			}
		CleanupStack::PopAndDestroy(objectMetaData);
		}
	else 
		{
		// The element object handle fields should be either 0x00000000 or match with parameter 1. 
		TUint16 formatCode = aObjectMetaData.Uint(CMTPObjectMetaData::EFormatCode);
		const TUint count(aPropList.NumberOfElements());
		for (TUint i = 0; i < count; i++)
			{
			TMTPTypeDataPair& element = aPropList.ElementL(i);
			TUint32 handle = element.Uint32(TMTPTypeDataPair::EOwnerHandle);	
			// Check dataset according to spec
			if (handle != aObjectMetaData.Uint(CMTPObjectMetaData::EHandle) && handle != KMTPHandleNone)
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				}
			else
				{
				TUint16 propCode = element.Uint16(TMTPTypeDataPair::EDataCode);
				MMTPSvcObjectHandler* pHandler = iDataProvider.ObjectHandler(formatCode);
				if (pHandler)
					{
					responseCode = pHandler->DeleteObjectPropertyL(aObjectMetaData, propCode);
					if (!lastHandler)
						{
						lastHandler = pHandler;
						}
					}
				else
					{
					responseCode = EMTPRespCodeInvalidObjectHandle;
					}
				}
			if (responseCode != EMTPRespCodeOK)
				{
				aParameter = i;
				if (i != 0 && lastHandler)
					{
					TRAPD(err, lastHandler->CommitL());
					if (KErrNone != err)
						{
						lastHandler->RollBack();
						aParameter = errIndex;
						}
					}
				break;
				}
			}
		}
	// Commint for each case
	if (responseCode == EMTPRespCodeOK && lastHandler)
		{
		TRAPD(err, lastHandler->CommitL());
		if (KErrNone != err)
			{
			lastHandler->RollBack();
			responseCode = EMTPRespCodeInvalidObjectHandle;
			aParameter = errIndex;
			}
		}
	__FLOG_VA((_L8("DeleteObjectPropList - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}
