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
// NOTE: This class provides inline functions - DO NOT EXPORT THIS HEADER.
// 
//

/**
 @file
 @internalComponent
*/

#ifndef OBEXSERVERREQUESTPACKETENGINE_H
#define OBEXSERVERREQUESTPACKETENGINE_H

#include <e32base.h>
#include <obex/internal/mobexserverrequestpacketnotifyregister.h>

class CObexServer;
class MObexServerRequestPacketNotify;

/**
Utility class for CObexServer - to allow the OBEX Server extension API for 
packet access.
*/
NONSHARABLE_CLASS(CObexServerRequestPacketEngine) : public CBase, public MObexServerRequestPacketNotifyRegister
	{
public:
	static CObexServerRequestPacketEngine* NewL(CObexServer& aServer);
	~CObexServerRequestPacketEngine();
	
	virtual void SetObexServerRequestPacketObserver(MObexServerRequestPacketNotify& aPacketNotify);
	virtual void DeleteObexServerRequestPacketNotifyRegister();
	
	inline MObexServerRequestPacketNotify* RequestPacketNotify() const;
	
private:
	CObexServerRequestPacketEngine(CObexServer& aServer);

private:
	CObexServer& iServer;
	MObexServerRequestPacketNotify* iRequestPacketNotify;
	};

inline MObexServerRequestPacketNotify* CObexServerRequestPacketEngine::RequestPacketNotify() const
	{
	return iRequestPacketNotify;
	}

#endif // OBEXSERVERREQUESTPACKETENGINE_H

