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
//

#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeuint8.h>
#include <mtp/tmtptypeuint32.h>
#include <mtp/tmtptypeuint64.h>
#include <mtp/tmtptypeuint128.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpgetobjectpropvalue.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetobjectpropvalueTraces.h"
#endif



/**
Verification data for the GetObjectPropValue request
*/
const TMTPRequestElementInfo KMTPGetObjectPropValuePolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 0, 0, 0},   	
     };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPGetObjectPropValue::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPGetObjectPropValue* self = new (ELeave) CMTPGetObjectPropValue(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/		
EXPORT_C CMTPGetObjectPropValue::~CMTPGetObjectPropValue()
	{
	delete iMTPTypeString;	
	delete iObjMeta;
	}

/**
Standard c++ constructor
*/		
CMTPGetObjectPropValue::CMTPGetObjectPropValue(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPropValuePolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPropValuePolicy),
	iRfs(aFramework.Fs())
	{
	}

/**
Verify request
*/
TMTPResponseCode CMTPGetObjectPropValue::CheckRequestL()
	{
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	if(responseCode == EMTPRespCodeOK)
		{
		if(propCode != EMTPObjectPropCodeAssociationType && propCode != EMTPObjectPropCodeAssociationDesc)
			{
			const TInt count = sizeof(KMTPDpSupportedProperties) / sizeof(TUint16);
			TInt i = 0;
			for(i = 0; i < count; i++)
				{
				if(KMTPDpSupportedProperties[i] == propCode)
					{
					break;
					}
				}
			if(i == count)
				{
				responseCode = EMTPRespCodeInvalidObjectPropCode;
				}		
			}
		else
			{
			TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
			CMTPObjectMetaData* meta = iRequestChecker->GetObjectInfo(handle);
			__ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
			
			TUint16 format = meta->Uint(CMTPObjectMetaData::EFormatCode);
			if(format != EMTPFormatCodeAssociation)
				responseCode = EMTPRespCodeInvalidObjectPropCode;
			}
		}
	return responseCode;	
	}
	
/**
GetObjectPropValue request handler
*/		
void CMTPGetObjectPropValue::ServiceL()
	{
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	
	TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	iFramework.ObjectMgr().ObjectL(TMTPTypeUint32(handle), *iObjMeta);
	LEAVEIFERROR(iRfs.Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), iFileEntry),
	        OstTraceExt1(TRACE_ERROR, CMTPGETOBJECTPROPVALUE_SERVICEL, 
	                "can't get entry details for %S!", iObjMeta->DesC(CMTPObjectMetaData::ESuid)));

	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	switch(propCode)
		{
		case EMTPObjectPropCodeStorageID:
			ServiceStorageIdL();
			break;
		case EMTPObjectPropCodeObjectFormat:
			ServiceObjectFormatL();
			break;
		case EMTPObjectPropCodeProtectionStatus:
			ServiceProtectionStatusL();
			break;
		case EMTPObjectPropCodeObjectSize:
			ServiceObjectSizeL();
			break;
		case EMTPObjectPropCodeAssociationType:						
			ServiceObjectAssociationTypeL();
			break;
		case EMTPObjectPropCodeAssociationDesc:
			ServiceObjectAssociationDescL();
			break;
		case EMTPObjectPropCodeObjectFileName:
			ServiceFileNameL();
			break;
		case EMTPObjectPropCodeDateModified:
			ServiceDateModifiedL();
			break;
		case EMTPObjectPropCodeParentObject:
			ServiceParentObjectL();
			break;
		case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
			ServicePuidL();
			break;
		case EMTPObjectPropCodeName:
			ServiceNameL();
			break;
		case EMTPObjectPropCodeNonConsumable:
			ServiceNonConsumableL();
			break;
		case EMTPObjectPropCodeHidden:
		    ServiceHiddenL();
		    break;
		default:
		    OstTrace1(TRACE_ERROR, DUP1_CMTPGETOBJECTPROPVALUE_SERVICEL, "Invalid property code %d", propCode );
		    User::Leave( KErrNotSupported );
			break;
		}	
	}

