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
#include <centralrepository.h>

#include <mtp/cmtptypefile.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpimagedpgetobject.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedp.h"
#include "mtpimagedpconst.h"
#include "mtpimagedputilits.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"ImageDpGetObject");)

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
MMTPRequestProcessor* CMTPImageDpGetObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpGetObject* self = new (ELeave) CMTPImageDpGetObject(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/	
CMTPImageDpGetObject::~CMTPImageDpGetObject()
    {
    __FLOG(_L8(">> ~CMTPImageDpGetObject"));
    delete iFileObject;
    __FLOG(_L8("<< ~CMTPImageDpGetObject"));
    __FLOG_CLOSE;
    }
	
/**
Standard c++ constructor
*/	
CMTPImageDpGetObject::CMTPImageDpGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider) : 
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPolicy), 
    iDataProvider(aDataProvider)
    {
    
    }

/**
Second-phase constructor.
*/        
void CMTPImageDpGetObject::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPImageDpGetObject::ConstructL"));   
    __FLOG(_L8("<< CMTPImageDpGetObject::ConstructL"));
    }

/**
GetObject request handler
*/		
void CMTPImageDpGetObject::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpGetObject::ServiceL"));
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    //does not take ownership    
    CMTPObjectMetaData* objectInfo = iRequestChecker->GetObjectInfo(objectHandle);
    __ASSERT_DEBUG(objectInfo, Panic(EMTPImageDpObjectNull));
    
    BuildFileObjectL(objectInfo->DesC(CMTPObjectMetaData::ESuid));
    SendDataL(*iFileObject);
    __FLOG(_L8("<< CMTPImageDpGetObject::ServiceL"));
    }
		

/**
Build the file object data set for the file requested
@param aFileName	The file name of the requested object
*/
void CMTPImageDpGetObject::BuildFileObjectL(const TDesC& aFileName)
    {
    __FLOG(_L8(">> CMTPImageDpGetObject::BuildFileObjectL"));
    delete iFileObject;
    iFileObject = NULL;
    iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), aFileName, EFileShareReadersOnly);
    __FLOG(_L8("<< CMTPImageDpGetObject::BuildFileObjectL"));
    }

TBool CMTPImageDpGetObject::DoHandleCompletingPhaseL()
    {
    __FLOG(_L8(" CMTPImageDpGetObject::DoHandleResponsePhaseL - Entry"));        
    TInt currentNewPics = 0;
    iDataProvider.Repository().Get(ENewImagesCount, currentNewPics);
    if (currentNewPics != 0)
        {
        /**
		Zero the new pictures of RProperty.
		Because we think the end-use has import all pictures as long as MTP receive one getobject operation

        There are two different phases to collect new pictures:
		1. In enumeration phase, calculate new pictures value from MSS in one go.
		2. After enumeration phase, dynamically calculate new pictures value from MdS by Notifications
		*/
        iDataProvider.Repository().Set(ENewImagesCount, 0);
        RProperty::Set(TUid::Uid(KMTPServerUID), KMTPNewPicKey, 0);        
        }    
    
    __FLOG(_L8("CMTPImageDpGetObject::DoHandleResponsePhaseL - Exit"));
    return CMTPRequestProcessor::DoHandleCompletingPhaseL();
    }
