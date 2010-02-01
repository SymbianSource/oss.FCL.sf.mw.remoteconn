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

__FLOG_STMT(_LIT8(KComponent,"SetObjectReferences");)

/**
Two-phase construction method
@param aPlugin The data provider plugin
@param aFramework The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPImageDpSetObjectReferences::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& /*aDataProvider*/)
    {
    CMTPImageDpSetObjectReferences* self = new (ELeave) CMTPImageDpSetObjectReferences(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);    
    return self;
    }

/**
Destructor
*/    
CMTPImageDpSetObjectReferences::~CMTPImageDpSetObjectReferences()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectReferences::~CMTPImageDpSetObjectReferences"));
    delete iReferences;
    __FLOG(_L8("<< CMTPImageDpSetObjectReferences::~CMTPImageDpSetObjectReferences"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/    
CMTPImageDpSetObjectReferences::CMTPImageDpSetObjectReferences(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    }

/**
Second phase constructor
*/
void CMTPImageDpSetObjectReferences::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpSetObjectReferences::ConstructL"));
    __FLOG(_L8("<< CMTPImageDpSetObjectReferences::ConstructL"));
    }

TMTPResponseCode CMTPImageDpSetObjectReferences::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpSetObjectReferences::CheckRequestL"));
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    CMTPObjectMetaData* objectInfo = CMTPObjectMetaData::NewLC();    
    TMTPResponseCode responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *objectInfo);      
    CleanupStack::PopAndDestroy(objectInfo);
    __FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
    __FLOG(_L8("<< CMTPImageDpSetObjectReferences::CheckRequestL"));
    return responseCode;
    }


/**
Apply the references to the specified object
@return EFalse
*/    
TBool CMTPImageDpSetObjectReferences::DoHandleResponsePhaseL()
    {
    if(!VerifyReferenceHandlesL())
        {
        SendResponseL(EMTPRespCodeInvalidObjectReference);
        }
    else
        {
        MMTPReferenceMgr& referenceMgr = iFramework.ReferenceMgr();
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        referenceMgr.SetReferencesL(TMTPTypeUint32(objectHandle), *iReferences);
        SendResponseL(EMTPRespCodeOK);
        }
    return EFalse;    
    }

/**
GetReferences request handler
*/    
void CMTPImageDpSetObjectReferences::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::ServiceL"));
    delete iReferences;
    iReferences = NULL;
    iReferences = CMTPTypeArray::NewL(EMTPTypeAUINT32);
    ReceiveDataL(*iReferences);
    __FLOG(_L8("<< CMTPImageDpCopyObject::ServiceL"));
    }

TBool CMTPImageDpSetObjectReferences::HasDataphase() const
    {
    return ETrue;
    }

/**
Verify if the references are valid handles to objects
@return ETrue if all the references are good, otherwise, EFalse
*/    
TBool CMTPImageDpSetObjectReferences::VerifyReferenceHandlesL() const
    {
    __ASSERT_DEBUG(iReferences, User::Invariant());
    TBool result = ETrue;
    TInt count = iReferences->NumElements();
    CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
    MMTPObjectMgr& objectMgr = iFramework.ObjectMgr();
    TMTPTypeUint32 handle;
    for(TInt i = 0; i < count; i++)
        {
        iReferences->ElementL(i, handle);
        if(!objectMgr.ObjectL(handle, *object))
            {
            result = EFalse;
            break;
            }
        }
    CleanupStack::PopAndDestroy(object);
    return result; 
    }


