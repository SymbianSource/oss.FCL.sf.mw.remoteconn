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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpsetobjectpropvalueTraces.h"
#endif


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
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_CMTPIMAGEDPSETOBJECTPROPVALUE_DES_ENTRY );
	delete iMTPTypeString;
	delete iObjectMeta;
	OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_CMTPIMAGEDPSETOBJECTPROPVALUE_DES_EXIT );
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
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_ISPROPCODEREADONLY_ENTRY );
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
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleWidth
		|| aObjectPropCode == EMTPObjectPropCodeRepresentativeSampleData)
		{
		returnCode = ETrue;
		}
	OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_ISPROPCODEREADONLY_EXIT );
	return returnCode;
	}

/**
Verify object handle, prop code
*/
TMTPResponseCode CMTPImageDpSetObjectPropValue::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_CHECKREQUESTL_ENTRY );
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
	OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_CHECKREQUESTL_EXIT );
	return responseCode;
	}
		
/**
SetObjectPropValue request handler
*/	
void CMTPImageDpSetObjectPropValue::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_SERVICEL_ENTRY );
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
        case EMTPObjectPropCodeHidden:
            ReceiveDataL(iMTPTypeUint16);
            break;
		default:
		    OstTrace1( TRACE_ERROR, CMTPIMAGEDPSETOBJECTPROPVALUE_SERVICEL, "Invalid propCode %d", propCode );
			User::Leave(KErrGeneral);
		}	
	OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_SERVICEL_EXIT );
	}

/**
Apply the references to the specified object
@return EFalse
*/	
TBool CMTPImageDpSetObjectPropValue::DoHandleResponsePhaseL()
	{
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_DOHANDLERESPONSEPHASEL_ENTRY );
	
    iObjectPropertyMgr.SetCurrentObjectL(*iObjectMeta, ETrue);
    /*
    [Winlog]If file is readonly, all property of this file shoule not be changed.
    */
    TUint16 protection;
    iObjectPropertyMgr.GetPropertyL(EMTPObjectPropCodeProtectionStatus, protection);
    if(EMTPProtectionReadOnly == protection)
        {
        SendResponseL(EMTPRespCodeAccessDenied);
        OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_DOHANDLERESPONSEPHASEL_EXIT );
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
        case EMTPObjectPropCodeHidden:
            {
            iObjectPropertyMgr.SetPropertyL(TMTPObjectPropertyCode(propCode), iMTPTypeUint16.Value());
            responseCode = EMTPRespCodeOK;
            }
            break;    
 		default:
			responseCode = EMTPRespCodeInvalidObjectPropFormat;
			//Panic(EMTPImageDpUnsupportedProperty);
		}
	
	SendResponseL(responseCode);

	OstTraceFunctionExit0( DUP1_CMTPIMAGEDPSETOBJECTPROPVALUE_DOHANDLERESPONSEPHASEL_EXIT );
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
	OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROPVALUE_CONSTRUCTL_ENTRY );
	
	iObjectMeta = CMTPObjectMetaData::NewL();

	OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROPVALUE_CONSTRUCTL_EXIT );
	}

	
