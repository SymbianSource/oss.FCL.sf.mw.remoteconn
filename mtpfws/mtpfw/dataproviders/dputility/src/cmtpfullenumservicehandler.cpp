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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpfullenumservicehandler.cpp

#include <centralrepository.h>
#include <mtp/cmtptypearray.h>

#include <mtp/cmtptypeformatcapabilitylist.h>
#include <mtp/cmtptypeservicepropdesclist.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>

#include "mtpsvcdpconst.h"
#include "cmtpfullenumservicehandler.h"
#include "cmtpabstractdatacodemgr.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent, "FullEnumServiceHandler");)

EXPORT_C CMTPFullEnumServiceHandler* CMTPFullEnumServiceHandler::NewL(MMTPDataProviderFramework& aFramework,
		const CMTPFullEnumDataCodeMgr& aDataCodeMgr,
		CRepository& aRepository,
		TUint aNormalServiceID,
		const TDesC& aKnowledgeObjectSUID,
		const TMTPTypeGuid& aServiceFormatGUID)
	{
	CMTPFullEnumServiceHandler* self = new(ELeave) CMTPFullEnumServiceHandler(aFramework, aDataCodeMgr,
			aRepository, aNormalServiceID,
			aKnowledgeObjectSUID, aServiceFormatGUID);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPFullEnumServiceHandler::~CMTPFullEnumServiceHandler()
	{
	__FLOG(_L8("~CMTPFullEnumServiceHandler - Destructed"));
	__FLOG_CLOSE;
	}

CMTPFullEnumServiceHandler::CMTPFullEnumServiceHandler(MMTPDataProviderFramework& aFramework,
		const CMTPFullEnumDataCodeMgr& aDataCodeMgr, CRepository& aRepository,
		TUint aNormalServiceID, const TDesC& aKnowledgeObjectSUID,
		const TMTPTypeGuid& aServiceFormatGUID) :
		iFramework(aFramework), iDataCodeMgr(aDataCodeMgr), iRepository(aRepository),
		iNormalServiceID(aNormalServiceID), iKnowledgeObjectSUID(aKnowledgeObjectSUID)
	{
	iNormalServiceFormatGUID = aServiceFormatGUID;
	}

void CMTPFullEnumServiceHandler::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	// Initialize the service properties's value stored in iRepository
	LoadServicePropValueL();
	__FLOG(_L8("ConstructL - Exit"));
	}

// GetServiceCapabilities
EXPORT_C TMTPResponseCode CMTPFullEnumServiceHandler::GetServiceCapabilityL(
	TUint16 aServiceFormatCode, CMTPTypeServiceCapabilityList& aServiceCapabilityList) const
	{
	__FLOG(_L8("GetServiceCapabilitiesL - Entry"));

	__ASSERT_DEBUG((aServiceFormatCode == iDataCodeMgr.KnowledgeFormat().iFormatCode), User::Invariant());

	//only Knowledge format supported
	CMTPTypeFormatCapability* element = CMTPTypeFormatCapability::NewLC(aServiceFormatCode, NULL);

	//only Generic Object namespace properties supported
	const RMTPServiceFormat& serviceFormat = iDataCodeMgr.KnowledgeFormat();
	TUint propCout = serviceFormat.iProps.Count();
	for (TUint i = 0; i < propCout; i++)
		{
		//only Generic Object namespace properties supported
		CMTPTypeObjectPropDesc* objectProperty
		= MMTPServiceHandler::GenerateGenericObjectPropDescLC(serviceFormat.iProps[i]);
		if (objectProperty)
			{
			element->AppendL(objectProperty);
			CleanupStack::Pop(objectProperty);
			}
		} // End of loop for every format
	aServiceCapabilityList.AppendL(element);
	CleanupStack::Pop(element);
	__FLOG(_L8("GetServiceCapabilitiesL - Exit"));
	return EMTPRespCodeOK;
	}

