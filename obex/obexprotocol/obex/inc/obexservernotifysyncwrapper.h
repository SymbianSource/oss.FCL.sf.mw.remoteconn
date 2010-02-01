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
// All rights reserved.
// 
//

#ifndef OBEXSERVERNOTIFYSYNCWRAPPER_H
#define OBEXSERVERNOTIFYSYNCWRAPPER_H


/**
@file
@internalComponent
*/

#include <e32base.h>

#ifdef __FLOG_ACTIVE
_LIT8(KSyncWrapper, "syncwrapper");

#define WRAPPER_LOG(str) CObexLog::Write(KSyncWrapper, str)
#else
#define WRAPPER_LOG(str)
#endif

/**
Implements a synchronous wrapper around the asynchronous notification API
now used by the Obex server.  It therefore provides an SC and BC migration
path for existing code.
@see MObexServerNotify
@see MObexServerNotifyAsync
*/
NONSHARABLE_CLASS(CObexServerNotifySyncWrapper) : public CBase, public MObexServerNotifyAsync
	{
public:
	static CObexServerNotifySyncWrapper* NewL(CObexServer& aOwner, CObexServerStateMachine& aStateMachine);
	~CObexServerNotifySyncWrapper();
	void SetNotifier(MObexServerNotify* aNotify);
	
private:
	CObexServerNotifySyncWrapper(CObexServer& aOwner, CObexServerStateMachine& aStateMachine);

private:
	// From MObexServerNotifyAsync
	void ErrorIndication(TInt aError);
	void TransportUpIndication();
	void TransportDownIndication();
	void ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
	void ObexDisconnectIndication(const TDesC8& aInfo);
	void PutRequestIndication();
	TInt PutPacketIndication();
	void PutCompleteIndication();
	void GetRequestIndication(CObexBaseObject* aRequiredObject);
	TInt GetPacketIndication();
	void GetCompleteIndication();
	void SetPathIndication(const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
	void AbortIndication();
	void CancelIndicationCallback();

private:
	CObexServer& iOwner;
	MObexServerNotify* iNotify;
	TBool		iCallbackOutstanding;
	CObexServerStateMachine& iStateMachine;
	};

#endif	// OBEXSERVERNOTIFYSYNCWRAPPER_H
