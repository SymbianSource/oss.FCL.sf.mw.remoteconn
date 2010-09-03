// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/cmtptypearray.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpreferencemgr.h>

#include "cmtpimagedpgetobjectreferences.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpgetobjectreferencesTraces.h"
#endif


/**
Two-phase construction method
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPImageDpGetObjectReferences::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpGetObjectReferences* self = new (ELeave) CMTPImageDpGetObjectReferences(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);    
    return self;
    }

/**
Destructor
*/    
CMTPImageDpGetObjectReferences::~CMTPImageDpGetObjectReferences()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTREFERENCES_CMTPIMAGEDPGETOBJECTREFERENCES_ENTRY );
    delete iReferences;
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTREFERENCES_CMTPIMAGEDPGETOBJECTREFERENCES_EXIT );
    }

/**
Standard c++ constructor
*/    
CMTPImageDpGetObjectReferences::CMTPImageDpGetObjectReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    }

/**
Second phase constructor
*/
void CMTPImageDpGetObjectReferences::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTREFERENCES_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTREFERENCES_CONSTRUCTL_EXIT );
    }

TMTPResponseCode CMTPImageDpGetObjectReferences::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTREFERENCES_CHECKREQUESTL_ENTRY );
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPObjectMetaData* objectInfo = CMTPObjectMetaData::NewLC();    
    TMTPResponseCode responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *objectInfo);      
    CleanupStack::PopAndDestroy(objectInfo);

    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPGETOBJECTREFERENCES_CHECKREQUESTL, "CheckRequestL - Exit with responseCode = 0x%04X", responseCode );
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTREFERENCES_CHECKREQUESTL_EXIT );
    return responseCode;
    }

/**
GetReferences request handler
*/    
void CMTPImageDpGetObjectReferences::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECTREFERENCES_SERVICEL_ENTRY );
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    MMTPReferenceMgr& referenceMgr = iFramework.ReferenceMgr();
    delete iReferences;
    iReferences = NULL;
    iReferences = referenceMgr.ReferencesLC(TMTPTypeUint32(objectHandle));
    CleanupStack::Pop(iReferences);
    SendDataL(*iReferences);
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECTREFERENCES_SERVICEL_EXIT );
    }


