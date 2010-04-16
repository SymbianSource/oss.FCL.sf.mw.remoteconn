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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetObject");)

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
    __FLOG(_L8("~CMTPGetObject - Entry"));
	delete iFileObject;
    __FLOG(_L8("~CMTPGetObject - Exit"));
    __FLOG_CLOSE;
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
GetObject request handler
*/		
void CMTPGetObject::ServiceL()
	{
    __FLOG(_L8("ServiceL - Entry"));
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
    __FLOG(_L8("ServiceL - Exit"));
	}
		

/**
Build the file object data set for the file requested
@param aFileName	The file name of the requested object
*/
void CMTPGetObject::BuildFileObjectL(const TDesC& aFileName)
	{
    __FLOG(_L8("BuildFileObjectL - Entry"));
	delete iFileObject;
	iFileObject = NULL;
	iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), aFileName, EFileRead);
    __FLOG(_L8("BuildFileObjectL - Exit"));
	}
	

/**
Handle the response phase of the current request
@return EFalse
*/		
TBool CMTPGetObject::DoHandleResponsePhaseL()
	{
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : iError);
	SendResponseL(responseCode);
    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));
	return EFalse;
	}
