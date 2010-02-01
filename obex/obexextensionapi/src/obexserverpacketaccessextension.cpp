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

#include <obex/extensionapis/obexserverpacketaccessextension.h>
#include <obexserver.h>
#include <obex/internal/mobexserverrequestpacketnotifyregister.h>
#include "obexextensionapifaults.h"

//Category used for internal panics
_LIT(KPanicCat, "ObexExtApis");

/**
Factory function for CObexServerPacketAccessExtension. Will leave if the associated CObexServer 
does not support this extension type.

Note that this object needs to be deleted before the CObexServer object it is registered to is
deleted.

@param aServer Reference to the instance of CObexServer being extended.
@param aPacketNotify Reference to the instance of MObexRequestPacketNotify to be notified of 
incoming packets. Notifications will stop when this CObexServerPacketAccessExtension object is
deleted.
@return A pointer to a new CObexServerPacketAccessExtension object.
*/
EXPORT_C CObexServerPacketAccessExtension* CObexServerPacketAccessExtension::NewL(CObexServer& aServer, MObexServerRequestPacketNotify& aRequestPacketNotify)
	{
	CObexServerPacketAccessExtension* self = new (ELeave) CObexServerPacketAccessExtension();
	CleanupStack::PushL(self);
	self->ConstructL(aServer, aRequestPacketNotify);
	CleanupStack::Pop(self);
	return self;
	}

/**
The destructor will not delete the instance of the MObexServerRequestPacketNotify
it holds.  It will signal to the CObexServer instance that packet notification is
no longer required, and so re-allow another CObexServerPacketAccessExtension to bind
to the CObexServer instance.

This should be called before the CObexServer instance the object is registered to
is deleted.
*/
CObexServerPacketAccessExtension::~CObexServerPacketAccessExtension()
	{
	if (iRequestPacketNotify)
		{
		iRequestPacketNotify->DeleteObexServerRequestPacketNotifyRegister();
		}
	}

/**
Constructor.
*/
CObexServerPacketAccessExtension::CObexServerPacketAccessExtension()
	{
	
	}

/**
Performs the main construction of the CObexServerPacketAccessExtension.

@param aServer The CObexServer instance to register request packet notification callbacks.
@param aRequestPacketNotify The mixin instance that will receive notification of request packets.
*/
void CObexServerPacketAccessExtension::ConstructL(CObexServer& aServer, MObexServerRequestPacketNotify& aRequestPacketNotify)
	{
	iRequestPacketNotify = static_cast<MObexServerRequestPacketNotifyRegister*>(aServer.ExtensionInterfaceL(KObexServerRequestPacketNotifyRegisterInterface));
	// by here we should have a correctly cast instance, as otherwise we should have already left.
	__ASSERT_ALWAYS(iRequestPacketNotify, User::Panic(KPanicCat, EInterfaceInstanceIsANullPointer));
	iRequestPacketNotify->SetObexServerRequestPacketObserver(aRequestPacketNotify);
	}

