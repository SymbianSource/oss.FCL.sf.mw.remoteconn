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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetobjecthandlesTraces.h"
#endif


// Class constants.

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
    CMTPGetNumObjects::ConstructL();
    }

/**
GetObjectHandles request handler
*/	
void CMTPGetObjectHandles::ServiceL()
	{
    OstTraceFunctionEntry0( CMTPGETOBJECTHANDLES_SERVICEL_ENTRY );
    
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
                OstTrace0( TRACE_NORMAL, CMTPGETOBJECTHANDLES_SERVICEL, 
                        "Wait for enumeration time out, return busy." );
                SendResponseL(EMTPRespCodeDeviceBusy);
                iTimeoutCount = 0;
                OstTraceFunctionExit0( CMTPGETOBJECTHANDLES_SERVICEL_EXIT );
                return;
                }
            else
                {
                OstTrace0( TRACE_NORMAL, DUP1_CMTPGETOBJECTHANDLES_SERVICEL, 
                        "Enumeration not completed, suspend request." );
                RegisterPendingRequest(20);
                OstTraceFunctionExit0( DUP1_CMTPGETOBJECTHANDLES_SERVICEL_EXIT );
                return; 
                }
            }
        }
    
    iTimeoutCount = 0;
    
	delete iHandles;
	iHandles = NULL;
	iHandles = CMTPTypeArray::NewL(EMTPTypeAUINT32);

	OstTraceExt2( TRACE_NORMAL, DUP2_CMTPGETOBJECTHANDLES_SERVICEL, 
	        "IsConnectMac = %d; ERequestParameter2 = %d", iDevDpSingletons.DeviceDataStore().IsConnectMac(), Request().Uint32(TMTPTypeRequest::ERequestParameter2));	
    if(iDevDpSingletons.DeviceDataStore().IsConnectMac()
       &&(KMTPFormatsAll == Request().Uint32(TMTPTypeRequest::ERequestParameter2)))
        {
        OstTrace0( TRACE_NORMAL, DUP3_CMTPGETOBJECTHANDLES_SERVICEL, "ConnectMac and Fetch all." );
        HandleObjectHandlesUnderMacL(*iHandles);
        }
    else
        {
    	RMTPObjectMgrQueryContext   context;
    	RArray<TUint>               handles;
    	CleanupClosePushL(context);
    	CleanupClosePushL(handles);        
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
	                    handles.InsertL(handle,0);
	                    }
	                }   
	            delete StorageSuid;
	            StorageSuid = NULL;
	            CleanupStack::PopAndDestroy();
            	}
        	iHandles->AppendL(handles);
    	    }
    	while (!context.QueryComplete()); 
    	CleanupStack::PopAndDestroy(&handles);
    	CleanupStack::PopAndDestroy(&context);        
        }        
    						
	SendDataL(*iHandles);
	OstTraceFunctionExit0( DUP2_CMTPGETOBJECTHANDLES_SERVICEL_EXIT );
	}
	
