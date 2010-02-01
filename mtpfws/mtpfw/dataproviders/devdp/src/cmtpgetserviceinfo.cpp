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
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtptypeserviceinfo.h>
#include <mtp/cmtptypeserviceprop.h>
#include <mtp/cmtptypeserviceformat.h>
#include <mtp/cmtptypeservicemethod.h>
#include <mtp/cmtptypeserviceevent.h>
#include <mtp/tmtptypeguid.h>

#include "cmtpgetserviceinfo.h"
#include "rmtpframework.h"
#include "mtpdevdppanic.h"
#include "cmtpservicemgr.h"



// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetServiceInfo");)

/**
Two-phase construction method
@param aPlugin    The data provider plugin
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetServiceInfo::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPGetServiceInfo* self = new (ELeave) CMTPGetServiceInfo(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPGetServiceInfo::~CMTPGetServiceInfo()
    {    
    __FLOG(_L8("~CMTPGetServiceInfo - Entry"));
    delete iServiceInfo;

    iSingletons.Close();
    __FLOG(_L8("~CMTPGetServiceInfo - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPGetServiceInfo::CMTPGetServiceInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
    
/**
GetServiceInfo request handler. Build and send device info data set.
*/    
void CMTPGetServiceInfo::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
       
    TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    if ( KErrNotFound == iSingletons.ServiceMgr().GetServiceIDs().Find(serviceId) )
    	{
    	SendResponseL(EMTPRespCodeInvalidServiceID);
    	}
    else if(iSingletons.ServiceMgr().IsSupportedService(serviceId))
    	{
    	BuildServiceInfoL();
    	SendDataL(*iServiceInfo);
    	}
    else
        {
        //The ServiceID has been allocated by MTP Datacode Generator
        //but Parser&Router fail to get the target DP.
        //it may be caused by: 
        //    1. DP plugin does not register the ServiceID by the Supported() function. Mostly.
        //    2. Framework have some errors while setup the router mapping table.
        Panic(EMTPDevDpUnknownServiceID);
        }
    
    __FLOG(_L8("ServiceL - Exit"));
    }

/**
Second-phase constructor.
*/        
void CMTPGetServiceInfo::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
    iSingletons.OpenL();
    __FLOG(_L8("ConstructL - Exit")); 
    }

/**
Populates service info data set
*/
void CMTPGetServiceInfo::BuildServiceInfoL()
    {
    __FLOG(_L8("BuildServiceInfoL - Entry")); 
    
    delete iServiceInfo;
    iServiceInfo = CMTPTypeServiceInfo::NewL();
    
    TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPServiceInfo* svcinfo = iSingletons.ServiceMgr().ServiceInfo( serviceId );
    if( NULL == svcinfo )
        {
        __FLOG_1(_L8("BuildServiceInfoL - CMTPServiceInfo is NULL!!! ServiceID is %d."),serviceId ); 
        __FLOG(_L8("BuildServiceInfoL - Exit")); 
        return;
        }
    BuildServiceInfoHeadL(*svcinfo);
    BuildUsedServiceGUIDL(*svcinfo);
	BuildServicePropertyL(*svcinfo);
	BuildServiceFormatL(*svcinfo);
	BuildServiceMethodL(*svcinfo);
	BuildDataBlockL(*svcinfo);    
    
    __FLOG(_L8("BuildServiceInfoL - Exit")); 
    }


void CMTPGetServiceInfo::BuildServiceInfoHeadL(CMTPServiceInfo& aServiceInfo)
	{
	__FLOG(_L8("BuildServiceInfoHeadL - Entry"));
	
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceID,aServiceInfo.ServiceID());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceStorageID,aServiceInfo.ServiceStorageID());
	iServiceInfo->SetL(CMTPTypeServiceInfo::EServicePGUID,aServiceInfo.ServicePersistentGUID());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceVersion,aServiceInfo.ServiceVersion());
	iServiceInfo->SetL(CMTPTypeServiceInfo::EServiceGUID,aServiceInfo.ServiceGUID());
	iServiceInfo->SetStringL(CMTPTypeServiceInfo::EServiceName,aServiceInfo.ServiceName());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceType,aServiceInfo.ServiceType());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EBaseServiceID,aServiceInfo.BaseServiceID());
	
	__FLOG(_L8("BuildServiceInfoHeadL - Exit")); 
	}

