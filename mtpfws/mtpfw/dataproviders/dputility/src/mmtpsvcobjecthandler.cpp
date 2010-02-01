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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/src/mmtpsvcobjecthandler.cpp

#include <mtp/mmtptype.h>

#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mtpdatatypeconstants.h>
#include "mmtpsvcobjecthandler.h"
#include "mtpsvcdpconst.h"

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::SendObjectInfoL(const CMTPTypeObjectInfo& /*aObjectInfo*/, TUint32& /*aParentHandle*/, TDes& /*aSuid*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::GetObjectInfoL(const CMTPObjectMetaData& /*aObjectMetaData*/, CMTPTypeObjectInfo& /*aObjectInfo*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::SetObjectReferenceL(const CMTPObjectMetaData& /*aObjectMetaData*/, const CMTPTypeArray& /*aReferences*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::GetObjectReferenceL(const CMTPObjectMetaData& /*aObjectMetaData*/, CMTPTypeArray& /*aReferences*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::CopyObject(const CMTPObjectMetaData& /*aObjectMetaData*/, TUint32 /*aDestStorageId*/, TUint32 /*aNewParentHandle*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::MoveObject(const CMTPObjectMetaData& /*aObjectMetaData*/, TUint32 /*aDestStorageId*/, TUint32 /*aNewParentHandle*/)
	{
	return EMTPRespCodeOperationNotSupported;
	}

EXPORT_C void MMTPSvcObjectHandler::CommitL()
	{
	
	}

EXPORT_C void MMTPSvcObjectHandler::CommitForNewObjectL(TDes& /*aSuid*/)
	{
	
	}

EXPORT_C void MMTPSvcObjectHandler::RollBack()
	{
	
	}

EXPORT_C void MMTPSvcObjectHandler::ReleaseObjectBuffer()
	{
	
	}

EXPORT_C TMTPResponseCode MMTPSvcObjectHandler::CheckGenObjectPropertyL(const CMTPTypeObjectPropListElement& element, TMTPOperationCode aOperationCode) const
	{
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint16 propertyCode = element.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode);
	TUint16 dataType = element.Uint16L(CMTPTypeObjectPropListElement::EDatatype);
	// If create new object request, then only SendObjectPropList will call this function
	switch (propertyCode)
		{
		//Access denied Properties, if not for new object purpose , should response access denied.
		case EMTPGenObjPropCodeParentID:
		case EMTPGenObjPropCodeStorageID:
		case EMTPGenObjPropCodeObjectFormat:
		case EMTPGenObjPropCodeObjectSize:
			{
			// Properties which are contained in the operation parameters (StorageID, ParentObject, 
			// ObjectFormat, ObjectSize) should not be included in the sent dataset for SendObjectPropList
			if (EMTPOpCodeSendObjectPropList == aOperationCode)
				{
				responseCode = EMTPRespCodeInvalidDataset;
				break;
				}
			
			if(EMTPOpCodeUpdateObjectPropList != aOperationCode)
				{
				responseCode = EMTPRespCodeAccessDenied;
				break;
				}

			//EMTPOpCodeUpdateObjectPropList == aOperationCode
			if(EMTPGenObjPropCodeParentID == propertyCode)
				{
				if (dataType != EMTPTypeUINT32)
					{
					responseCode = EMTPRespCodeInvalidObjectPropFormat;
					}
				}
			else if(EMTPGenObjPropCodeObjectSize == propertyCode)
				{
				if (dataType != EMTPTypeUINT64)
					{
					responseCode = EMTPRespCodeInvalidObjectPropFormat;
					}
				}
			else
				{
				if (dataType != EMTPTypeUINT16)
					{
					responseCode = EMTPRespCodeInvalidObjectPropFormat;
					}
				}
			break;
			}
		case EMTPGenObjPropCodeName:
			{
			if (aOperationCode == EMTPOpCodeSetObjectPropList)
				{
				responseCode = EMTPRespCodeAccessDenied;
				break;
				}
			
			if (dataType != EMTPTypeString)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
			}
		case EMTPGenObjPropCodePersistentUniqueObjectIdentifier:
			{
			// PC should not set this prop.
			responseCode = EMTPRespCodeAccessDenied;
			break;
			}
		case EMTPGenObjPropCodeObjectHidden:
			{
			if (aOperationCode == EMTPOpCodeSetObjectPropList)
				{
				// RO prop can't be changed.
				responseCode = EMTPRespCodeAccessDenied;
				break;
				}

			if (dataType != EMTPTypeUINT16)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
			}
		case EMTPGenObjPropCodeDateModified:
			{
			if (dataType != EMTPTypeString)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
			}
		case EMTPObjectPropCodeNonConsumable:
			{
			if (aOperationCode == EMTPOpCodeSetObjectPropList)
				{
				responseCode = EMTPRespCodeAccessDenied;
				break;
				}

			if (dataType != EMTPTypeUINT8)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				break;
				}
			
			// Knowledge object must be non consumable
			if (0x01 != element.Uint8L(CMTPTypeObjectPropListElement::EValue))
				{
				responseCode = EMTPRespCodeInvalidObjectPropValue;
				}
			break;
			}
		case EMTPSvcObjPropCodeLastAuthorProxyID:
			{
			if (dataType != EMTPTypeUINT128)
				{
				responseCode = EMTPRespCodeInvalidObjectPropFormat;
				}
			break;
			}
		default:
			{
			// Conform with enhanced spec, but DeleteObjectPropList return invalidpropcode
			responseCode = EMTPRespCodeObjectPropNotSupported;
			break;
			}
		}
	return responseCode;
	}
