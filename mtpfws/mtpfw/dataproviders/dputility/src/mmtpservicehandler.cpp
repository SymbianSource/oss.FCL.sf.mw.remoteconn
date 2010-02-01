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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/mmtpservicehandler.cpp

#include <centralrepository.h>
#include <mtp/cmtptypeserviceobjpropextnform.h>

#include "mmtpservicehandler.h"



// Some form need to destroy before the property push to stack.
EXPORT_C CMTPTypeObjectPropDesc* MMTPServiceHandler::GenerateSvcObjPropertyL(
	TUint aDataType, const TMTPTypeGuid& aPKNamespace, TUint aPKID, const TDesC& aName,
	TUint8 aFormFlag, const MMTPType* aForm, CMTPTypeObjectPropDesc::TMTPObjectPropDescGetSet aObjGetSet,
	TUint16 aObjPropCode, TUint32 aGroupCode)
	{
	CMTPTypeServiceObjPropExtnForm* serviceObjectForm
	= CMTPTypeServiceObjPropExtnForm::NewLC(aDataType, aPKNamespace, aPKID,
											aName, aFormFlag, aForm);
	CMTPTypeObjectPropDesc::TPropertyInfo info;
	info.iDataType     = aDataType;
	info.iFormFlag     = CMTPTypeObjectPropDesc::EServiceObjPropExtnForm;
	info.iGetSet       = aObjGetSet;
	CMTPTypeObjectPropDesc* pObjectProperty
	= CMTPTypeObjectPropDesc::NewL(aObjPropCode, info, serviceObjectForm);
	pObjectProperty->SetUint32L(CMTPTypeObjectPropDesc::EGroupCode, aGroupCode);
	// Form pointer is special and need to be deleted by caller.
	CleanupStack::PopAndDestroy(serviceObjectForm);
	return pObjectProperty;
	}

EXPORT_C CMTPTypeObjectPropDesc* MMTPServiceHandler::GenerateSvcObjPropertyLC(
	TUint aDataType, const TMTPTypeGuid&  aPKNamespace, TUint aPKID, const TDesC& aName,
	TUint8 aFormFlag, const MMTPType* aForm, CMTPTypeObjectPropDesc::TMTPObjectPropDescGetSet aObjGetSet,
	TUint16 aObjPropCode, TUint32 aGroupCode)
	{
	CMTPTypeObjectPropDesc* pObjectProperty = NULL;
	pObjectProperty = GenerateSvcObjPropertyL(aDataType, aPKNamespace, aPKID, aName, aFormFlag, aForm, aObjGetSet, aObjPropCode, aGroupCode);
	CleanupStack::PushL(pObjectProperty);
	return pObjectProperty;
	}

EXPORT_C TMTPResponseCode MMTPServiceHandler::SaveServicePropValue(
	CRepository& aRepository, TUint aColumnNum, TInt aNewData)
	{
	TInt ret;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	ret = aRepository.Set(aColumnNum, aNewData);
	if (KErrNone != ret)
		{
		responseCode = EMTPRespCodeGeneralError;
		}
	return responseCode;
	}

EXPORT_C TMTPResponseCode MMTPServiceHandler::SaveServicePropValue(
	CRepository& aRepository, TUint aColumnNum, TMTPTypeGuid& aNewData)
	{
	TInt ret;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TBuf8<KMTPTypeINT128Size>  data;
	data.FillZ(data.MaxLength());
	TUint64 upperValue = aNewData.UpperValue();
	TUint64 lowerValue = aNewData.LowerValue();

	/**
	Least significant 64-bit buffer offset.
	*/
	const TInt           KMTPTypeUint128OffsetLS = 0;
	/**
	Most significant 64-bit buffer offset.
	*/
	const TInt           KMTPTypeUint128OffsetMS = 8;

	memcpy(&data[KMTPTypeUint128OffsetMS], &upperValue, sizeof(upperValue));
	memcpy(&data[KMTPTypeUint128OffsetLS], &lowerValue, sizeof(lowerValue));

	ret = aRepository.Set(aColumnNum, data);
	if (KErrNone != ret)
		{
		responseCode = EMTPRespCodeGeneralError;
		}
	return responseCode;
	}

