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


#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include "cmtppictbridgedpgetobjectproplist.h"
#include "mtppictbridgedppanic.h"
#include "mtppictbridgedpconst.h"
#include "cptpserver.h"

/**
Two-phase construction method
@param aFramework   The data provider framework
@param aConnection  The connection object
@param aDataProvider  The pictbridge Dp
*/
MMTPRequestProcessor* CMTPPictBridgeDpGetObjectPropList::NewL(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider )
    {  
    CMTPPictBridgeDpGetObjectPropList* self = new (ELeave) CMTPPictBridgeDpGetObjectPropList(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

/**
Destructor
*/
CMTPPictBridgeDpGetObjectPropList::~CMTPPictBridgeDpGetObjectPropList()
    {
    __FLOG(_L8("+~CMTPPictBridgeDpGetObjectPropList()"));
    
    delete iHandles;
    delete iPropertyList;
    iDpSingletons.Close();
    delete iObjMeta;
    
    __FLOG(_L8("-~CMTPPictBridgeDpGetObjectPropList()"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/  
CMTPPictBridgeDpGetObjectPropList::CMTPPictBridgeDpGetObjectPropList(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider ) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL ),
    iPictBridgeDP(aDataProvider)
    {
    }

/**
ConstructL
*/      
void CMTPPictBridgeDpGetObjectPropList::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::ConstructL()"));
    
	iDpSingletons.OpenL(iFramework);
    iPropertyList = CMTPTypeObjectPropList::NewL();
	iObjMeta = CMTPObjectMetaData::NewL();
	
	__FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::ConstructL()"));
    }

/**
ServiceL
*/  
void CMTPPictBridgeDpGetObjectPropList::ServiceL()
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::ServiceL()"));
    
    GetObjectHandlesL();
    TInt numObjects(iHandles->NumElements());
    TUint32 propCode( Request().Uint32( TMTPTypeRequest::ERequestParameter3 ));
    // get group code
	TUint32 groupCode(Request().Uint32(TMTPTypeRequest::ERequestParameter4));
	TBool isDiscoveryFile( EFalse );
	
    for ( TInt i(0); i < numObjects; i++ )
        {
        TUint32 handle( iHandles->ElementUint(i) );
        iFramework.ObjectMgr().ObjectL(handle, *iObjMeta);
        TFileName file(iObjMeta->DesC(CMTPObjectMetaData::ESuid));
        TInt ret = iFramework.Fs().Entry(iObjMeta->DesC(CMTPObjectMetaData::ESuid), iFileEntry);
        if (( ret != KErrNone )&& ( handle == iPictBridgeDP.PtpServer()->DeviceDiscoveryHandle() ))
            {
            isDiscoveryFile = ETrue;
            }
        else
            {
            User::Leave( ret );
            }
        
        if ( KMaxTUint == propCode )
            {
            ServiceAllPropertiesL( handle, isDiscoveryFile );
            }
        else if ( 0 == propCode )
            {
            ServiceGroupPropertiesL( handle, groupCode, isDiscoveryFile );
            }
        else 
            {
            ServiceOneObjectPropertyL( handle, propCode, isDiscoveryFile );
            } 
        isDiscoveryFile = EFalse;
        }
    SendDataL( *iPropertyList );
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::ServiceL()"));
    }

/**
CheckRequestL
*/
TMTPResponseCode CMTPPictBridgeDpGetObjectPropList::CheckRequestL()
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::CheckRequestL()"));
    
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();

    if ( result == EMTPRespCodeOK )
        {
        result = CheckPropCode();
        }
    if ( result == EMTPRespCodeOK )
        {
        result = CheckDepth();
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::CheckRequestL()"));
    return result;
    }

/**
CheckPropCode
*/
TMTPResponseCode CMTPPictBridgeDpGetObjectPropList::CheckPropCode() const
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::CheckPropCode()"));
    
    TMTPResponseCode response = EMTPRespCodeOK;
    TUint32 formatCode = Request().Uint32( TMTPTypeRequest::ERequestParameter2 );
    TUint32 propCode = Request().Uint32( TMTPTypeRequest::ERequestParameter3 );

    if(( propCode != KMaxTUint ) && ( propCode != 0 ))
        {
         const TInt count = sizeof(KMTPPictBridgeDpSupportedProperties) / sizeof(KMTPPictBridgeDpSupportedProperties[0]);
         TInt i(0);
         for( i = 0; i < count; i++ )
           {
           if( KMTPPictBridgeDpSupportedProperties[i] == propCode )
             {
             break;
             }
           }
         if( i == count )
           {
           response = EMTPRespCodeInvalidObjectPropCode;
           }
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::CheckPropCode()"));
    return response;
    }

/**
CheckDepth
*/
TMTPResponseCode CMTPPictBridgeDpGetObjectPropList::CheckDepth() const
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::CheckDepth()"));
    TMTPResponseCode response = EMTPRespCodeSpecificationByDepthUnsupported;

    // We either support no depth at all, or 1 depth or (0xFFFFFFFF) with objecthandle as 0x00000000
    TUint32 handle( Request().Uint32(TMTPTypeRequest::ERequestParameter1 ));
    TUint32 depth( Request().Uint32(TMTPTypeRequest::ERequestParameter5 ));

    if ((0 == depth ) || ( 1 == depth ) ||
            ( KMaxTUint == depth && KMTPHandleNone == handle )) 
        {
        response = EMTPRespCodeOK;
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::CheckDepth()"));
    return response; 
    }

/**
GetObjectHandlesL
*/
void CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL()
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL()"));
    
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TUint32 formatCode(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    TUint32 depth(Request().Uint32(TMTPTypeRequest::ERequestParameter5));

    delete iHandles;
    iHandles = NULL;

    if (KMTPHandleAll == handle)
        {
        GetObjectHandlesL(KMTPStorageAll, formatCode, KMTPHandleNone);
        }
    else if (KMTPHandleNone == handle)
        {
        GetRootObjectHandlesL(formatCode, depth);
        }
    else 
        {
        iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);
        iHandles->AppendUintL(handle);
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL()"));
    }

/**
GetObjectHandlesL
*/
void CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL( TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle )
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL()"));
    
    RMTPObjectMgrQueryContext   context;
    RArray<TUint>               handles;
    TMTPObjectMgrQueryParams    params( aStorageId, aFormatCode, aParentHandle );
    CleanupClosePushL( context );
    CleanupClosePushL( handles );

    delete iHandles;
    iHandles = NULL;
    iHandles = CMTPTypeArray::NewL( EMTPTypeAUINT32 );

    do
        {
        iFramework.ObjectMgr().GetObjectHandlesL( params, context, handles );
        iHandles->AppendL( handles );
        }
    while( !context.QueryComplete() );

    CleanupStack::PopAndDestroy( &handles );
    CleanupStack::PopAndDestroy( &context );
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::GetObjectHandlesL()"));
    }

/**
GetRootObjectHandlesL
*/
void CMTPPictBridgeDpGetObjectPropList::GetRootObjectHandlesL( TUint32 aFormatCode, TUint32 aDepth )
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::GetRootObjectHandlesL()"));
    
    if ( KMaxTUint == aDepth )
        {
        GetObjectHandlesL( KMTPStorageAll, aFormatCode, KMTPHandleNone );
        }
    else if ( 0 == aDepth )
        {
        delete iHandles;
        iHandles = NULL;
        iHandles = CMTPTypeArray::NewL( EMTPTypeAUINT32 );    //empty array
        }
    else
        {
        GetObjectHandlesL( KMTPStorageAll, aFormatCode, KMTPHandleNoParent );
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::GetRootObjectHandlesL()"));
    }

/**
ServiceAllPropertiesL
*/
void CMTPPictBridgeDpGetObjectPropList::ServiceAllPropertiesL( TUint32 aHandle, TBool aDiscoveryFile )
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::ServiceAllPropertiesL()"));
    
    if ( iFramework.ObjectMgr().ObjectOwnerId( aHandle ) == iFramework.DataProviderId() )
        {
        for ( TUint i(0); i < (sizeof(KMTPPictBridgeDpSupportedProperties) / sizeof(KMTPPictBridgeDpSupportedProperties[0])); i++ )
            {
            ServiceOneObjectPropertyL( aHandle, KMTPPictBridgeDpSupportedProperties[i], aDiscoveryFile );
            }
        }
    
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::ServiceAllPropertiesL()"));
    }

