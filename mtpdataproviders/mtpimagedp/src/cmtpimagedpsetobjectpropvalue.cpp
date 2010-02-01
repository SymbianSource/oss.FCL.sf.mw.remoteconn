// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalTechnology
*/

#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
              
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "cmtpimagedpsetobjectpropvalue.h"

__FLOG_STMT(_LIT8(KComponent,"CMTPImageDpSetObjectPropValue");)
/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPImageDpSetObjectPropValue::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
	{
	CMTPImageDpSetObjectPropValue* self = new (ELeave) CMTPImageDpSetObjectPropValue(aFramework, aConnection,aDataProvider);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
CMTPImageDpSetObjectPropValue::~CMTPImageDpSetObjectPropValue()
	{	
	__FLOG(_L8(">> ~CMTPImageDpSetObjectPropValue"));
	delete iMTPTypeString;
	delete iObjectMeta;
	__FLOG(_L8("<< ~CMTPImageDpSetObjectPropValue"));
	__FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPImageDpSetObjectPropValue::CMTPImageDpSetObjectPropValue(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
	:CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
	iDataProvider(aDataProvider),
	iObjectPropertyMgr(aDataProvider.PropertyMgr())
	{
	}

									
/**
A helper function of CheckRequestL. To check whether the object property code is readonly.
@param aObjectPropCode the object property code passed in.
@return ETrue if the object property code is readonly. Otherwise EFalse.
*/	
TBool CMTPImageDpSetObjectPropValue::IsPropCodeReadonly(TUint32 aObjectPropCode)
	{
	__FLOG(_L8(">> CMTPImageDpSetObjectPropValue::IsPropCodeReadonly"));
	TBool returnCode = EFalse;
	if(aObjectPropCode == EMTPObjectPropCodeStorageID
		|| aObjectPropCode == EMTPObjectPropCodeObjectFormat
		|| aObjectPropCode == EMTPObjectPropCodeProtectionStatus
		|| aObjectPropCode == EMTPObjectPropCodeObjectSize
		|| aObjectPropCode == EMTPObjectPropCodeParentObject
		|| aObjectPropCode == EMTPObjectPropCodeDateCreated
		|| aObjectPropCode == EMTPObjectPropCodePersistentUniqueObjectIdentifier
		|| aObjectPropCode == EMTPObjectPropCodeWidth
		|| aObjectPropCode == EMTPObjectPropCodeHeight
		|| aObjectPropCode == EMTPObjectPropCodeImageBitDepth
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleFormat
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleSize
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleHeight
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleWidth)
		{
		returnCode = ETrue;
		}
	__FLOG(_L8("<< CMTPImageDpSetObjectPropValue::IsPropCodeReadonly"));
	return returnCode;
	}

/**
Verify object handle, prop code
*/
TMTPResponseCode CMTPImageDpSetObjectPropValue::CheckRequestL()
	{
	__FLOG(_L8(">> CMTPImageDpSetObjectPropValue::CheckRequestL"));
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if(responseCode == EMTPRespCodeOK)
		{
		responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
		}
	
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	if(responseCode == EMTPRespCodeOK)
		{
			const TInt count = sizeof(KMTPImageDpSupportedProperties) / sizeof(TUint16);
			TInt i = 0;
			for(i = 0; i < count; i++)
				{
				if(KMTPImageDpSupportedProperties[i] == propCode
					&& IsPropCodeReadonly(propCode))
					// Object property code supported, but cann't be set.
					{
					responseCode = EMTPRespCodeAccessDenied;
					break;
					}
				else if(KMTPImageDpSupportedProperties[i] == propCode)
					// Object property code supported and can be set.
					{
					break;
					}
				}
			if(i == count)
				{
				responseCode = EMTPRespCodeInvalidObjectPropCode;
				}
		}
	__FLOG(_L8("<< CMTPImageDpSetObjectPropValue::CheckRequestL"));
	return responseCode;
	}
		
/**
SetObjectPropValue request handler
*/	
void CMTPImageDpSetObjectPropValue::ServiceL()
	{
	__FLOG(_L8(">> CMTPImageDpSetObjectPropValue::ServiceL"));
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	delete iMTPTypeString;
	iMTPTypeString = NULL;
	iMTPTypeString = CMTPTypeString::NewL();
	switch(propCode)
		{
	    case EMTPObjectPropCodeDateModified:
		case EMTPObjectPropCodeObjectFileName:
		case EMTPObjectPropCodeName:
			ReceiveDataL(*iMTPTypeString);
			break;
        case EMTPObjectPropCodeNonConsumable:
            ReceiveDataL(iMTPTypeUint8);
            break;			
		default:
			User::Leave(KErrGeneral);
		}	
	__FLOG(_L8("<< CMTPImageDpSetObjectPropValue::ServiceL"));
	}

/**
Apply the references to the specified object
@return EFalse
*/	
TBool CMTPImageDpSetObjectPropValue::DoHandleResponsePhaseL()
	{
	__FLOG(_L8(">> CMTPImageDpSetObjectPropValue::DoHandleResponsePhaseL"));
	
    iObjectPropertyMgr.SetCurrentObjectL(*iObjectMeta, ETrue);
    /*
    [Winlog]If file is readonly, all property of this file shoule not be changed.
    */
    TUint16 protection;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeProtectionStatus, protection);
    if(EMTPProtectionReadOnly == protection)
        {
        SendResponseL(EMTPRespCodeAccessDenied);
        return EFalse;  
        }
	TInt32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	switch(propCode)
		{
		case EMTPObjectPropCodeDateModified:
			{
			if (iMTPTypeString->NumChars() == (iMTPTypeString->StringChars().Length() + 1)) 
				{
				iObjectPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propCode), iMTPTypeString->StringChars());				
				}
			else if ( iMTPTypeString->NumChars() == 0 )
				{
				responseCode = EMTPRespCodeOK;
				}			
			}
	        break;

		case EMTPObjectPropCodeObjectFileName:
		case EMTPObjectPropCodeName:
			{
			if (iMTPTypeString->NumChars() == (iMTPTypeString->StringChars().Length() + 1)) 
				{
				iObjectPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propCode), iMTPTypeString->StringChars());
				iFramework.ObjectMgr().ModifyObjectL(*iObjectMeta);
				}
			else if ( iMTPTypeString->NumChars() == 0 )
				{
				responseCode = EMTPRespCodeOK;
				}	
			}
            break;
        case EMTPObjectPropCodeNonConsumable:
            {
            iObjectPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propCode), iMTPTypeUint8.Value());
            iFramework.ObjectMgr().ModifyObjectL(*iObjectMeta);
            responseCode = EMTPRespCodeOK;
            }
            break;            
 		default:
			responseCode = EMTPRespCodeInvalidObjectPropFormat;
			//Panic(EMTPImageDpUnsupportedProperty);
		}
	
	SendResponseL(responseCode);
	
	__FLOG(_L8("<< CMTPImageDpSetObjectPropValue::DoHandleResponsePhaseL"));
	return EFalse;	
	}
	
TBool CMTPImageDpSetObjectPropValue::HasDataphase() const
	{
	return ETrue;
	}
	
/**
Second-phase construction
*/			
void CMTPImageDpSetObjectPropValue::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8(">> CMTPImageDpSetObjectPropList::ConstructL"));
	
	iObjectMeta = CMTPObjectMetaData::NewL();
	
	__FLOG(_L8("<< CMTPImageDpSetObjectPropList::ConstructL"));
	}

	
