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
#include "cmtpserviceinfo.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpserviceinfoTraces.h"
#endif



// Class constants.


TPKeyPair::TPKeyPair( const TMTPTypeGuid& aNamespace, const TUint aID ):
    iNamespace( aNamespace ),
    iPKeyId(aID)
    {
    }


CServiceElementBase::CServiceElementBase(const TMTPTypeGuid& aGUID, HBufC* aName):
    iCode( KInvliadU16DataCode ),
    iGUID( aGUID ),
    iName( aName )
    {
    }

CServiceElementBase::~CServiceElementBase()
    {
    delete iName;
    }

EXPORT_C TBool CServiceElementBase::IsUsed()
    {
    return ( KInvliadU16DataCode != iCode );
    }

EXPORT_C TUint CServiceElementBase::Code() const
    {
    return iCode;
    }
EXPORT_C const TDesC& CServiceElementBase::Name() const
    {
    return *iName;
    }
EXPORT_C const TMTPTypeGuid& CServiceElementBase::GUID() const
    {
    return iGUID;
    }

EXPORT_C void CServiceElementBase::SetCode( const TUint aCode )
    {
    iCode = aCode;
    }

TInt CServiceElementBase::Compare( const CServiceElementBase& aR ) const
    {
    return iGUID.Compare( aR.GUID() );
    }

CServiceProperty::CServiceProperty( const TMTPTypeGuid& aGUID, const TUint aPkeyID, HBufC* aName, const TUint aDataType, const TUint aGetSet, const TUint aFormFlag):
    CServiceElementBase( aGUID, aName ),
    iPKeyId( aPkeyID ),
    iDataType( aDataType ),
    iGetSet( aGetSet ),
    iFormFlag( aFormFlag )
    {
    }

CServiceProperty* CServiceProperty::NewLC(const TMTPTypeGuid& aGUID, const TUint aPkeyID, HBufC* aName, const TUint aDataType, const TUint aGetSet, const TUint aFormFlag)
    {
    CServiceProperty* self = new (ELeave) CServiceProperty(aGUID, aPkeyID, aName, aDataType, aGetSet, aFormFlag);
    CleanupStack::PushL(self);
    self->ConstructL(); 
    return self;
    }

void CServiceProperty::ConstructL()
    {
    }

EXPORT_C const TMTPTypeGuid& CServiceProperty::Namespace() const
    {
    return GUID();
    }
EXPORT_C TUint CServiceProperty::PKeyID() const
    {
    return iPKeyId;
    }

EXPORT_C TUint CServiceProperty::DataType() const
    {
    return iDataType;
    }
EXPORT_C TUint CServiceProperty::GetSet() const
    {
    return iGetSet;
    }
EXPORT_C TUint CServiceProperty::FormFlag() const
    {
    return iFormFlag;
    }
    


CServiceFormat::CServiceFormat(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aBaseFormat, HBufC* aMime ):
    CServiceElementBase( aGUID, aName ),
    iFormatBase(aBaseFormat),
    iMIMEType1(aMime)
    {
    }

CServiceFormat* CServiceFormat::NewLC(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aBaseFormat, HBufC* aMime )
    {
    CServiceFormat* self = new (ELeave) CServiceFormat( aGUID, aName, aBaseFormat, aMime );
    CleanupStack::PushL(self);
    self->ConstructL(); 
    return self;
    }

void CServiceFormat::ConstructL()
    {
    }

CServiceFormat::~CServiceFormat()
	{
	delete iMIMEType1;
	}

EXPORT_C TUint CServiceFormat::FormatBase() const
    {
    return iFormatBase;
    }
EXPORT_C const TDesC& CServiceFormat::MIMEType1() const
    {
    return *iMIMEType1;
    }

CServiceMethod::CServiceMethod(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aAssFormat):
    CServiceElementBase( aGUID, aName ),
    iObjAssociateFormatCode(aAssFormat)
    {
    }

CServiceMethod* CServiceMethod::NewLC(const TMTPTypeGuid& aGUID, HBufC* aName, const TUint aAssFormat)
    {
    CServiceMethod* self = new (ELeave) CServiceMethod( aGUID,  aName, aAssFormat );
    CleanupStack::PushL(self);
    self->ConstructL(); 
    return self;
    }

void CServiceMethod::ConstructL()
    {
    }

EXPORT_C TUint CServiceMethod::ObjAssociateFormatCode() const
    {
    return iObjAssociateFormatCode;
    }


CServiceEvent::CServiceEvent( const TUint aCode, const TMTPTypeGuid& aGUID, HBufC* aName):
    CServiceElementBase( aGUID, aName )
    {
    SetCode(aCode);
    }
CServiceEvent* CServiceEvent:: NewLC(const TUint aCode, const TMTPTypeGuid& aGUID, HBufC* aName)
    {
    CServiceEvent* self = new (ELeave) CServiceEvent( aCode, aGUID, aName );
    CleanupStack::PushL(self);
    self->ConstructL(); 
    return self;
    }