// GetServicePropDesc
EXPORT_C TMTPResponseCode CMTPFullEnumServiceHandler::GetServicePropDescL(
	TUint16 aServicePropertyCode, CMTPTypeServicePropDescList& aPropDescList) const
	{
	__FLOG(_L8("GetServicePropDescL - Entry"));
	TMTPResponseCode respCode(EMTPRespCodeOK);
	CMTPTypeServicePropDesc* servicePropDesc = NULL;
	const TMTPServicePropertyInfo* pPropInfo = iDataCodeMgr.ServicePropertyInfo(aServicePropertyCode);
	TUint16 servicePropEnum = pPropInfo ? pPropInfo->iIndex : EMTPAbstractServicePropertyEnd;
	switch (servicePropEnum)
		{
		case EMTPServicePropertyVersionProps:
			{
			const TUint32 KMaxNumVersionProps = 0x000000FF;
			TMTPTypeUint32 expectedForm(KMaxNumVersionProps);
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeAUINT8,
								  CMTPTypeObjectPropDesc::EReadOnly,
								  CMTPTypeObjectPropDesc::EByteArrayForm, // Form tag
								  &expectedForm);
			break;
			}

		case EMTPServicePropertyReplicaID:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT128,
								  CMTPTypeObjectPropDesc::EReadWrite,
								  CMTPTypeObjectPropDesc::ENone,
								  NULL);
			break;
			}

		case EMTPServicePropertyKnowledgeObjectID:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT32,
								  CMTPTypeObjectPropDesc::EReadOnly,
								  CMTPTypeObjectPropDesc::EObjectIDForm,
								  NULL);
			break;
			}
		case EMTPServicePropertyLastSyncProxyID:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT128,
								  CMTPTypeObjectPropDesc::EReadWrite,
								  CMTPTypeObjectPropDesc::ENone,
								  NULL);
			break;
			}
		case EMTPServicePropertyProviderVersion:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT16,
								  CMTPTypeObjectPropDesc::EReadOnly,
								  CMTPTypeObjectPropDesc::ENone,
								  NULL);
			break;
			}
		case EMTPServicePropertySyncFormat:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT128,
								  CMTPTypeObjectPropDesc::EReadOnly,
								  CMTPTypeObjectPropDesc::ENone,
								  NULL);
			break;
			}
		case EMTPServicePropertyLocalOnlyDelete:
			{
			CMTPTypeObjectPropDescEnumerationForm* expectedForm
			= CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT8);
			CleanupStack::PushL(expectedForm);
			expectedForm->AppendSupportedValueL(TMTPTypeUint8(EMTPLocalOnlyDeleteFalse));
			expectedForm->AppendSupportedValueL(TMTPTypeUint8(EMTPLocalOnlyDeleteTrue));
			servicePropDesc = CMTPTypeServicePropDesc::NewL(
								  aServicePropertyCode,
								  EMTPTypeUINT8,
								  CMTPTypeObjectPropDesc::EReadWrite,
								  CMTPTypeObjectPropDesc::EEnumerationForm,
								  expectedForm);
			// Form can be NULL, so need destroy here for MTPType object here.
			CleanupStack::PopAndDestroy(expectedForm);
			CleanupStack::PushL(servicePropDesc);
			break;
			}

		case EMTPServicePropertyFilterType:
			{
			servicePropDesc = CMTPTypeServicePropDesc::NewLC(
								  aServicePropertyCode,
								  EMTPTypeUINT8,
								  CMTPTypeObjectPropDesc::EReadWrite,
								  CMTPTypeObjectPropDesc::ENone,
								  NULL);
			break;
			}
		case EMTPServicePropertySyncObjectReferences:
			{
			CMTPTypeObjectPropDescEnumerationForm* expectedForm
			    = CMTPTypeObjectPropDescEnumerationForm::NewL(EMTPTypeUINT8);
			CleanupStack::PushL(expectedForm);
			expectedForm->AppendSupportedValueL(TMTPTypeUint8(EMTPSyncObjectReferencesDisabled));
			expectedForm->AppendSupportedValueL(TMTPTypeUint8(EMTPSyncObjectReferencesEnabled));
			servicePropDesc = CMTPTypeServicePropDesc::NewL(
								  aServicePropertyCode,
								  EMTPTypeUINT8,
								  CMTPTypeObjectPropDesc::EReadOnly,
								  CMTPTypeObjectPropDesc::EEnumerationForm,
								  expectedForm);
			// Form can be NULL, so need destroy here for MTPType object here.
			CleanupStack::PopAndDestroy(expectedForm);
			CleanupStack::PushL(servicePropDesc);
			break;
			}

		default:
			respCode = EMTPRespCodeParameterNotSupported;
			break;
		}

	if (servicePropDesc)
		{
		aPropDescList.AppendL(servicePropDesc);
		CleanupStack::Pop(servicePropDesc);
		}
	__FLOG(_L8("GetServicePropDescL - Exit"));
	return respCode;
	}

