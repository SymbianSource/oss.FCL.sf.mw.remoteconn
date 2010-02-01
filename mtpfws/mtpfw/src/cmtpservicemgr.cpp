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

/**
@file
@internalComponent
*/ 
#include <e32cmn.h>

#include "mtpservicecommon.h"
#include "cmtpserviceconfig.h"
#include "cmtpservicemgr.h"
#include <mtp/mtpprotocolconstants.h>

__FLOG_STMT(_LIT8(KComponent,"ServiceMgr");)


/**

*/
CMTPServiceMgr* CMTPServiceMgr::NewL()
    {
    CMTPServiceMgr* self = new (ELeave) CMTPServiceMgr();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CMTPServiceMgr::CMTPServiceMgr()
    {
    
    }

void CMTPServiceMgr::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	
    iSingletons.OpenL();
	iServiceCfg = CMTPServiceConfig::NewL( iSingletons.Fs() );
	   
	__FLOG(_L8("ConstructL - Exit"));
	}
/**
Destructor
*/    
CMTPServiceMgr::~CMTPServiceMgr()
    {
    __FLOG(_L8("~CMTPServiceMgr - Entry"));
    
    delete iServiceCfg;
    
    iSingletons.Close();
    
    iServiceIDs.Close();
    
    __FLOG(_L8("~CMTPServiceMgr - Exit"));
    __FLOG_CLOSE;
    }

EXPORT_C TBool CMTPServiceMgr::IsSupportedService( const TUint aServiceID ) const
    {
    return ( ServiceInfo( aServiceID ) != NULL );
    }

EXPORT_C TBool CMTPServiceMgr::IsSupportedService( const TMTPTypeGuid& aPGUID ) const
    {
    return iServiceCfg->IsSupportedService( aPGUID );
    }

TInt CMTPServiceMgr::EnableService(const TMTPTypeGuid& aPGUID, const TUint aServiceID )
    {
    __FLOG(_L8("CMTPServiceMgr::EnableService : "));
    
    if( NULL == iServiceCfg->ServiceInfo(aPGUID)  )
        {
        TRAPD(err,LoadServiceL( aPGUID ));
        if( KErrNone == err)
            {
            iServiceCfg->ServiceInfo(aPGUID)->SetServiceID( aServiceID );
            }
        
        __FLOG_1(_L8("CMTPServiceMgr::EnableService - Fail to Load service! error = %d "), err );
        return err;
        }
    
    __FLOG(_L8("CMTPServiceMgr::EnableService - Has been loaded!"));
    
    return KErrNone;
    }

TInt CMTPServiceMgr::ServiceTypeOfSupportedService( const TMTPTypeGuid& aPGUID ) const
    {
    return iServiceCfg->ServiceTypeOfSupportedService( aPGUID );
    }

EXPORT_C CMTPServiceInfo* CMTPServiceMgr::ServiceInfo(const TMTPTypeGuid& aServiceGUID )const
    {
    return iServiceCfg->ServiceInfo( aServiceGUID );
    }

EXPORT_C CMTPServiceInfo* CMTPServiceMgr::ServiceInfo(const TUint aServiceID) const
    {
    return iServiceCfg->ServiceInfo( aServiceID );
    }

EXPORT_C TBool CMTPServiceMgr::IsServiceFormatCode(const TUint32 aDatacode ) const
    {
    return ( (EMTPFormatCodeVendorExtDynamicStart <= aDatacode) && ( aDatacode <= EMTPFormatCodeVendorExtDynamicEnd ) );
    }

EXPORT_C const RArray<TUint>& CMTPServiceMgr::GetServiceIDs() const
	{
	return iServiceIDs;
	}

TInt CMTPServiceMgr::InsertServiceId(const TUint aServiceId)
    {
    return iServiceIDs.InsertInOrder( aServiceId );
    }

void CMTPServiceMgr::LoadServiceL( const TMTPTypeGuid& aPGUID )
	{
	__FLOG(_L8("CMTPServiceMgr::LoadServiceL - Entry"));

	iServiceCfg->LoadServiceDataL(aPGUID);
	
    __FLOG(_L8("CMTPServiceMgr::LoadServiceL - Exit"));
	}