void CMTPGetServiceInfo::BuildUsedServiceGUIDL(CMTPServiceInfo& aServiceInfo)
	{
	__FLOG(_L8("BuildUsedServiceGUIDL - Entry"));
	TInt count = aServiceInfo.UsedServiceGUIDs().Count();
	const RArray<TMTPTypeGuid> UsedServiceGUIDs = aServiceInfo.UsedServiceGUIDs();
	for (TInt i=0;i<count;i++)
		{
          iServiceInfo->AppendUsedServiceL( UsedServiceGUIDs[i] );
		}
	
	__FLOG(_L8("BuildUsedServiceGUIDL - Exit"));
	}

void CMTPGetServiceInfo::BuildServicePropertyL(CMTPServiceInfo& aServiceInfo)
	{
	__FLOG(_L8("BuildServicePropertyL - Entry"));

	TInt count = aServiceInfo.ServiceProperties().Count();
	CMTPTypeServicePropertyElement* PropElement = NULL;
	CServiceProperty* prop = NULL;
	const RPointerArray<CServiceProperty> ServiceProperties = aServiceInfo.ServiceProperties();
	for (TInt i=0;i<count;i++)
		{
		prop = ServiceProperties[i];
		if(!prop->IsUsed())
		    continue;
		
		PropElement = CMTPTypeServicePropertyElement::NewLC(prop->Code(),prop->Namespace(),prop->PKeyID(), prop->Name());
		iServiceInfo->ServicePropList().AppendL(PropElement);
		CleanupStack::Pop(PropElement);
		}
	
	__FLOG(_L8("BuildServicePropertyL - Exit"));
	}

void CMTPGetServiceInfo::BuildServiceFormatL(CMTPServiceInfo& aServiceInfo)
	{
	__FLOG(_L8("BuildServiceFormatL - Entry"));

	CMTPTypeServiceFormatElement* FormatElement = NULL; 
	CServiceFormat* format = NULL;  
	TInt count = aServiceInfo.ServiceFormats().Count();
	const RPointerArray<CServiceFormat> ServiceFormats = aServiceInfo.ServiceFormats();
	for (TInt i=0;i<count;i++)
		{
		format = ServiceFormats[i];
		if(!format->IsUsed())
		    continue;
		
		FormatElement = CMTPTypeServiceFormatElement::NewLC( format->Code(), format->GUID(), format->Name(), format->FormatBase(), format->MIMEType1() );
		iServiceInfo->ServiceFormatList().AppendL(FormatElement);
		CleanupStack::Pop(FormatElement);
		}
	
	__FLOG(_L8("BuildServiceFormatL - Exit"));
	}

void CMTPGetServiceInfo::BuildServiceMethodL(CMTPServiceInfo& aServiceInfo)
	{
	__FLOG(_L8("BuildServiceMethodL - Entry"));

	CMTPTypeServiceMethodElement* methodElement = NULL;
	CServiceMethod* method = NULL;
	TInt count = aServiceInfo.ServiceMethods().Count();
	const RPointerArray<CServiceMethod> ServiceMethods = aServiceInfo.ServiceMethods();
	for (TInt i=0;i<count;i++)
		{
		method = ServiceMethods[i];
		if(!method->IsUsed())
		    continue;
		methodElement = CMTPTypeServiceMethodElement::NewLC( method->Code(), method->GUID(), method->Name(), method->ObjAssociateFormatCode() );
		iServiceInfo->ServiceMethodList().AppendL(methodElement);
		CleanupStack::Pop(methodElement);		
		}
	
	__FLOG(_L8("BuildServiceMethodL - Exit"));
	}


void CMTPGetServiceInfo::BuildDataBlockL(CMTPServiceInfo& aServiceInfo)
    {
    __FLOG(_L8("BuildDataBlockL - Entry"));
    TInt count = aServiceInfo.DataBlockGUIDs().Count();
    const RArray<TMTPTypeGuid> DataBlockGUIDs = aServiceInfo.DataBlockGUIDs();
    for (TInt i=0;i<count;i++)
        {
        iServiceInfo->AppendServiceDataBlockL( DataBlockGUIDs[i] );
        }

    __FLOG(_L8("BuildDataBlockL - Exit"));
    }


