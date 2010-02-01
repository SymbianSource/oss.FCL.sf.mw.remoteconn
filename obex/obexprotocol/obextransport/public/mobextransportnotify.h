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

#ifndef __MOBEXTRANSPORTNOTIFY_H__
#define __MOBEXTRANSPORTNOTIFY_H__

#include <e32std.h>
#include <obex/internal/obextransportconstants.h>

class CObexPacket;
struct TObexConnectionInfo;

/**
Provides the call back interface for anything owned  by CObexTransportControllerBase 
(ie CObexConnector or CObexActiveRW derived classes).
Note:  This is an internal class which is not intended for use outside of
the Transport<->Connector/Reader/Writer  interface.  Even where access rules allow it, external
users should not call these functions as their implementation may change.

@publishedPartner
@released
*/
NONSHARABLE_CLASS(MObexTransportNotify)
	{
	
public:
	IMPORT_C void Process(CObexPacket &aPacket) ;
	IMPORT_C void Error(TInt aError);
	IMPORT_C void TransportUp(TObexConnectionInfo& aInfo) ;			// Call back to start the obex sessio 
	IMPORT_C void SignalPacketProcessEvent(TObexPacketProcessEvent aEvent);
		
protected:
	virtual void DoProcess(CObexPacket &aPacket) =0;
	virtual void DoError(TInt aError) =0;
	virtual void DoTransportUp(TObexConnectionInfo& aInfo) =0;
	virtual void DoSignalPacketProcessEvent(TObexPacketProcessEvent aEvent) =0;
			
	};
	
#endif // __MOBEXTRANSPORTNOTIFY_H__
