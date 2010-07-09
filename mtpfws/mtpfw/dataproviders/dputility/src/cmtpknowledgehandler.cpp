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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/cmtpknowledgehandler.cpp

#include <bautils.h> 
#include <centralrepository.h>
#include <mtp/cmtptypestring.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypefile.h>
#include <mtp/mtpdatatypeconstants.h>

#include "cmtpknowledgehandler.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"KwgObjHdler");)

const TInt KDateTimeMaxLength = 22;
const TInt KNameMaxLength = 255;

_LIT16(KEmptyContent16, "");
_LIT(KMTPKnowledgeObjDriveLocation, "c:");
_LIT(KMTPNoBackupFolder, "nobackup\\");	
_LIT(KMTPSlash, "\\");
_LIT(KMTPKnowledgeObjFileName, "mtp_knowledgeobj.dat");
_LIT(KMTPKnowledgeObjSwpFileName, "mtp_knowledgeobj.swp");


CKnowledgeObject* CKnowledgeObject::NewL(CRepository& aRepository)
	{
	CKnowledgeObject *self = new (ELeave) CKnowledgeObject(aRepository);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CKnowledgeObject::CKnowledgeObject(CRepository& aRepository) : iRepository(aRepository)
	{
	
	}
CKnowledgeObject::~CKnowledgeObject()
	{
	__FLOG(_L8("~CKnowledgeObject - Entry")); 
	iDateModified.Close();
	iName.Close();
	__FLOG(_L8("~CKnowledgeObject - Exit")); 
	__FLOG_CLOSE;
	}
void CKnowledgeObject::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("CKnowledgeObject Construct - Entry"));
	iKnowledgeObjectSize = KObjectSizeNotAvaiable;
	iDateModified.CreateL(KDateTimeMaxLength);
	iName.CreateL(KNameMaxLength);
	iLastAuthorProxyID.Set(KMTPUnInitialized64, KMTPUnInitialized64);
	iDirty = EBitFlagAll;
	__FLOG(_L8("CKnowledgeObject Construct - Exit"));
	}

void CKnowledgeObject::LoadL()
	{
	__FLOG(_L8("CKnowledgeObject LoadL - Entry"));
	// Load ObjectSize
	TInt objSize;
	User::LeaveIfError(iRepository.Get(ESize, objSize));
	iKnowledgeObjectSize = objSize;

	// Load DateModify
	iDateModified.Zero();
	User::LeaveIfError(iRepository.Get(EDateModified, iDateModified));
	
	// Load Name
	iName.Zero();
	User::LeaveIfError(iRepository.Get(EName, iName));

	// Load LastAuthorProxyID:
	TPtr8 writeBuf(NULL, 0); //walkaroud for the TMTPTypeUint128
	iLastAuthorProxyID.FirstWriteChunk(writeBuf);
	User::LeaveIfError(iRepository.Get(ELastAuthorProxyID, writeBuf));
	iDirty = EBitFlagNone;
	__FLOG(_L8("CKnowledgeObject LoadL - Exit"));
	return;
	}


void CKnowledgeObject::Clear()
	{
	__FLOG(_L8("CKnowledgeObject Clear - Entry"));
	iKnowledgeObjectSize = KObjectSizeNotAvaiable;
	iDateModified.Zero();
	iName.Zero();
	iLastAuthorProxyID.Set(KMTPUnInitialized64, KMTPUnInitialized64);
	iDirty = EBitFlagAll;
	__FLOG(_L8("CKnowledgeObject Clear - Exit"));
	}

void CKnowledgeObject::SetSize(TUint64 aSize)
	{
	iKnowledgeObjectSize = aSize;
	iDirty |= EBitFlagSize;
	}

void CKnowledgeObject::SetDateModified(const TDesC& /*aDateModified*/)
	{
	/**
	 * DateModifed will be auto updated when commit. If PC set this prop, it will be ignored.
	 * If someday PC want to force the DateModifed be saved, the following code piece should 
	 * be opened, and add check in commit when save DateModified.
	 *iDateModified.Copy(aDateModified);
	 *iDirty |= EBitFlagDateModified;
	**/
	}

