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

#include "mtpservicecommon.h"
#include "cmtpserviceconfig.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpserviceconfigTraces.h"
#endif



// Class constants.
_LIT(KMTPServiceCofigfile, "z:\\resource\\mtp\\services.rsc");



CMTPServiceConfig::TSupportedService::TSupportedService( const TMTPTypeGuid& aGUID, const TUint aServiceType, const TUint aResID ):
    iGUID(aGUID),
    iServiceType(aServiceType),
    iResourceID(aResID)
    {
    
    }

/**

*/
CMTPServiceConfig* CMTPServiceConfig::NewL( RFs& aRFs)  
    {
    CMTPServiceConfig* self = new (ELeave) CMTPServiceConfig( aRFs );
    CleanupStack::PushL(self);
    self->ConstructL( KMTPServiceCofigfile ); 
    CleanupStack::Pop(self);
    return self;
    }


/**
Destructor
*/    
CMTPServiceConfig::~CMTPServiceConfig()
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_CMTPSERVICECONFIG_DES_ENTRY );
        
    iSupportedServices.Close();
    iServiceInfos.ResetAndDestroy();
    OstTraceFunctionExit0( CMTPSERVICECONFIG_CMTPSERVICECONFIG_DES_EXIT );
    }

TInt CMTPServiceConfig::SupportedServiceOrderFromAscending( const TSupportedService& aL, const TSupportedService& aR)
    {
    return aL.iGUID.Compare( aR.iGUID );
    }

TInt CMTPServiceConfig::SupportedServiceOrderFromKeyAscending( const TMTPTypeGuid* aL, const TSupportedService& aR)
    {
    return aL->Compare( aR.iGUID );
    }

CMTPServiceConfig::CMTPServiceConfig( RFs& aRFs ):
    iRFs(aRFs)
    {
    
    }

void CMTPServiceConfig::ConstructL( const TDesC& aResourceFilename )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_CONSTRUCTL_ENTRY );
    
    
    RResourceFile file;
    file.OpenL( iRFs, aResourceFilename );
    CleanupClosePushL(file);
       
    
    // Create the resource reader.
    const TInt KDefaultResourceId(1);
    HBufC8* buffer(file.AllocReadLC(KDefaultResourceId));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    InitializeL( reader );
    
    CleanupStack::PopAndDestroy(buffer);
    CleanupStack::PopAndDestroy(&file);
    
    OstTraceFunctionExit0( CMTPSERVICECONFIG_CONSTRUCTL_EXIT );
    }


void CMTPServiceConfig::InitializeL( TResourceReader& aReader )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_INITIALIZEL_ENTRY );
    
    //read the supported services
    TUint serviceCount = aReader.ReadInt16();

    TUint resID(0);
    TUint type(EMTPServiceTypeAbstract);
    while( serviceCount-- )
        {
        TMTPTypeGuid svcGUID = ReadGUID( aReader );
        type = aReader.ReadUint32();
        resID = aReader.ReadUint32();
        iSupportedServices.InsertInOrderL( TSupportedService( svcGUID, type, resID ), TLinearOrder<TSupportedService>(SupportedServiceOrderFromAscending) );      
        }
    

    OstTraceFunctionExit0( CMTPSERVICECONFIG_INITIALIZEL_EXIT );
    }

TBool CMTPServiceConfig::IsSupportedService( const TMTPTypeGuid& aServiceGUID ) const
    {   
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_ISSUPPORTEDSERVICE_ENTRY );
    OstTraceFunctionExit0( CMTPSERVICECONFIG_ISSUPPORTEDSERVICE_EXIT );
    return !( KErrNotFound == iSupportedServices.FindInOrder( aServiceGUID, SupportedServiceOrderFromKeyAscending  ) );
    }

