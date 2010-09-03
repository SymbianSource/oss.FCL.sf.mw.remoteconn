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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpgetpartialobjectTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_CMTPIMAGEDPGETPARTIALOBJECT_ENTRY );
    delete iFileObject;
    delete iObjectMeta;
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_CMTPIMAGEDPGETPARTIALOBJECT_EXIT );
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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_CONSTRUCTL_ENTRY );
    iObjectMeta = CMTPObjectMetaData::NewL();
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_CONSTRUCTL_EXIT );
    }
    
/**
Check the GetPartialObject reqeust
@return EMTPRespCodeOK if the request is good, otherwise, one of the error response codes
*/	
TMTPResponseCode CMTPImageDpGetPartialObject::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_CHECKREQUESTL_ENTRY );
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
    if(result == EMTPRespCodeOK)
        {
        result = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, Request().Uint32(TMTPTypeRequest::ERequestParameter1), *iObjectMeta);
        }
    if(result == EMTPRespCodeOK && !VerifyParametersL())
        {
        result = EMTPRespCodeInvalidParameter;
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPGETPARTIALOBJECT_CHECKREQUESTL, "CheckRequestL result 0x%x", result );
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_CHECKREQUESTL_EXIT );
    return result;	
    }
    
/**
Verify if the parameter of the request (i.e. offset) is good.
@return ETrue if the parameter is good, otherwise, EFalse
*/		
TBool CMTPImageDpGetPartialObject::VerifyParametersL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_VERIFYPARAMETERSL_ENTRY );
    TBool result = EFalse;
    iOffset = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    iMaxLength = Request().Uint32(TMTPTypeRequest::ERequestParameter3);

    TEntry fileEntry;
    LEAVEIFERROR(iFs.Entry(iObjectMeta->DesC(CMTPObjectMetaData::ESuid), fileEntry),
            OstTraceExt2( TRACE_ERROR, DUP1_CMTPIMAGEDPGETPARTIALOBJECT_VERIFYPARAMETERSL, 
                    "Gets the entry details for %S failed! error code %d", iObjectMeta->DesC(CMTPObjectMetaData::ESuid), munged_err));
         
    if((iOffset < fileEntry.FileSize())) 
        {
        result = ETrue;
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPGETPARTIALOBJECT_VERIFYPARAMETERSL, 
            "VerifyParametersL result %d", result );    
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_VERIFYPARAMETERSL_EXIT );
    return result;	
    }
/**
GetPartialObject request handler
Send the partial object data to the initiator
*/	
void CMTPImageDpGetPartialObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_SERVICEL_ENTRY );
    // Get file information
        // Pass the complete file back to the host
    iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), iObjectMeta->DesC(CMTPObjectMetaData::ESuid), (TFileMode)(EFileRead | EFileShareReadersOnly), iMaxLength, iOffset);
    SendDataL(*iFileObject);	
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_SERVICEL_EXIT );
    }
    
    
/**
Signal to the initiator how much data has been sent
@return EFalse
*/
TBool CMTPImageDpGetPartialObject::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETPARTIALOBJECT_DOHANDLERESPONSEPHASEL_ENTRY );
    TUint32 dataLength = iFileObject->GetByteSent();
    SendResponseL(EMTPRespCodeOK, 1, &dataLength);
    OstTraceFunctionExit0( CMTPIMAGEDPGETPARTIALOBJECT_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;
    }
