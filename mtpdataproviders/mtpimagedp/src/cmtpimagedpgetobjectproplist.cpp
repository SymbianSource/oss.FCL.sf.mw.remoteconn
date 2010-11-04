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

#include <bautils.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptyperequest.h>

#include "mtpdpconst.h"
#include "cmtpimagedpgetobjectproplist.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedpconst.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedp.h"
#include "mtpimagedputilits.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpgetobjectproplistTraces.h"
#endif


MMTPRequestProcessor* CMTPImageDpGetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, 
                                                               MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetObjectPropList* self = new (ELeave) CMTPImageDpGetObjectPropList(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/*
 * Constructor
 */  
CMTPImageDpGetObjectPropList::CMTPImageDpGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0,NULL),
    iPropertyMgr(aDataProvider.PropertyMgr())
    {
    }
    
/*
 * Constructor
 */   
void CMTPImageDpGetObjectPropList::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
    
    iPropertyList = CMTPTypeObjectPropList::NewL();
    iObjectMeta = CMTPObjectMetaData::NewL();

    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_CONSTRUCTL_EXIT );
    }

/*
 * Destructor
 */       
CMTPImageDpGetObjectPropList::~CMTPImageDpGetObjectPropList()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_CMTPIMAGEDPGETOBJECTPROPLIST_DES_ENTRY );
    Cancel();
    
    iHandles.Close();
    delete iPropertyList;
    delete iObjectMeta;
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_CMTPIMAGEDPGETOBJECTPROPLIST_DES_EXIT );
    }

/*
 * 
 */ 
void CMTPImageDpGetObjectPropList::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEL_ENTRY );
    
    //get all the handles
    GetObjectHandlesL();
    
    if ( iHandles.Count() > 0 )
        {
        ServicePropertiesL( iHandles[0] );
        iHandles.Remove( 0 );
        }
    
    StartL();

    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEL_EXIT );
    }
    
/*
 * Check object Handle, prop code and Depth, Group Code
 */   
TMTPResponseCode CMTPImageDpGetObjectPropList::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKREQUESTL_ENTRY );      

    TMTPResponseCode result = EMTPRespCodeOK;
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    
    //getobjectproplist support KMTPHandleAll and KMTPHandleNone
    if ( handle != KMTPHandleAll && handle != KMTPHandleNone )
        {
        CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
        result = MTPImageDpUtilits::VerifyObjectHandleL( iFramework, handle, *object );
        CleanupStack::PopAndDestroy(object);
        }
    if (EMTPRespCodeOK == result)
        {        
        result = CheckPropCode();        
        }

    if (EMTPRespCodeOK == result)
        {
        result = CheckDepth();
        }    
		
	OstTrace1( TRACE_NORMAL, CMTPIMAGEDPGETOBJECTPROPLIST_CHECKREQUESTL, "result: %d", result );
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKREQUESTL_EXIT );
    
    return result;    
    }

TMTPResponseCode CMTPImageDpGetObjectPropList::CheckPropCode() const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKPROPCODE_ENTRY );
    
    TMTPResponseCode response = EMTPRespCodeOK; 
    TUint32 propCode = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
    if (propCode == 0)
        {
            /*Do nothing */
        }
    else if (propCode != KMaxTUint)
        {
        const TInt count = sizeof(KMTPImageDpSupportedProperties)/sizeof(KMTPImageDpSupportedProperties[0]);
		TInt i(0);
        for (i = 0; i < count; i++)
            {
            if (KMTPImageDpSupportedProperties[i] == propCode)
                {
                break;
                }
            }
        if (i == count)
            {
            response = EMTPRespCodeInvalidObjectPropCode;
            }
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKPROPCODE_EXIT );
    return response;
    }
 
TMTPResponseCode CMTPImageDpGetObjectPropList::CheckDepth() const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKDEPTH_ENTRY );
    
    TMTPResponseCode response = EMTPRespCodeSpecificationByDepthUnsupported;
    
    // We either support no depth at all, or 1 depth or (0xFFFFFFFF) with objecthandle as 0x00000000
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TUint32 depth = Request().Uint32(TMTPTypeRequest::ERequestParameter5);        
    if ( (depth == 0) || (depth == 1 ) ||
            (depth == KMaxTUint && handle == KMTPHandleNone ) ) 
        {
        response = EMTPRespCodeOK; 
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_CHECKDEPTH_EXIT );
    return response;    
    }