void CServiceEvent::ConstructL()
    {
    }


CMTPServiceInfo* CMTPServiceInfo::NewLC()
    {
    CMTPServiceInfo* self = new (ELeave) CMTPServiceInfo();
    CleanupStack::PushL(self);
    self->ConstructL(); 
    return self;
    }

CMTPServiceInfo::~CMTPServiceInfo()
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_CMTPSERVICEINFO_DES_ENTRY );
    
    delete iServiceName;
    
    iUsedServiceGUIDs.Close();
    iServiceProperties.ResetAndDestroy();
    iServiceFormats.ResetAndDestroy();
    iServiceMethods.ResetAndDestroy();
    iServiceEvents.ResetAndDestroy();
    iDataBlock.Close();

    OstTraceFunctionExit0( CMTPSERVICEINFO_CMTPSERVICEINFO_DES_EXIT );
    }
    

CMTPServiceInfo::CMTPServiceInfo()
    {
    }
void CMTPServiceInfo::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPSERVICEINFO_CONSTRUCTL_EXIT );
    }


TInt CMTPServiceInfo::ServicePropertyOrderFromAscending( const CServiceProperty& aL, const CServiceProperty& aR)
    {
    TInt ret = aL.Compare( aR );
    if( 0 == ret )
        {
        return aL.PKeyID() - aR.PKeyID();
        }
    else
        {
        return ret;
        }
    }

TInt CMTPServiceInfo::ServicePropertyOrderFromKeyAscending( const TPKeyPair* aL, const CServiceProperty& aR)
    {
    TInt ret = aL->iNamespace.Compare( aR.Namespace() );
        if( 0 == ret )
            {
            return aL->iPKeyId - aR.PKeyID();
            }
        else
            {
            return ret;
            }
    }

TInt CMTPServiceInfo::ServiceElementBaseOrderFromAscending( const CServiceElementBase& aL, const CServiceElementBase& aR)
    {
    return aL.Compare( aR );
    }

TInt CMTPServiceInfo::ServiceElementBaseOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceElementBase& aR )
    {
    return aL->Compare( aR.GUID() );
    }

TInt CMTPServiceInfo::ServiceFormatOrderFromAscending( const CServiceFormat& aL, const CServiceFormat& aR)
    {
    return ServiceElementBaseOrderFromAscending(aL, aR);
    }

TInt CMTPServiceInfo::ServiceFormatOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceFormat& aR )
    {
    return ServiceElementBaseOrderFromKeyAscending(aL, aR);
    }

TInt CMTPServiceInfo::ServiceMethodOrderFromAscending( const CServiceMethod& aL, const CServiceMethod& aR )
    {
    return ServiceElementBaseOrderFromAscending(aL, aR);
    }

TInt CMTPServiceInfo::ServiceMethodOrderFromKeyAscending( const TMTPTypeGuid* aL, const CServiceMethod& aR )
    {
    return ServiceElementBaseOrderFromKeyAscending(aL, aR);
    }

TInt CMTPServiceInfo::ServiceEventOrderFromAscending( const CServiceEvent& aL, const CServiceEvent& aR )
    {
    return ServiceElementBaseOrderFromAscending(aL, aR);
    }


void CMTPServiceInfo::SetServiceID( TUint aServiceID)
    {
    iServiceId = aServiceID;
    }

void CMTPServiceInfo::SetServiceStorageID( TUint aStorageID )
    {
    iServiceStorageId = aStorageID;
    }

void CMTPServiceInfo::SetPersistentGUIDL( const TMTPTypeGuid& aGUID )
    {
    MMTPType::CopyL( aGUID, iPersistentGUID);
    }

void CMTPServiceInfo::SetServiceVersion( TUint aVersion )
    {
    iServiceVersion = aVersion;
    }

void CMTPServiceInfo::SetServiceGUIDL( const TMTPTypeGuid& aGUID )
    {
    MMTPType::CopyL( aGUID, iServiceGUID);
    }

void CMTPServiceInfo::SetServiceName( HBufC* aName )
    {
    delete iServiceName;
    iServiceName = aName;
    }

void CMTPServiceInfo::SetServiceType( TUint aServiceType )
    {
    iServiceType = aServiceType;
    }

void CMTPServiceInfo::SetBaseServiceID( TUint aID )
    {
    iBaseServiceId = aID;
    }

void CMTPServiceInfo::AppendUserServiceL( const TMTPTypeGuid& aServiceGUID )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_APPENDUSERSERVICEL_ENTRY );
    
    iUsedServiceGUIDs.AppendL( aServiceGUID );
    
    OstTraceFunctionExit0( CMTPSERVICEINFO_APPENDUSERSERVICEL_EXIT );
    }

