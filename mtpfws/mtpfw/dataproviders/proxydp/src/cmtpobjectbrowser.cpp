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

#include "cmtpobjectbrowser.h"
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mtpobjectmgrquerytypes.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtprequestchecker.h"
#include "mtpdppanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpobjectbrowserTraces.h"
#endif



CMTPObjectBrowser* CMTPObjectBrowser::NewL( MMTPDataProviderFramework& aDpFw )
    {
    CMTPObjectBrowser* self = new( ELeave ) CMTPObjectBrowser( aDpFw );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

TBool CMTPObjectBrowser::IsFolderFormat( TUint aFmtCode, TUint aFmtSubCode )
    {
    return ( ( EMTPFormatCodeAssociation == aFmtCode ) && ( EMTPAssociationTypeGenericFolder == aFmtSubCode ) );
    }

CMTPObjectBrowser::~CMTPObjectBrowser()
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_CMTPOBJECTBROWSER_ENTRY );
    delete iObjMetaCache;
    OstTraceFunctionExit0( CMTPOBJECTBROWSER_CMTPOBJECTBROWSER_EXIT );
    }

void CMTPObjectBrowser::GoL( TUint32 aFormatCode, TUint32 aHandle, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GOL_ENTRY );
    OstTraceExt3(TRACE_NORMAL, CMTPOBJECTBROWSER_GOL, "0x%08X, 0x%08X, %d",
            aFormatCode, aHandle, aDepth);
    
    switch ( aHandle )
        {
        case KMTPHandleAll:
            GetObjectHandlesL( 0, KMTPStorageAll, aFormatCode, KMaxTUint, KMTPHandleNoParent, aBrowseCallback );
           
            break;
        case KMTPHandleNone:
            GetRootObjectHandlesL( 0, aFormatCode, aDepth, aBrowseCallback );
            break;
        default:
            GetObjectHandlesTreeL( 0, aFormatCode, aDepth, aHandle, aBrowseCallback );
            break;
        }
    
    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GOL_EXIT );
    }

CMTPObjectBrowser::CMTPObjectBrowser( MMTPDataProviderFramework& aDpFw ):
    iDpFw( aDpFw )
    {
    OstTraceFunctionEntry0( DUP1_CMTPOBJECTBROWSER_CMTPOBJECTBROWSER_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPOBJECTBROWSER_CMTPOBJECTBROWSER_EXIT );
    }

void CMTPObjectBrowser::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_CONSTRUCTL_ENTRY );
    iObjMetaCache = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPOBJECTBROWSER_CONSTRUCTL_EXIT );
    }

void CMTPObjectBrowser::GetObjectHandlesL( TUint32 aCurDepth, TUint32 aStorageId, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GETOBJECTHANDLESL_ENTRY );
    OstTraceExt5(TRACE_NORMAL, CMTPOBJECTBROWSER_GETOBJECTHANDLESL, "%d, 0x%08X, 0x%08X, %d, 0x%08X",
            aCurDepth, aStorageId, aFormatCode, aDepth, aParentHandle);
    
    RMTPObjectMgrQueryContext   context;
    RArray< TUint >             handles;
    TMTPObjectMgrQueryParams    params( aStorageId, KMTPFormatsAll, aParentHandle );
    // if parenthandle = 0, depth >0 and != ffffffff, for service dp, 
    // 1. there is no tree structure
    // 2. KMTPFormatsAll may make the low lever api filter the handls of service dp.
    if( (aFormatCode >= EMTPFormatCodeVendorExtStart)&&(aFormatCode <= EMTPFormatCodeVendorExtEnd) )
        {
        params.iFormatCode = aFormatCode ;
        }
     
      
    CleanupClosePushL( context );
    CleanupClosePushL( handles );
    
    do
        {
        iDpFw.ObjectMgr().GetObjectHandlesL( params, context, handles );
        TUint handleCount = handles.Count();
        if ( aDepth > 0 )
            {
            for ( TUint i = 0; i < handleCount; i++ )
                {
                GetObjectHandlesTreeL( aCurDepth, aFormatCode, aDepth, handles[i], aBrowseCallback );
                }
            }
        else
            {
            for ( TUint i = 0; i < handleCount; i++ )
                {
                //before insert the handle to th result list , check it's format.
                iDpFw.ObjectMgr().ObjectL( handles[i], *iObjMetaCache );
                if ( (KMTPFormatsAll == aFormatCode) || ( aFormatCode == iObjMetaCache->Uint( CMTPObjectMetaData::EFormatCode )) )
                   {
                   aBrowseCallback.iCallback( aBrowseCallback.iContext, handles[i], aCurDepth );
                   }
           
                }
            }
        }
    while ( !context.QueryComplete() );
    
    CleanupStack::PopAndDestroy( &handles );
    CleanupStack::PopAndDestroy( &context );

    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GETOBJECTHANDLESL_EXIT );
    }

