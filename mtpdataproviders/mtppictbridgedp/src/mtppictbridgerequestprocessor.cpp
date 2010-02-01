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
#include "mtppictbridgedpconst.h"
#include "mtppictbridgedpprocessor.h"
#include "cmtppictbridgedpgetobject.h"
#include "cmtppictbridgedpgetobjectinfo.h"
#include "cmtppictbridgedpsendobjectinfo.h"
#include "cmtprequestunknown.h"
#include "cmtpcommonrequestprocessor.h"

/** 
Defines a processor factory function pointer

@internalTechnology
*/
typedef MMTPRequestProcessor* (*TMTPPictBridgeRequestProcessorCreateFunc)(
                                                                MMTPDataProviderFramework& aFramework, 
                                                                MMTPConnection& aConnection,
                                                                CMTPPictBridgeDataProvider& aDataProvider);


/** 
Defines an entry which maps from operation code to the request processor

@internalTechnology
*/
typedef struct 
    {
    TUint16                                  iOperationCode;
    TMTPPictBridgeRequestProcessorCreateFunc iCreateFunc;
    }TMTPPictBridgeRequestProcessorEntry;

template<TMTPResponseCode RESPCODE, TBool HASITORDATA>
static MMTPRequestProcessor* CommonProcessorCreator(MMTPDataProviderFramework& aFramework, 
                                                    MMTPConnection& aConnection,
                                                    CMTPPictBridgeDataProvider& )
    {
    return CMTPCommonRequestProcessor::NewL<RESPCODE, HASITORDATA>(aFramework, aConnection);
    }
    

// --------------------------------------------------------------------------
// A mapping table from the operation code to the request processor factory method
// --------------------------------------------------------------------------
//
static const TMTPPictBridgeRequestProcessorEntry KMTPPictBridgeDpRequestProcessorTable[]=
    {
    {EMTPOpCodeGetObjectInfo,           CMTPPictBridgeDpGetObjectInfo::NewL},
    {EMTPOpCodeGetObject,               CMTPPictBridgeDpGetObject::NewL},
    {EMTPOpCodeSendObjectInfo,          CMTPPictBridgeDpSendObjectInfo::NewL},
    {EMTPOpCodeSendObject,              CMTPPictBridgeDpSendObjectInfo::NewL}  
    };

// --------------------------------------------------------------------------
// Create a request processor that matches the request
// @param aPlugin   The reference to the data provider plugin 
// @param aFramework The reference to the data provider framework
// @param aRequest  The request to be processed
// @param aConnection The connection from which the request comes from
// @return a pointer to the request processor
// --------------------------------------------------------------------------
//
MMTPRequestProcessor* MTPPictBridgeDpProcessor::CreateL(
                                                    MMTPDataProviderFramework& aFramework,
                                                    const TMTPTypeRequest& aRequest, 
                                                    MMTPConnection& aConnection,
                                                    CMTPPictBridgeDataProvider& aDataProvider)
    {
    

    TMTPPictBridgeRequestProcessorCreateFunc createFunc = NULL; 
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
    TInt count = sizeof(KMTPPictBridgeDpRequestProcessorTable) / sizeof(TMTPRequestProcessorEntry);

    for(TInt i = 0; i < count; i++)
        {
        if(KMTPPictBridgeDpRequestProcessorTable[i].iOperationCode == operationCode)
            {
            createFunc = KMTPPictBridgeDpRequestProcessorTable[i].iCreateFunc;
            break;
            }
        }

    if(!createFunc) 
        {
        return CMTPRequestUnknown::NewL(aFramework, aConnection);
        }

    return (*createFunc)(aFramework, aConnection, aDataProvider);
    }