void CKnowledgeObject::SetName(const TDesC& aName)
	{
	iName.Copy(aName);
	iDirty |= EBitFlagName;
	}

void CKnowledgeObject::SetLastAuthorProxyID(TUint64 aHigh, TUint64 aLow) 
	{
	iLastAuthorProxyID.Set(aHigh, aLow);
	iDirty |= EBitFlagLastAuthorProxyID;
	}

void CKnowledgeObject::CommitL()
	{
	__FLOG(_L8("CKnowledgeObject CommitL - Entry"));
	if (EBitFlagSize == (iDirty & EBitFlagSize))
		{
		// TUint64 -> TInt, some capability is lost, 
		// anyway, it's enough for knowledge object.   
		User::LeaveIfError(iRepository.Set(ESize, (TInt)iKnowledgeObjectSize));
		}

	if (EBitFlagName == (iDirty & EBitFlagName))
		{
		User::LeaveIfError(iRepository.Set(EName, iName));
		}
	
	if (EBitFlagLastAuthorProxyID == (iDirty & EBitFlagLastAuthorProxyID))
		{
		if (EMTPRespCodeOK != SetColumnType128Value(ELastAuthorProxyID, iLastAuthorProxyID))
			{
			User::Leave(KErrGeneral);
			}
		}
	
	// update DateModified to be the time of Commit
	RefreshDateModifed();
	User::LeaveIfError(iRepository.Set(EDateModified, iDateModified));

	iDirty = EBitFlagNone;
	__FLOG(_L8("CKnowledgeObject KnowledgeObject CommitL - Exit"));
	return;
	}

TMTPResponseCode CKnowledgeObject::SetColumnType128Value(TMTPKnowledgeStoreKeyNum aColumnNum, TMTPTypeUint128& aNewData)
	{
	__FLOG(_L8("CKnowledgeObject SetColumnType128ValueL - Entry"));
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
	
	ret = iRepository.Set(aColumnNum, data);
	if (KErrNone != ret)
		{
		responseCode = EMTPRespCodeGeneralError;
		}
	__FLOG_VA((_L8("CKnowledgeObject SetColumnType128ValueL - Exit with responseCode = 0x%04X"), responseCode));
	return responseCode;
	}


void CKnowledgeObject::RefreshDateModifed()
	{
	__FLOG(_L8("CKnowledgeObject RefreshDateModifed - Entry"));
	//get current time
	TTime now;	
	now.UniversalTime();
	_LIT(KFormat,"%F%Y%M%DT%H%T%SZ");
	iDateModified.Zero();
	now.FormatL(iDateModified, KFormat);
	__FLOG(_L8("CKnowledgeObject RefreshDateModifed - Exit"));
	return;
	}