TInt CMTPServiceMgr::GetServiceProperty( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, CServiceProperty** aServicePropertye) const
    {
    __FLOG(_L8("CMTPServiceMgr::GetServiceProperty :"));
        
    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceProperty* prop = svcinfo->ServiceProperty( aPKNamespace, aPKID );
    if( NULL == prop)
       return KErrNotSupported;
    
    *aServicePropertye = prop;
    
    __FLOG(_L8("CMTPServiceMgr::GetServiceProperty Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceFormat( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceFormat** aServiceFormat ) const
   {
    __FLOG(_L8("CMTPServiceMgr::GetServiceFormat :"));
    
    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceFormat* format = svcinfo->ServiceFormat( aGUID );
    if( NULL == format)
       return KErrNotSupported;
    
    *aServiceFormat = format;
   
    __FLOG(_L8("CMTPServiceMgr::GetServiceFormat Exit"));
    return KErrNone;
   }

TInt CMTPServiceMgr::GetServiceMethod( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceMethod** aServiceMethod ) const
   {
    __FLOG(_L8("CMTPServiceMgr::GetServiceMethod :"));
    
    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceMethod* method = svcinfo->ServiceMethod( aGUID );
    if( NULL == method)
       return KErrNotSupported;
    
    *aServiceMethod = method ;
    
    __FLOG(_L8("CMTPServiceMgr::GetServiceMethod - Exit"));
    return KErrNone;
   }

TInt CMTPServiceMgr::GetServiceId( const TMTPTypeGuid& aServiceGUID, TUint& aServiceID) const
	{
	__FLOG(_L8("CMTPServiceMgr::FindServiceId :"));
	
	CMTPServiceInfo* svcinfo = ServiceInfo( aServiceGUID );
	
	if( NULL ==  svcinfo )
	    {
	    __FLOG(_L8("CMTPServiceMgr::GetServiceId - Invalid serviceID"));
	    
	    return KErrNotFound;
	    }
	else
	    {
	    aServiceID = svcinfo->ServiceID();
	    
	    __FLOG_1(_L8("CMTPServiceMgr::GetServiceId = %d"),aServiceID );
	    
	    return KErrNone;
	    }
	}

TInt CMTPServiceMgr::GetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint& aServicePropertyCode ) const
    {
    __FLOG(_L8("CMTPServiceMgr::GetServicePropertyCode :"));
    
    CServiceProperty* prop = NULL;
    TInt err =  GetServiceProperty( aServicePGUID, aPKNamespace, aPKID, &prop );
    if( KErrNone != err)
        return err;
    
    aServicePropertyCode = prop->Code();
 
    __FLOG(_L8("CMTPServiceMgr::GetServicePropertyCode - Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, const TUint aCurrPropertyCode )
    {
    __FLOG(_L8("CMTPServiceMgr::SetServicePropertyCode :"));
    
    CServiceProperty* prop = NULL;
    TInt err =  GetServiceProperty( aServicePGUID, aPKNamespace, aPKID, &prop );
    if( KErrNone != err)
        return err;
    
    prop->SetCode( aCurrPropertyCode );
    
    __FLOG(_L8("CMTPServiceMgr::SetServicePropertyCode - Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceFormatCode ) const
    {
    __FLOG(_L8("CMTPServiceMgr::GetServiceFormatCode :"));
    
    CServiceFormat* format = NULL;
    TInt err = GetServiceFormat( aServicePGUID, aGUID, &format );
    if( KErrNone != err )
        return err;
    
    aServiceFormatCode = format->Code();
    
    __FLOG(_L8("CMTPServiceMgr::GetServiceFormatCode - Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrFormatCode )
    {
    __FLOG(_L8("CMTPServiceMgr::SetServiceFormatCode :"));
    
    CServiceFormat* format = NULL;
    TInt err = GetServiceFormat( aServicePGUID, aGUID, &format );
    if( KErrNone != err )
       return err;
    
    format->SetCode( aCurrFormatCode );
    
    __FLOG(_L8("CMTPServiceMgr::SetServiceFormatCode - Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceMethodCode ) const
    {
    __FLOG(_L8("CMTPServiceMgr::GetServiceMethodCode :"));
    
    CServiceMethod* method = NULL;
    TInt err = GetServiceMethod( aServicePGUID, aGUID, &method );
    if ( KErrNone != err )
        return err;
    
    aServiceMethodCode = method->Code();
    
    __FLOG(_L8("CMTPServiceMgr::GetServiceMethodCode - Exit"));
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrMethodCode )
    {
    __FLOG(_L8("CMTPServiceMgr::SetServiceMethodCode :"));
    
    CServiceMethod* method = NULL;
    TInt err = GetServiceMethod( aServicePGUID, aGUID, &method );
    if ( KErrNone != err )
        return err;
    
    method->SetCode( aCurrMethodCode );
    
    __FLOG(_L8("CMTPServiceMgr::SetServiceMethodCode - Exit"));
    return KErrNone;
    }


