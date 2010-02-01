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

#include <f32file.h>

#include <mtp/cmtptypefile.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpgetpartialobject.h"
#include "mtpdppanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetObject");)

/**
Verification data for the GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetPartialObjectPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFile, 0, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
EXPORT_C MMTPRequestProcessor* CMTPGetPartialObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetPartialObject* self = new (ELeave) CMTPGetPartialObject(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPGetPartialObject::~CMTPGetPartialObject()
	{	
    __FLOG(_L8("~CMTPGetPartialObject - Entry"));
	delete iFileObject;
    __FLOG(_L8("~CMTPGetPartialObject - Exit"));
    __FLOG_CLOSE;
	}
	
/**
Standard c++ constructor
*/	
CMTPGetPartialObject::CMTPGetPartialObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) : 
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetPartialObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetPartialObjectPolicy)
	{
	
	}

/**
Second-phase constructor.
*/        
void CMTPGetPartialObject::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
Check the GetPartialObject reqeust
@return EMTPRespCodeOK if the request is good, otherwise, one of the error response codes
*/  
TMTPResponseCode CMTPGetPartialObject::CheckRequestL()
    {
    __FLOG(_L8("CheckRequestL - Entry"));
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
    if(result == EMTPRespCodeOK)
        {
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        iOffset = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
        iLength = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
        
        //does not take ownership
        iObjectInfo = iRequestChecker->GetObjectInfo(objectHandle);
        if (!iObjectInfo)
            {
            // The object handle has already been checked, so an invalid handle can
            // only occur if it was invalidated during a context switch between
            // the validation time and now.
            result = EMTPRespCodeInvalidObjectHandle;
            }
        else
            {
            TEntry fileEntry;
            User::LeaveIfError(iFramework.Fs().Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), fileEntry));

            if((iOffset >= fileEntry.iSize)) 
                {
                result = EMTPRespCodeInvalidParameter;
                }
            }
        }

    __FLOG(_L8("CheckRequestL - Exit"));
    return result;  
    }

/**
GetObject request handler
*/		
void CMTPGetPartialObject::ServiceL()
	{
    __FLOG(_L8("ServiceL - Entry"));
    
	if (!iObjectInfo)
	    {
	    SendResponseL(EMTPRespCodeInvalidObjectHandle);
	    }
    else
        {
        delete iFileObject;
        iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), iObjectInfo->DesC(CMTPObjectMetaData::ESuid), EFileRead, iLength, iOffset);
    	SendDataL(*iFileObject);
        }
	
    __FLOG(_L8("ServiceL - Exit"));
	}

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPGetPartialObject::DoHandleResponsePhaseL()
	{
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));
    __ASSERT_DEBUG(iFileObject, Panic(EMTPDpObjectNull));

    TUint32 dataLength = iFileObject->GetByteSent();
	SendResponseL(EMTPRespCodeOK, 1, &dataLength);
	
    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));
	return EFalse;
	}


