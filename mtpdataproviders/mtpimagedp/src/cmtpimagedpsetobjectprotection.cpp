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
#include <mtp/mmtpobjectmgr.h>

#include "cmtpimagedpsetobjectreferences.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
#include "cmtpimagedpsetobjectprotection.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpsetobjectprotectionTraces.h"
#endif


/**
Two-phase construction method
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPImageDpSetObjectProtection::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpSetObjectProtection* self = new (ELeave) CMTPImageDpSetObjectProtection(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPImageDpSetObjectProtection::~CMTPImageDpSetObjectProtection()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROTECTION_CMTPIMAGEDPSETOBJECTPROTECTION_DES_ENTRY );
    delete iObjMeta;
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROTECTION_CMTPIMAGEDPSETOBJECTPROTECTION_DES_EXIT );
    }

/**
Standard c++ constructor
*/    
CMTPImageDpSetObjectProtection::CMTPImageDpSetObjectProtection(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL), 
    iRfs(aFramework.Fs())
    {
    }

/**
Second phase constructor
*/
void CMTPImageDpSetObjectProtection::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROTECTION_CONSTRUCTL_ENTRY );
    iObjMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROTECTION_CONSTRUCTL_EXIT );
    }

TMTPResponseCode CMTPImageDpSetObjectProtection::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROTECTION_CHECKREQUESTL_ENTRY );
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    TUint32 statusValue = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
   
    //Check ObjectHanlde
    TMTPResponseCode responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *iObjMeta);
    
    if(EMTPRespCodeOK == responseCode)
        {    
        //Check parameter value
        switch(statusValue)
            {
            case EMTPProtectionNoProtection:
            case EMTPProtectionReadOnly:
                {
                responseCode = EMTPRespCodeOK;
                }
                break;
            default:
                responseCode = EMTPRespCodeInvalidParameter;
                break;     
            }
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSETOBJECTPROTECTION_CHECKREQUESTL, 
            "Exit with responseCode = 0x%04X", responseCode );

    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROTECTION_CHECKREQUESTL_EXIT );
    return responseCode;
    }


/**
Apply the references to the specified object
@return EFalse
*/    
TBool CMTPImageDpSetObjectProtection::DoHandleResponsePhaseL()
    {
    return EFalse; 
    }

/**
GetReferences request handler
*/    
void CMTPImageDpSetObjectProtection::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSETOBJECTPROTECTION_SERVICEL_ENTRY );
    TUint32 statusValue = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    TMTPResponseCode responseCode = EMTPRespCodeOK;
    TInt ret = KErrNone;
    
    switch(statusValue)
        {
        case EMTPProtectionNoProtection:
            {
            ret = iRfs.SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid),KEntryAttNormal,KEntryAttReadOnly);
            }
            break;
        case EMTPProtectionReadOnly:
            {
            ret = iRfs.SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid),KEntryAttReadOnly,KEntryAttNormal);
            }
            break;
        default:
            responseCode = EMTPRespCodeInvalidParameter;
            break;
        }
    
    if (ret != KErrNone)
        {
        responseCode = EMTPRespCodeAccessDenied;
        }
    
    SendResponseL(responseCode);    
    OstTraceFunctionExit0( CMTPIMAGEDPSETOBJECTPROTECTION_SERVICEL_EXIT );
    }

