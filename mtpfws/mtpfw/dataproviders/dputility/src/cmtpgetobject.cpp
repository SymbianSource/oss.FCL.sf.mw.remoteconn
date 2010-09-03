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

#include "cmtpgetobject.h"
#include "mtpdppanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetobjectTraces.h"
#endif


/**
Verification data for the GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetObjectPolicy[] = 
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
EXPORT_C MMTPRequestProcessor* CMTPGetObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetObject* self = new (ELeave) CMTPGetObject(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destructor
*/	
EXPORT_C CMTPGetObject::~CMTPGetObject()
	{	
    OstTraceFunctionEntry0( CMTPGETOBJECT_CMTPGETOBJECT_DES_ENTRY );
	delete iFileObject;
	OstTraceFunctionExit0( CMTPGETOBJECT_CMTPGETOBJECT_DES_EXIT );
	}
	
/**
Standard c++ constructor
*/	
CMTPGetObject::CMTPGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) : 
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPolicy),
	iError(EMTPRespCodeOK)
	{
	
	}

/**
Second-phase constructor.
*/        
void CMTPGetObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECT_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPGETOBJECT_CONSTRUCTL_EXIT );
    }

/**
GetObject request handler
*/		
void CMTPGetObject::ServiceL()
	{
    OstTraceFunctionEntry0( CMTPGETOBJECT_SERVICEL_ENTRY );
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	//does not take ownership
	CMTPObjectMetaData* objectInfo = iRequestChecker->GetObjectInfo(objectHandle);
	if (!objectInfo)
	    {
	    // The object handle has already been checked, so an invalid handle can
	    // only occur if it was invalidated during a context switch between
	    // the validation time and now.
	    SendResponseL(EMTPRespCodeInvalidObjectHandle);
	    }
	else if ( objectInfo->Uint(CMTPObjectMetaData::EFormatCode)==EMTPFormatCodeAssociation 
	        && objectInfo->Uint(CMTPObjectMetaData::EFormatSubCode)==EMTPAssociationTypeGenericFolder)
	    {
	    SendResponseL(EMTPRespCodeInvalidObjectHandle);
	    }
    else
        {
    		TRAPD(err, BuildFileObjectL(objectInfo->DesC(CMTPObjectMetaData::ESuid)));
    		if (err == KErrNone)
    			{
    			SendDataL(*iFileObject);	
    			}
    		else
    			{
    			SendResponseL(EMTPRespCodeAccessDenied);
    			}
        }
	OstTraceFunctionExit0( CMTPGETOBJECT_SERVICEL_EXIT );
	}
		

/**
Build the file object data set for the file requested
@param aFileName	The file name of the requested object
*/
void CMTPGetObject::BuildFileObjectL(const TDesC& aFileName)
	{
    OstTraceFunctionEntry0( CMTPGETOBJECT_BUILDFILEOBJECTL_ENTRY );
	delete iFileObject;
	iFileObject = NULL;
	iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), aFileName, EFileRead);
	OstTraceFunctionExit0( CMTPGETOBJECT_BUILDFILEOBJECTL_EXIT );
	}
	

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPGetObject::DoHandleResponsePhaseL()
	{
    OstTraceFunctionEntry0( CMTPGETOBJECT_DOHANDLERESPONSEPHASEL_ENTRY );
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iError);
	SendResponseL(responseCode);
	OstTraceFunctionExit0( CMTPGETOBJECT_DOHANDLERESPONSEPHASEL_EXIT );
	return EFalse;
	}
