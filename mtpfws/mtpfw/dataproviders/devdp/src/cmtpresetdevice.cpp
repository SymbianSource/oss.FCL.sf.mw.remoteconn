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

/**
 @file
 @internalComponent
*/

#include <mtp/tmtptyperequest.h>

#include "cmtpresetdevice.h"
#include "cmtpconnection.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "mtpdevdppanic.h"
#include "cmtpconnectionmgr.h"
#include "rmtpframework.h"

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPResetDevice::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPResetDevice* self = new (ELeave) CMTPResetDevice(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPResetDevice::~CMTPResetDevice()
    {
    iSingletons.Close();
    }

/**
Standard c++ constructor
*/    
CMTPResetDevice::CMTPResetDevice(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPResetDevice::ConstructL()
    {
    iSingletons.OpenL();
    }
    
/**
Request checker
*/
TMTPResponseCode CMTPResetDevice::CheckRequestL()
    {
    TMTPResponseCode response = CMTPRequestProcessor::CheckRequestL();
    if (response == EMTPRespCodeOK)
        {
        TUint32 sessionId = iRequest->Uint32(TMTPTypeRequest::ERequestParameter1);
        //default session (with id 0) should never be closed
        if (!iConnection.SessionWithMTPIdExists(sessionId) || sessionId == 0)
            {
            response = EMTPRespCodeSessionNotOpen;
            }
        }
    return response;    
    }
        
/**
ResetDevice request handler. Simply close the session because we don't support multiple session for now.
*/    
void CMTPResetDevice::ServiceL()    
    {
    // Send response first before close the session
    SendResponseL(EMTPRespCodeOK);
     
    TUint connectionId = iConnection.ConnectionId();
    CMTPConnectionMgr& connectionMgr = iSingletons.ConnectionMgr();
    CMTPConnection& connection = connectionMgr.ConnectionL(connectionId);
    connection.SessionClosedL(0x0FFFFFFF);   
    }

