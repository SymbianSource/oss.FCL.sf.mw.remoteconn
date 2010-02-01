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

__FLOG_STMT(_LIT8(KComponent,"ObjectBrowser");)


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
    delete iObjMetaCache;
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

void CMTPObjectBrowser::GoL( TUint32 aFormatCode, TUint32 aHandle, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const
    {
    __FLOG_VA( ( _L8("+GoL( 0x%08X, 0x%08X, %d )"), aFormatCode, aHandle, aDepth ) );
    
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
    
    __FLOG( _L8("-GoL") );
    }

CMTPObjectBrowser::CMTPObjectBrowser( MMTPDataProviderFramework& aDpFw ):
    iDpFw( aDpFw )
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }

void CMTPObjectBrowser::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    iObjMetaCache = CMTPObjectMetaData::NewL();
    __FLOG( _L8("-ConstructL") );
    }

void CMTPObjectBrowser::GetObjectHandlesL( TUint32 aCurDepth, TUint32 aStorageId, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    __FLOG_VA( ( _L8("+GetObjectHandlesL( %d, 0x%08X, 0x%08X, %d, 0x%08X )"), aCurDepth, aStorageId, aFormatCode, aDepth, aParentHandle ) );
    
    RMTPObjectMgrQueryContext   context;
    RArray< TUint >             handles;
    TMTPObjectMgrQueryParams    params( aStorageId, aFormatCode, aParentHandle );
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
                aBrowseCallback.iCallback( aBrowseCallback.iContext, handles[i], aCurDepth );
                }
            }
        }
    while ( !context.QueryComplete() );
    
    CleanupStack::PopAndDestroy( &handles );
    CleanupStack::PopAndDestroy( &context );
    
    __FLOG( _L8("-GetObjectHandlesL") );
    }

void CMTPObjectBrowser::GetFolderObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    __FLOG_VA( ( _L8("+GetFolderObjectHandlesL( %d, 0x%08X, %d, 0x%08X )"), aCurDepth, aFormatCode, aDepth, aParentHandle ) );
    
    if ( 0 == aDepth )
        {
        aBrowseCallback.iCallback( aBrowseCallback.iContext, aParentHandle, aCurDepth );
        }
    else
        {
        GetObjectHandlesL( aCurDepth + 1, KMTPStorageAll, aFormatCode, aDepth - 1, aParentHandle, aBrowseCallback );
        aBrowseCallback.iCallback( aBrowseCallback.iContext, aParentHandle, aCurDepth );
        }
    
    __FLOG( _L8("-GetFolderObjectHandlesL") );
    }

void CMTPObjectBrowser::GetRootObjectHandlesL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, const TBrowseCallback& aBrowseCallback ) const
    {
    __FLOG_VA( ( _L8("+GetRootObjectHandlesL( %d, 0x%08X, %d )"), aCurDepth, aFormatCode, aDepth ) );
    
    switch ( aDepth )
        {
        case KMaxTUint:
            GetObjectHandlesL( aCurDepth, KMTPStorageAll, aFormatCode, aDepth, KMTPHandleNoParent, aBrowseCallback );
            break;
        case 0:
            // do nothing
            break;
        default:
            GetObjectHandlesL( aCurDepth, KMTPStorageAll, aFormatCode, aDepth, KMTPHandleNoParent, aBrowseCallback );
            break;
        }
    
    __FLOG( _L8("-GetRootObjectHandlesL") );
    }

void CMTPObjectBrowser::GetObjectHandlesTreeL( TUint32 aCurDepth, TUint32 aFormatCode, TUint32 aDepth, TUint32 aParentHandle, const TBrowseCallback& aBrowseCallback ) const
    {
    __FLOG_VA( ( _L8("+GetObjectHandlesTreeL( %d, 0x%08X, %d, 0x%08X )"), aCurDepth, aFormatCode, aDepth, aParentHandle ) );
    
    iDpFw.ObjectMgr().ObjectL( aParentHandle, *iObjMetaCache );
#ifdef __FLOG_ACTIVE
    RBuf suid;
    suid.Create( iObjMetaCache->DesC( CMTPObjectMetaData::ESuid ) );
#endif
    if ( IsFolderFormat( iObjMetaCache->Uint( CMTPObjectMetaData::EFormatCode ), iObjMetaCache->Uint( CMTPObjectMetaData::EFormatSubCode ) ) )
        {
        GetFolderObjectHandlesL( aCurDepth, aFormatCode, aDepth, aParentHandle, aBrowseCallback );
        }
    else
        {
        aBrowseCallback.iCallback( aBrowseCallback.iContext, aParentHandle, aCurDepth );
        }
#ifdef __FLOG_ACTIVE
    __FLOG_1( _L8("recursion_depth: %d"), aCurDepth );
    __FLOG_1( _L("recursion_suid: %S"), &suid );
    suid.Close();
#endif
    __FLOG( _L8("-GetObjectHandlesTreeL") );
    }