// Get specific property of the service
EXPORT_C TMTPResponseCode CMTPFullEnumServiceHandler::GetServicePropetyL(TUint16 aPropertyCode, CMTPTypeServicePropList& aPropList) const
	{
	__FLOG(_L8("GetServicePropetyL - Entry"));

	TMTPResponseCode responseCode = EMTPRespCodeOK;
	const TMTPServicePropertyInfo* pPropInfo = iDataCodeMgr.ServicePropertyInfo(aPropertyCode);
	TUint16 servicePropEnum = pPropInfo ? pPropInfo->iIndex : EMTPAbstractServicePropertyEnd;
	CMTPTypeServicePropListElement* propertyElement = NULL;
	switch (servicePropEnum)
		{
		case EMTPServicePropertyVersionProps:
			{
			RArray<TUint> elementData;
			CleanupClosePushL(elementData);
			const TUint values[] =
				{
				// Change Unit Count
				0x01, 0x00, 0x00, 0x00,
				// Change Unit 1
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				// Count for Change Unit 1
				0x01, 0x00, 0x00, 0x00,
				// Version Prop DateModified Namespace and PID
				0x0D, 0x49, 0x6B, 0xEF, 0xD8, 0x5C, 0x7A, 0x43, 0xAF, 0xFC, 0xDA, 0x8B, 0x60, 0xEE, 0x4A, 0x3C,
				0x28, 0x00, 0x00, 0x00
				};
			TUint numValues((sizeof(values) / sizeof(values[0])));
			for (TUint i = 0; i < numValues; i++)
				{
				elementData.AppendL(values[i]);
				}

			CMTPTypeArray* versionProp = CMTPTypeArray::NewLC(EMTPTypeAUINT8, elementData);
			propertyElement = CMTPTypeServicePropListElement::NewL(iNormalServiceID, aPropertyCode,
							  EMTPTypeAUINT8, *versionProp);
			CleanupStack::PopAndDestroy(versionProp);
			CleanupStack::PopAndDestroy(&elementData);
			CleanupStack::PushL(propertyElement);
			break;
			}

		case EMTPServicePropertyReplicaID:
			{
			const TMTPTypeGuid unInitValue(MAKE_TUINT64(KMTPUnInitialized32, KMTPUnInitialized32),
											  MAKE_TUINT64(KMTPUnInitialized32, KMTPUnInitialized32));
			if (!unInitValue.Equal(iReplicateID))
				{
				propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
								  EMTPTypeUINT128, iReplicateID);
				}
			break;
			}

		case EMTPServicePropertyKnowledgeObjectID:
			{
			CMTPObjectMetaData* object = CMTPObjectMetaData::NewLC();
			if (iFramework.ObjectMgr().ObjectL(iKnowledgeObjectSUID, *object))
				{
				// Use knowledge object's handle as service property KnowledgeObjectId.
				TUint knowledgeObjectID = object->Uint(CMTPObjectMetaData::EHandle);
				CleanupStack::PopAndDestroy(object);
				TMTPTypeUint32 objId;
				objId.Set(knowledgeObjectID);
				propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
								  EMTPTypeUINT32, objId);
				}
			else
				{
				CleanupStack::PopAndDestroy(object);
				}
			break;
			}
		case EMTPServicePropertyLastSyncProxyID:
			{
			const TMTPTypeGuid unInitValue(MAKE_TUINT64(KMTPUnInitialized32, KMTPUnInitialized32),
											  MAKE_TUINT64(KMTPUnInitialized32, KMTPUnInitialized32));
			if (!unInitValue.Equal(iLastSyncProxyID))
				{
				propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
								  EMTPTypeUINT128, iLastSyncProxyID);
				}
			break;
			}
		case EMTPServicePropertyProviderVersion:
			{
			TMTPTypeUint16 version(KMTPDefaultProviderVersion);
			propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
							  EMTPTypeUINT16, version);
			break;
			}
		case EMTPServicePropertySyncFormat:
			{
			propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
							  EMTPTypeUINT128, iNormalServiceFormatGUID);
			break;
			}
		case EMTPServicePropertyLocalOnlyDelete:
			{
			if (EMTPLocalOnlyDeleteUnInitialized != iLocalOnlyDelete)
				{
				TMTPTypeUint8 localOnlyDelete(iLocalOnlyDelete);
				propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
								  EMTPTypeUINT8, localOnlyDelete);
				}
			break;
			}

		case EMTPServicePropertyFilterType:
			if (EMTPSyncSvcFilterUnInitialized != iFilterType)
				{
				TMTPTypeUint8 filterType(iFilterType);
				propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
								  EMTPTypeUINT8, filterType);
				}
			break;
		case EMTPServicePropertySyncObjectReferences:
		    {
            TMTPTypeUint8 syncObjectRef(iSyncObjectReference);
            propertyElement = CMTPTypeServicePropListElement::NewLC(iNormalServiceID, aPropertyCode,
                              EMTPTypeUINT8, syncObjectRef);
            break;
		    }
			
		default:
			responseCode = EMTPRespCodeInvalidServicePropCode;
		}
	if (propertyElement)
		{
		aPropList.AppendL(propertyElement);
		CleanupStack::Pop(propertyElement);
		}
	__FLOG_VA((_L8("GetServicePropetyL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

EXPORT_C TMTPResponseCode CMTPFullEnumServiceHandler::SetServicePropetyL(
	TUint16 aPropEnumIndex, const CMTPTypeServicePropListElement& aElement)
	{
	__FLOG(_L8("SetServicePropetyL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;

	TUint16 dataType = aElement.Uint16L(CMTPTypeServicePropListElement::EDatatype);
	switch (aPropEnumIndex)
		{
		case EMTPServicePropertyReplicaID:
			{
			if (dataType == EMTPTypeUINT128)
				{
				TMTPTypeGuid unInitValue;
				aElement.GetL(CMTPTypeServicePropListElement::EValue, unInitValue);
				iReplicateID = unInitValue;
				responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, EReplicaID, iReplicateID);
				}
			else
				{
				responseCode = EMTPRespCodeInvalidDataset;
				}
			break;
			}
		case EMTPServicePropertyLastSyncProxyID:
			{
			if (dataType == EMTPTypeUINT128)
				{
				TMTPTypeGuid unInitValue;
				aElement.GetL(CMTPTypeServicePropListElement::EValue, unInitValue);
				iLastSyncProxyID = unInitValue;
				responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, ELastSyncProxyID, iLastSyncProxyID);
				}
			else
				{
				responseCode = EMTPRespCodeInvalidDataset;
				}
			break;
			}
		case EMTPServicePropertyLocalOnlyDelete:
			{
			if (dataType == EMTPTypeUINT8)
				{
				TMTPSyncSvcLocalOnlyDelete localOnlyDelete
				= static_cast<TMTPSyncSvcLocalOnlyDelete>(aElement.Uint8L(CMTPTypeServicePropListElement::EValue));
				if (localOnlyDelete < EMTPLocalOnlyDeleteFalse || localOnlyDelete > EMTPLocalOnlyDeleteTrue)
					{
					responseCode = EMTPRespCodeInvalidDataset;
					}
				else
					{
					iLocalOnlyDelete = localOnlyDelete;
					responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, ELocalOnlyDelete, iLocalOnlyDelete);
					}
				}
			else
				{
				responseCode = EMTPRespCodeInvalidDataset;
				}
			break;
			}

		case EMTPServicePropertyFilterType:
			{
			if (dataType == EMTPTypeUINT8)
				{
				TMTPSyncSvcFilterType filterType
				= static_cast<TMTPSyncSvcFilterType>(aElement.Uint8L(CMTPTypeServicePropListElement::EValue));
				if (filterType < EMTPSyncSvcFilterNone ||
						filterType > EMTPSyncSvcFilterCalendarWindowWithRecurrence)
					{
					responseCode = EMTPRespCodeInvalidDataset;
					}
				else
					{
					iFilterType = filterType;
					responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, EFilterType, iFilterType);
					}
				}
			else
				{
				responseCode = EMTPRespCodeInvalidDataset;
				}
			break;
			}

		case EMTPServicePropertyVersionProps:
		case EMTPServicePropertyKnowledgeObjectID:
		case EMTPServicePropertyProviderVersion:
		case EMTPServicePropertySyncFormat:
		case EMTPServicePropertySyncObjectReferences:
			responseCode = EMTPRespCodeAccessDenied;
			break;
		default:
			responseCode = EMTPRespCodeInvalidServicePropCode;
			break;
		} //End of switch
	__FLOG_VA((_L8("SetServicePropertyL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}

EXPORT_C TMTPResponseCode CMTPFullEnumServiceHandler::DeleteServiceProperty(TUint16 aPropEnumIndex)
	{
	__FLOG(_L8("DeleteServiceProperty - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	switch (aPropEnumIndex)
		{
			// Deleteserviceproplist set all preoperties to uninitialized value
		case EMTPServicePropertyReplicaID:
			{
			TMTPTypeGuid tmp(MAKE_TINT64(KMTPUnInitialized32, KMTPUnInitialized32),
								MAKE_TINT64(KMTPUnInitialized32, KMTPUnInitialized32));
			iReplicateID = tmp;
			responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, EReplicaID, iReplicateID);
			break;
			}
		case EMTPServicePropertyLastSyncProxyID:
			{
			TMTPTypeGuid tmp(MAKE_TINT64(KMTPUnInitialized32, KMTPUnInitialized32),
								MAKE_TINT64(KMTPUnInitialized32, KMTPUnInitialized32));
			iLastSyncProxyID = tmp;
			responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, ELastSyncProxyID, iLastSyncProxyID);
			break;
			}
		case EMTPServicePropertyLocalOnlyDelete:
			{
			iLocalOnlyDelete = EMTPLocalOnlyDeleteUnInitialized;
			responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, ELocalOnlyDelete, iLocalOnlyDelete);
			break;
			}
		case EMTPServicePropertyFilterType:
			{
			iFilterType = EMTPSyncSvcFilterUnInitialized;
			responseCode = MMTPServiceHandler::SaveServicePropValue(iRepository, EFilterType, iFilterType);
			break;
			}
		case EMTPServicePropertyVersionProps:
		case EMTPServicePropertyKnowledgeObjectID:
		case EMTPServicePropertyProviderVersion:
		case EMTPServicePropertySyncFormat:
		case EMTPServicePropertySyncObjectReferences:
			{
			responseCode = EMTPRespCodeAccessDenied;
			break;
			}
		default:
			{
			responseCode = EMTPRespCodeInvalidServicePropCode;
			__FLOG(_L8("Invalid service propcode"));
			break;
			}
		}
	__FLOG(_L8("DeleteServiceProperty - Exit"));
	return responseCode;
	}

void CMTPFullEnumServiceHandler::LoadServicePropValueL()
	{
	__FLOG(_L8("LoadServicePropValueL - Entry"));
	// Load ReplicaID:
	TPtr8 writeBuf(NULL, 0); //walkaroud for the TMTPTypeGuid
	iReplicateID.FirstWriteChunk(writeBuf);
	User::LeaveIfError(iRepository.Get(EReplicaID, writeBuf));

	//load LastSyncProxyID
	iLastSyncProxyID.FirstWriteChunk(writeBuf);
	User::LeaveIfError(iRepository.Get(ELastSyncProxyID, writeBuf));

	TInt value;
	// Load LocalOnlyDelete
	User::LeaveIfError(iRepository.Get(ELocalOnlyDelete, value));
	iLocalOnlyDelete = static_cast<TMTPSyncSvcLocalOnlyDelete>(value);

	// Load EFilterType
	User::LeaveIfError(iRepository.Get(EFilterType, value));
	iFilterType = static_cast<TMTPSyncSvcFilterType>(value);

	//Load SyncObjectReferenceEnabled
	//Only dp which support SyncObjectReference need to save it into central responsitory
	value = EMTPSyncObjectReferencesDisabled;
	TInt ret = iRepository.Get(ESyncObjectReference, value);
	if (ret != KErrNone && ret != KErrNotFound )
	    {
	    User::Leave(ret);
	    }
	iSyncObjectReference = static_cast<TMTPSyncSvcSyncObjectReferences>(value);
	
	__FLOG(_L8("LoadServicePropValueL - Exit"));
	return;
	}
