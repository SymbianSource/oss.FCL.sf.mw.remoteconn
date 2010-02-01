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

/**
 @file
 @released
 @internalComponent
*/

#include "obexnotifyhandlerserver.h"

/** Constructor.

@param aObex The CObexServer to pass notifications onto.
*/
CObexNotifyHandlerServer::CObexNotifyHandlerServer(CObexServer& aObex)
	: CObexNotifyHandlerBase(aObex)
	{
	}

/** Signals that a packet process event has occurred.

@param aEvent The event that has occurred.
*/
void CObexNotifyHandlerServer::SignalPacketProcessEvent(TObexPacketProcessEvent aEvent)
	{
	(static_cast<CObexServer&>(iObex)).SignalPacketProcessEvent(aEvent);
	}
