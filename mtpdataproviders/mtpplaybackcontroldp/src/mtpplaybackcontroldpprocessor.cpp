// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include "mtpplaybackcontroldpprocessor.h"
#include "cmtprequestprocessor.h"
#include "cmtppbcgetdevicepropdesc.h"
#include "cmtppbcgetdevicepropvalue.h"
#include "cmtppbcsetdevicepropvalue.h"
#include "cmtppbcresetdevicepropvalue.h"
#include "cmtppbcskip.h"
#include "cmtprequestunknown.h"

typedef MMTPRequestProcessor*(*TMTPPlaybackRequestProcessorCreateFunc)(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPlaybackControlDataProvider& aDataProvider);

/**
Defines an entry which maps from operation code to the request processor

@internalComponent
*/
typedef struct
    {
    TUint16                                  iOperationCode;
    TMTPPlaybackRequestProcessorCreateFunc    iCreateFunc;
    } TMTPPlaybackRequestProcessorEntry;

/**
Playback control data provider mapping table from request ID to factory method of the request processor
*/
static const TMTPPlaybackRequestProcessorEntry KMTPRequestProcessorTable[] = 
	{
		{EMTPOpCodeGetDevicePropDesc, CMTPPbcGetDevicePropDesc::NewL},
		{EMTPOpCodeGetDevicePropValue, CMTPPbcGetDevicePropValue::NewL},
		{EMTPOpCodeSetDevicePropValue, CMTPPbcSetDevicePropValue::NewL},
		{EMTPOpCodeResetDevicePropValue, CMTPPbcResetDevicePropValue::NewL},
		{EMTPOpCodeSkip, CMTPPbcSkip::NewL},
	};

/**
Create a request processor that matches the request
@param aPlugin	The reference to the data provider plugin 
@param aFramework The reference to the data provider framework
@param aRequest	The request to be processed
@param aConnection The connection from which the request comes from
@return a pointer to the request processor
*/	
MMTPRequestProcessor* MTPPlaybackControlDpProcessor::CreateL(
													MMTPDataProviderFramework& aFramework,
													const TMTPTypeRequest& aRequest, 
													MMTPConnection& aConnection, 
													CMTPPlaybackControlDataProvider& aDataProvider)
	{
	TMTPPlaybackRequestProcessorCreateFunc createFunc = NULL; 
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
        return CMTPRequestUnknown::NewL(aFramework, aConnection);
        }
    else
        {
        return (*createFunc)(aFramework, aConnection, aDataProvider);
        }
	}



