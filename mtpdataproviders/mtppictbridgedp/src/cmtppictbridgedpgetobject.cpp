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


#include <f32file.h>

#include <mtp/cmtptypefile.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include "cmtppictbridgedpgetobject.h"
#include "cmtppictbridgeprinter.h"
#include "mtppictbridgedppanic.h"
#include "cmtprequestchecker.h"
#include "cptpserver.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtppictbridgedpgetobjectTraces.h"
#endif


/**
Verification data for the GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetObjectPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFile, 0, 0, 0}
    };

//=============================================================================
//
//=============================================================================
MMTPRequestProcessor* CMTPPictBridgeDpGetObject::NewL(
	MMTPDataProviderFramework& aFramework,
	MMTPConnection& aConnection,
	CMTPPictBridgeDataProvider& aDataProvider)
    {       
    CMTPPictBridgeDpGetObject* self = new (ELeave) CMTPPictBridgeDpGetObject(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);         
    return self;
    }

/**
Destructor
*/  
CMTPPictBridgeDpGetObject::~CMTPPictBridgeDpGetObject()
    {   
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECT_CMTPPICTBRIDGEDPGETOBJECT_DES_ENTRY );    
    delete iFileObject;
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECT_CMTPPICTBRIDGEDPGETOBJECT_DES_EXIT );
    }
    
/**
Standard c++ constructor
*/  
CMTPPictBridgeDpGetObject::CMTPPictBridgeDpGetObject(
	MMTPDataProviderFramework& aFramework,
	MMTPConnection& aConnection,
	CMTPPictBridgeDataProvider& aDataProvider): 
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPolicy),
    iPictBridgeDP(aDataProvider),
    iError(EMTPRespCodeOK)
    {    
    }

/**
Second-phase constructor.
*/        
void CMTPPictBridgeDpGetObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECT_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECT_CONSTRUCTL_EXIT );
    }

/**
GetObject request handler
*/      
void CMTPPictBridgeDpGetObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECT_SERVICEL_ENTRY );     
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPPictBridgeDpRequestCheckNull));
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    //does not take ownership
    CMTPObjectMetaData* objectInfo = iRequestChecker->GetObjectInfo(objectHandle);
    if (!objectInfo)
        {
        OstTrace1( TRACE_WARNING, CMTPPICTBRIDGEDPGETOBJECT_SERVICEL, "no object info for objectHandle %d", objectHandle );
        // The object handle has already been checked, so an invalid handle can
        // only occur if it was invalidated during a context switch between
        // the validation time and now.
        iError = EMTPRespCodeInvalidObjectHandle;
        iPictBridgeDP.PtpServer()->Printer()->DpsFileSent(KErrBadHandle);
        }
    else
        {
        BuildFileObjectL(objectInfo->DesC(CMTPObjectMetaData::ESuid));
        SendDataL(*iFileObject);
        iPictBridgeDP.PtpServer()->Printer()->DpsFileSent(KErrNone);
        }
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECT_SERVICEL_EXIT );
    }

//=============================================================================
//
//=============================================================================
void CMTPPictBridgeDpGetObject::BuildFileObjectL(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECT_BUILDFILEOBJECTL_ENTRY );        
    delete iFileObject;
    iFileObject = NULL;
    iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), aFileName, EFileRead);           
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECT_BUILDFILEOBJECTL_EXIT );
    }
    

//=============================================================================
//
//=============================================================================
TBool CMTPPictBridgeDpGetObject::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPPICTBRIDGEDPGETOBJECT_DOHANDLERESPONSEPHASEL_ENTRY );          
    TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iError);
    SendResponseL(responseCode);
    OstTraceFunctionExit0( CMTPPICTBRIDGEDPGETOBJECT_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;
    }

