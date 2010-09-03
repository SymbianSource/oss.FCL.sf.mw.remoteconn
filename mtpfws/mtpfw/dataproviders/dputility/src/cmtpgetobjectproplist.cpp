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
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpgetobjectproplist.h"
#include "cmtpfsexclusionmgr.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"
#include "cmtpfsentrycache.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetobjectproplistTraces.h"
#endif


/**
Verification data for the GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetObjectPropListPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 2, KMTPHandleAll, KMTPHandleNone}
    };
    
EXPORT_C MMTPRequestProcessor* CMTPGetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetObjectPropList* self = new (ELeave) CMTPGetObjectPropList(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
EXPORT_C CMTPGetObjectPropList::~CMTPGetObjectPropList()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_DES_ENTRY );
    delete iHandles;
    delete iPropertyList;
    iDpSingletons.Close();
    delete iObjMeta;
    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_DES_EXIT );
    }

void CMTPGetObjectPropList::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_SERVICEL_ENTRY );
	TUint32 propCode(Request().Uint32(TMTPTypeRequest::ERequestParameter3));
	TUint32 groupCode(Request().Uint32(TMTPTypeRequest::ERequestParameter4));
	
	if( ( propCode != 0 ) || ( (groupCode !=0) && (groupCode <= KMTPDpPropertyGroupNumber) ) )
		{
		GetObjectHandlesL();
		TInt numObjects(iHandles->NumElements());
		for (TInt i(0); (i < numObjects); i++)
			{
			TUint32 handle(iHandles->ElementUint(i));
			iFramework.ObjectMgr().ObjectL(handle, *iObjMeta);
			
			TInt err = iFramework.Fs().Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), iFileEntry);

			if ( err != KErrNone )
				{
				if( (iDpSingletons.CopyingBigFileCache().TargetHandle() == handle) &&
						(iDpSingletons.CopyingBigFileCache().IsOnGoing()))
					{
					// The object is being copied, it is not created in fs yet. Use its cache entry to get properties
					OstTrace0( TRACE_NORMAL, CMTPGETOBJECTPROPLIST_SERVICEL, 
					        "ServiceL - The object is being copied, use its cache entry to get properties" );
					iFileEntry = iDpSingletons.CopyingBigFileCache().FileEntry();
					err = KErrNone;
					}
				else if( (iDpSingletons.MovingBigFileCache().TargetHandle() == handle) &&
									(iDpSingletons.MovingBigFileCache().IsOnGoing()))
					{
					// The object is being moved, it is not created in fs yet. Use its cache entry to get properties
					OstTrace0( TRACE_NORMAL, DUP1_CMTPGETOBJECTPROPLIST_SERVICEL, 
					        "ServiceL - The object is being moved, use its cache entry to get properties" );
					iFileEntry = iDpSingletons.MovingBigFileCache().FileEntry();
					err = KErrNone;
					}	
				}
			
			LEAVEIFERROR(err,
			        OstTraceExt2( TRACE_ERROR, DUP2_CMTPGETOBJECTPROPLIST_SERVICEL, 
			                "Can't get entry details for %S even after error handling! error code %d", iObjMeta->DesC(CMTPObjectMetaData::ESuid), err));

			if (propCode == KMaxTUint)
				{
				ServiceAllPropertiesL(handle);
				}
			else if( propCode == 0)
				{		
				ServiceGroupPropertiesL(handle,groupCode);
				}
			else    
				{
				ServiceOneObjectPropertyL(handle, propCode);
				}
			}
		}
	//else
	//it means the groupcode is not supported, return EMTPRespCodeGroupNotSupported(0xA805) response.
	//but we use one empty ObjectPropList to replace the EMTPRespCodeGroupNotSupported(0xA805) response.
	
	SendDataL(*iPropertyList);

	OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_SERVICEL_EXIT );
	}
	
TMTPResponseCode CMTPGetObjectPropList::CheckRequestL()
    {
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
    if (result == EMTPRespCodeOK)
        {
        result = CheckFormatL();
        }
        
    if (result == EMTPRespCodeOK)
        {        
        result = CheckPropCode();        
        }
    
    if (result == EMTPRespCodeOK)
        {
        result = CheckDepth();
        }
    
    return result;    
    }
    
CMTPGetObjectPropList::CMTPGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, (sizeof(KMTPGetObjectPropListPolicy) / sizeof(TMTPRequestElementInfo)), KMTPGetObjectPropListPolicy)
    {
    }
    
void CMTPGetObjectPropList::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
    iDpSingletons.OpenL(iFramework);
    iPropertyList = CMTPTypeObjectPropList::NewL();
	iObjMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_CONSTRUCTL_EXIT );
    }

TMTPResponseCode CMTPGetObjectPropList::CheckFormatL() const
    {
    TMTPResponseCode response = EMTPRespCodeOK; 
    
    const TUint32 KFormat = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    
    if ( (KFormat!=KMTPFormatsAll)&&(KFormat!=EMTPFormatCodeAssociation)&&(KFormat!=EMTPFormatCodeUndefined)&&(KFormat!=EMTPFormatCodeScript) && (iDpSingletons.ExclusionMgrL().IsFormatValid(TMTPFormatCode(KFormat))) )
        {
        response = EMTPRespCodeSpecificationByFormatUnsupported;
        }
    return response;    
    }
    
TMTPResponseCode CMTPGetObjectPropList::CheckPropCode() const
    {
    TMTPResponseCode response = EMTPRespCodeOK; 
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
    if (propCode == 0)
        {
        //GroupCode should be supported.
        return response;
        }
    else if (propCode != KMaxTUint)
        {
        if(propCode != EMTPObjectPropCodeAssociationType && propCode != EMTPObjectPropCodeAssociationDesc)
			{
			const TInt count = sizeof(KMTPDpSupportedProperties) / sizeof(TUint16);
			TInt i(0);
			for(i = 0; i < count; i++)
				{
				if(KMTPDpSupportedProperties[i] == propCode)
					{
					break;
					}
				}
			if(i == count)
				{
				response = EMTPRespCodeInvalidObjectPropCode;
				}		
			}
		else
			{
			if(formatCode != EMTPFormatCodeAssociation && formatCode != KMTPFormatsAll)
			response = EMTPRespCodeInvalidObjectPropCode;
			}	
        }
    return response;
    }
    
/**
Ensures that the requested object depth is one we support.
@return EMTPRespCodeOK, or EMTPRespCodeSpecificationByDepthUnsupported if the depth is unsupported
*/
TMTPResponseCode CMTPGetObjectPropList::CheckDepth() const
    {
    TMTPResponseCode response = EMTPRespCodeSpecificationByDepthUnsupported;
    
    // We either support no depth at all, or 1 depth or (0xFFFFFFFF) with objecthandle as 0x00000000 
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TUint32 depth = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
        
    if ( (depth == 0) || (depth == 1 ) ||
    	 (depth == KMaxTUint && handle == KMTPHandleNone ) ) 
    	{
        response = EMTPRespCodeOK; 
        }
    return response;    
    
    }

