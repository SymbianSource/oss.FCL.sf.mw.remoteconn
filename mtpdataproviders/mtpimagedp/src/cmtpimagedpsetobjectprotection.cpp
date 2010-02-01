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

__FLOG_STMT(_LIT8(KComponent,"SetObjectProtection");)

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
    __FLOG(_L8(">> CMTPImageDpSetObjectProtection::~CMTPImageDpSetObjectProtection"));
    delete iObjMeta;
    __FLOG(_L8("<< CMTPImageDpSetObjectProtection::~CMTPImageDpSetObjectProtection"));
    
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    
    __FLOG(_L8(">> CMTPImageDpSetObjectProtection::ConstructL"));
    iObjMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpSetObjectProtection::ConstructL"));
    
    }

TMTPResponseCode CMTPImageDpSetObjectProtection::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectProtection::CheckRequestL"));
    
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
    __FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
    __FLOG(_L8("<< CMTPImageDpSetObjectProtection::CheckRequestL"));
    
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
    __FLOG(_L8(">> CMTPImageDpCopyObject::ServiceL"));
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
    __FLOG(_L8("<< CMTPImageDpCopyObject::ServiceL"));
    }

