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

#include <obexbase.h>
#include "obexnotifyhandlerbase.h"

/**
@file
@released
@internalComponent
*/

/** Constructor.

@param aObex The object to pass notifications to.
*/
CObexNotifyHandlerBase::CObexNotifyHandlerBase(CObex& aObex)
	: iObex(aObex)
	{
	}

/** Passes on Process notification.

@param aPacket The packet to pass on to the CObex that will 
			   handle this notification.
*/
void CObexNotifyHandlerBase::Process(CObexPacket& aPacket)
	{
	iObex.NotifyProcess(aPacket);
	}

/** Passes on Error notification.

@param aError The error to pass on to the CObex that will 
			  handle this notification.
*/
void CObexNotifyHandlerBase::Error(TInt aError)
	{
	iObex.NotifyError(aError);
	}

/** Passes on TransportUp notification.
*/	
void CObexNotifyHandlerBase::TransportUp()
	{
	iObex.NotifyTransportUp();
	}

/** Passes on TransportDown notification.
*/	
void CObexNotifyHandlerBase::TransportDown(TBool aForceTransportDeletion)
	{
	iObex.NotifyTransportDown(aForceTransportDeletion);
	}