void CMTPImageDpGetObjectPropList::GetObjectHandlesL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_GETOBJECTHANDLESL_ENTRY );
    
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TUint32 formatCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    TUint32 depth(Request().Uint32(TMTPTypeRequest::ERequestParameter5));
    
    iHandles.Reset();
    
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
        iHandles.AppendL( handle );
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_GETOBJECTHANDLESL_EXIT );
    }

void CMTPImageDpGetObjectPropList::GetObjectHandlesL(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle)
    {
    OstTraceFunctionEntry0( DUP1_CMTPIMAGEDPGETOBJECTPROPLIST_GETOBJECTHANDLESL_ENTRY );
    
    RMTPObjectMgrQueryContext   context;
    RArray<TUint>               handles;
    TMTPObjectMgrQueryParams    params(aStorageId, aFormatCode, aParentHandle, iFramework.DataProviderId());
    CleanupClosePushL(context);
    CleanupClosePushL(handles);
    
    iHandles.Reset();
    
    do
        {
        iFramework.ObjectMgr().GetObjectHandlesL(params, context, handles);
        for ( TInt i = 0; i < handles.Count(); i++ )
            {
            iHandles.AppendL(handles[i]);
            }
        }
    while(!context.QueryComplete());
    
    CleanupStack::PopAndDestroy(&handles);
    CleanupStack::PopAndDestroy(&context);

    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPGETOBJECTPROPLIST_GETOBJECTHANDLESL_EXIT );
    }
    
void CMTPImageDpGetObjectPropList::GetRootObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_GETROOTOBJECTHANDLESL_ENTRY );
    
    if (aDepth == KMaxTUint)
        {
        GetObjectHandlesL(KMTPStorageAll, aFormatCode, KMTPHandleNone);
        }
    else if (aDepth == 0)
        {
        OstTrace0( TRACE_NORMAL, CMTPIMAGEDPGETOBJECTPROPLIST_GETROOTOBJECTHANDLESL, "depth is 0, iHandles is an empty array" );
        iHandles.Reset();
        }
    else
        {
        GetObjectHandlesL(KMTPStorageAll, aFormatCode, KMTPHandleNoParent);
        }

    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_GETROOTOBJECTHANDLESL_EXIT );
    }

void CMTPImageDpGetObjectPropList::ServicePropertiesL( TUint32 aHandle )
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEPROPERTIESL_ENTRY );
    
    TUint32 propCode  = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
    TUint32 groupCode = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
    
    iFramework.ObjectMgr().ObjectL(aHandle, *iObjectMeta);
    iPropertyMgr.SetCurrentObjectL(*iObjectMeta, EFalse);
    
    if (propCode == KMaxTUint)
        {
        ServiceAllPropertiesL(aHandle);
        }
    else if( propCode == 0)
        {
        ServiceGroupPropertiesL(aHandle, groupCode);
        }
    else
        {
        ServiceOneObjectPropertyL(aHandle, propCode);
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEPROPERTIESL_EXIT );
    }

void CMTPImageDpGetObjectPropList::ServiceAllPropertiesL(TUint32 aHandle)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEALLPROPERTIESL_ENTRY );
    
    if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
        {
        for (TUint i(0); (i < (sizeof(KMTPImageDpSupportedProperties) / sizeof(TUint16))); i++)
            {
            ServiceOneObjectPropertyL(aHandle, KMTPImageDpSupportedProperties[i]);
            }
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEALLPROPERTIESL_EXIT );
    }
    
