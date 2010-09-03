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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpgetobjectTraces.h"
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
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_CMTPIMAGEDPGETOBJECT_ENTRY );
    delete iFileObject;
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECT_CMTPIMAGEDPGETOBJECT_EXIT );
    }
	
/**
Standard c++ constructor
*/	
CMTPImageDpGetObject::CMTPImageDpGetObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider) : 
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPolicy),
    iFramework(aFramework),
    iDataProvider(aDataProvider)
    {
    
    }

/**
Second-phase constructor.
*/        
void CMTPImageDpGetObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECT_CONSTRUCTL_EXIT );
    }
TMTPResponseCode CMTPImageDpGetObject::CheckRequestL()
    {    
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_CHECKREQUESTL_ENTRY );
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    
    TUint32 handle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    if ( handle != KMTPHandleAll )
        {
        CMTPObjectMetaData* ObjectMeta = CMTPObjectMetaData::NewL();
        responseCode = MTPImageDpUtilits::VerifyObjectHandleL(
                iFramework, handle, *ObjectMeta);
        delete ObjectMeta;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECT_CHECKREQUESTL_EXIT );
    return responseCode;
    }
/**
GetObject request handler
*/		
void CMTPImageDpGetObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_SERVICEL_ENTRY );
    
    TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    //does not take ownership    
    CMTPObjectMetaData* objectInfo = iRequestChecker->GetObjectInfo(objectHandle);
    __ASSERT_DEBUG(objectInfo, Panic(EMTPImageDpObjectNull));
    
    BuildFileObjectL(objectInfo->DesC(CMTPObjectMetaData::ESuid));
    SendDataL(*iFileObject);    
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECT_SERVICEL_EXIT );
    }
		

/**
Build the file object data set for the file requested
@param aFileName	The file name of the requested object
*/
void CMTPImageDpGetObject::BuildFileObjectL(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_BUILDFILEOBJECTL_ENTRY );
    delete iFileObject;
    iFileObject = NULL;
    iFileObject = CMTPTypeFile::NewL(iFramework.Fs(), aFileName, EFileShareReadersOnly);
    OstTraceFunctionExit0( CMTPIMAGEDPGETOBJECT_BUILDFILEOBJECTL_EXIT );
    }

TBool CMTPImageDpGetObject::DoHandleCompletingPhaseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPGETOBJECT_DOHANDLECOMPLETINGPHASEL_ENTRY );
    
    /**
     * end-user does not cancel the operation, we think the getobject operation is successful.
     */
    if (!iCancelled)
        {
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        CMTPObjectMetaData* objectInfo = iRequestChecker->GetObjectInfo(objectHandle);    
        if (MTPImageDpUtilits::IsNewPicture(*objectInfo))
            {       
            //update new picture status            
            MTPImageDpUtilits::UpdateObjectStatusToOldL(iFramework, *objectInfo);
            iDataProvider.ResetNewPictures();
            }
        }

    return CMTPRequestProcessor::DoHandleCompletingPhaseL();
    }