void CMTPGetObjectPropList::GetObjectHandlesL()
    {
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TUint32 formatCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    TUint32 depth(Request().Uint32(TMTPTypeRequest::ERequestParameter5));
        	
    delete iHandles;
    iHandles = NULL;
    
    if (handle == KMTPHandleAll)
        {
        GetObjectHandlesL(KMTPStorageAll, formatCode, KMTPHandleNone);
        }
    else if (handle == KMTPHandleNone)
        {
        GetRootObjectHandlesL(formatCode, depth);
        }
    else 
    	{
    	CMTPObjectMetaData* meta(iRequestChecker->GetObjectInfo(handle));
		__ASSERT_DEBUG(meta, Panic(EMTPDpObjectNull));
		
    	if ((meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation) &&
              (meta->Uint(CMTPObjectMetaData::EFormatSubCode) == EMTPAssociationTypeGenericFolder))
    		{
    		GetFolderObjectHandlesL(formatCode, depth, handle);
    		}
    		else	
       		{
       		GetSpecificObjectHandlesL(handle, formatCode, depth);
       		}	
    	}
    
    }

void CMTPGetObjectPropList::GetObjectHandlesL(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle)
    {
	RMTPObjectMgrQueryContext   context;
	RArray<TUint>               handles;
	TMTPObjectMgrQueryParams    params(aStorageId, aFormatCode, aParentHandle, iFramework.DataProviderId());
	CleanupClosePushL(context);
	CleanupClosePushL(handles);
	
	delete iHandles;
	iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);
	
	do
	    {
    	iFramework.ObjectMgr().GetObjectHandlesL(params, context, handles);
    	iHandles->AppendL(handles);
	    }
	while (!context.QueryComplete());
	
	CleanupStack::PopAndDestroy(&handles);
	CleanupStack::PopAndDestroy(&context);
    }
    
