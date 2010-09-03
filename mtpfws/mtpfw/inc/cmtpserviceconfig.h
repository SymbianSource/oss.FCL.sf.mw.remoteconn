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

#ifndef CMTPSERVICECONFIG_H_
#define CMTPSERVICECONFIG_H_

#include <e32base.h>
#include <e32des8.h> 
#include <barsc.h>
#include <barsread.h>
#include <e32cmn.h>

#include <mtp/tmtptypeuint128.h>
#include <mtp/tmtptypeguid.h>
#include "cmtpserviceinfo.h"

class TResourceReader;


class CMTPServiceConfig : public CBase
	{
private:

    class TSupportedService
        {
        public:
            TSupportedService( const TMTPTypeGuid& aGUID, const TUint aServiceType, const TUint aResID );
        public:
            TMTPTypeGuid iGUID;
            TUint   iServiceType;
            TUint   iResourceID;
        };
    
public:   
 
    static CMTPServiceConfig* NewL( RFs& aRFs );
    ~CMTPServiceConfig();	
    
    static TInt SupportedServiceOrderFromAscending( const TSupportedService& aL, const TSupportedService& aR);
    static TInt SupportedServiceOrderFromKeyAscending( const TMTPTypeGuid* aL, const TSupportedService& aR);

public:
    TBool IsSupportedService( const TMTPTypeGuid& aServiceGUID ) const;
    void LoadServiceDataL( const TMTPTypeGuid& aServiceGUID );
    CMTPServiceInfo* ServiceInfo(const TMTPTypeGuid& aServiceGUID ) const;
    CMTPServiceInfo* ServiceInfo(const TUint aServiceID) const;
    TInt ServiceTypeOfSupportedService( const TMTPTypeGuid& aServiceGUID ) const;
    
private:
    
	CMTPServiceConfig( RFs& aRFs );
    void ConstructL( const TDesC& aResourceFilename );
    void InitializeL( TResourceReader& aReader );
    TUint ServiceResourceIDL( const TMTPTypeGuid& aServiceGUID ) const;
    
    void StartReadUseServicesL( RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo );
    void StartReadServicePropertyNamespacesL(  RResourceFile& aResFile, const TUint aResID, RArray<TMTPTypeGuid>& aNamespaces );
    void StartReadServicePropertiesL(  RResourceFile& aResFile, const TUint aNamespaceResID, const TUint aPropertiesResID, CMTPServiceInfo& aServiceInfo );
    void StartReadServiceFormatsL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo );
    void StartReadServiceMethodsL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo );
    void StartReadServiceEventsL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo );
    void StartReadServiceDataBlockL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo );
    
    TMTPTypeGuid ReadGUID( TResourceReader& aReader );
    
private:
    
    RFs&                             iRFs;
    
    /**
     * service info store after loaded.
     */
    RPointerArray<CMTPServiceInfo>	        iServiceInfos;

	/*
	 * supported services
	 */
	RArray<TSupportedService>  iSupportedServices;
		
	};

#endif /*CMTPSERVICECONFIG_H_*/
