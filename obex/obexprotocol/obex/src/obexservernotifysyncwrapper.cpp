// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// All methods in this file act either to initialise the wrapper object or to forward calls
// from the async server notify class to the synchronous version.
// 
//

/**
 @file
 @internalComponent
 @see MObexServerNotify
 @see MObexServerNotifyAsync
*/

#include <e32base.h>
#include <obexserver.h>
#include <obexobjects.h>
#include "logger.h"
#include "OBEXUTIL.H"
#include "obexservernotifysyncwrapper.h"
#include "obexserverstatemachine.h"


#ifdef _DEBUG
// Save the return value as debugger will (hopefully) then make it easier to see...
#define CHECK_NOERROR(expr) TInt _saved_error = (expr); __ASSERT_ALWAYS(_saved_error == KErrNone, IrOBEXUtil::Fault(ESyncWrapperCallbackError));
#else
#define CHECK_NOERROR(expr) (void)expr
#endif
//
// Initialisation methods
//
CObexServerNotifySyncWrapper* CObexServerNotifySyncWrapper::NewL(CObexServer& aOwner, CObexServerStateMachine& aStateMachine)
	{
	CObexServerNotifySyncWrapper* self = new (ELeave) CObexServerNotifySyncWrapper(aOwner, aStateMachine);
	return self;
	}

CObexServerNotifySyncWrapper::CObexServerNotifySyncWrapper(CObexServer& aOwner, CObexServerStateMachine& aStateMachine)
	: iOwner(aOwner), iStateMachine(aStateMachine)
	{}

CObexServerNotifySyncWrapper::~CObexServerNotifySyncWrapper()
	{
	}

void CObexServerNotifySyncWrapper::SetNotifier(MObexServerNotify* aNotify)
	{
	// if aNotify is NULL this will cause any forwarding methods to dereference NULL.
	// But this should never happen.  K-E 3 will result if they do, but all we'd
	// do otherwise would be to panic anyway...
	iNotify = aNotify;
	}

//
// Forwarding methods
//
void CObexServerNotifySyncWrapper::ErrorIndication(TInt aError)
	{
	iNotify->ErrorIndication(aError);
	}

void CObexServerNotifySyncWrapper::TransportUpIndication()
	{
	iNotify->TransportUpIndication();
	}

void CObexServerNotifySyncWrapper::TransportDownIndication()
	{
	iNotify->TransportDownIndication();
	}

void CObexServerNotifySyncWrapper::ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo)
	{
	// Explicitly code that we're ignoring the return value.  See the doxygen comment.
	(void)iNotify->ObexConnectIndication(aRemoteInfo, aInfo);
	}

void CObexServerNotifySyncWrapper::ObexDisconnectIndication(const TDesC8& aInfo)
	{
	iNotify->ObexDisconnectIndication(aInfo);
	}

//
// Forward the request to the MObexServerNotify implementation, then immediately
// downcall into CObexServer.
//
void CObexServerNotifySyncWrapper::PutRequestIndication()
	{
	WRAPPER_LOG(_L8("Put request indication"));
	iCallbackOutstanding = ETrue;
	iStateMachine.SetAppResponse(ERespSuccess);
	// A NULL return here is OK as it is the response to reject the request.
	CObexBaseObject* object = iNotify->PutRequestIndication();
	
	WRAPPER_LOG(_L8("Put request indication complete"));
	if(iCallbackOutstanding && (iStateMachine.AppResponse() == ERespSuccess))
		{
		CHECK_NOERROR(iOwner.RequestIndicationCallback(object));
		}
	}

TInt CObexServerNotifySyncWrapper::PutPacketIndication()
	{
	return iNotify->PutPacketIndication();
	}

void CObexServerNotifySyncWrapper::PutCompleteIndication()
	{
	iCallbackOutstanding = ETrue;
	TInt err = iNotify->PutCompleteIndication();
	TObexResponse resp = IrOBEXUtil::ObexResponse(err, ERespSuccess);
	if(iCallbackOutstanding)
		{
		CHECK_NOERROR(iOwner.RequestCompleteIndicationCallback(resp));
		}
	}

//
// Forward the request to the MObexServerNotify implementation, then immediately
// downcall into CObexServer.
//
void CObexServerNotifySyncWrapper::GetRequestIndication(CObexBaseObject* aRequiredObject)
	{
	WRAPPER_LOG(_L8("Get request indication"));
	iCallbackOutstanding = ETrue;
	iStateMachine.SetAppResponse(ERespSuccess);
	// A NULL return here is OK as it is the response to reject the request.
	CObexBaseObject* object = iNotify->GetRequestIndication(aRequiredObject);
	
	WRAPPER_LOG(_L8("Get request indication complete"));
	if(iCallbackOutstanding && (iStateMachine.AppResponse() == ERespSuccess))
		{
		CHECK_NOERROR(iOwner.RequestIndicationCallback(object));
		}
	}

TInt CObexServerNotifySyncWrapper::GetPacketIndication()
	{
	return iNotify->GetPacketIndication();
	}

void CObexServerNotifySyncWrapper::GetCompleteIndication()
	{
	iCallbackOutstanding = ETrue;
	TInt err = iNotify->GetCompleteIndication();
	TObexResponse resp = IrOBEXUtil::ObexResponse(err, ERespSuccess);
	if(iCallbackOutstanding)
		{
		CHECK_NOERROR(iOwner.RequestCompleteIndicationCallback(resp));
		}
	}

void CObexServerNotifySyncWrapper::SetPathIndication(const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo)
	{
	iCallbackOutstanding = ETrue;
	TInt err = iNotify->SetPathIndication(aPathInfo, aInfo);
	TObexResponse resp = IrOBEXUtil::ObexResponse(err, ERespSuccess);
	if(iCallbackOutstanding)
		{
		CHECK_NOERROR(iOwner.RequestCompleteIndicationCallback(resp));
		}
	}

void CObexServerNotifySyncWrapper::AbortIndication()
	{
	iNotify->AbortIndication();
	}

void CObexServerNotifySyncWrapper::CancelIndicationCallback()
	{
	//Appilcation may stop the obex server for whatever reason during an Indication 
	//thus resets the state machine. We have to make sure not to call indication 
	//callback in such cases because the statemachine will panic. 
	iCallbackOutstanding = EFalse;
	}
