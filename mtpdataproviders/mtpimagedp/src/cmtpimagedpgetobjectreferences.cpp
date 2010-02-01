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

__FLOG_STMT(_LIT8(KComponent,"GetObjectReferences");)

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
    __FLOG(_L8(">> CMTPImageDpGetObjectReferences::~CMTPImageDpGetObjectReferences"));
    delete iReferences;
    __FLOG(_L8("<< CMTPImageDpGetObjectReferences::~CMTPImageDpGetObjectReferences"));
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpGetObjectReferences::ConstructL"));
    __FLOG(_L8("<< CMTPImageDpGetObjectReferences::ConstructL"));
    }

TMTPResponseCode CMTPImageDpGetObjectReferences::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObjectReferences::CheckRequestL"));
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPObjectMetaData* objectInfo = CMTPObjectMetaData::NewLC();    
    TMTPResponseCode responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *objectInfo);      
    CleanupStack::PopAndDestroy(objectInfo);
    
    __FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
    __FLOG(_L8("<< CMTPImageDpGetObjectReferences::CheckRequestL"));
    return responseCode;
    }

/**
GetReferences request handler
*/    
void CMTPImageDpGetObjectReferences::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::ServiceL"));
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    MMTPReferenceMgr& referenceMgr = iFramework.ReferenceMgr();
    delete iReferences;
    iReferences = NULL;
    iReferences = referenceMgr.ReferencesLC(TMTPTypeUint32(objectHandle));
    CleanupStack::Pop(iReferences);
    SendDataL(*iReferences);
    __FLOG(_L8("<< CMTPImageDpCopyObject::ServiceL"));
    }


