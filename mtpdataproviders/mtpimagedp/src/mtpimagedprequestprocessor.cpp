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


#include <mtp/tmtptyperequest.h>

#include "mtpimagedpconst.h"
#include "cmtprequestprocessor.h"
#include "mtpimagedprequestprocessor.h"

//processors
#include "cmtpimagedpgetobjectpropdesc.h"
#include "cmtpimagedpgetobjectproplist.h"
#include "cmtpimagedpgetobject.h"
#include "cmtpimagedpgetobjectinfo.h"
#include "cmtpimagedpgetthumb.h"
#include "cmtpimagedpgetobjectpropssupported.h"
#include "cmtpimagedpsendobjectinfo.h"
#include "cmtpimagedpgetpartialobject.h"
#include "cmtpimagedpcopyobject.h"
#include "cmtpimagedpmoveobject.h"
#include "cmtpimagedpdeleteobject.h"
#include "cmtpimagedpgetobjectpropvalue.h"
#include "cmtpimagedpsetobjectpropvalue.h"
#include "cmtpimagedpsetobjectproplist.h"
#include "cmtpimagedpgetformatcapabilities.h"
#include "cmtpimagedpgetobjectreferences.h"
#include "cmtpimagedpsetobjectreferences.h"
#include "cmtprequestunknown.h"
#include "cmtpimagedpsetobjectprotection.h"

/** 
Defines a processor factory function pointer
*/
typedef MMTPRequestProcessor* (*TMTPImageDpRequestProcessorCreateFunc)(
                                                                MMTPDataProviderFramework& aFramework, 
                                                                MMTPConnection& aConnection,
                                                                CMTPImageDataProvider& aDataProvider);

/** 
Defines an entry which maps from operation code to the request processor
*/
typedef struct 
	{
	TUint16                                  iOperationCode;
	TMTPImageDpRequestProcessorCreateFunc    iCreateFunc;
	}TMTPImageDpRequestProcessorEntry;

/**
A mapping table from the operation code to the request processor factory method
*/

// --------------------------------------------------------------------------
// A mapping table from the operation code to the request processor factory method
// --------------------------------------------------------------------------
//
static const TMTPImageDpRequestProcessorEntry KMTPImageDpRequestProcessorTable[] = 
	{
	{EMTPOpCodeGetObjectInfo, CMTPImageDpGetObjectInfo::NewL},
	{EMTPOpCodeGetObject,  CMTPImageDpGetObject::NewL},
	{EMTPOpCodeGetPartialObject,  CMTPImageDpGetPartialObject::NewL},		
    {EMTPOpCodeGetThumb, CMTPImageDpGetThumb::NewL}, 
	{EMTPOpCodeGetObjectPropsSupported, CMTPImageDpGetObjectPropsSupported::NewL}, 
	{EMTPOpCodeGetObjectPropDesc, CMTPImageDpGetObjectPropDesc::NewL},
	{EMTPOpCodeGetObjectPropValue, CMTPImageDpGetObjectPropValue::NewL},
	{EMTPOpCodeGetObjectPropList, CMTPImageDpGetObjectPropList::NewL},
	{EMTPOpCodeGetObjectReferences, CMTPImageDpGetObjectReferences::NewL},
	{EMTPOpCodeSetObjectReferences, CMTPImageDpSetObjectReferences::NewL},
	{EMTPOpCodeDeleteObject, CMTPImageDpDeleteObject::NewL},		
	{EMTPOpCodeSendObjectInfo,  CMTPImageDpSendObjectInfo::NewL},
	{EMTPOpCodeSendObject,  CMTPImageDpSendObjectInfo::NewL},	//force the SendObject request to be processed by the SendObjectInfo processor 
	{EMTPOpCodeSendObjectPropList, CMTPImageDpSendObjectInfo::NewL},	//force the SendObjectPropList request to be processed by the SendObjectInfo processor 
	{EMTPOpCodeSetObjectPropValue, CMTPImageDpSetObjectPropValue::NewL},
	{EMTPOpCodeSetObjectPropList, CMTPImageDpSetObjectPropList::NewL},	
	{EMTPOpCodeMoveObject, CMTPImageDpMoveObject::NewL},	
	{EMTPOpCodeCopyObject, CMTPImageDpCopyObject::NewL},
	{EMTPOpCodeGetFormatCapabilities,CMTPImageDpGetFormatCapabilities::NewL},
	{EMTPOpCodeSetObjectProtection,CMTPImageDpSetObjectProtection::NewL},
	};

// --------------------------------------------------------------------------
// Create a request processor that matches the request
// @param aPlugin	The reference to the data provider plugin 
// @param aFramework The reference to the data provider framework
// @param aRequest	The request to be processed
// @param aConnection The connection from which the request comes from
// @return a pointer to the request processor
// --------------------------------------------------------------------------
//
MMTPRequestProcessor* MTPImageDpProcessor::CreateL(
													MMTPDataProviderFramework& aFramework,
													const TMTPTypeRequest& aRequest, 
													MMTPConnection& aConnection,
													CMTPImageDataProvider& aDataProvider)
	{
    TMTPImageDpRequestProcessorCreateFunc createFunc = NULL; 
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	TInt count = sizeof(KMTPImageDpRequestProcessorTable) / sizeof(TMTPRequestProcessorEntry);

	for(TInt i = 0; i < count; i++)
		{
		if(KMTPImageDpRequestProcessorTable[i].iOperationCode == operationCode)
			{
			createFunc = KMTPImageDpRequestProcessorTable[i].iCreateFunc;
			break;
			}
		}
		
	if(!createFunc)	
		{
		return CMTPRequestUnknown::NewL(aFramework, aConnection);
		}
	else
	    {
	    return (*createFunc)(aFramework, aConnection, aDataProvider);
	    }
	}
