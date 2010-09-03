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

#include <bautils.h>
#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpsetobjectpropvalue.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"
#include "cmtpdataprovidercontroller.h"
#include "mtpframeworkconst.h"
#include "rmtpdpsingletons.h"
#include "rmtputility.h"
#include "cmtpstoragemgr.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsetobjectpropvalueTraces.h"
#endif


/**
Verification data for the SetObjectPropValue request
*/
const TMTPRequestElementInfo KMTPSetObjectPropValuePolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrWrite, 0, 0, 0},   	
     };
     
/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPSetObjectPropValue::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPSetObjectPropValue* self = new (ELeave) CMTPSetObjectPropValue(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPSetObjectPropValue::~CMTPSetObjectPropValue()
	{	
	delete iMTPTypeString;
	iSingleton.Close();
	iDpSingletons.Close();
	delete iObjMeta;
	}

/**
Standard c++ constructor
*/	
CMTPSetObjectPropValue::CMTPSetObjectPropValue(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPSetObjectPropValuePolicy)/sizeof(TMTPRequestElementInfo), KMTPSetObjectPropValuePolicy),
	iRfs(aFramework.Fs())
	{
	}
	
	
/**
A helper function of CheckRequestL. To check whether the object property code is readonly.
@param aObjectPropCode the object property code passed in.
@return ETrue if the object property code is readonly. Otherwise EFalse.
*/	
TBool CMTPSetObjectPropValue::IsPropCodeReadonly(TUint32 aObjectPropCode)
	{
	TBool returnCode = EFalse;
	if(aObjectPropCode == EMTPObjectPropCodeStorageID
		|| aObjectPropCode == EMTPObjectPropCodeObjectFormat
		|| aObjectPropCode == EMTPObjectPropCodeProtectionStatus
		|| aObjectPropCode == EMTPObjectPropCodeObjectSize
		|| aObjectPropCode == EMTPObjectPropCodeParentObject
		|| aObjectPropCode == EMTPObjectPropCodePersistentUniqueObjectIdentifier)
		{
		returnCode = ETrue;
		}
	return returnCode;
	}

/**
Verify request
*/
TMTPResponseCode CMTPSetObjectPropValue::CheckRequestL()
	{
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();

    TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPObjectMetaData* meta = iRequestChecker->GetObjectInfo(handle);
    __ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
    
    if(!iSingleton.StorageMgr().IsReadWriteStorage(meta->Uint(CMTPObjectMetaData::EStorageId)))
   		{
		responseCode = EMTPRespCodeAccessDenied;
   		}
    
	if(responseCode == EMTPRespCodeOK)
		{
		TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
		
		if(propCode != EMTPObjectPropCodeAssociationType && propCode != EMTPObjectPropCodeAssociationDesc)
			{
			const TInt count = sizeof(KMTPDpSupportedProperties) / sizeof(TUint16);
			TInt i = 0;
			for(i = 0; i < count; i++)
				{
				if(KMTPDpSupportedProperties[i] == propCode
					&& IsPropCodeReadonly(propCode))
					// Object property code supported, but cann't be set.
					{
					responseCode = EMTPRespCodeAccessDenied;
					break;
					}
				else if(KMTPDpSupportedProperties[i] == propCode)
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
		else if(meta->Uint(CMTPObjectMetaData::EFormatCode) != EMTPFormatCodeAssociation)
			{
			responseCode = EMTPRespCodeInvalidObjectFormatCode;
			}
		}
	else
	    {
	    const TDesC& suid(meta->DesC(CMTPObjectMetaData::ESuid));
	    TEntry entry;
	    LEAVEIFERROR( iFramework.Fs().Entry(suid, entry),
	            OstTraceExt1( TRACE_ERROR, CMTPSETOBJECTPROPVALUE_CHECKREQUESTL, "Gets entry details for %S failed!", suid));
	    //According to spec, there are 4 statuses: No Protection; Read-only; Read-only data; Non-transferrable data
	    //Currently, we only use FS's Read-only attribute to support No Protection and Read-only statuses.
	    //so if the attribute is read-only, we will return EMTPRespCodeAccessDenied.
        if (entry.IsReadOnly())
             {
             responseCode = EMTPRespCodeAccessDenied;
             }        
	    }
	
	return responseCode;	
	}
		
/**
SetObjectPropValue request handler
*/	
void CMTPSetObjectPropValue::ServiceL()
	{
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	iFramework.ObjectMgr().ObjectL(TMTPTypeUint32(handle), *iObjMeta);
	LEAVEIFERROR(iRfs.Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), iFileEntry),
	           OstTraceExt1(TRACE_ERROR, CMTPSETOBJECTPROPVALUE_SERVICEL, 
	                    "can't get entry details for %S!", iObjMeta->DesC(CMTPObjectMetaData::ESuid)));
	
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
		case EMTPObjectPropCodeAssociationType:
		case EMTPObjectPropCodeHidden:
			{
			ReceiveDataL(iMTPTypeUint16);
			}
			break;
		case EMTPObjectPropCodeAssociationDesc:
			{
			ReceiveDataL(iMTPTypeUint32);
			}
			break;
			
		case EMTPObjectPropCodeStorageID:
		case EMTPObjectPropCodeObjectFormat:
		case EMTPObjectPropCodeProtectionStatus:
		case EMTPObjectPropCodeObjectSize:		
		case EMTPObjectPropCodeParentObject:
		case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
			{
			//!!! Never come here, it should be blocked on CheckRequestL()
			// and EMTPRespCodeAccessDenied will be reported.
			//No break sentance, goto Panic
			}
		default:
		    OstTrace1( TRACE_ERROR, DUP1_CMTPSETOBJECTPROPVALUE_SERVICEL, "Invalid property code %d", propCode);
		    User::Leave( KErrNotSupported );
		    break;
		}	
	}

