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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpknowledgehandlerTraces.h"
#endif


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
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_CKNOWLEDGEOBJECT_DES_ENTRY );
	iDateModified.Close();
	iName.Close();
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_CKNOWLEDGEOBJECT_DES_EXIT );
	}
void CKnowledgeObject::ConstructL()
	{
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_CONSTRUCTL_ENTRY );
	iKnowledgeObjectSize = KObjectSizeNotAvaiable;
	iDateModified.CreateL(KDateTimeMaxLength);
	iName.CreateL(KNameMaxLength);
	iLastAuthorProxyID.Set(KMTPUnInitialized64, KMTPUnInitialized64);
	iDirty = EBitFlagAll;
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_CONSTRUCTL_EXIT );
	}

void CKnowledgeObject::LoadL()
	{
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_LOADL_ENTRY );
	// Load ObjectSize
	TInt objSize;
	LEAVEIFERROR(iRepository.Get(ESize, objSize),
	        OstTrace0( TRACE_ERROR, CKNOWLEDGEOBJECT_LOADL, "Can't Load ObjectSize from iRepository!" ));
	iKnowledgeObjectSize = objSize;

	// Load DateModify
	iDateModified.Zero();
	LEAVEIFERROR(iRepository.Get(EDateModified, iDateModified),
	        OstTrace0( TRACE_ERROR, DUP1_CKNOWLEDGEOBJECT_LOADL, "Can't Load DateModify from iRepository!" ));
	
	// Load Name
	iName.Zero();
	LEAVEIFERROR(iRepository.Get(EName, iName),
	        OstTrace0( TRACE_ERROR, DUP2_CKNOWLEDGEOBJECT_LOADL, "Can't Load Name from iRepository!" ));

	// Load LastAuthorProxyID:
	TPtr8 writeBuf(NULL, 0); //walkaroud for the TMTPTypeUint128
	iLastAuthorProxyID.FirstWriteChunk(writeBuf);
	LEAVEIFERROR(iRepository.Get(ELastAuthorProxyID, writeBuf),
	        OstTrace0( TRACE_ERROR, DUP3_CKNOWLEDGEOBJECT_LOADL, "Can't Load LastAuthorProxyID from iRepository!" ));
	iDirty = EBitFlagNone;
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_LOADL_EXIT );
	return;
	}


void CKnowledgeObject::Clear()
	{
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_CLEAR_ENTRY );
	iKnowledgeObjectSize = KObjectSizeNotAvaiable;
	iDateModified.Zero();
	iName.Zero();
	iLastAuthorProxyID.Set(KMTPUnInitialized64, KMTPUnInitialized64);
	iDirty = EBitFlagAll;
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_CLEAR_EXIT );
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
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_COMMITL_ENTRY );
	if (EBitFlagSize == (iDirty & EBitFlagSize))
		{
		// TUint64 -> TInt, some capability is lost, 
		// anyway, it's enough for knowledge object.   
		LEAVEIFERROR(iRepository.Set(ESize, (TInt)iKnowledgeObjectSize),
		        OstTrace1( TRACE_ERROR, CKNOWLEDGEOBJECT_COMMITL, "set Size to %d failed against iRepository!", iKnowledgeObjectSize)); 
		}

	if (EBitFlagName == (iDirty & EBitFlagName))
		{
		LEAVEIFERROR(iRepository.Set(EName, iName),
		        OstTraceExt1( TRACE_ERROR, DUP1_CKNOWLEDGEOBJECT_COMMITL, "set name to %S failed against iRepository!", iName)); 
		}
	
	if (EBitFlagLastAuthorProxyID == (iDirty & EBitFlagLastAuthorProxyID))
		{
		if (EMTPRespCodeOK != SetColumnType128Value(ELastAuthorProxyID, iLastAuthorProxyID))
			{
            OstTrace0( TRACE_ERROR, DUP2_CKNOWLEDGEOBJECT_COMMITL, "set LastAuthorProxyID failed!" );
			User::Leave(KErrGeneral);
			}
		}
	
	// update DateModified to be the time of Commit
	RefreshDateModifed();
	LEAVEIFERROR(iRepository.Set(EDateModified, iDateModified),
	        OstTraceExt1( TRACE_ERROR, DUP3_CKNOWLEDGEOBJECT_COMMITL, "set DateModified to %S failed against iRepository!", iDateModified));  

	iDirty = EBitFlagNone;
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_COMMITL_EXIT );
	return;
	}