/**
ServiceGroupPropertiesL
*/
void CMTPPictBridgeDpGetObjectPropList::ServiceGroupPropertiesL(TUint32 aHandle,TUint16 aGroupCode, TBool aDiscoveryFile )
	{
	__FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::ServiceGroupPropertiesL()"));
	TBool supportGroupCode = ( aGroupCode >0 )&&( aGroupCode < KMTPDpPropertyGroupNumber+1 );
	if ( supportGroupCode && ( iFramework.ObjectMgr().ObjectOwnerId(aHandle) == iFramework.DataProviderId() ))
        {
        for (TUint propCodeIndex(0); propCodeIndex < KMTPDpPropertyNumber; propCodeIndex++)
            {
            TUint16 propCode = KMTPDpPropertyGroups[aGroupCode][propCodeIndex];  
            if(propCode != 0)
            	{
            	ServiceOneObjectPropertyL(aHandle, propCode, aDiscoveryFile );
            	}
            }
        }
	
	__FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::ServiceGroupPropertiesL()"));
	}

/**
ServiceOneObjectPropertyL
*/
void CMTPPictBridgeDpGetObjectPropList::ServiceOneObjectPropertyL( TUint32 aHandle, TUint32 aPropCode, TBool aDiscoveryFile )
    {
    __FLOG(_L8("+CMTPPictBridgeDpGetObjectPropList::ServiceOneObjectPropertyL()"));
    
    if ( iFramework.ObjectMgr().ObjectOwnerId( aHandle ) == iFramework.DataProviderId() )
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
            TUint16 value( 1 );
            if( !aDiscoveryFile ) 
                {
                value = iFileEntry.IsReadOnly();
                }
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
            TUint32 value = iObjMeta->Uint(CMTPObjectMetaData::EParentHandle);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetUint32L(CMTPTypeObjectPropListElement::EValue, value);
            iPropertyList->CommitPropElemL(propElem);
            }
            break;
            case EMTPObjectPropCodeObjectSize:
            {
            TUint64 value( 0 );
            if( !aDiscoveryFile ) 
                {
                value = iFileEntry.FileSize();
                }
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
            if ( !aDiscoveryFile )
                {
                iDpSingletons.MTPUtility().TTime2MTPTimeStr(iFileEntry.iModified, dateString);
                }
            else{
                TTime modifiedTime( 0 );
                modifiedTime.HomeTime();
                iDpSingletons.MTPUtility().TTime2MTPTimeStr( modifiedTime, dateString );
                }
                    	
            CMTPTypeString* value = CMTPTypeString::NewLC(dateString);
            CMTPTypeObjectPropListElement& propElem = iPropertyList->ReservePropElemL(aHandle, aPropCode);
            propElem.SetStringL(CMTPTypeObjectPropListElement::EValue, value->StringChars());
            iPropertyList->CommitPropElemL(propElem);
            CleanupStack::PopAndDestroy(value);
        	}
        	break;
            case EMTPObjectPropCodeObjectFileName:
        	{
        	CMTPTypeString* value( NULL );
        	if ( !aDiscoveryFile )
        	    {
        	    value = CMTPTypeString::NewLC(iFileEntry.iName);
        	    }
        	else
        	    {
        	    TParse parse;
        	    parse.Set( iObjMeta->DesC(CMTPObjectMetaData::ESuid), NULL, NULL );
        	    value = CMTPTypeString::NewLC( parse.NameAndExt() );
        	    }
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
            default:
                {
                User::Leave( KErrNotSupported );
                }
                break;
            }
        }
    __FLOG(_L8("-CMTPPictBridgeDpGetObjectPropList::ServiceOneObjectPropertyL()"));
    }

