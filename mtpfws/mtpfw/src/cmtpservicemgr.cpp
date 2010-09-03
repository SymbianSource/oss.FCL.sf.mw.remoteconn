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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpservicemgrTraces.h"
#endif



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
	OstTraceFunctionEntry0( CMTPSERVICEMGR_CONSTRUCTL_ENTRY );
	
    iSingletons.OpenL();
	iServiceCfg = CMTPServiceConfig::NewL( iSingletons.Fs() );

	OstTraceFunctionExit0( CMTPSERVICEMGR_CONSTRUCTL_EXIT );
	}
/**
Destructor
*/    
CMTPServiceMgr::~CMTPServiceMgr()
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_CMTPSERVICEMGR_DES_ENTRY );
    
    delete iServiceCfg;
    
    iSingletons.Close();
    
    iServiceIDs.Close();

    OstTraceFunctionExit0( CMTPSERVICEMGR_CMTPSERVICEMGR_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPSERVICEMGR_ENABLESERVICE_ENTRY );
    
    if( NULL == iServiceCfg->ServiceInfo(aPGUID)  )
        {
        TRAPD(err,LoadServiceL( aPGUID ));
        if( KErrNone == err)
            {
            iServiceCfg->ServiceInfo(aPGUID)->SetServiceID( aServiceID );
            }
        else
            OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, CMTPSERVICEMGR_ENABLESERVICE, 
                    "CMTPServiceMgr::EnableService - Fail to Load service! error = %d", err);
        OstTraceFunctionExit0( CMTPSERVICEMGR_ENABLESERVICE_EXIT );
        return err;
        }
    
    OstTrace0(TRACE_NORMAL, DUP1_CMTPSERVICEMGR_ENABLESERVICE, "CMTPServiceMgr::EnableService - Has been loaded!");
    OstTraceFunctionExit0( DUP1_CMTPSERVICEMGR_ENABLESERVICE_EXIT );
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
	OstTraceFunctionEntry0( CMTPSERVICEMGR_LOADSERVICEL_ENTRY );

	iServiceCfg->LoadServiceDataL(aPGUID);

	OstTraceFunctionExit0( CMTPSERVICEMGR_LOADSERVICEL_EXIT );
	}

TInt CMTPServiceMgr::GetServiceProperty( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, CServiceProperty** aServicePropertye) const
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEPROPERTY_ENTRY );

    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceProperty* prop = svcinfo->ServiceProperty( aPKNamespace, aPKID );
    if( NULL == prop)
       return KErrNotSupported;
    
    *aServicePropertye = prop;

    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEPROPERTY_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceFormat( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceFormat** aServiceFormat ) const
   {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEFORMAT_ENTRY );

    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceFormat* format = svcinfo->ServiceFormat( aGUID );
    if( NULL == format)
       return KErrNotSupported;
    
    *aServiceFormat = format;

    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEFORMAT_EXIT );
    return KErrNone;
   }

TInt CMTPServiceMgr::GetServiceMethod( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceMethod** aServiceMethod ) const
   {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEMETHOD_ENTRY );
    
    CMTPServiceInfo* svcinfo = iServiceCfg->ServiceInfo( aServicePGUID );
    if( NULL == svcinfo )
       return KErrNotSupported;
    
    CServiceMethod* method = svcinfo->ServiceMethod( aGUID );
    if( NULL == method)
       return KErrNotSupported;
    
    *aServiceMethod = method ;
    
    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEMETHOD_EXIT );
    return KErrNone;
   }

TInt CMTPServiceMgr::GetServiceId( const TMTPTypeGuid& aServiceGUID, TUint& aServiceID) const
	{
	OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEID_ENTRY );
	
	CMTPServiceInfo* svcinfo = ServiceInfo( aServiceGUID );
	
	if( NULL ==  svcinfo )
	    {
	    OstTrace0(TRACE_ERROR, CMTPSERVICEMGR_GETSERVICEID, "CMTPServiceMgr::GetServiceId - Invalid serviceID");   
	    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEID_EXIT );
	    return KErrNotFound;
	    }
	else
	    {
	    aServiceID = svcinfo->ServiceID();
	    
	    OstTrace1(TRACE_NORMAL, DUP1_CMTPSERVICEMGR_GETSERVICEID, 
	            "CMTPServiceMgr::GetServiceId = %d", aServiceID);
	    OstTraceFunctionExit0( DUP1_CMTPSERVICEMGR_GETSERVICEID_EXIT );
	    return KErrNone;
	    }
	}

TInt CMTPServiceMgr::GetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint& aServicePropertyCode ) const
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEPROPERTYCODE_ENTRY );
    
    CServiceProperty* prop = NULL;
    TInt err =  GetServiceProperty( aServicePGUID, aPKNamespace, aPKID, &prop );
    if( KErrNone != err)
        return err;
    
    aServicePropertyCode = prop->Code();

    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEPROPERTYCODE_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, const TUint aCurrPropertyCode )
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_SETSERVICEPROPERTYCODE_ENTRY );
    
    CServiceProperty* prop = NULL;
    TInt err =  GetServiceProperty( aServicePGUID, aPKNamespace, aPKID, &prop );
    if( KErrNone != err)
        return err;
    
    prop->SetCode( aCurrPropertyCode );
    
    OstTraceFunctionExit0( CMTPSERVICEMGR_SETSERVICEPROPERTYCODE_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceFormatCode ) const
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEFORMATCODE_ENTRY );

    CServiceFormat* format = NULL;
    TInt err = GetServiceFormat( aServicePGUID, aGUID, &format );
    if( KErrNone != err )
        return err;
    
    aServiceFormatCode = format->Code();

    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEFORMATCODE_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrFormatCode )
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_SETSERVICEFORMATCODE_ENTRY );

    CServiceFormat* format = NULL;
    TInt err = GetServiceFormat( aServicePGUID, aGUID, &format );
    if( KErrNone != err )
       return err;
    
    format->SetCode( aCurrFormatCode );

    OstTraceFunctionExit0( CMTPSERVICEMGR_SETSERVICEFORMATCODE_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::GetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceMethodCode ) const
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_GETSERVICEMETHODCODE_ENTRY );

    CServiceMethod* method = NULL;
    TInt err = GetServiceMethod( aServicePGUID, aGUID, &method );
    if ( KErrNone != err )
        return err;
    
    aServiceMethodCode = method->Code();

    OstTraceFunctionExit0( CMTPSERVICEMGR_GETSERVICEMETHODCODE_EXIT );
    return KErrNone;
    }

TInt CMTPServiceMgr::SetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrMethodCode )
    {
    OstTraceFunctionEntry0( CMTPSERVICEMGR_SETSERVICEMETHODCODE_ENTRY );
    
    CServiceMethod* method = NULL;
    TInt err = GetServiceMethod( aServicePGUID, aGUID, &method );
    if ( KErrNone != err )
        return err;
    
    method->SetCode( aCurrMethodCode );
    
    OstTraceFunctionExit0( CMTPSERVICEMGR_SETSERVICEMETHODCODE_EXIT );
    return KErrNone;
    }


