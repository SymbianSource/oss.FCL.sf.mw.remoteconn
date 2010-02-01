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


#ifndef CMTPSERVICEMGR_H_
#define CMTPSERVICEMGR_H_

#include <e32base.h>
#include <e32cmn.h>


#include "cmtpserviceinfo.h"
#include "rmtpframework.h"
#include "mtpdebug.h"

class CMTPServiceConfig;

/** 
Implements the MTP service manager interface.
@internalComponent
 
*/

class CMTPServiceMgr : public CBase
	{
public:   
 
    static CMTPServiceMgr* NewL();
    ~CMTPServiceMgr();	
    
private:
    
    CMTPServiceMgr();
    void ConstructL();

public:    
	TInt GetServiceId(const TMTPTypeGuid& aServiceGUID, TUint& aServiceID) const;
	IMPORT_C const RArray<TUint>& GetServiceIDs() const;
	
	TInt InsertServiceId(const TUint aServiceId);
	
	TInt GetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, TUint& aServicePropertyCode ) const;
	TInt SetServicePropertyCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, const TUint aCurrPropertyCode );
	
	TInt GetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceFormatCode ) const;
	TInt SetServiceFormatCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrFormatCode );
	
	TInt GetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, TUint& aServiceFormatCode ) const;
    TInt SetServiceMethodCode( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, const TUint aCurrMethodCode );
	
    IMPORT_C TBool IsSupportedService( const TMTPTypeGuid& aPGUID ) const;
    IMPORT_C TBool IsSupportedService( const TUint aServiceID ) const;
    
	TInt  EnableService(const TMTPTypeGuid& aPGUID, const TUint aServiceID );
	TInt ServiceTypeOfSupportedService( const TMTPTypeGuid& aPGUID ) const;
	
	
	IMPORT_C CMTPServiceInfo* ServiceInfo(const TMTPTypeGuid& aServiceGUID ) const;
	IMPORT_C CMTPServiceInfo* ServiceInfo(const TUint aServiceID) const;
    IMPORT_C TBool IsServiceFormatCode(const TUint32 aDatacode ) const;

private:
    void LoadServiceL(const TMTPTypeGuid& aPGUID);
    TInt GetServiceProperty( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aPKNamespace, const TUint aPKID, CServiceProperty** aServicePropertye) const;
    TInt GetServiceFormat( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceFormat** aServiceFormat ) const;
    TInt GetServiceMethod( const TMTPTypeGuid& aServicePGUID, const TMTPTypeGuid& aGUID, CServiceMethod** aServiceMethod ) const;
private:

    /**
    The framework singletons.
    */
    RMTPFramework                   iSingletons;

    /*
     *  Service IDs store
     */
    RArray<TUint>     iServiceIDs;

    /**
     * Service Config
     */
	CMTPServiceConfig*	iServiceCfg;
	
	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;    
    
	};

#endif /*CMTPSERVICEMGR_H_*/