EXPORT_C CMTPTypeObjectPropDesc* MMTPServiceHandler::GenerateGenericObjectPropDescLC(TUint16 aObjPropCode)
	{
	const TMTPTypeGuid KMTPGenObjPropNamespaceGUID(
		MAKE_TUINT64(KMTPGenericObjectNSGUID[0], KMTPGenericObjectNSGUID[1]),
		MAKE_TUINT64(KMTPGenericObjectNSGUID[2], KMTPGenericObjectNSGUID[3]));
	const TMTPTypeGuid KMTPSyncObjPropNamespace(
		MAKE_TUINT64(KMTPSyncObjcetNSGUID[0], KMTPSyncObjcetNSGUID[1]),
		MAKE_TUINT64(KMTPSyncObjcetNSGUID[2], KMTPSyncObjcetNSGUID[3]));

	CMTPTypeObjectPropDesc* objectProperty = NULL;
	TMTPTypeUint32 longStringForm(KLongStringMaxLength);

	switch (aObjPropCode)
		{
			/* Generic Ojbect Namespace properties */
			// Parent Object
		case EMTPGenObjPropCodeParentID:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT32, KMTPGenObjPropNamespaceGUID, 3, KObjPropNameParentID, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly, 
					EMTPGenObjPropCodeParentID, 0x2);
			break;
			}

		// Name
		case EMTPGenObjPropCodeName:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeString, KMTPGenObjPropNamespaceGUID, 4, KObjPropNameName, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly, 
					EMTPGenObjPropCodeName, 0x5);
			break;
			}

		// Unique Object Identifier
		case EMTPGenObjPropCodePersistentUniqueObjectIdentifier:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT128, KMTPGenObjPropNamespaceGUID, 5, KObjPropNamePUOID, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly, 
					EMTPGenObjPropCodePersistentUniqueObjectIdentifier, 0x2);
			break;
			}

		// Format Code
		case EMTPGenObjPropCodeObjectFormat:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT16, KMTPGenObjPropNamespaceGUID, 6, KObjPropNameObjectFormat, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly, 
					EMTPGenObjPropCodeObjectFormat, 0x2);
			break;
			}

		// Object Size
		case EMTPGenObjPropCodeObjectSize:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT64, KMTPGenObjPropNamespaceGUID, 11, KObjPropNameObjectSize, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly, 
					EMTPGenObjPropCodeObjectSize, 0x2);
			break;
			}

		//Storage ID
		case EMTPGenObjPropCodeStorageID:
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT32, KMTPGenObjPropNamespaceGUID, 23, KObjPropNameStorageID, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadOnly,
					EMTPGenObjPropCodeStorageID, 0x2);
			break;
			}

		// Object Hidden
		case EMTPGenObjPropCodeObjectHidden:
			{
			CMTPTypeObjectPropDescEnumerationForm* hiddenForm
			= CMTPTypeObjectPropDescEnumerationForm::NewLC(EMTPTypeUINT16);
			TUint16 values[] = {0x0000, 0x0001};
			TUint numValues((sizeof(values) / sizeof(values[0])));
			for (TUint i = 0; i < numValues; i++)
				{
				TMTPTypeUint16 data(values[i]);
				hiddenForm->AppendSupportedValueL(data);
				}
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyL(
					EMTPTypeUINT16, KMTPGenObjPropNamespaceGUID, 28, KObjPropNameHidden,
					CMTPTypeObjectPropDesc::EEnumerationForm, hiddenForm, 
					CMTPTypeObjectPropDesc::EReadOnly, EMTPGenObjPropCodeObjectHidden, 0x2);
			CleanupStack::PopAndDestroy(hiddenForm);
			CleanupStack::PushL(objectProperty);
			break;
			}

		// NonConsumable
		case EMTPGenObjPropCodeNonConsumable:
			{
			CMTPTypeObjectPropDescEnumerationForm* nonConsumeForm
			= CMTPTypeObjectPropDescEnumerationForm::NewLC(EMTPTypeUINT8);
			TUint8 values[] = {0x00, 0x01};
			TUint numValues((sizeof(values) / sizeof(values[0])));
			for (TUint i = 0; i < numValues; i++)
				{
				TMTPTypeUint8 data(values[i]);
				nonConsumeForm->AppendSupportedValueL(data);
				}
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyL(
					EMTPTypeUINT8, KMTPGenObjPropNamespaceGUID, 13, KObjPropNameNonConsumable, 
					CMTPTypeObjectPropDesc::EEnumerationForm, nonConsumeForm,
					CMTPTypeObjectPropDesc::EReadOnly, EMTPGenObjPropCodeNonConsumable, 0x2);
			CleanupStack::PopAndDestroy(nonConsumeForm);
			CleanupStack::PushL(objectProperty);
			break;
			}

		// Date Modified
		case EMTPGenObjPropCodeDateModified:
			{
			objectProperty = MMTPServiceHandler::MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeString, KMTPGenObjPropNamespaceGUID, 40, KObjPropNameDateModified, 
					CMTPTypeObjectPropDesc::EDateTimeForm, NULL, CMTPTypeObjectPropDesc::EReadWrite, 
					EMTPGenObjPropCodeDateModified, 0x2);
			break;
			}

		case EMTPSvcObjPropCodeLastAuthorProxyID :
			{
			objectProperty = MMTPServiceHandler::GenerateSvcObjPropertyLC(
					EMTPTypeUINT128, KMTPSyncObjPropNamespace, 2, KObjPropNameLastAuthorProxyID, 
					CMTPTypeObjectPropDesc::ENone, NULL, CMTPTypeObjectPropDesc::EReadWrite, 
					EMTPSvcObjPropCodeLastAuthorProxyID, 0x2);

			break;
			}
		//Error
		default:
			{
			// Internal error, get wrong object property, just ignore the property
			break;
			}
		} // End of switch
	return objectProperty;
	}
