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

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mmtpstoragemgr.h>  

#include "cmtpdataprovidercontroller.h"
#include "cmtpdevicedatastore.h"
#include "cmtpgetobjecthandles.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetObjectHandles");)

static const TInt KMTPGetObjectHandlesTimeOut(1);

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetObjectHandles::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetObjectHandles* self = new (ELeave) CMTPGetObjectHandles(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
GetObjectHandles request handler
*/	
CMTPGetObjectHandles::~CMTPGetObjectHandles()
	{
	delete iHandles;
    __FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPGetObjectHandles::CMTPGetObjectHandles(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPGetNumObjects(aFramework, aConnection)
	{
	
	}
    
/**
Second phase constructor.
*/
void CMTPGetObjectHandles::ConstructL()
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);    
    CMTPGetNumObjects::ConstructL();
    }

/**
GetObjectHandles request handler
*/	
void CMTPGetObjectHandles::ServiceL()
	{
    __FLOG(_L8("ServiceL - Entry"));
    
    if(iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted)
        {
        TUint storageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        TUint handle = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
        TUint enumerateState = iSingletons.DpController().StorageEnumerateState(storageId);
        if ( (enumerateState < CMTPDataProviderController::EEnumeratingPhaseOneDone)
            || (enumerateState != CMTPDataProviderController::EEnumeratedFulllyCompleted && handle != KMTPHandleAll))
            {
            if (iTimeoutCount++ >= KMTPGetObjectHandlesTimeOut)
                {
                __FLOG(_L8("Wait for enumeration time out, return busy."));
                SendResponseL(EMTPRespCodeDeviceBusy);
                iTimeoutCount = 0;
                return;
                }
            else
                {
                __FLOG(_L8("Enumeration not completed, suspend request."));
                RegisterPendingRequest(20);
                return; 
                }
            }
        }
    
    iTimeoutCount = 0;
    
	RMTPObjectMgrQueryContext   context;
	RArray<TUint>               handles;
	CleanupClosePushL(context);
	CleanupClosePushL(handles);
	delete iHandles;
	iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);

    __FLOG_VA((_L8("IsConnectMac = %d; ERequestParameter2 = %d" ), iDevDpSingletons.DeviceDataStore().IsConnectMac(), Request().Uint32(TMTPTypeRequest::ERequestParameter2)));
	if(iDevDpSingletons.DeviceDataStore().IsConnectMac()
       &&(KMTPFormatsAll == Request().Uint32(TMTPTypeRequest::ERequestParameter2)))
        {
        __FLOG(_L8("ConnectMac and Fetch all."));
        //get folder object handles
    	TMTPObjectMgrQueryParams    paramsFolder(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeAssociation, Request().Uint32(TMTPTypeRequest::ERequestParameter3));	
    	do
    	    {
        	iFramework.ObjectMgr().GetObjectHandlesL(paramsFolder, context, handles);
        	iHandles->AppendL(handles);
    	    }
    	while (!context.QueryComplete());

        //get script object handles
    	RMTPObjectMgrQueryContext   contextScript;
    	RArray<TUint>               handlesScript;
    	CleanupClosePushL(contextScript);
    	CleanupClosePushL(handlesScript);            
    	TMTPObjectMgrQueryParams    paramsScript(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeScript, Request().Uint32(TMTPTypeRequest::ERequestParameter3));	
    	do
    	    {
        	iFramework.ObjectMgr().GetObjectHandlesL(paramsScript, contextScript, handlesScript);
        	iHandles->AppendL(handlesScript);
    	    }
    	while (!contextScript.QueryComplete());
    	CleanupStack::PopAndDestroy(&contextScript);
    	CleanupStack::PopAndDestroy(&handlesScript);        

        //get image object handles
    	RMTPObjectMgrQueryContext   contextImage;
    	RArray<TUint>               handlesImage;
    	CleanupClosePushL(contextImage);
    	CleanupClosePushL(handlesImage);            
    	TMTPObjectMgrQueryParams    paramsImage(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeEXIFJPEG, Request().Uint32(TMTPTypeRequest::ERequestParameter3));	
    	do
    	    {
        	iFramework.ObjectMgr().GetObjectHandlesL(paramsImage, contextImage, handlesImage);
        	iHandles->AppendL(handlesImage);
    	    }
    	while (!contextImage.QueryComplete());
    	CleanupStack::PopAndDestroy(&contextImage);
    	CleanupStack::PopAndDestroy(&handlesImage);                            
        }
    else
        {
    	TMTPObjectMgrQueryParams    params(Request().Uint32(TMTPTypeRequest::ERequestParameter1), Request().Uint32(TMTPTypeRequest::ERequestParameter2), Request().Uint32(TMTPTypeRequest::ERequestParameter3));	
    	do
    	    {
        	iFramework.ObjectMgr().GetObjectHandlesL(params, context, handles);
			
        	TUint32 storageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        	TUint32 parentHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
        	if ( storageId != KMTPStorageAll && parentHandle == KMTPHandleNoParent )
	            {
	            const CMTPStorageMetaData& storage(iFramework.StorageMgr().StorageL(storageId));
	            HBufC* StorageSuid = storage.DesC(CMTPStorageMetaData::EStorageSuid).AllocL();
            
	            RBuf suid;
	            suid.CleanupClosePushL();
	            suid.CreateL(KMaxFileName);
	            suid = *StorageSuid;
	            _LIT(WMPInfoXml,"WMPInfo.xml");
	            suid.Append(WMPInfoXml); 
	            TUint32 handle = iFramework.ObjectMgr().HandleL(suid);
	            if ( handle != KMTPHandleNone )
	                {
	                TInt index = handles.Find(handle);
	                if ( index != KErrNotFound )
	                    {
	                    handles.Remove(index);
	                    handles.Insert(handle,0);
	                    }
	                }   
	            delete StorageSuid;
	            StorageSuid = NULL;
	            CleanupStack::PopAndDestroy();
            	}
        	iHandles->AppendL(handles);
    	    }
    	while (!context.QueryComplete());        
        }        
    	
        

	CleanupStack::PopAndDestroy(&handles);
	CleanupStack::PopAndDestroy(&context);					
	SendDataL(*iHandles);
    __FLOG(_L8("ServiceL - Exit"));	    
	}
	