EXPORT_C CMTPKnowledgeHandler* CMTPKnowledgeHandler::NewL(MMTPDataProviderFramework& aFramework, TUint16 aFormatCode, 
												CRepository& aReposotry, const TDesC& aKwgSuid)
	{
	CMTPKnowledgeHandler *self = new (ELeave) CMTPKnowledgeHandler(aFramework, aFormatCode, aReposotry, aKwgSuid);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CMTPKnowledgeHandler::~CMTPKnowledgeHandler()
	{
	__FLOG(_L8("~CMTPKnowledgeHandler - Entry")); 
	delete iKnowledgeObj;
	delete iKnowledgeSwpBuffer;
	delete iCachedKnowledgeObject;
	__FLOG(_L8("~CMTPKnowledgeHandler - Exit"));
	__FLOG_CLOSE;
	}

CMTPKnowledgeHandler::CMTPKnowledgeHandler(MMTPDataProviderFramework& aFramework, TUint16 aFormatCode, 
										CRepository& aReposotry, const TDesC& aKwgSuid) :
	iFramework(aFramework), iRepository(aReposotry), iKnowledgeFormatCode(aFormatCode), iSuid(aKwgSuid)
	{
	}

void CMTPKnowledgeHandler::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("Construct - Entry"));
	
	iFramework.Fs().PrivatePath(iKnowObjFileName);
	iKnowObjFileName.Insert(0, KMTPKnowledgeObjDriveLocation);
	iKnowObjFileName.Append(KMTPNoBackupFolder);
	iKnowObjFileName.Append(iSuid);
	iKnowObjFileName.Append(KMTPSlash);

	iKnowObjSwpFileName.Append(iKnowObjFileName);
	iKnowObjSwpFileName.Append(KMTPKnowledgeObjSwpFileName);
	
	iKnowObjFileName.Append(KMTPKnowledgeObjFileName);

	// Create knowledge object file directory if not exist.
	BaflUtils::EnsurePathExistsL(iFramework.Fs(), iKnowObjFileName);

	// Recover for previous failed transaction
	if(BaflUtils::FileExists(iFramework.Fs(), iKnowObjSwpFileName))
		{
		// In case DP received some object content
		User::LeaveIfError(iFramework.Fs().Delete(iKnowObjSwpFileName));
		}

	// create and load knowledge object properties
	iCachedKnowledgeObject = CKnowledgeObject::NewL(iRepository);
	iCachedKnowledgeObject->LoadL();
	__FLOG(_L8("ConstructL - Exit"));
	}

EXPORT_C void CMTPKnowledgeHandler::SetStorageId(TUint32 aStorageId)
	{
	iStorageID = aStorageId;
	}

void CMTPKnowledgeHandler::CommitL()
	{
	__FLOG(_L8("CommitL - Entry"));
	User::LeaveIfError(iRepository.StartTransaction(CRepository::EReadWriteTransaction));
	iRepository.CleanupCancelTransactionPushL();
	
	iCachedKnowledgeObject->CommitL();
	
	// Close all knowledge file and reset pointer.
	if (iKnowledgeObj)
		{
		delete iKnowledgeObj;
		iKnowledgeObj = NULL;
		}
	
	if (iKnowledgeSwpBuffer)
		{
		delete iKnowledgeSwpBuffer;
		iKnowledgeSwpBuffer = NULL;
		}

	if(BaflUtils::FileExists(iFramework.Fs(), iKnowObjSwpFileName))
		{
		// In case DP received some object content
		User::LeaveIfError(iFramework.Fs().Replace(iKnowObjSwpFileName, iKnowObjFileName));
		}
	// If swp file isn't exsited, that means 0 sized object received, need do nothing.

	TUint32 keyInfo;
	User::LeaveIfError(iRepository.CommitTransaction(keyInfo));
	CleanupStack::Pop(&iRepository);

	__FLOG(_L8("CommitL - Exit"));
	}

void CMTPKnowledgeHandler::CommitForNewObjectL(TDes& aSuid)
	{
	aSuid = iSuid;
	CommitL();
	}
	
void CMTPKnowledgeHandler::RollBack()
	{
	__FLOG(_L8("Rollback - Entry"));
	iCachedKnowledgeObject->Clear();
	TRAP_IGNORE(iCachedKnowledgeObject->LoadL());
	__FLOG(_L8("Rollback - Exit"));
	}

EXPORT_C void CMTPKnowledgeHandler::GetObjectSuidL(TDes& aSuid) const
	{
	__FLOG(_L8("GetObjectSuidL - Entry"));
	if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
		{
		aSuid.Append(iSuid);
		}
	__FLOG(_L8("GetObjectSuidL - Exit"));
	}