void CMTPObjectBrowser::GetFolderObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GETFOLDEROBJECTHANDLESL_ENTRY );
    OstTraceExt4(TRACE_NORMAL, CMTPOBJECTBROWSER_GETFOLDEROBJECTHANDLESL, 
            "%d, 0x%08X, %d, 0x%08X", aCurDepth, aFormatCode, aDepth, aParentHandle);
    
    if (  aDepth > 0)
        {
        GetObjectHandlesL( aCurDepth + 1, KMTPStorageAll, aFormatCode, aDepth - 1, aParentHandle, aBrowseCallback );
        }
    
    //before insert the handle to th result list , check it's format.
    iDpFw.ObjectMgr().ObjectL( aParentHandle, *iObjMetaCache );
    if ( (KMTPFormatsAll == aFormatCode) || ( aFormatCode == iObjMetaCache->Uint( CMTPObjectMetaData::EFormatCode )) )
       {
       aBrowseCallback.iCallback( aBrowseCallback.iContext, aParentHandle, aCurDepth );
       }

    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GETFOLDEROBJECTHANDLESL_EXIT );
    }

void CMTPObjectBrowser::GetRootObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GETROOTOBJECTHANDLESL_ENTRY );
    OstTraceExt3(TRACE_NORMAL, CMTPOBJECTBROWSER_GETROOTOBJECTHANDLESL, 
            "%d, 0x%08X, %d", aCurDepth, aFormatCode, aDepth);
    
    if( aDepth > 0)
        {        
                // attention: aDepth should be reduced by 1 here
                GetObjectHandlesL( aCurDepth, KMTPStorageAll, aFormatCode, aDepth-1, KMTPHandleNoParent, aBrowseCallback );
        }
    
    // if aDepth == 0, no handles should be returned.

    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GETROOTOBJECTHANDLESL_EXIT );
    }

void CMTPObjectBrowser::GetObjectHandlesTreeL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GETOBJECTHANDLESTREEL_ENTRY );
    OstTraceExt4(TRACE_NORMAL, CMTPOBJECTBROWSER_GETOBJECTHANDLESTREEL, 
            "%d, 0x%08X, %d, 0x%08X", aCurDepth, aFormatCode, aDepth, aParentHandle);

    iDpFw.ObjectMgr().ObjectL( aParentHandle, *iObjMetaCache );
#ifdef OST_TRACE_COMPILER_IN_USE
    RBuf suid;
    suid.CreateL( iObjMetaCache->DesC( CMTPObjectMetaData::ESuid ) );
#endif
    if ( IsFolderFormat( iObjMetaCache->Uint( CMTPObjectMetaData::EFormatCode ), iObjMetaCache->Uint( CMTPObjectMetaData::EFormatSubCode ) ) )
        {
        GetFolderObjectHandlesL( aCurDepth, aFormatCode, aDepth, aParentHandle, aBrowseCallback );
        }
    else if ( (KMTPFormatsAll == aFormatCode) || ( aFormatCode == iObjMetaCache->Uint( CMTPObjectMetaData::EFormatCode )) )
        {
        aBrowseCallback.iCallback( aBrowseCallback.iContext, aParentHandle, aCurDepth );
        }
    else
        {
          // format doesn't match, do nothing
        }
#ifdef OST_TRACE_COMPILER_IN_USE
    OstTrace1( TRACE_NORMAL, DUP1_CMTPOBJECTBROWSER_GETOBJECTHANDLESTREEL, "recursion_depth: %d", aCurDepth );
    OstTraceExt1( TRACE_NORMAL, DUP2_CMTPOBJECTBROWSER_GETOBJECTHANDLESTREEL, "recursion_suid: %S", suid );
    suid.Close();
#endif
    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GETOBJECTHANDLESTREEL_EXIT );
    }

/**
 * This function retrives all the object handles with the format code of aFormatCode.
 * 
 * If aFormatCode == KMTPFormatsAll, only Legacy dp's object handles are returned. Object
 * Handles of service dp's will not be returned.
 * 
 */

void CMTPObjectBrowser::GetAllObjectHandlesL(TUint32 aFormatCode,const TBrowseCallback& aBrowseCallback ) const
    {
    OstTraceFunctionEntry0( CMTPOBJECTBROWSER_GETALLOBJECTHANDLESL_ENTRY );

    RMTPObjectMgrQueryContext   context;
    RArray< TUint >             handles;
    TMTPObjectMgrQueryParams    params( KMTPStorageAll, aFormatCode, KMTPHandleNone );
    
   
    CleanupClosePushL( context );
    CleanupClosePushL( handles );
      
    do
        {
        iDpFw.ObjectMgr().GetObjectHandlesL( params, context, handles );
        TUint handleCount = handles.Count();
        
        for ( TUint i = 0; i < handleCount; i++ )
          {
            /* the argument aCurDepth of the callback is set to 0, since the call back never use this argument. 
             * If aCurDepth is used later, this arugment shoud be changed to the valid value.
             */
            aBrowseCallback.iCallback( aBrowseCallback.iContext, handles[i], 0 );
          }
        
        }
    while ( !context.QueryComplete() );
      
    CleanupStack::PopAndDestroy( &handles );
    CleanupStack::PopAndDestroy( &context );
    
    OstTraceFunctionExit0( CMTPOBJECTBROWSER_GETALLOBJECTHANDLESL_EXIT );
    }

