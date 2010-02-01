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

#include <obex/transport/mobextransportnotify.h>
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

/**
Process the received packet.
@param aPacket An obex packet.
*/
EXPORT_C void MObexTransportNotify::Process(CObexPacket &aPacket) 
	{
	LOG_LINE
	LOG_FUNC

	DoProcess(aPacket);
	}
	
/**
Indicate that an error has occurred in the transport.
@param aError The error that has occurred.
*/	
EXPORT_C void MObexTransportNotify::Error(TInt aError)
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\taError = %d"), aError);

	DoError(aError);		
	}
	
/**
Indicate that the transport is up.
@param aInfo The connection info. 
*/	
EXPORT_C void MObexTransportNotify::TransportUp(TObexConnectionInfo& aInfo) 
	{
	LOG_LINE
	LOG_FUNC

	DoTransportUp(aInfo);		
	}

/** Signals that an event related to processing the packet has occurred.
@param aEvent The event that has occurred.
*/	
EXPORT_C void MObexTransportNotify::SignalPacketProcessEvent(TObexPacketProcessEvent aEvent)
	{
	DoSignalPacketProcessEvent(aEvent);
	}