void CMTPServiceInfo::InsertPropertyL( const CServiceProperty* aProperty )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_INSERTPROPERTYL_ENTRY );
    
    iServiceProperties.InsertInOrderL( aProperty, TLinearOrder<CServiceProperty>(ServicePropertyOrderFromAscending) );

    OstTraceFunctionExit0( CMTPSERVICEINFO_INSERTPROPERTYL_EXIT );
    }

void CMTPServiceInfo::InsertFormatL( const CServiceFormat* aFormat )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_INSERTFORMATL_ENTRY );
    
    iServiceFormats.InsertInOrderL( aFormat, TLinearOrder<CServiceFormat>(ServiceFormatOrderFromAscending) );

    OstTraceFunctionExit0( CMTPSERVICEINFO_INSERTFORMATL_EXIT );
    }

void CMTPServiceInfo::InsertMethodL( const CServiceMethod* aMethod )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_INSERTMETHODL_ENTRY );
    
    iServiceMethods.InsertInOrderL( aMethod , TLinearOrder<CServiceMethod>(ServiceMethodOrderFromAscending) );

    OstTraceFunctionExit0( CMTPSERVICEINFO_INSERTMETHODL_EXIT );
    }

void CMTPServiceInfo::InsertEventL( const CServiceEvent* aEvent )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_INSERTEVENTL_ENTRY );
    
    iServiceEvents.InsertInOrderL( aEvent , TLinearOrder<CServiceEvent>(ServiceEventOrderFromAscending) );

    OstTraceFunctionExit0( CMTPSERVICEINFO_INSERTEVENTL_EXIT );
    }
  
void CMTPServiceInfo::AppendDataGUIDL( const TMTPTypeGuid& aGUID )
    {
    OstTraceFunctionEntry0( CMTPSERVICEINFO_APPENDDATAGUIDL_ENTRY );

    iDataBlock.AppendL( aGUID );

    OstTraceFunctionExit0( CMTPSERVICEINFO_APPENDDATAGUIDL_EXIT );
    }
    
EXPORT_C TUint CMTPServiceInfo::ServiceID() const
    {
    return iServiceId;
    }

EXPORT_C TUint CMTPServiceInfo::ServiceStorageID() const
    {
    return iServiceStorageId;
    }

EXPORT_C const TMTPTypeGuid& CMTPServiceInfo::ServicePersistentGUID() const
    {
    return iPersistentGUID;
    }
EXPORT_C TUint CMTPServiceInfo::ServiceVersion() const
    {
    return iServiceVersion;
    }

EXPORT_C const TMTPTypeGuid& CMTPServiceInfo::ServiceGUID() const
    {
    return iServiceGUID;
    }

EXPORT_C const TDesC& CMTPServiceInfo::ServiceName() const
    {
    return *iServiceName;
    }

EXPORT_C TUint CMTPServiceInfo::ServiceType() const
    {
    return iServiceType;
    }

EXPORT_C TUint CMTPServiceInfo::BaseServiceID() const
    {
    return iBaseServiceId;
    }

CServiceProperty* CMTPServiceInfo::ServiceProperty( const TMTPTypeGuid& aPKNamespace, const TUint aPKID ) const
    {
    TInt index = iServiceProperties.FindInOrder( TPKeyPair(aPKNamespace, aPKID), ServicePropertyOrderFromKeyAscending );
    if( index != KErrNotFound )
        return iServiceProperties[index];
    
    return NULL;
    }

CServiceFormat* CMTPServiceInfo::ServiceFormat( const TMTPTypeGuid& aGUID ) const
    {
    TInt index = iServiceFormats.FindInOrder( aGUID, ServiceFormatOrderFromKeyAscending );
    if( index != KErrNotFound )
        return iServiceFormats[index];
    
    return NULL;
    }

CServiceMethod* CMTPServiceInfo::ServiceMethod( const TMTPTypeGuid& aGUID ) const
    {
    TInt index = iServiceMethods.FindInOrder( aGUID , ServiceMethodOrderFromKeyAscending );
    if( index != KErrNotFound )
        return iServiceMethods[index];
    
    return NULL;
    }

EXPORT_C const RArray<TMTPTypeGuid>& CMTPServiceInfo::UsedServiceGUIDs() const
	{
	return iUsedServiceGUIDs;
	}

EXPORT_C const RPointerArray<CServiceProperty>& CMTPServiceInfo::ServiceProperties() const
	{
	return iServiceProperties;
	}

EXPORT_C const RPointerArray<CServiceFormat>& CMTPServiceInfo::ServiceFormats() const
	{
	return iServiceFormats;
	}

EXPORT_C const RPointerArray<CServiceMethod>& CMTPServiceInfo::ServiceMethods() const
	{
	return iServiceMethods;
	}

EXPORT_C const RPointerArray<CServiceEvent>& CMTPServiceInfo::ServiceEvents() const
	{
	return iServiceEvents;
	}

EXPORT_C const RArray<TMTPTypeGuid>& CMTPServiceInfo::DataBlockGUIDs() const
    {
    return iDataBlock;
    }