TMTPResponseCode CKnowledgeObject::SetColumnType128Value(TMTPKnowledgeStoreKeyNum aColumnNum, TMTPTypeUint128& aNewData)
	{
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_SETCOLUMNTYPE128VALUE_ENTRY );
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
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_SETCOLUMNTYPE128VALUE_EXIT );
	OstTrace1( TRACE_NORMAL, CKNOWLEDGEOBJECT_SETCOLUMNTYPE128VALUE, "responseCode = 0x%04X", responseCode );
	return responseCode;
	}


void CKnowledgeObject::RefreshDateModifed()
	{
	OstTraceFunctionEntry0( CKNOWLEDGEOBJECT_REFRESHDATEMODIFED_ENTRY );
	//get current time
	TTime now;	
	now.UniversalTime();
	_LIT(KFormat,"%F%Y%M%DT%H%T%SZ");
	iDateModified.Zero();
	now.FormatL(iDateModified, KFormat);
	OstTraceFunctionExit0( CKNOWLEDGEOBJECT_REFRESHDATEMODIFED_EXIT );
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
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_CMTPKNOWLEDGEHANDLER_DES_ENTRY );
	delete iKnowledgeObj;
	delete iKnowledgeSwpBuffer;
	delete iCachedKnowledgeObject;
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_CMTPKNOWLEDGEHANDLER_DES_EXIT );
	}

CMTPKnowledgeHandler::CMTPKnowledgeHandler(MMTPDataProviderFramework& aFramework, TUint16 aFormatCode, 
										CRepository& aReposotry, const TDesC& aKwgSuid) :
	iFramework(aFramework), iRepository(aReposotry), iKnowledgeFormatCode(aFormatCode), iSuid(aKwgSuid)
	{
	}

void CMTPKnowledgeHandler::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_CONSTRUCTL_ENTRY );
	
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
		LEAVEIFERROR(iFramework.Fs().Delete(iKnowObjSwpFileName),
		        OstTraceExt1( TRACE_ERROR, CMTPKNOWLEDGEHANDLER_CONSTRUCTL, "delete %S failed!", iKnowObjSwpFileName ));
		}

	// create and load knowledge object properties
	iCachedKnowledgeObject = CKnowledgeObject::NewL(iRepository);
	iCachedKnowledgeObject->LoadL();
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_CONSTRUCTL_EXIT );
	}

EXPORT_C void CMTPKnowledgeHandler::SetStorageId(TUint32 aStorageId)
	{
	iStorageID = aStorageId;
	}

void CMTPKnowledgeHandler::CommitL()
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_COMMITL_ENTRY );
	LEAVEIFERROR(iRepository.StartTransaction(CRepository::EReadWriteTransaction),
	        OstTrace0( TRACE_ERROR, CMTPKNOWLEDGEHANDLER_COMMITL, "start readwrite transaction for iRepository failed!"));
	        
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

	if(BaflUtils::FileExists(iFramework.Fs(), iKnowObjSwpFileName) && iCachedKnowledgeObject->Size() > 0)
		{
		// In case DP received some object content
		LEAVEIFERROR(iFramework.Fs().Replace(iKnowObjSwpFileName, iKnowObjFileName),
		        OstTraceExt2( TRACE_ERROR, DUP1_CMTPKNOWLEDGEHANDLER_COMMITL, 
		                "replace %S with %S failed!", iKnowObjSwpFileName, iKnowObjFileName));      
		}
	// If swp file isn't exsited, that means 0 sized object received, need do nothing.

	TUint32 keyInfo;
	LEAVEIFERROR(iRepository.CommitTransaction(keyInfo),
	        OstTrace1( TRACE_ERROR, DUP2_CMTPKNOWLEDGEHANDLER_COMMITL, 
	                "Commits transaction failed. The number of keys whose values were modified is %d", keyInfo));
	        
	        
	CleanupStack::Pop(&iRepository);

	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_COMMITL_EXIT );
	}

