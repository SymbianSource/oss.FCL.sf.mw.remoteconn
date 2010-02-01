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

#ifndef OBEXNOTIFYHANDLERBASE_H
#define OBEXNOTIFYHANDLERBASE_H

/**
@file
@released
@internalComponent
*/

#include <obex.h>
#include <obex/internal/mobexnotifyextend.h>

class CObexPacket;
struct TObexConnectionInfo;
/** Receives notifications from transport.

This is provided as a way of retaining BC for the CObexClient and 
CObexServer classes.

This class implements the MObexNotifyExtend interface which is provided
to the transport.  Functions can safely be added to MObexNotifyExtend to 
allow the transport to provide additional notifications in future without
altering the vtable for the published classes.  

A CObex owns a CObexNotifyHandlerBase.  The CObexNotifyHandler base
should not be instantiated directly, instead one of the derived classes
should be used.  The derived class then knows the type of the CObex
object created it, as the constructor will require an appropriately
derived type.  This allows the non-virtual functions in the appropriate
class to be called to handle notifications.

Functions that are generic CObex functions, rather than functions that
are handled differently by CObexClient and CObexServer, may be implemented
in the base class.
*/
NONSHARABLE_CLASS(CObexNotifyHandlerBase) : public CBase, public MObexNotifyExtend
	{
public:
	virtual void Process(CObexPacket& aPacket);
	virtual void Error(TInt aError);
	virtual void TransportUp();	
	virtual void TransportDown(TBool aForceTransportDeletion);
	
protected:
	CObexNotifyHandlerBase(CObex& aObex);

protected:
	CObex& iObex;
	};
	
#endif // OBEXNOTIFYHANDLERBASE_H
