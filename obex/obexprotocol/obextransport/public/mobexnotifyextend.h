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

#ifndef MOBEXNOTIFYEXTEND_H
#define MOBEXNOTIFYEXTEND_H

/**
@file
@released
@internalTechnology
*/

#include <e32std.h>
#include <mobexnotify.h>
#include <obex/internal/obextransportconstants.h>

class CObexPacket;
/**
Provides the call back interface for anything using CObexTransport (ie CObex).
Note:  This is an internal class which is not intended for use outside of
the Transport<->Obex interface.  Even where access rules allow it, external
users should not call these functions as their implementation may change.
*/
NONSHARABLE_CLASS(MObexNotifyExtend) : public MObexNotify
	{
public:
	// original functions as found in MObexNotify
	virtual void Process(CObexPacket& aPacket) =0;
	virtual void Error(TInt aError) =0;
	virtual void TransportUp() =0;
	
	// This is non-pure as derivers may not care about any events so we
	// don't want to force them into handling them.
	virtual void SignalPacketProcessEvent(TObexPacketProcessEvent /*aEvent*/){}; 
	};

#endif // MOBEXNOTIFYEXTEND_H
