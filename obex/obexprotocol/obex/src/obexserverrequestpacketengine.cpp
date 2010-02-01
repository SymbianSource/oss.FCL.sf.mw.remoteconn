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

#include "obexserverrequestpacketengine.h"
#include <obexserver.h>

CObexServerRequestPacketEngine* CObexServerRequestPacketEngine::NewL(CObexServer& aServer)
	{
	CObexServerRequestPacketEngine* self = new (ELeave) CObexServerRequestPacketEngine(aServer);
	return self;
	}
	
CObexServerRequestPacketEngine::~CObexServerRequestPacketEngine()
	{

	}
	
CObexServerRequestPacketEngine::CObexServerRequestPacketEngine(CObexServer& aServer)
	: iServer(aServer)
	{

	}

void CObexServerRequestPacketEngine::SetObexServerRequestPacketObserver(MObexServerRequestPacketNotify& aRequestPacketNotify)
	{
	iRequestPacketNotify = &aRequestPacketNotify;
	}
	
void CObexServerRequestPacketEngine::DeleteObexServerRequestPacketNotifyRegister()
	{
	iServer.iServerRequestPacketEngine = NULL;
	delete this;
	}


