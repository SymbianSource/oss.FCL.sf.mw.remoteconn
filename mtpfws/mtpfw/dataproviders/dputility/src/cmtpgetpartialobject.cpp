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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetpartialobjectTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPGETPARTIALOBJECT_CMTPGETPARTIALOBJECT_DES_ENTRY );
	delete iFileObject;
	OstTraceFunctionExit0( CMTPGETPARTIALOBJECT_CMTPGETPARTIALOBJECT_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPGETPARTIALOBJECT_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPGETPARTIALOBJECT_CONSTRUCTL_EXIT );
    }

/**
Check the GetPartialObject reqeust
@return EMTPRespCodeOK if the request is good, otherwise, one of the error response codes
*/  
TMTPResponseCode CMTPGetPartialObject::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPGETPARTIALOBJECT_CHECKREQUESTL_ENTRY );
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
            LEAVEIFERROR(iFramework.Fs().Entry(iObjectInfo->DesC(CMTPObjectMetaData::ESuid), fileEntry),
                    OstTraceExt1(TRACE_ERROR, CMTPGETPARTIALOBJECT_CHECKREQUESTL, 
                            "can't get entry details for %S!", iObjectInfo->DesC(CMTPObjectMetaData::ESuid)));

            if((iOffset >= fileEntry.FileSize())) 
                {
                result = EMTPRespCodeInvalidParameter;
                }
            }
        }

    OstTraceFunctionExit0( CMTPGETPARTIALOBJECT_CHECKREQUESTL_EXIT );
    return result;  
    }

/**
GetObject request handler
*/		
void CMTPGetPartialObject::ServiceL()
	{
    OstTraceFunctionEntry0( CMTPGETPARTIALOBJECT_SERVICEL_ENTRY );
    
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

	OstTraceFunctionExit0( CMTPGETPARTIALOBJECT_SERVICEL_EXIT );
	}

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPGetPartialObject::DoHandleResponsePhaseL()
	{
    OstTraceFunctionEntry0( CMTPGETPARTIALOBJECT_DOHANDLERESPONSEPHASEL_ENTRY );
    __ASSERT_DEBUG(iFileObject, Panic(EMTPDpObjectNull));

    TUint32 dataLength = iFileObject->GetByteSent();
	SendResponseL(EMTPRespCodeOK, 1, &dataLength);

	OstTraceFunctionExit0( CMTPGETPARTIALOBJECT_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}