TMTPResponseCode CMTPKnowledgeHandler::SendObjectInfoL(const CMTPTypeObjectInfo& aObjectInfo, TUint32& aParentHandle, TDes& aSuid)
	{
	__FLOG(_L("SendObjectInfoL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (aParentHandle != KMTPHandleNone && aParentHandle != KMTPHandleNoParent)
		{
		responseCode = EMTPRespCodeInvalidParentObject;
		}
	else
		{
		//if there's a read error reread
		if(iCachedKnowledgeObject->IsDirty())
			{
			__FLOG(_L8("Warning: The cached knowledge is dirty"));
			iCachedKnowledgeObject->Clear();
			iCachedKnowledgeObject->LoadL();
			}
		//already has a knowledge object
		if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
			{
			responseCode = EMTPRespCodeAccessDenied;
			}
		else
			{
			iCachedKnowledgeObject->Clear();
			iCachedKnowledgeObject->SetSize(aObjectInfo.Uint32L(CMTPTypeObjectInfo::EObjectCompressedSize));
			// DateModified will be updated when commit.
			aSuid = iSuid;
			}
		}
	__FLOG(_L("SendObjectInfoL - Exit"));
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::SendObjectPropListL(TUint64 aObjectSize, const CMTPTypeObjectPropList& /*aObjectPropList*/, 
															TUint32& aParentHandle, TDes& aSuid)
	{
	__FLOG(_L8("SendObjectPropListL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (aParentHandle != KMTPHandleNone && aParentHandle != KMTPHandleNoParent)
		{
		responseCode = EMTPRespCodeInvalidParentObject;
		}
	else
		{
		//if there's a read error reread
		aParentHandle = KMTPHandleNoParent;
		if(iCachedKnowledgeObject->IsDirty())
			{
			__FLOG(_L8("Warning: The cached knowledge is dirty"));
			iCachedKnowledgeObject->Clear();
			iCachedKnowledgeObject->LoadL();
			}
		//already has a knowledge object
		if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
			{
			responseCode = EMTPRespCodeAccessDenied;
			}
		else
			{
			iCachedKnowledgeObject->Clear();
			iCachedKnowledgeObject->SetSize(aObjectSize);
			aSuid = iSuid;
			}
		}
	__FLOG(_L8("SendObjectPropListL - Exit"));
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, 
																TUint16 aPropertyCode, CMTPTypeObjectPropList& aPropList)
	{
	__FLOG(_L8("GetObjectPropertyL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	CMTPTypeObjectPropListElement* propertyElement = NULL;
	TUint32 aHandle = aObjectMetaData.Uint(CMTPObjectMetaData::EHandle);
	
	if(iCachedKnowledgeObject->IsDirty())
		{
		__FLOG(_L8("Warning: The cached knowledge is dirty"));
		iCachedKnowledgeObject->LoadL();
		}
	
	switch (aPropertyCode)
		{
		case EMTPGenObjPropCodeParentID:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint32L(CMTPTypeObjectPropListElement::EValue, 0x0000);
			break;
			}
				
		case EMTPGenObjPropCodeName:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			if (iCachedKnowledgeObject->Name().Length() > 0)
				{
				propertyElement->SetStringL(CMTPTypeObjectPropListElement::EValue, iCachedKnowledgeObject->Name());
				}
			else
				{
				_LIT(KDefaultKwgObjName, "GenericObject01");
				propertyElement->SetStringL(CMTPTypeObjectPropListElement::EValue, KDefaultKwgObjName());	
				}
			break;
			}
			
		case EMTPGenObjPropCodePersistentUniqueObjectIdentifier:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint128L(CMTPTypeObjectPropListElement::EValue,
								iFramework.ObjectMgr().PuidL(aHandle).UpperValue(),
								iFramework.ObjectMgr().PuidL(aHandle).LowerValue());
			break;
			}
			
		case EMTPGenObjPropCodeObjectFormat:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint16L(CMTPTypeObjectPropListElement::EValue, iKnowledgeFormatCode);
			break;
			}

		case EMTPGenObjPropCodeObjectSize:
			{
			if (iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
				{
				propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
				propertyElement->SetUint64L(CMTPTypeObjectPropListElement::EValue, iCachedKnowledgeObject->Size());
				}
			break;
			}

		case EMTPGenObjPropCodeStorageID:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint32L(CMTPTypeObjectPropListElement::EValue, iStorageID);
			break;
			}

		case EMTPGenObjPropCodeObjectHidden:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint16L(CMTPTypeObjectPropListElement::EValue, 0x0001);
			break;
			}

		case EMTPGenObjPropCodeNonConsumable:
			{
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetUint8L(CMTPTypeObjectPropListElement::EValue, 0x01);
			break;
			}

		case EMTPGenObjPropCodeDateModified:
			{
			if(iCachedKnowledgeObject->DateModified().Length() == 0)
				{
				iCachedKnowledgeObject->RefreshDateModifed();
				}
			propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
			propertyElement->SetStringL(CMTPTypeObjectPropListElement::EValue, iCachedKnowledgeObject->DateModified());
			break;
			}
			
		case EMTPSvcObjPropCodeLastAuthorProxyID:
			{
			if ((iCachedKnowledgeObject->LastAuthorProxyID().UpperValue() != KMTPUnInitialized64)
				 && (iCachedKnowledgeObject->LastAuthorProxyID().LowerValue() !=KMTPUnInitialized64))
				{
				propertyElement = &(aPropList.ReservePropElemL(aHandle, aPropertyCode));
				propertyElement->SetUint128L(CMTPTypeObjectPropListElement::EValue,
						iCachedKnowledgeObject->LastAuthorProxyID().UpperValue(),
						iCachedKnowledgeObject->LastAuthorProxyID().LowerValue());
				}
			break;	
			}
			
		default:
			responseCode = EMTPRespCodeInvalidObjectPropCode;
			break;
		}
	if(propertyElement)
		{
		aPropList.CommitPropElemL(*propertyElement);
		}
	__FLOG(_L8("GetObjectPropertyL - Exit"));
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::SetObjectPropertyL(const TDesC& /*aSuid*/, 
															const CMTPTypeObjectPropListElement& aElement, 
															TMTPOperationCode aOperationCode)
	{
	__FLOG(_L8("SetObjectPropertyL - Entry"));
	TMTPResponseCode responseCode = CheckGenObjectPropertyL(aElement, aOperationCode);
	if (responseCode == EMTPRespCodeOK)
		{
		TUint16 propertyCode(aElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode));
		switch (propertyCode)
			{
			case EMTPGenObjPropCodeObjectSize:
				{
				iCachedKnowledgeObject->SetSize(aElement.Uint64L(CMTPTypeObjectPropListElement::EValue));
				break;
				}
			case EMTPGenObjPropCodeDateModified:
				{
				// DateModified will be updated when Commit
				iCachedKnowledgeObject->SetDateModified(aElement.StringL(CMTPTypeObjectPropListElement::EValue));
				break;
				}
			case EMTPGenObjPropCodeName:
				{
				iCachedKnowledgeObject->SetName(aElement.StringL(CMTPTypeObjectPropListElement::EValue));
				break;
				}
			case EMTPSvcObjPropCodeLastAuthorProxyID:
				{
				TUint64 high_value = 0;
				TUint64 low_value  = 0;
				aElement.Uint128L(CMTPTypeObjectPropListElement::EValue, high_value, low_value);
				iCachedKnowledgeObject->SetLastAuthorProxyID(high_value, low_value);
				break;
				}
				
			default:
				responseCode = EMTPRespCodeObjectPropNotSupported;
				break;
			}
		}
	__FLOG(_L8("SetObjectPropertyL - Exit"));
	return responseCode;
	}

// Remove the knowledge object
TMTPResponseCode CMTPKnowledgeHandler::DeleteObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/)
	{
	__FLOG(_L8("DeleteObjectL - Entry"));

	User::LeaveIfError(iRepository.StartTransaction(CRepository::EReadWriteTransaction));
	iRepository.CleanupCancelTransactionPushL();

	// Delete obejct properties in transaction, if leave, mgr will rollback all properties.
	iCachedKnowledgeObject->Clear();
	iCachedKnowledgeObject->CommitL();
	
	// Reset knowledgeobject pointer and close the file.
	if (iKnowledgeObj)
		{
		delete iKnowledgeObj;
		iKnowledgeObj = NULL;
		}
	
	// Keep file delete is atomic.
	if (BaflUtils::FileExists(iFramework.Fs(), iKnowObjFileName))
		{
		User::LeaveIfError(iFramework.Fs().Delete(iKnowObjFileName));
		}

	TUint32 keyInfo;
	User::LeaveIfError(iRepository.CommitTransaction(keyInfo));
	CleanupStack::Pop(&iRepository);

	__FLOG(_L8("DeleteObjectL - Exit"));
	return EMTPRespCodeOK;
	}

// Return the knowledge object content
TMTPResponseCode CMTPKnowledgeHandler::GetObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/, MMTPType** aBuffer)
	{
	__FLOG(_L8("GetObjectL - Entry"));
	if (!BaflUtils::FileExists(iFramework.Fs(), iKnowObjFileName))
		{
		RFile file;
		CleanupClosePushL(file);
		User::LeaveIfError(file.Create(iFramework.Fs(), iKnowObjFileName, EFileRead));
		CleanupStack::PopAndDestroy(&file);
		}
	
	// iKnowledgeObj will be NULL in four cases: 1. Initialize; 2. The Object has been deleted.
	// 3. it has been commited by SendObject. 4 released by GetObject.
	if (!iKnowledgeObj)
		{
		iKnowledgeObj = CMTPTypeFile::NewL(iFramework.Fs(), iKnowObjFileName, EFileRead);
		}
	*aBuffer = iKnowledgeObj;
	__FLOG(_L8("GetObjectL - Exit"));
	return EMTPRespCodeOK;
	}