void CMTPKnowledgeHandler::CommitForNewObjectL(TDes& aSuid)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_COMMITFORNEWOBJECTL_ENTRY );
	aSuid = iSuid;
	CommitL();
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_COMMITFORNEWOBJECTL_EXIT );
	}
	
void CMTPKnowledgeHandler::RollBack()
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_ROLLBACK_ENTRY );
	iCachedKnowledgeObject->Clear();
	TRAP_IGNORE(iCachedKnowledgeObject->LoadL());
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_ROLLBACK_EXIT );
	}

EXPORT_C void CMTPKnowledgeHandler::GetObjectSuidL(TDes& aSuid) const
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETOBJECTSUIDL_ENTRY );
	if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
		{
		aSuid.Append(iSuid);
		}
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETOBJECTSUIDL_EXIT );
	}

TMTPResponseCode CMTPKnowledgeHandler::SendObjectInfoL(const CMTPTypeObjectInfo& aObjectInfo, TUint32& aParentHandle, TDes& aSuid)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_SENDOBJECTINFOL_ENTRY );
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
			OstTrace0( TRACE_WARNING, CMTPKNOWLEDGEHANDLER_SENDOBJECTINFOL, "The cached knowledge is dirty" );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_SENDOBJECTINFOL_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::SendObjectPropListL(TUint64 aObjectSize, const CMTPTypeObjectPropList& /*aObjectPropList*/, 
															TUint32& aParentHandle, TDes& aSuid)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_SENDOBJECTPROPLISTL_ENTRY );
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
			OstTrace0( TRACE_WARNING, CMTPKNOWLEDGEHANDLER_SENDOBJECTPROPLISTL, "The cached knowledge is dirty" );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_SENDOBJECTPROPLISTL_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, 
																TUint16 aPropertyCode, CMTPTypeObjectPropList& aPropList)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETOBJECTPROPERTYL_ENTRY );
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	CMTPTypeObjectPropListElement* propertyElement = NULL;
	TUint32 aHandle = aObjectMetaData.Uint(CMTPObjectMetaData::EHandle);
	
	if(iCachedKnowledgeObject->IsDirty())
		{
		OstTrace0( TRACE_WARNING, CMTPKNOWLEDGEHANDLER_GETOBJECTPROPERTYL, "The cached knowledge is dirty" );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETOBJECTPROPERTYL_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::SetObjectPropertyL(const TDesC& /*aSuid*/, 
															const CMTPTypeObjectPropListElement& aElement, 
															TMTPOperationCode aOperationCode)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_SETOBJECTPROPERTYL_ENTRY );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_SETOBJECTPROPERTYL_EXIT );
	return responseCode;
	}

// Remove the knowledge object
TMTPResponseCode CMTPKnowledgeHandler::DeleteObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_DELETEOBJECTL_ENTRY );

	LEAVEIFERROR(iRepository.StartTransaction(CRepository::EReadWriteTransaction),
	        OstTrace0( TRACE_ERROR, CMTPKNOWLEDGEHANDLER_DELETEOBJECTL, "Start readwrite transaction failed!"));
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
		LEAVEIFERROR(iFramework.Fs().Delete(iKnowObjFileName),
		        OstTraceExt1( TRACE_ERROR, DUP1_CMTPKNOWLEDGEHANDLER_DELETEOBJECTL, "delete %S failed!", iKnowObjFileName));   
		}

	TUint32 keyInfo;
	LEAVEIFERROR(iRepository.CommitTransaction(keyInfo),
	           OstTrace1( TRACE_ERROR, DUP2_CMTPKNOWLEDGEHANDLER_DELETEOBJECTL, 
	                    "Commits transaction failed. The number of keys whose values were modified is %d", keyInfo));

	CleanupStack::Pop(&iRepository);

	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_DELETEOBJECTL_EXIT );
	return EMTPRespCodeOK;
	}

