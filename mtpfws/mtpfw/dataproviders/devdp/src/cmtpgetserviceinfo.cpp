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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetserviceinfoTraces.h"
#endif




// Class constants.

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
    OstTraceFunctionEntry0( CMTPGETSERVICEINFO_CMTPGETSERVICEINFO_ENTRY );
    delete iServiceInfo;

    iSingletons.Close();
    OstTraceFunctionExit0( CMTPGETSERVICEINFO_CMTPGETSERVICEINFO_EXIT );
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
    OstTraceFunctionEntry0( CMTPGETSERVICEINFO_SERVICEL_ENTRY );
       
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
        __DEBUG_ONLY(Panic(EMTPDevDpUnknownServiceID));
        }
    
    OstTraceFunctionExit0( CMTPGETSERVICEINFO_SERVICEL_EXIT );
    }

/**
Second-phase constructor.
*/        
void CMTPGetServiceInfo::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETSERVICEINFO_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
    OstTraceFunctionExit0( CMTPGETSERVICEINFO_CONSTRUCTL_EXIT );
    }

/**
Populates service info data set
*/
void CMTPGetServiceInfo::BuildServiceInfoL()
    {
    OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDSERVICEINFOL_ENTRY );
    
    delete iServiceInfo;
    iServiceInfo = CMTPTypeServiceInfo::NewL();
    
    TUint32 serviceId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPServiceInfo* svcinfo = iSingletons.ServiceMgr().ServiceInfo( serviceId );
    if( NULL == svcinfo )
        {
        OstTrace1( TRACE_WARNING, CMTPGETSERVICEINFO_BUILDSERVICEINFOL, 
                "BuildServiceInfoL - CMTPServiceInfo is NULL!!! ServiceID is %d.", serviceId );        
        OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDSERVICEINFOL_EXIT );
        return;
        }
    BuildServiceInfoHeadL(*svcinfo);
    BuildUsedServiceGUIDL(*svcinfo);
	BuildServicePropertyL(*svcinfo);
	BuildServiceFormatL(*svcinfo);
	BuildServiceMethodL(*svcinfo);
	BuildDataBlockL(*svcinfo);    

    OstTraceFunctionExit0( DUP1_CMTPGETSERVICEINFO_BUILDSERVICEINFOL_EXIT );
    }


void CMTPGetServiceInfo::BuildServiceInfoHeadL(CMTPServiceInfo& aServiceInfo)
	{
	OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDSERVICEINFOHEADL_ENTRY );
	
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceID,aServiceInfo.ServiceID());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceStorageID,aServiceInfo.ServiceStorageID());
	iServiceInfo->SetL(CMTPTypeServiceInfo::EServicePGUID,aServiceInfo.ServicePersistentGUID());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceVersion,aServiceInfo.ServiceVersion());
	iServiceInfo->SetL(CMTPTypeServiceInfo::EServiceGUID,aServiceInfo.ServiceGUID());
	iServiceInfo->SetStringL(CMTPTypeServiceInfo::EServiceName,aServiceInfo.ServiceName());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EServiceType,aServiceInfo.ServiceType());
	iServiceInfo->SetUint32L(CMTPTypeServiceInfo::EBaseServiceID,aServiceInfo.BaseServiceID());

	OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDSERVICEINFOHEADL_EXIT );
	}

void CMTPGetServiceInfo::BuildUsedServiceGUIDL(CMTPServiceInfo& aServiceInfo)
	{
	OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDUSEDSERVICEGUIDL_ENTRY );
	TInt count = aServiceInfo.UsedServiceGUIDs().Count();
	const RArray<TMTPTypeGuid> UsedServiceGUIDs = aServiceInfo.UsedServiceGUIDs();
	for (TInt i=0;i<count;i++)
		{
          iServiceInfo->AppendUsedServiceL( UsedServiceGUIDs[i] );
		}
	
	OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDUSEDSERVICEGUIDL_EXIT );
	}

void CMTPGetServiceInfo::BuildServicePropertyL(CMTPServiceInfo& aServiceInfo)
	{
	OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDSERVICEPROPERTYL_ENTRY );

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
	
	OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDSERVICEPROPERTYL_EXIT );
	}

void CMTPGetServiceInfo::BuildServiceFormatL(CMTPServiceInfo& aServiceInfo)
	{
	OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDSERVICEFORMATL_ENTRY );

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
	
	OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDSERVICEFORMATL_EXIT );
	}

void CMTPGetServiceInfo::BuildServiceMethodL(CMTPServiceInfo& aServiceInfo)
	{
	OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDSERVICEMETHODL_ENTRY );

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
	
	OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDSERVICEMETHODL_EXIT );
	}


void CMTPGetServiceInfo::BuildDataBlockL(CMTPServiceInfo& aServiceInfo)
    {
    OstTraceFunctionEntry0( CMTPGETSERVICEINFO_BUILDDATABLOCKL_ENTRY );
    TInt count = aServiceInfo.DataBlockGUIDs().Count();
    const RArray<TMTPTypeGuid> DataBlockGUIDs = aServiceInfo.DataBlockGUIDs();
    for (TInt i=0;i<count;i++)
        {
        iServiceInfo->AppendServiceDataBlockL( DataBlockGUIDs[i] );
        }
    
    OstTraceFunctionExit0( CMTPGETSERVICEINFO_BUILDDATABLOCKL_EXIT );
    }