TMTPResponseCode CMTPKnowledgeHandler::DeleteObjectPropertyL(const CMTPObjectMetaData& /*aObjectMetaData*/, const TUint16 aPropertyCode)
	{
	__FLOG(_L8("DeleteObjectPropertyL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	switch (aPropertyCode)
		{
		case EMTPGenObjPropCodeDateModified:
			// DateModified will be updated when Commit
			iCachedKnowledgeObject->SetDateModified(KEmptyContent16);
			break;
		case EMTPSvcObjPropCodeLastAuthorProxyID:
			{
			iCachedKnowledgeObject->SetLastAuthorProxyID(KMTPUnInitialized64, KMTPUnInitialized64);
			break;
			}
			
		case EMTPObjectPropCodeParentObject:
		case EMTPObjectPropCodeName:
		case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
		case EMTPObjectPropCodeObjectFormat:
		case EMTPObjectPropCodeObjectSize:
		case EMTPObjectPropCodeStorageID:
		case EMTPObjectPropCodeHidden:
		case EMTPObjectPropCodeNonConsumable:
			responseCode = EMTPRespCodeAccessDenied;
			break;
			
		default:
			responseCode = EMTPRespCodeInvalidObjectPropCode;
			break;
		}
	__FLOG(_L8("DeleteObjectPropertyL - Exit"));
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetBufferForSendObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/, MMTPType** aBuffer)
	{
	__FLOG(_L8("GetBufferForSendObjectL - Entry"));
	if (iKnowledgeSwpBuffer)
		{
		delete iKnowledgeSwpBuffer;
		iKnowledgeSwpBuffer = NULL;
		}
	iKnowledgeSwpBuffer = CMTPTypeFile::NewL(iFramework.Fs(), iKnowObjSwpFileName, EFileWrite);
	iKnowledgeSwpBuffer->SetSizeL(0);
	*aBuffer = iKnowledgeSwpBuffer;
	__FLOG(_L8("GetBufferForSendObjectL - Exit"));
	return EMTPRespCodeOK;
	}

void CMTPKnowledgeHandler::BuildObjectInfoL(CMTPTypeObjectInfo& aObjectInfo) const
	{
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EStorageID, iStorageID);	
	aObjectInfo.SetUint16L(CMTPTypeObjectInfo::EObjectFormat, iKnowledgeFormatCode);
	// Not use
	aObjectInfo.SetUint16L(CMTPTypeObjectInfo::EProtectionStatus, 0x0000);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EObjectCompressedSize, iCachedKnowledgeObject->Size());
	aObjectInfo.SetUint16L(CMTPTypeObjectInfo::EThumbFormat, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EThumbCompressedSize, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EThumbPixWidth, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EThumbPixHeight, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EImagePixWidth, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EImagePixHeight, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EImageBitDepth, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EParentObject, KMTPHandleNoParent);
	aObjectInfo.SetUint16L(CMTPTypeObjectInfo::EAssociationType, 0x0000);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::EAssociationDescription, 0);
	aObjectInfo.SetUint32L(CMTPTypeObjectInfo::ESequenceNumber, 0);
	aObjectInfo.SetStringL(CMTPTypeObjectInfo::EFilename, KNullDesC);
	aObjectInfo.SetStringL(CMTPTypeObjectInfo::EDateModified, KNullDesC);
	aObjectInfo.SetStringL(CMTPTypeObjectInfo::EDateCreated, KNullDesC);
	aObjectInfo.SetStringL(CMTPTypeObjectInfo::EKeywords, KNullDesC);
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectInfoL(const CMTPObjectMetaData& /*aObjectMetaData*/, CMTPTypeObjectInfo& aObjectInfo)
	{
	__FLOG(_L8("GetObjectInfoL - Entry"));
	if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
		{
		BuildObjectInfoL(aObjectInfo);
		}
	__FLOG(_L8("GetObjectInfoL - Exit"));
	return EMTPRespCodeOK;
	}