void CMTPGetObjectPropList::GetFolderObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth,TUint32 aParentHandle)
    {
    // Folder object request has been filtered by Proxy DP with setting depth as zero
    __ASSERT_DEBUG( ( 0 == aDepth ), User::Invariant() );
    GetSpecificObjectHandlesL( aParentHandle, aFormatCode, aDepth );
    }
    
void CMTPGetObjectPropList::GetRootObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth)
    {
	if (aDepth == KMaxTUint)
        {
        GetObjectHandlesL(KMTPStorageAll, aFormatCode, KMTPHandleNone);
        }
    else if (aDepth == 0)
        {
        iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);    //empty array        
        }
    else
        {
        GetObjectHandlesL(KMTPStorageAll, aFormatCode, KMTPHandleNoParent);
        }    
	}
    
void CMTPGetObjectPropList::GetSpecificObjectHandlesL(TUint32 aHandle, TUint32 /*aFormatCode*/, TUint32 /*aDepth*/)
    {
    iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);
    iHandles->AppendUintL(aHandle);
    }

void CMTPGetObjectPropList::ServiceAllPropertiesL(TUint32 aHandle)
    {
    if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
        {
        for (TUint i(0); (i < (sizeof(KMTPDpSupportedProperties) / sizeof(TUint16))); i++)
            {
            ServiceOneObjectPropertyL(aHandle, KMTPDpSupportedProperties[i]);
            }
        }


	if (iObjMeta->Uint( CMTPObjectMetaData::EFormatCode ) == EMTPFormatCodeAssociation)
		{
        CMTPTypeObjectPropListElement& propElem1 = iPropertyList->ReservePropElemL(aHandle, static_cast<TUint16>(EMTPObjectPropCodeAssociationType));
        propElem1.SetUint16L(CMTPTypeObjectPropListElement::EValue, EMTPAssociationTypeGenericFolder);
        iPropertyList->CommitPropElemL(propElem1);
        
        CMTPTypeObjectPropListElement& propElem2 = iPropertyList->ReservePropElemL(aHandle, static_cast<TUint16>(EMTPObjectPropCodeAssociationDesc));
        propElem2.SetUint32L(CMTPTypeObjectPropListElement::EValue, 0);
        iPropertyList->CommitPropElemL(propElem2); 
        
		}
			
    }
    
void CMTPGetObjectPropList::ServiceGroupPropertiesL(TUint32 aHandle,TUint16 aGroupCode)
	{
	if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
        {
        for (TUint propCodeIndex(0); propCodeIndex < KMTPDpPropertyNumber; propCodeIndex++)
            {
            TUint16 propCode = KMTPDpPropertyGroups[aGroupCode][propCodeIndex];  
            if(propCode != 0)
            	{
            	ServiceOneObjectPropertyL(aHandle, propCode);
            	}
            }
        }

	}
