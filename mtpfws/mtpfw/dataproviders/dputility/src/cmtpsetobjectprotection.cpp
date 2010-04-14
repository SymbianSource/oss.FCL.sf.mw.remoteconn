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

#include <bautils.h>
#include <f32file.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpsetobjectprotection.h"
#include "mtpdpconst.h"
#include "mtpdppanic.h"
#include "cmtpdataprovidercontroller.h"
#include "mtpframeworkconst.h"
#include "rmtpdpsingletons.h"
#include "rmtputility.h"

/**
Verification data for the SetObjectPropValue request
*/
const TMTPRequestElementInfo KMTPSetObjectProtectionPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrWrite, 0, 0, 0},      
     };

const TInt KRetryTimes = 10;
const TInt KRetryInterval = 150 * 1000; //150ms
/**
Two-phase construction method
@param aPlugin  The data provider plugin
@param aFramework   The data provider framework
@param aConnection  The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPSetObjectProtection::NewL(
                                            MMTPDataProviderFramework& aFramework,
                                            MMTPConnection& aConnection)
    {
    CMTPSetObjectProtection* self = new (ELeave) CMTPSetObjectProtection(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/  
EXPORT_C CMTPSetObjectProtection::~CMTPSetObjectProtection()
    {   
    delete iObjMeta;
    }

/**
Standard c++ constructor
*/  
CMTPSetObjectProtection::CMTPSetObjectProtection(
                                    MMTPDataProviderFramework& aFramework,
                                    MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPSetObjectProtectionPolicy)/sizeof(TMTPRequestElementInfo), KMTPSetObjectProtectionPolicy),
    iRfs(aFramework.Fs())
    {
    }
                                    
TMTPResponseCode CMTPSetObjectProtection::CheckRequestL()
    {
    //TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    iFramework.ObjectMgr().ObjectL(TMTPTypeUint32(handle), *iObjMeta);
    if (!iObjMeta )
        {
        return EMTPRespCodeInvalidObjectHandle;
        }
    TUint32 statusValue = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    //Currently we only support EMTPProtectionNoProtection and EMTPProtectionReadOnly
    if ( statusValue!=EMTPProtectionNoProtection && statusValue!=EMTPProtectionReadOnly )
        {
        return EMTPRespCodeInvalidParameter;
        }
    
    return EMTPRespCodeOK;
    }

void CMTPSetObjectProtection::ServiceL()
    {
    TUint32 handle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    TUint32 statusValue = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    TMTPResponseCode rsp = EMTPRespCodeAccessDenied;
    //iFramework.ObjectMgr().ObjectL(TMTPTypeUint32(handle), *iObjMeta);
    
    switch(statusValue)
        {
        case EMTPProtectionNoProtection:
            {
            for(TInt i = 0; i < KRetryTimes; ++ i)
                {
                if(KErrNone == iRfs.SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid),KEntryAttNormal,KEntryAttReadOnly))
                    {
                    rsp = EMTPRespCodeOK;
                    break;
                    }
                else
                    {
                    User::After(KRetryInterval);	
                    }
                }
            }
            break;
        case EMTPProtectionReadOnly:
        case EMTPProtectionReadOnlyData:
            {
            for(TInt i = 0; i < KRetryTimes; ++ i)
                {
                if(KErrNone == iRfs.SetAtt(iObjMeta->DesC(CMTPObjectMetaData::ESuid),KEntryAttReadOnly,KEntryAttNormal))
                    {
                        rsp = EMTPRespCodeOK;
                        break;
                    }
                    else
                    {
                        User::After(KRetryInterval);	
                    }
                }
            }
            break;
        default:
            rsp = EMTPRespCodeInvalidParameter;
            break;

        }
    SendResponseL(rsp);
    }

/**
Second-phase construction
*/          
void CMTPSetObjectProtection::ConstructL()
    {   
    iObjMeta = CMTPObjectMetaData::NewL();
    }
