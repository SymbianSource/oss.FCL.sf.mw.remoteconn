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
 @internalTechnology
*/

#ifndef OBEXSERVERREQUESTPACKETNOTIFYREGISTER_H
#define OBEXSERVERREQUESTPACKETNOTIFYREGISTER_H

#include <e32std.h>

class MObexServerRequestPacketNotify;

/**
	UID with which one can request the 
	MObexServerRequestPacketNotifyRegister API through	
	CObexServer::ExtensionInterface(). 
	
	Some implementations may use KObexRequestPacketNotifyInterfaceId
	which is considered deprecated.
*/
const TUid KObexServerRequestPacketNotifyRegisterInterface = {0x10274CB4};

/**
Mixin class to be implemented by instances providing a means to
set an observer for the receipt of Obex request packets by a server.

This provides part of the packet access API.
*/
class MObexServerRequestPacketNotifyRegister
	{
public:
	/**
	Sets the notifier to be called when an Obex server receives a request packet.
	
	@param aRequestPacketNotify A pointer to the observer interface instance to receive the notifications.
	*/
	virtual void SetObexServerRequestPacketObserver(MObexServerRequestPacketNotify& aRequestPacketNotify) = 0;
	
	/**
	Inform the Obex server that we have finished with this interface,
	and delete the engine to allow a new one to be created.
	*/
	virtual void DeleteObexServerRequestPacketNotifyRegister() = 0;
	};

#endif // OBEXSERVERREQUESTPACKETNOTIFYREGISTER_H

