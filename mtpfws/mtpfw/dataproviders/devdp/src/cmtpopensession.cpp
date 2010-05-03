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

#include "cmtpopensession.h"
#include "cmtpconnection.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "mtpdevdppanic.h"
#include "cmtpconnectionmgr.h"
#include "rmtpframework.h"

/**
Verification data for OpenSession request
*/
const TMTPRequestElementInfo KMTPOpenSessionPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeSessionID, EMTPElementAttrNone, 0, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin    The data provider plugin
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPOpenSession::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPOpenSession* self = new (ELeave) CMTPOpenSession(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPOpenSession::~CMTPOpenSession()
    {
    iSingletons.Close();
    }

/**
Standard c++ constructor
*/    
CMTPOpenSession::CMTPOpenSession(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPOpenSessionPolicy)/sizeof(TMTPRequestElementInfo), KMTPOpenSessionPolicy)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPOpenSession::ConstructL()
    {
    iSingletons.OpenL();
    }
    
/**
OpenSession request handler.  Adds the session to the connection.
*/    
void CMTPOpenSession::ServiceL()    
    {
    if(iSingletons.DpController().EnumerateState() < CMTPDataProviderController::EEnumeratingPhaseOneDone)
        {
        RegisterPendingRequest();
        return;
        }
    
    if(iSingletons.DpController().EnumerateState() < CMTPDataProviderController::EEnumeratingPhaseOneDone)
    	{
    	SendResponseL(EMTPRespCodeDeviceBusy);
    	}
    else
    	{
    	const TUint connectionId = iConnection.ConnectionId();
    	CMTPConnectionMgr& connectionMgr = iSingletons.ConnectionMgr();
    	CMTPConnection& connection = connectionMgr.ConnectionL(connectionId);
    	connection.SessionOpenedL(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    	SendResponseL(EMTPRespCodeOK);
    	}
    }
