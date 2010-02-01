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

#ifndef OBEXSERVERPACKETACCESSEXTENSION_H
#define OBEXSERVERPACKETACCESSEXTENSION_H

#include <e32base.h>

class CObexServer;
class MObexServerRequestPacketNotify;
class MObexServerRequestPacketNotifyRegister;

/**
This class provides the extension API to give an OBEX server application
the option of registering a callback interface for being informed of
packets the server receives before the server processes them.

@publishedPartner
@released
*/
NONSHARABLE_CLASS(CObexServerPacketAccessExtension) : public CBase
	{
public:
	IMPORT_C static CObexServerPacketAccessExtension* NewL(CObexServer& aServer, MObexServerRequestPacketNotify& aRequestPacketNotify);
	
	~CObexServerPacketAccessExtension();
	
private:
	CObexServerPacketAccessExtension();
	void ConstructL(CObexServer& aServer, MObexServerRequestPacketNotify& aRequestPacketNotify);
	
private:
	MObexServerRequestPacketNotifyRegister* iRequestPacketNotify;
	
	};

#endif // OBEXSERVERPACKETACCESSEXTENSION_H

