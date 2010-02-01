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
// mtpdevicedpprocessor.h
// 
//

#include <mtp/tmtptyperequest.h>

#include "mtpfiledpprocessor.h"
#include "cmtprequestprocessor.h"
#include "cmtpgetobjectinfo.h"
#include "cmtpgetobject.h"
#include "cmtpdeleteobject.h"
#include "cmtpsendobjectinfo.h"
#include "cmtpcopyobject.h"
#include "cmtpmoveobject.h"
#include "cmtpgetobjectpropssupported.h"
#include "cmtpgetobjectpropdesc.h"
#include "cmtpgetobjectpropvalue.h"
#include "cmtpsetobjectpropvalue.h"
#include "cmtprequestunknown.h"
#include "cmtpgetobjectproplist.h"
#include "cmtpsetobjectproplist.h"
#include "cmtpgetreferences.h"
#include "cmtpsetreferences.h"
#include "cmtpgetformatcapabilities.h"
#include "cmtpgetpartialobject.h"
#include "cmtpsetobjectprotection.h"
/**
A mapping table from the operation code to the request processor factory method
*/
static const TMTPRequestProcessorEntry KMTPFileDpRequestProcessorTable[] = 
	{
		{EMTPOpCodeGetObjectInfo, CMTPGetObjectInfo::NewL},
		{EMTPOpCodeGetObject, CMTPGetObject::NewL},
		{EMTPOpCodeDeleteObject, CMTPDeleteObject::NewL},		
		{EMTPOpCodeSendObjectInfo, CMTPSendObjectInfo::NewL},
		{EMTPOpCodeSendObject, CMTPSendObjectInfo::NewL},	//force the SendObject request to be processed by the SendObjectInfo processor 
		{EMTPOpCodeSendObjectPropList, CMTPSendObjectInfo::NewL},	//force the SendObjectPropList request to be processed by the SendObjectInfo processor 
		{EMTPOpCodeMoveObject, CMTPMoveObject::NewL},	
		{EMTPOpCodeCopyObject, CMTPCopyObject::NewL},
		{EMTPOpCodeGetObjectPropsSupported, CMTPGetObjectPropsSupported::NewL},
		{EMTPOpCodeGetObjectPropDesc, CMTPGetObjectPropDesc::NewL},
		{EMTPOpCodeGetObjectPropValue, CMTPGetObjectPropValue::NewL},
		{EMTPOpCodeSetObjectPropValue, CMTPSetObjectPropValue::NewL},
		{EMTPOpCodeSetObjectPropList, CMTPSetObjectPropList::NewL},
		{EMTPOpCodeGetObjectPropList, CMTPGetObjectPropList::NewL},
		{EMTPOpCodeGetObjectReferences, CMTPGetReferences::NewL},
		{EMTPOpCodeSetObjectReferences, CMTPSetReferences::NewL},
        {EMTPOpCodeGetFormatCapabilities,CMTPGetFormatCapabilities::NewL},
        {EMTPOpCodeGetPartialObject, CMTPGetPartialObject::NewL},
	 {EMTPOpCodeSetObjectProtection, CMTPSetObjectProtection::NewL}				
	};

/**
Create a request processor that matches the request
@param aPlugin	The reference to the data provider plugin 
@param aFramework The reference to the data provider framework
@param aRequest	The request to be processed
@param aConnection The connection from which the request comes from
@return a pointer to the request processor
*/	
MMTPRequestProcessor* MTPFileDpProcessor::CreateL(
													MMTPDataProviderFramework& aFramework,
													const TMTPTypeRequest& aRequest, 
													MMTPConnection& aConnection)
	{
	TMTPRequestProcessorCreateFunc createFunc = NULL; 
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	TInt count = sizeof(KMTPFileDpRequestProcessorTable) / sizeof(TMTPRequestProcessorEntry);
	for(TInt i = 0; i < count; i++)
		{
		if(KMTPFileDpRequestProcessorTable[i].iOperationCode == operationCode)
			{
			createFunc = KMTPFileDpRequestProcessorTable[i].iCreateFunc;
			break;
			}
		}
		
	if(!createFunc)	
		{
		createFunc = CMTPRequestUnknown::NewL;
		}
		
	return (*createFunc)(aFramework, aConnection);
	}
	


