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

#ifndef OBEXNOTIFYHANDLERCLIENT_H
#define OBEXNOTIFYHANDLERCLIENT_H

/**
@file
@released
@internalComponent
*/

#include "obexnotifyhandlerbase.h"

class CObexClient;
/** Recieves notifications from transport.

This handles MObexNotifyExtend functions that require knowledge
that the CObex that owns this class is a CObexClient.
*/
NONSHARABLE_CLASS(CObexNotifyHandlerClient) : public CObexNotifyHandlerBase
	{
public:
	CObexNotifyHandlerClient(CObexClient& aObex);

private:
	virtual void SignalPacketProcessEvent(TObexPacketProcessEvent aEvent);
	};
	
#endif // OBEXNOTIFYHANDLERCLIENT_H