/**
Second-phase construction
*/					
void CMTPGetObjectPropValue::ConstructL()
	{
	iObjMeta	= CMTPObjectMetaData::NewL();
	iMTPTypeString = CMTPTypeString::NewL();
	}		

void CMTPGetObjectPropValue::ServiceStorageIdL()
	{
	iMTPTypeUint32.Set(iObjMeta->Uint(CMTPObjectMetaData::EStorageId));	
	SendDataL(iMTPTypeUint32);
	}
	
void CMTPGetObjectPropValue::ServiceObjectFormatL()
	{
	iMTPTypeUint16.Set(iObjMeta->Uint(CMTPObjectMetaData::EFormatCode));
	
	SendDataL(iMTPTypeUint16);	
	}
	
void CMTPGetObjectPropValue::ServiceProtectionStatusL()
	{
	iMTPTypeUint16.Set(iFileEntry.IsReadOnly());

	SendDataL(iMTPTypeUint16);	
	}
	
void CMTPGetObjectPropValue::ServiceObjectSizeL()
	{
	iMTPTypeUint64.Set(iFileEntry.FileSize());

	SendDataL(iMTPTypeUint64);
	}
	
void CMTPGetObjectPropValue::ServiceObjectAssociationTypeL()	
	{
	iMTPTypeUint16.Set(iObjMeta->Uint(CMTPObjectMetaData::EFormatSubCode));

	SendDataL(iMTPTypeUint16);
	}

void CMTPGetObjectPropValue::ServiceObjectAssociationDescL()	
	{
	iMTPTypeUint64.Set(0x00000000);
	SendDataL(iMTPTypeUint64);
	}	
	
void CMTPGetObjectPropValue::ServiceFileNameL()
	{
	iMTPTypeString->SetL(iFileEntry.iName);
	SendDataL(*iMTPTypeString);
	}
	
void CMTPGetObjectPropValue::ServiceDateModifiedL()
	{
	_LIT(KMTPDateStringFormat,"%F%Y%M%DT%H%T%S");	
	TBuf<64> dateString;	
	iFileEntry.iModified.FormatL(dateString,KMTPDateStringFormat);
	iMTPTypeString->SetL(dateString);
	SendDataL(*iMTPTypeString);
	}
	
void CMTPGetObjectPropValue::ServiceParentObjectL()
	{
    TUint32 parentHandle = iObjMeta->Uint(CMTPObjectMetaData::EParentHandle);
    if (parentHandle == KMTPHandleNoParent)
        {
        parentHandle = 0;
        }
    iMTPTypeUint32.Set(parentHandle);

    SendDataL(iMTPTypeUint32);
	}
	
void CMTPGetObjectPropValue::ServicePuidL()
	{
	iMTPTypeUint128 = iFramework.ObjectMgr().PuidL((TUint32)iObjMeta->Uint(CMTPObjectMetaData::EHandle));
	SendDataL(iMTPTypeUint128);
	}
	
void CMTPGetObjectPropValue::ServiceNameL()
	{
	iMTPTypeString->SetL(iObjMeta->DesC(CMTPObjectMetaData::EName));
	SendDataL(*iMTPTypeString);
	}
	
void CMTPGetObjectPropValue::ServiceNonConsumableL()
	{
	iMTPTypeUint8.Set((iObjMeta->Uint(CMTPObjectMetaData::ENonConsumable)));
	SendDataL(iMTPTypeUint8);
	}

void CMTPGetObjectPropValue::ServiceHiddenL()
    {
    TBool isHidden = iFileEntry.IsHidden();
    if ( isHidden )
        {
        iMTPTypeUint16.Set(EMTPHidden);
        }
    else
        {
        iMTPTypeUint16.Set(EMTPVisible);
        } 
    SendDataL(iMTPTypeUint16);
    }


	
















	

	


   	

	