// Return the knowledge object content
TMTPResponseCode CMTPKnowledgeHandler::GetObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/, MMTPType** aBuffer)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETOBJECTL_ENTRY );
	if (!BaflUtils::FileExists(iFramework.Fs(), iKnowObjFileName))
		{
		RFile file;
		CleanupClosePushL(file);
		LEAVEIFERROR(file.Create(iFramework.Fs(), iKnowObjFileName, EFileRead),
		        OstTraceExt1( TRACE_ERROR, CMTPKNOWLEDGEHANDLER_GETOBJECTL, "Create readonly %S failed!", iKnowObjFileName));
		CleanupStack::PopAndDestroy(&file);
		}
	
	// iKnowledgeObj will be NULL in four cases: 1. Initialize; 2. The Object has been deleted.
	// 3. it has been commited by SendObject. 4 released by GetObject.
	if (!iKnowledgeObj)
		{
		iKnowledgeObj = CMTPTypeFile::NewL(iFramework.Fs(), iKnowObjFileName, EFileRead);
		}
	*aBuffer = iKnowledgeObj;
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETOBJECTL_EXIT );
	return EMTPRespCodeOK;
	}

TMTPResponseCode CMTPKnowledgeHandler::DeleteObjectPropertyL(const CMTPObjectMetaData& /*aObjectMetaData*/, const TUint16 aPropertyCode)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_DELETEOBJECTPROPERTYL_ENTRY );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_DELETEOBJECTPROPERTYL_EXIT );
	return responseCode;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetBufferForSendObjectL(const CMTPObjectMetaData& /*aObjectMetaData*/, MMTPType** aBuffer)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETBUFFERFORSENDOBJECTL_ENTRY );
	if (iKnowledgeSwpBuffer)
		{
		delete iKnowledgeSwpBuffer;
		iKnowledgeSwpBuffer = NULL;
		}
	iKnowledgeSwpBuffer = CMTPTypeFile::NewL(iFramework.Fs(), iKnowObjSwpFileName, EFileWrite);
	iKnowledgeSwpBuffer->SetSizeL(0);
	*aBuffer = iKnowledgeSwpBuffer;
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETBUFFERFORSENDOBJECTL_EXIT );
	return EMTPRespCodeOK;
	}

void CMTPKnowledgeHandler::BuildObjectInfoL(CMTPTypeObjectInfo& aObjectInfo) const
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_BUILDOBJECTINFOL_ENTRY );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_BUILDOBJECTINFOL_EXIT );
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectInfoL(const CMTPObjectMetaData& /*aObjectMetaData*/, CMTPTypeObjectInfo& aObjectInfo)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETOBJECTINFOL_ENTRY );
	if(iCachedKnowledgeObject->Size() != KObjectSizeNotAvaiable)
		{
		BuildObjectInfoL(aObjectInfo);
		}
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETOBJECTINFOL_EXIT );
	return EMTPRespCodeOK;
	}


void CMTPKnowledgeHandler::ReleaseObjectBuffer()
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_RELEASEOBJECTBUFFER_ENTRY );
	if (iKnowledgeObj)
		{
		delete iKnowledgeObj;
		iKnowledgeObj = NULL;
		}
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_RELEASEOBJECTBUFFER_EXIT );
	}

TMTPResponseCode CMTPKnowledgeHandler::GetObjectSizeL(const TDesC& aSuid, TUint64& aObjectSize)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETOBJECTSIZEL_ENTRY );
	if (aSuid != iSuid)
		{
		OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETOBJECTSIZEL_EXIT );
		return EMTPRespCodeGeneralError;
		}
	aObjectSize = iCachedKnowledgeObject->Size();
	OstTraceFunctionExit0( DUP1_CMTPKNOWLEDGEHANDLER_GETOBJECTSIZEL_EXIT );
	return EMTPRespCodeOK;
	}

TMTPResponseCode CMTPKnowledgeHandler::GetAllObjectPropCodeByGroupL(TUint32 aGroupId, RArray<TUint32>& aPropCodes)
	{
	OstTraceFunctionEntry0( CMTPKNOWLEDGEHANDLER_GETALLOBJECTPROPCODEBYGROUPL_ENTRY );
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
	OstTraceFunctionExit0( CMTPKNOWLEDGEHANDLER_GETALLOBJECTPROPCODEBYGROUPL_EXIT );
	return responseCode;
	}


