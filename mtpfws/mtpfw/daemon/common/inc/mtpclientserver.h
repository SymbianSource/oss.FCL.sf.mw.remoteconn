// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef MTPCLIENTSERVER_H
#define MTPCLIENTSERVER_H

#include <e32std.h>

/**
The MTP daemon server process name.
*/
_LIT(KMTPServerName,"mtpserver");

/**
The current major version number of the MTP daemon.
*/
const TUint KMTPVersionMajor(1);

/**
The current minor version number of the MTP daemon.
*/
const TUint KMTPVersionMinor(0);

/**
The current build number of the MTP daemon.
*/
const TUint KMTPVersionBuild(0);

/**
The MTP daemon server process UID3.
*/
const TUid KMTPServerUid3 = {0x102827A2};

/**
The number of message slots available to an MTP client session.
*/
const TInt KMTPASyncMessageSlots(1);

/**
The MTP client/server IPC function codes.
*/
enum TMTPClientIpcCodes
	{
	/**
	RMTPClient::StartTransport
	*/
	EMTPClientStartTransport    = 0,
	
	/**
	RMTPClient::StopTransport
	*/
	EMTPClientStopTransport     = 1,
	
	/**
	RMTPClient::StopTransport
	*/
	EMTPClientClose             = 2,

/**
	RMTPClient::IsAvailable
	*/
	EMTPClientIsAvailable       = 3,

	/**
	The invalid function code placeholder.
	*/
	EMTPClientNotSupported
	};

#endif // MTPCLIENTSERVER_H