/**
Apply the references to the specified object
@return EFalse
*/	
TBool CMTPSetObjectPropValue::DoHandleResponsePhaseL()
	{
	TMTPResponseCode responseCode = EMTPRespCodeInvalidObjectPropFormat;
	TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	
	switch(propCode)
		{
		case EMTPObjectPropCodeDateModified:
			{
			TTime modifiedTime;
			if( iDpSingletons.MTPUtility().MTPTimeStr2TTime(iMTPTypeString->StringChars(), modifiedTime) )
				{
				iRfs.SetModified( iObjMeta->DesC(CMTPObjectMetaData::ESuid), modifiedTime );
				responseCode = EMTPRespCodeOK;
				
				}
			else
				{
				responseCode = EMTPRespCodeInvalidObjectPropValue;
				}
			}
			break;
		case EMTPObjectPropCodeHidden:
		    {
            if ( EMTPHidden == iMTPTypeUint16.Value())
                  {
                  TEntry entry;
                  User::LeaveIfError(iFramework.Fs().Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), entry));
                  if ( !entry.IsHidden())
                      {
                      entry.iAtt &= ~KEntryAttHidden;
                      entry.iAtt |= KEntryAttHidden;
                      User::LeaveIfError(iFramework.Fs().SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid), entry.iAtt, ~entry.iAtt));
                      }
                  responseCode = EMTPRespCodeOK;
                  }
              else if ( EMTPVisible == iMTPTypeUint16.Value())
                  {
                  TEntry entry;
                  User::LeaveIfError(iFramework.Fs().Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), entry));
                  if ( entry.IsHidden())
                      {
                      entry.iAtt &= ~KEntryAttHidden;
                      User::LeaveIfError(iFramework.Fs().SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid), entry.iAtt, ~entry.iAtt));
                      }
                  responseCode = EMTPRespCodeOK;
                  }
              else
                  {
                  responseCode = EMTPRespCodeInvalidObjectPropValue;
                  }
		    }
		    break;
		case EMTPObjectPropCodeObjectFileName:
			{

			TRAPD(err, iDpSingletons.MTPUtility().RenameObjectL(iObjMeta->Uint(CMTPObjectMetaData::EHandle),iMTPTypeString->StringChars())) ;
			if( KErrNone == err )
				{
				responseCode = EMTPRespCodeOK;
				}
			else if(KErrNotFound == err)
				{
				responseCode = EMTPRespCodeInvalidObjectHandle;
				}
			else if( KErrAlreadyExists == err)
				{
				responseCode = EMTPRespCodeInvalidObjectPropValue;
				}
			else 
				{
				responseCode = EMTPRespCodeAccessDenied;
				}
			}
			break;
		case EMTPObjectPropCodeName:
			{			
			//Might need to consider to save this name into meta-data base.
			//Notify all the Data Providers if the Owner of the object is DeviceDP
			const TDesC& name = iMTPTypeString->StringChars();
			if(name != iFileEntry.iName)
				{
				iObjMeta->SetDesCL( CMTPObjectMetaData::EName, name);
				iFramework.ObjectMgr().ModifyObjectL(*iObjMeta);
				}
			responseCode = EMTPRespCodeOK;
			}
			break;
		case EMTPObjectPropCodeNonConsumable:
			{
			iObjMeta->SetUint( CMTPObjectMetaData::ENonConsumable, iMTPTypeUint8.Value());
			iFramework.ObjectMgr().ModifyObjectL(*iObjMeta);
			responseCode = EMTPRespCodeOK;
			}
			break;
		case EMTPObjectPropCodeAssociationType:
			{
		    if (EModeMTP != iFramework.Mode())
		    	{
		    	responseCode = EMTPRespCodeOK;	
		    	}
		    else if( iMTPTypeUint16.Value() != EMTPAssociationTypeGenericFolder )
				{
				responseCode = EMTPRespCodeInvalidObjectPropValue;
				}
			else
				{
				responseCode = EMTPRespCodeOK;
				}
			}
			break;
		case EMTPObjectPropCodeAssociationDesc:
			{
			if(TUint32(iMTPTypeUint32.Value()) == 0 )
				{
				responseCode = EMTPRespCodeOK;
				}
			else
				responseCode = EMTPRespCodeInvalidObjectPropValue;
			}	
			break;
			
			
		default:
		    OstTrace1( TRACE_ERROR, CMTPSETOBJECTPROPVALUE_DOHANDLERESPONSEPHASEL, "Invalid property code %d", propCode);
		    User::Leave( KErrNotSupported );
		    break;
		}

	SendResponseL(responseCode);
	return EFalse;	
	}
	
TBool CMTPSetObjectPropValue::HasDataphase() const
	{
	return ETrue;
	}
	
/**
Second-phase construction
*/			
void CMTPSetObjectPropValue::ConstructL()
	{	
	iSingleton.OpenL();
	iObjMeta = CMTPObjectMetaData::NewL();
	iDpSingletons.OpenL(iFramework);
	}

