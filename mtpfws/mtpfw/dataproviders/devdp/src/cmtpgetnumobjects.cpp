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
#include <mtp/mmtpdataprovider.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "cmtpdevicedatastore.h"

#include "cmtpgetnumobjects.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetNumObjects");)

/**
Verification data for GetNumObjects request
*/
const TMTPRequestElementInfo KMTPGetNumObjectsPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrNone, 1, KMTPStorageAll, 0},
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeFormatCode, EMTPElementAttrNone, 1, 0, 0},
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 2, KMTPHandleAll, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetNumObjects::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetNumObjects* self = new (ELeave) CMTPGetNumObjects(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

/**
Destructor
*/	
CMTPGetNumObjects::~CMTPGetNumObjects()
	{	
	iDevDpSingletons.Close();
    iSingletons.Close();
    __FLOG_CLOSE;
	}
/**
Standard c++ constructor
*/	
CMTPGetNumObjects::CMTPGetNumObjects(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetNumObjectsPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetNumObjectsPolicy)
	{

	}
    
/**
Second phase constructor.
*/
void CMTPGetNumObjects::ConstructL()
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    iSingletons.OpenL();
    iDevDpSingletons.OpenL(iFramework);
    }

TMTPResponseCode CMTPGetNumObjects::CheckRequestL()
	{
	TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
	if(responseCode != EMTPRespCodeOK)
		{
		return responseCode;	
		}
	
	TUint32 formatCode = Request().Uint32(TMTPTypeRequest::ERequestParameter2); 
	if(formatCode != 0 && !IsSupportedFormatL(formatCode))
		{
		return EMTPRespCodeInvalidObjectFormatCode;
		}
	
	if(iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted)
		{
		TUint storageID = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
		TUint handle = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
		if(iDevDpSingletons.PendingStorages().FindInOrder(storageID) != KErrNotFound)
			{
			responseCode = EMTPRespCodeDeviceBusy;
			}
		else if( (handle != KMTPHandleNone) && (handle != KMTPHandleAll)  )
			{
			CMTPObjectMetaData* meta = iRequestChecker->GetObjectInfo(handle);
			__ASSERT_DEBUG(meta, Panic(EMTPDevDpObjectNull));
			
			if( meta->Uint(CMTPObjectMetaData::EFormatCode) == EMTPFormatCodeAssociation )
				{
				responseCode = EMTPRespCodeDeviceBusy;
				}
			}
		else if(EMTPFormatCodeUndefined == formatCode)
			{
			responseCode = EMTPRespCodeDeviceBusy;
			}
		}
	else if(iDevDpSingletons.PendingStorages().Count() > 0)
		{
		iDevDpSingletons.PendingStorages().Close();
		}
	
	return responseCode;	
	}
	
	
/**
GetNumObjects request handler
*/	
void CMTPGetNumObjects::ServiceL()
	{
    __FLOG(_L8("ServiceL - Entry"));
    __FLOG_VA((_L8("IsConnectMac = %d; ERequestParameter2 = %d" ), iDevDpSingletons.DeviceDataStore().IsConnectMac(), Request().Uint32(TMTPTypeRequest::ERequestParameter2)));
	if(iDevDpSingletons.DeviceDataStore().IsConnectMac()
        &&(KMTPFormatsAll == Request().Uint32(TMTPTypeRequest::ERequestParameter2)))
        {
        //get folder count
    	TMTPObjectMgrQueryParams paramsFolder(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeAssociation, Request().Uint32(TMTPTypeRequest::ERequestParameter3));        
    	TUint32 count = iFramework.ObjectMgr().CountL(paramsFolder);
        __FLOG_VA((_L8("ConnectMac and Fetch all, folder count = %d"), count));

        //get script count
        TMTPObjectMgrQueryParams paramsScript(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeScript, Request().Uint32(TMTPTypeRequest::ERequestParameter3));
        count += iFramework.ObjectMgr().CountL(paramsScript);
        
        //get Image file count
        TMTPObjectMgrQueryParams paramsImage(Request().Uint32(TMTPTypeRequest::ERequestParameter1), EMTPFormatCodeEXIFJPEG, Request().Uint32(TMTPTypeRequest::ERequestParameter3));
        count += iFramework.ObjectMgr().CountL(paramsImage);
        __FLOG_VA((_L8("ConnectMac and Fetch all, total count = %d"), count));        
    	SendResponseL(EMTPRespCodeOK, 1, &count); 
        }
    else
        {       
    	TMTPObjectMgrQueryParams params(Request().Uint32(TMTPTypeRequest::ERequestParameter1), Request().Uint32(TMTPTypeRequest::ERequestParameter2), Request().Uint32(TMTPTypeRequest::ERequestParameter3));
    	TUint32 count = iFramework.ObjectMgr().CountL(params);	
        __FLOG_VA((_L8("NOT ConnectMac or NOT Fetch all, total count = %d"), count));         
    	SendResponseL(EMTPRespCodeOK, 1, &count);
        }
    __FLOG(_L8("ServiceL - Exit"));	    
	}

/**
Check if the format code is supported by the current installed data providers
*/	
TBool CMTPGetNumObjects::IsSupportedFormatL(TUint32 aFormatCode)
	{
	TBool supported(EFalse);
		
	CMTPDataProviderController& dps(iSingletons.DpController());
	const TInt count(dps.Count());
	for (TInt i(0); ((i < count) && (!supported)); i++)
		{						
		CMTPDataProvider& dp = dps.DataProviderByIndexL(i);
		if (dp.DataProviderId() != dps.ProxyDpId())
			{
			supported = (
			    dp.Supported(EObjectCaptureFormats, aFormatCode) || 
			    dp.Supported(EObjectPlaybackFormats, aFormatCode));
			}			
		}
	return supported;	
	}











	

	


   	

	






