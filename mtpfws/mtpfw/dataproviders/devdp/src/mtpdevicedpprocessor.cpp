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

#include "mtpdevicedpprocessor.h"
#include "cmtprequestprocessor.h"
#include "cmtpgetdeviceinfo.h"
#include "cmtpopensession.h"
#include "cmtpclosesession.h"
#include "cmtpgetstorageids.h"
#include "cmtpgetstorageinfo.h"
#include "cmtpgetnumobjects.h"
#include "cmtpgetobjecthandles.h"
#include "cmtpresetdevice.h"
#include "cmtpgetdevicepropdesc.h"
#include "cmtpgetdevicepropvalue.h"
#include "cmtpsetdevicepropvalue.h"
#include "cmtpdevrequestunknown.h"

#include "cmtpcopyobject.h"
#include "cmtpmoveobject.h"
#include "cmtpdeleteobject.h"
#include "cmtpgetobject.h"
#include "cmtpgetobjectinfo.h"
#include "cmtpgetobjectpropdesc.h"
#include "cmtpgetobjectproplist.h"
#include "cmtpgetobjectpropssupported.h"
#include "cmtpgetobjectpropvalue.h"
#include "cmtpgetreferences.h"
#include "cmtpsendobjectinfo.h"
#include "cmtpsetobjectproplist.h"
#include "cmtpsetobjectpropvalue.h"
#include "cmtpresetdevicepropvalue.h"
#include "cmtpsetreferences.h"
#include "cmtpresetdevicepropvalue.h"
#include "cmtpgetinterdependentpropdesc.h"
#include "cmtpgetserviceids.h"
#include "cmtpgetserviceinfo.h"
#include "cmtpgetformatcapabilities.h"
#include "cmtpsetobjectprotection.h"
#include "cmtpcommonrequestprocessor.h"

/**
device data provider mapping table from request ID to factory method of the request processor
*/
static const TMTPRequestProcessorEntry KMTPRequestProcessorTable[] = 
	{
		{EMTPOpCodeGetDeviceInfo, CMTPGetDeviceInfo::NewL},
		{EMTPOpCodeOpenSession, CMTPOpenSession::NewL},
		{EMTPOpCodeCloseSession, CMTPCloseSession::NewL},
		{EMTPOpCodeGetStorageIDs, CMTPGetStorageIds::NewL},
		{EMTPOpCodeGetStorageInfo, CMTPGetStorageInfo::NewL},
		{EMTPOpCodeGetNumObjects, CMTPGetNumObjects::NewL},
		{EMTPOpCodeGetObjectHandles, CMTPGetObjectHandles::NewL},
		{EMTPOpCodeResetDevice, CMTPResetDevice::NewL},
		{EMTPOpCodeGetDevicePropDesc, CMTPGetDevicePropDesc::NewL},
		{EMTPOpCodeGetDevicePropValue, CMTPGetDevicePropValue::NewL},
		{EMTPOpCodeSetDevicePropValue, CMTPSetDevicePropValue::NewL},
		{EMTPOpCodeMoveObject, CMTPMoveObject::NewL},
		{EMTPOpCodeCopyObject, CMTPCopyObject::NewL},
		{EMTPOpCodeDeleteObject, CMTPDeleteObject::NewL},
		{EMTPOpCodeGetObject, CMTPGetObject::NewL},
		{EMTPOpCodeGetObjectInfo, CMTPGetObjectInfo::NewL},
		{EMTPOpCodeGetObjectPropDesc, CMTPGetObjectPropDesc::NewL},
		{EMTPOpCodeGetObjectPropList, CMTPGetObjectPropList::NewL},
		{EMTPOpCodeGetObjectPropsSupported, CMTPGetObjectPropsSupported::NewL},
		{EMTPOpCodeGetObjectPropValue, CMTPGetObjectPropValue::NewL},
		{EMTPOpCodeGetObjectReferences, CMTPGetReferences::NewL},
		{EMTPOpCodeGetInterdependentPropDesc, CMTPGetInterDependentPropDesc::NewL},
		{EMTPOpCodeSendObject, CMTPSendObjectInfo::NewL},
		{EMTPOpCodeSendObjectInfo, CMTPSendObjectInfo::NewL},
		{EMTPOpCodeSendObjectPropList, CMTPSendObjectInfo::NewL},
		{EMTPOpCodeSetObjectPropList, CMTPSetObjectPropList::NewL},
		{EMTPOpCodeSetObjectPropValue, CMTPSetObjectPropValue::NewL},
		{EMTPOpCodeSetObjectReferences, CMTPSetReferences::NewL},
		{EMTPOpCodeResetDevicePropValue, CMTPResetDevicePropValue::NewL},
		{EMTPOpCodeGetServiceIDs, CMTPGetServiceIds::NewL},
		{EMTPOpCodeGetServiceInfo, CMTPGetServiceInfo::NewL},
		{EMTPOpCodeGetFormatCapabilities,CMTPGetFormatCapabilities::NewL},
		{EMTPOpCodeSetObjectProtection, CMTPSetObjectProtection::NewL},
		{EMTPOpCodeSetServicePropList,CMTPCommonRequestProcessor::NewL<EMTPRespCodeInvalidServiceID, ETrue>},		
        {EMTPOpCodeDeleteServicePropList,CMTPCommonRequestProcessor::NewL<EMTPRespCodeInvalidServiceID, ETrue>}		
	};

/**
Create a request processor that matches the request
@param aPlugin	The reference to the data provider plugin 
@param aFramework The reference to the data provider framework
@param aRequest	The request to be processed
@param aConnection The connection from which the request comes from
@return a pointer to the request processor
*/	
MMTPRequestProcessor* MTPDeviceDpProcessor::CreateL(
													MMTPDataProviderFramework& aFramework,
													const TMTPTypeRequest& aRequest, 
													MMTPConnection& aConnection)
	{
	TMTPRequestProcessorCreateFunc createFunc = NULL; 
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	TInt count = sizeof(KMTPRequestProcessorTable) / sizeof(TMTPRequestProcessorEntry);
	for(TInt i = 0; i < count; i++)
		{
		if(KMTPRequestProcessorTable[i].iOperationCode == operationCode)
			{
			createFunc = KMTPRequestProcessorTable[i].iCreateFunc;
			break;
			}
		}

	if(!createFunc)	
		{
		createFunc = CMTPDevRequestUnknown::NewL;
		}
				
	return (*createFunc)(aFramework, aConnection);
	}



