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

/**
 @file
 @internalTechnology
*/

#include <f32file.h>

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypefile.h>

#include "cmtpimagedpgetpartialobject.h"
#include "mtpimagedppanic.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedp.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedpobjectpropertymgr.h"

__FLOG_STMT(_LIT8(KComponent,"ImageDpGetPartialObject");)
/**
Verification data for the GetPartialObject request
*/

/**
Two-phase construction method
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPImageDpGetPartialObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetPartialObject* self = new (ELeave) CMTPImageDpGetPartialObject(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor
*/	
CMTPImageDpGetPartialObject::~CMTPImageDpGetPartialObject()
    {
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::~CMTPImageDpGetPartialObject"));
    delete iFileObject;
    delete iObjectMeta;
    __FLOG(_L8("<< CMTPImageDpGetPartialObject::~CMTPImageDpGetPartialObject"));
    __FLOG_CLOSE;
    }
    
/**
Standard c++ constructor
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
*/	
CMTPImageDpGetPartialObject::CMTPImageDpGetPartialObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& /*aDataProvider*/)
    :CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iFs(iFramework.Fs())
    {
    }

/**
Second-phase construction
*/  
void CMTPImageDpGetPartialObject::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::ConstructL"));
    iObjectMeta = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpGetPartialObject::ConstructL"));
    }
    
/**
Check the GetPartialObject reqeust
@return EMTPRespCodeOK if the request is good, otherwise, one of the error response codes
*/	
TMTPResponseCode CMTPImageDpGetPartialObject::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::CheckRequestL"));
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
    if(result == EMTPRespCodeOK)
        {
        result = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
        }
    if(result == EMTPRespCodeOK && !VerifyParametersL())
        {
        result = EMTPRespCodeInvalidParameter;
        }
    __FLOG_VA((_L8("<< CMTPImageDpGetPartialObject::CheckRequestL 0x%x"), result));
    __FLOG(_L8("<< CMTPImageDpGetPartialObject::CheckRequestL"));
    return result;	
    }
    
/**
Verify if the parameter of the request (i.e. offset) is good.
@return ETrue if the parameter is good, otherwise, EFalse
*/		
TBool CMTPImageDpGetPartialObject::VerifyParametersL()
    {
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::VerifyParametersL"));
    TBool result = EFalse;
    iOffset = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    iMaxLength = Request().Uint32(TMTPTypeRequest::ERequestParameter3);

    TEntry fileEntry;
    User::LeaveIfError(iFs.Entry(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), fileEntry));
    if((iOffset < fileEntry.FileSize())) 
        {
        result = ETrue;
        }
    __FLOG_VA((_L8("<< CMTPImageDpGetPartialObject::VerifyParametersL %d"), result));
    return result;	
    }
/**
GetPartialObject request handler
Send the partial object data to the initiator
*/	
void CMTPImageDpGetPartialObject::ServiceL()
    {
    // Get file information
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::ServiceL"));
        // Pass the complete file back to the host
    iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), iObjectMeta->DesC(CMTPObjectMetaData::ESuid), (TFileMode)(EFileRead | EFileShareReadersOnly), iMaxLength, iOffset);
    SendDataL(*iFileObject);	
    __FLOG(_L8("<< CMTPImageDpGetPartialObject::ServiceL"));
    }
    
    
/**
Signal to the initiator how much data has been sent
@return EFalse
*/
TBool CMTPImageDpGetPartialObject::DoHandleResponsePhaseL()
    {
    __FLOG(_L8(">> CMTPImageDpGetPartialObject::DoHandleResponsePhaseL"));
    TUint32 dataLength = iFileObject->GetByteSent();
    SendResponseL(EMTPRespCodeOK, 1, &dataLength);
    __FLOG(_L8("<< CMTPImageDpGetPartialObject::DoHandleResponsePhaseL"));
    return EFalse;
    }