void CMTPServiceConfig::LoadServiceDataL( const TMTPTypeGuid& aServiceGUID )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_LOADSERVICEDATAL_ENTRY );
    
    RResourceFile file;
    file.OpenL( iRFs, KMTPServiceCofigfile );
    CleanupClosePushL(file);
   

    // Create the resource reader.
    HBufC8* buffer(file.AllocReadLC( ServiceResourceIDL( aServiceGUID ) ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    CMTPServiceInfo* serviceInfo = CMTPServiceInfo::NewLC();
    const TUint KStorageID = 0;
    serviceInfo->SetServiceStorageID( KStorageID );
    
    //read pguid
    TMTPTypeGuid KPGUID( ReadGUID( reader ) );
    serviceInfo->SetPersistentGUIDL( KPGUID );
    
    //read version
    serviceInfo->SetServiceVersion( reader.ReadUint32() );
    
    //read service guid
    TMTPTypeGuid KSvcGUID( ReadGUID( reader ) );
    serviceInfo->SetServiceGUIDL( KSvcGUID );
    
    HBufC* name = reader.ReadHBufCL();
    //read service name
    serviceInfo->SetServiceName( name );

    //read service type
    serviceInfo->SetServiceType( reader.ReadUint32() );
    
    //read service base service id
    serviceInfo->SetBaseServiceID( reader.ReadUint32() );
    
    //read use service guids
    TUint resID = reader.ReadUint32();
    StartReadUseServicesL( file, resID, *serviceInfo );
    
    //read service properties
    TUint resNamespace = reader.ReadUint32();
    resID = reader.ReadUint32();
    StartReadServicePropertiesL( file,resNamespace, resID, *serviceInfo );
    
    //read service formats
    resID = reader.ReadUint32();
    StartReadServiceFormatsL( file, resID, *serviceInfo );
    
    //read service methods
    resID = reader.ReadUint32();
    StartReadServiceMethodsL( file, resID, *serviceInfo );

    //read service events
    resID = reader.ReadUint32();
    StartReadServiceEventsL( file, resID, *serviceInfo );
    
    //read service data block.
    resID = reader.ReadUint32();
    StartReadServiceDataBlockL( file, resID, *serviceInfo );
    
    iServiceInfos.AppendL(serviceInfo);
    CleanupStack::Pop(serviceInfo);
    CleanupStack::PopAndDestroy(buffer);
    CleanupStack::PopAndDestroy(&file);

    OstTraceFunctionExit0( CMTPSERVICECONFIG_LOADSERVICEDATAL_EXIT );
    }

void CMTPServiceConfig::StartReadUseServicesL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADUSESERVICESL_ENTRY );
    
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    TUint num  = reader.ReadInt16();
    while( num-- )
        {
        TMTPTypeGuid KGUID(ReadGUID(reader));
        aServiceInfo.AppendUserServiceL( KGUID );
        }
    
    CleanupStack::PopAndDestroy(buffer);

    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADUSESERVICESL_EXIT );
    }

void CMTPServiceConfig::StartReadServicePropertyNamespacesL(  RResourceFile& aResFile, const TUint aResID, RArray<TMTPTypeGuid>& aNamespaces )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEPROPERTYNAMESPACESL_ENTRY );
        
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    //read namespaces
    TUint num = reader.ReadInt16();
    while( num-- )
        {
        aNamespaces.AppendL( ReadGUID(reader) );
        }
    
    CleanupStack::PopAndDestroy(buffer);
    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEPROPERTYNAMESPACESL_EXIT );
    }

void CMTPServiceConfig::StartReadServicePropertiesL(  RResourceFile& aResFile, const TUint aNamespaceResID, const TUint aPropertiesResID, CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEPROPERTIESL_ENTRY );
    
    //read namespaces
    RArray<TMTPTypeGuid> namespaces;
    CleanupClosePushL(namespaces);
    StartReadServicePropertyNamespacesL(aResFile, aNamespaceResID, namespaces);
    
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aPropertiesResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    //read properties  
    TUint num = reader.ReadInt16();
    TUint namespaceid = 0 ;
    TUint pkeyid = 0;
    HBufC* name( NULL );
    TUint16 datatype = 0;
    TUint8  getset = 0;
    TUint8  formflag = 0;
    CServiceProperty* newElement = NULL;
    while( num-- )
        {
        namespaceid = reader.ReadUint32();
        pkeyid = reader.ReadUint32();
        name = reader.ReadHBufCL();
        datatype = reader.ReadUint32();
        getset = reader.ReadUint8();
        formflag = reader.ReadUint8();
        
        newElement = CServiceProperty::NewLC(namespaces[namespaceid], pkeyid, name, datatype, getset, formflag);
        aServiceInfo.InsertPropertyL( newElement );
        CleanupStack::Pop(newElement);
        }
    
    CleanupStack::PopAndDestroy(buffer);
    CleanupStack::PopAndDestroy(&namespaces);
    
    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEPROPERTIESL_EXIT );
    }

void CMTPServiceConfig::StartReadServiceFormatsL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEFORMATSL_ENTRY );
    
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    TUint num = reader.ReadInt16();
    HBufC* name( NULL );
    HBufC* mimetype1( NULL );
    CServiceFormat* newElement = NULL;
    while( num-- )
        {
        TMTPTypeGuid guid = ReadGUID( reader );
        name = reader.ReadHBufCL();   
		CleanupStack::PushL(name);

        mimetype1 = reader.ReadHBufCL();
        if( NULL == mimetype1 )
            {
			//If mimetype is not configed in the config file, then alloc a zero length buffer for it. The mimetype is treated as an empty string for this situation.
            mimetype1 = HBufC16::NewLC(0);
            }
        else
            {
            CleanupStack::PushL(mimetype1);
            }
        CleanupStack::Pop(2);
        newElement = CServiceFormat::NewLC( guid, name, reader.ReadUint16() ,mimetype1 );
		
        aServiceInfo.InsertFormatL( newElement );
        CleanupStack::Pop(newElement);
        }
    
    CleanupStack::PopAndDestroy(buffer);
    
    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEFORMATSL_EXIT );
    }


