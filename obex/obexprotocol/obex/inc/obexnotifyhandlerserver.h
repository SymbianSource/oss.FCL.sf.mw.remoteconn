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
//

#ifndef OBEXNOTIFYHANDLERSERVER_H
#define OBEXNOTIFYHANDLERSERVER_H

/**
@file
@released
@internalComponent
*/

#include "obexnotifyhandlerbase.h"

class CObexServer;
/** Recieves notifications from transport.

This handles MObexNotifyExtend functions that require knowledge
that the CObex that owns this class is a CObexServer.
*/
NONSHARABLE_CLASS(CObexNotifyHandlerServer) : public CObexNotifyHandlerBase
	{
public:
	CObexNotifyHandlerServer(CObexServer& aObex);

private:
	virtual void SignalPacketProcessEvent(TObexPacketProcessEvent aEvent);
	};
	
#endif // OBEXNOTIFYHANDLERSERVER_H