void CMTPGetObjectPropList::ServiceOneObjectPropertyL(TUint32 aHandle, TUint32 aPropCode)
    {
    if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
        {
        switch(aPropCode)
            {
        case EMTPObjectPropCodeNonConsumable:
            {
            
            TUint8 value = iObjMeta->Uint(CMTPObjectMetaData::ENonConsumable);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint8L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }        
            break;
            
        case EMTPObjectPropCodeObjectFormat:
        	{
            TUint16 value = iObjMeta->Uint(CMTPObjectMetaData::EFormatCode);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
        	}
        	break;
        	
        case EMTPObjectPropCodeProtectionStatus:
            {
            TUint16 value = iFileEntry.IsReadOnly();
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            
        case EMTPObjectPropCodeStorageID:
        	{
            TUint32 value = iObjMeta->Uint(CMTPObjectMetaData::EStorageId);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
        	}
        	break;
        	
        case EMTPObjectPropCodeParentObject:
            {
            TUint32 parentHandle = iObjMeta->Uint(CMTPObjectMetaData::EParentHandle);
            // refer to Appendix B.2.11 of MTP Spec 1.0 
            if (parentHandle == KMTPHandleNoParent)
                {
                parentHandle = 0;
                }

            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, parentHandle);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            
        case EMTPObjectPropCodeObjectSize:
            {
            TUint64 value = iFileEntry.FileSize();
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint64L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            
        case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
            {
            TMTPTypeUint128 value = iFramework.ObjectMgr().PuidL(aHandle);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint128L(CMTPTypeObjectPropListElement::EValue, value.UpperValue(), value.LowerValue());
            iPropertyList->CommitPropElemL(propElem); 
            }        
            break;
            
        case EMTPObjectPropCodeDateModified:
        	{
            TBuf<64> dateString;    
            iDpSingletons.MTPUtility().TTime2MTPTimeStr(iFileEntry.iModified, dateString);        	
            CMTPTypeString* value = CMTPTypeString::NewLC(dateString);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetStringL(CMTPTypeObjectPropListElement::EValue, value->StringChars());
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);
        	}
        	break;
        	
        case EMTPObjectPropCodeObjectFileName:
        	{
            CMTPTypeString* value = CMTPTypeString::NewLC(iFileEntry.iName);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetStringL(CMTPTypeObjectPropListElement::EValue, value->StringChars());
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);
        	}
        	break;
        	
        case EMTPObjectPropCodeName:
            {
            CMTPTypeString* value = CMTPTypeString::NewLC(iObjMeta->DesC(CMTPObjectMetaData::EName));
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetStringL(CMTPTypeObjectPropListElement::EValue, value->StringChars());
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);
            }
            break;
        case EMTPObjectPropCodeAssociationType:
        	{        		
        	if (iObjMeta->Uint( CMTPObjectMetaData::EFormatCode ) == EMTPFormatCodeAssociation)
				{
				TUint16 value = iObjMeta->Uint(CMTPObjectMetaData::EFormatSubCode);
                CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
                propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, value);
                iPropertyList->CommitPropElemL(propElem);
				}
        	}
        	break;
         case EMTPObjectPropCodeAssociationDesc:
        	{
			if (iObjMeta->Uint( CMTPObjectMetaData::EFormatCode ) == EMTPFormatCodeAssociation)
				{
                CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
                propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, 0x0000);
                iPropertyList->CommitPropElemL(propElem);  
				}
        	}
        	break;
         case EMTPObjectPropCodeHidden:
             {
             TBool isHidden = iFileEntry.IsHidden();
             CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
             if ( isHidden )
                 {
                 propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, EMTPHidden);
                 }
             else
                 {
                 propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, EMTPVisible);
                 }
             iPropertyList->CommitPropElemL(propElem);  
             }
             break;
        default:
            OstTrace1(TRACE_ERROR, CMTPGETOBJECTPROPLIST_SERVICEONEOBJECTPROPERTYL, "invalid property code %d", aPropCode);
            User::Leave( KErrNotSupported );
            break;
            }       
        } 
    }


