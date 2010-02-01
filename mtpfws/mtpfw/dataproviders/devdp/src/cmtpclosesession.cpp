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

#include "cmtpclosesession.h"
#include "cmtpconnection.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "cmtpconnectionmgr.h"
#include "mtpdevdppanic.h"
#include "rmtpframework.h"
    
/**
Two-phase construction method
@param aPlugin    The data provider plugin
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/
MMTPRequestProcessor* CMTPCloseSession::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPCloseSession* self = new (ELeave) CMTPCloseSession(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPCloseSession::~CMTPCloseSession()
    {
    iSingletons.Close();
    }
    
/**
Standard c++ constructor
*/    
CMTPCloseSession::CMTPCloseSession(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPCloseSession::ConstructL()
    {
    iSingletons.OpenL();
    }

/**
Request checker
*/
TMTPResponseCode CMTPCloseSession::CheckRequestL()
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
CloseSession request handler
*/    
void CMTPCloseSession::ServiceL()
    {    
    // Send response first before close the session
    SendResponseL(EMTPRespCodeOK);
    
    // The sessionId for closesession is always 0, the actual session ID to close is
    // stored in payload parameter 1.
    RemoveSessionL(iRequest->Uint32(TMTPTypeRequest::ERequestParameter1));    
    }


/**
Remove the session from the connection
@param aSessionId the sessionId to be removed
*/
void CMTPCloseSession::RemoveSessionL(TUint32 aSessionId)
    {
    TUint connectionId = iConnection.ConnectionId();
    CMTPConnectionMgr& connectionMgr = iSingletons.ConnectionMgr();
    CMTPConnection& connection = connectionMgr.ConnectionL(connectionId);
    connection.SessionClosedL(aSessionId);    
    }