void CMTPServiceConfig::StartReadServiceMethodsL(  RResourceFile& aResFile, const TUint aResID,  CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEMETHODSL_ENTRY );
    
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    TUint num = reader.ReadInt16();
    HBufC* name( NULL );
    CServiceMethod* newElement = NULL;
    while( num-- )
        {
        TMTPTypeGuid guid = ReadGUID( reader );
        name = reader.ReadHBufCL();       
        
        newElement = CServiceMethod::NewLC(guid, name, reader.ReadUint16());
        aServiceInfo.InsertMethodL( newElement );
        CleanupStack::Pop(newElement);
        }
    
    CleanupStack::PopAndDestroy(buffer);
    
    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEMETHODSL_EXIT );
    }

void CMTPServiceConfig::StartReadServiceEventsL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEEVENTSL_ENTRY );
      
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    TUint num = reader.ReadInt16();
    TUint16 datacode(0); 
    CServiceEvent* newElement = NULL;
    while( num-- )
        {
        datacode = reader.ReadUint16();
        TMTPTypeGuid guid = ReadGUID( reader );     
        
        newElement = CServiceEvent::NewLC(datacode, guid, reader.ReadHBufCL() );
        aServiceInfo.InsertEventL( newElement );
        CleanupStack::Pop(newElement);
        }
      
    CleanupStack::PopAndDestroy(buffer);

    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEEVENTSL_EXIT );
    }

void CMTPServiceConfig::StartReadServiceDataBlockL(  RResourceFile& aResFile, const TUint aResID, CMTPServiceInfo& aServiceInfo )
    {
    OstTraceFunctionEntry0( CMTPSERVICECONFIG_STARTREADSERVICEDATABLOCKL_ENTRY );
    
    // Create the resource reader.
    HBufC8* buffer(aResFile.AllocReadLC( aResID ));
    TResourceReader reader;
    reader.SetBuffer(buffer);
    
    TUint num = reader.ReadInt16();
    while( num-- )
        {
        TMTPTypeGuid guid = ReadGUID( reader );
              
        aServiceInfo.AppendDataGUIDL( guid );
        }
    
    CleanupStack::PopAndDestroy(buffer);
    OstTraceFunctionExit0( CMTPSERVICECONFIG_STARTREADSERVICEDATABLOCKL_EXIT );
    }

CMTPServiceInfo* CMTPServiceConfig::ServiceInfo(const TMTPTypeGuid& aServiceGUID ) const
    {
    TInt count = iServiceInfos.Count();
    for(TInt i(0); i < count; i++ )
        {
        if(iServiceInfos[i]->ServicePersistentGUID().Equal(aServiceGUID))
            return iServiceInfos[i] ;
        }
    
    return NULL;
    }

CMTPServiceInfo* CMTPServiceConfig::ServiceInfo(const TUint aServiceID) const
	{
	OstTraceFunctionEntry0( CMTPSERVICECONFIG_SERVICEINFO_ENTRY );
	
	TInt count = iServiceInfos.Count();
    for(TInt i(0); i < count; i++ )
        {
        if(iServiceInfos[i]->ServiceID() == aServiceID)
            return iServiceInfos[i] ;
        }
    
    OstTraceFunctionExit0( CMTPSERVICECONFIG_SERVICEINFO_EXIT );
    return NULL;
	
	}

TInt CMTPServiceConfig::ServiceTypeOfSupportedService( const TMTPTypeGuid& aServiceGUID ) const
    {
    TInt index = iSupportedServices.FindInOrder( aServiceGUID, SupportedServiceOrderFromKeyAscending ) ;
       if( KErrNotFound == index)
           return KErrNotFound;
       
       return iSupportedServices[index].iServiceType;
    }

TUint CMTPServiceConfig::ServiceResourceIDL( const TMTPTypeGuid& aServiceGUID ) const
    {
    TInt index = iSupportedServices.FindInOrder( aServiceGUID, SupportedServiceOrderFromKeyAscending ) ;
    if( KErrNotFound == index)
        {
        OstTrace0( TRACE_ERROR, CMTPSERVICECONFIG_SERVICERESOURCEIDL, "serviceGUID not supported" );
        User::Leave( KErrNotFound );
        }
    
    return iSupportedServices[index].iResourceID;
    }

TMTPTypeGuid CMTPServiceConfig::ReadGUID( TResourceReader& aReader )
    {
    TUint upper2 = aReader.ReadUint32();
    TUint upper1 = aReader.ReadUint32();
    TUint lower2 = aReader.ReadUint32();
    TUint lower1 = aReader.ReadUint32();
    
    return TMTPTypeGuid( MAKE_TUINT64( upper2, upper1 ), MAKE_TUINT64( lower2, lower1 ) );
    }