void CMTPKnowledgeHandler::ReleaseObjectBuffer()
	{
	__FLOG(_L8("ReleaseObjectBuffer - Entry"));
	if (iKnowledgeObj)
		{
		delete iKnowledgeObj;
		iKnowledgeObj = NULL;
		}
	__FLOG(_L8("ReleaseObjectBuffer - Exit"));
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectSizeL(const TDesC& aSuid, TUint64& aObjectSize)
	{
	__FLOG(_L8("GetObjectSizeL - Entry"));
	if (aSuid != iSuid)
		{
		return EMTPRespCodeGeneralError;
		}
	aObjectSize = iCachedKnowledgeObject->Size();
	__FLOG(_L8("GetObjectSizeL - Exit"));
	return EMTPRespCodeOK;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetAllObjectPropCodeByGroupL(TUint32 aGroupId, RArray<TUint32>& aPropCodes)
	{
	__FLOG(_L8("GetAllObjectPropCodeByGroupL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	if (0 == aGroupId)
		{
		TInt count = sizeof(KMTPFullEnumSyncKnowledgeObjectProperties) / sizeof(KMTPFullEnumSyncKnowledgeObjectProperties[0]);
		for (TInt i = 0; i < count; i++)
			{
			aPropCodes.AppendL(KMTPFullEnumSyncKnowledgeObjectProperties[i]);
			}
		}
	else if(2 == aGroupId)
		{
		TInt count = sizeof(KMTPKnowledgeObjectPropertiesGroup2) / sizeof(KMTPKnowledgeObjectPropertiesGroup2[0]);
		for (TInt i = 0; i < count; i++)
			{
			aPropCodes.Append(KMTPKnowledgeObjectPropertiesGroup2[i]);
			}
		}
	else if(5 == aGroupId)
		{
		TInt count = sizeof(KMTPKnowledgeObjectPropertiesGroup5) / sizeof(KMTPKnowledgeObjectPropertiesGroup5[0]);
		for (TInt i = 0; i < count; i++)
			{
			aPropCodes.Append(KMTPKnowledgeObjectPropertiesGroup5[i]);
			}
		}
	else
		{
		responseCode = (TMTPResponseCode)0xA805;
		}
	__FLOG(_L8("GetAllObjectPropCodeByGroupL - Exit"));
	return responseCode;
	}