void CMTPImageDpGetObjectPropList::ServiceGroupPropertiesL(TUint32 aHandle,TUint16 /*aGroupCode*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEGROUPPROPERTIESL_ENTRY );
    TUint32 groupCode = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
    if (KMTPImageDpPropertyGroupOneNumber == groupCode) //only return data for group one
        {
        if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
            {
            for (TUint propCodeIndex(0); propCodeIndex < KMTPImageDpGroupOneSize; propCodeIndex++)
                {
                TUint16 propCode = KMTPImageDpGroupOneProperties[propCodeIndex];  
                if(propCode != 0)
                    {
                    ServiceOneObjectPropertyL(aHandle, propCode);
                    }
                }
            }
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEGROUPPROPERTIESL_EXIT );
    }
 
void CMTPImageDpGetObjectPropList::ServiceOneObjectPropertyL(TUint32 aHandle, TUint32 aPropCode)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEONEOBJECTPROPERTYL_ENTRY );
    
    if (iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId())
        {
        TUint16 propCode(static_cast<TUint16>(aPropCode));        
        switch(aPropCode)
            {   
        case EMTPObjectPropCodeObjectFormat:
        case EMTPObjectPropCodeRepresentativeSampleFormat:
        case EMTPObjectPropCodeProtectionStatus:
            {
            TUint16 value;
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
        case EMTPObjectPropCodeParentObject:
            {
            TUint32 value;
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value);
            if (value == KMTPHandleNoParent)
                {
                value = 0;
                }
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
        case EMTPObjectPropCodeStorageID:
        case EMTPObjectPropCodeWidth:
        case EMTPObjectPropCodeHeight:
        case EMTPObjectPropCodeImageBitDepth:
        case EMTPObjectPropCodeRepresentativeSampleSize:
        case EMTPObjectPropCodeRepresentativeSampleHeight:
        case EMTPObjectPropCodeRepresentativeSampleWidth:
            {
            TUint32 value(0);
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value, EFalse);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            
        case EMTPObjectPropCodeRepresentativeSampleData:
            {
            CMTPTypeArray* value = CMTPTypeArray::NewLC(EMTPTypeAUINT8);
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), *value, EFalse);            
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode);
            propElem.SetArrayL(CMTPTypeObjectPropListElement::EValue, *value);            
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);            
            }
            break;
            
        case EMTPObjectPropCodeObjectSize:
            {
            TUint64 value;
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetUint64L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            
        case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
            {
            TMTPTypeUint128 value;
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetUint128L(CMTPTypeObjectPropListElement::EValue, value.UpperValue(), value.LowerValue());
            iPropertyList->CommitPropElemL(propElem);
            }        
            break;
            
        case EMTPObjectPropCodeDateCreated:            
        case EMTPObjectPropCodeDateModified:
        case EMTPObjectPropCodeObjectFileName:
        case EMTPObjectPropCodeName:
            {
            CMTPTypeString* value = CMTPTypeString::NewLC();
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), *value);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, propCode); 
            propElem.SetStringL(CMTPTypeObjectPropListElement::EValue, value->StringChars());
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);
            }
            break;
        case EMTPObjectPropCodeNonConsumable:
            {
            
            TUint8 value;
            iPropertyMgr.GetPropertyL(TMTPObjectPropertyCode(aPropCode), value);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint8L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }        
            break;
        case EMTPObjectPropCodeHidden:
            {
            TEntry FileEntry;
            User::LeaveIfError(iFramework.Fs().Entry(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), FileEntry));
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            TBool isHidden = FileEntry.IsHidden();
            if ( isHidden )
                {
                propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue,EMTPHidden );
                }
            else
                {
                propElem.SetUint16L(CMTPTypeObjectPropListElement::EValue,EMTPVisible );
                }
            iPropertyList->CommitPropElemL(propElem); 
            }
            break;
        default:
            //Leave 
            {
            OstTrace1( TRACE_ERROR, CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEONEOBJECTPROPERTYL, "Invalid property code %d", aPropCode );
            User::Leave(KErrGeneral); 
            }
            break;
          }
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_SERVICEONEOBJECTPROPERTYL_EXIT );
    }

void CMTPImageDpGetObjectPropList::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_DOCANCEL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_DOCANCEL_EXIT );
    }

void CMTPImageDpGetObjectPropList::RunL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_RUNL_ENTRY );
    
    if ( iHandles.Count() > 0 )
        {
        ServicePropertiesL( iHandles[0] );
        iHandles.Remove( 0 );
        }
    StartL();
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_RUNL_EXIT );
    }

#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPImageDpGetObjectPropList::RunError(TInt aError)
#else
TInt CMTPImageDpGetObjectPropList::RunError(TInt /*aError*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_RUNERROR_ENTRY );
    
    TRAP_IGNORE( SendResponseL( EMTPRespCodeGeneralError ) );
    
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_RUNERROR_EXIT );
    return KErrNone;
    }

void CMTPImageDpGetObjectPropList::StartL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTPROPLIST_STARTL_ENTRY );
    
    if ( iHandles.Count() > 0 )
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, iStatus.Int() );
        SetActive();
        }
    else
        {
        SendDataL(*iPropertyList);
        }

    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTPROPLIST_STARTL_EXIT );
    }
