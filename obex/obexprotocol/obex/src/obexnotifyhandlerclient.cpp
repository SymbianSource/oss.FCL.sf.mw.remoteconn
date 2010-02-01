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

#include "obexnotifyhandlerclient.h"

/** Constructor.

@param aObex The CObexClient to pass notifications onto.
*/
CObexNotifyHandlerClient::CObexNotifyHandlerClient(CObexClient& aObex)
	: CObexNotifyHandlerBase(aObex)
	{
	}

/** Signals that a packet process event has occurred.

@param aEvent The event that has occurred.
*/
void CObexNotifyHandlerClient::SignalPacketProcessEvent(TObexPacketProcessEvent aEvent)
	{
	(static_cast<CObexClient&>(iObex)).SignalPacketProcessEvent(aEvent);
	}
