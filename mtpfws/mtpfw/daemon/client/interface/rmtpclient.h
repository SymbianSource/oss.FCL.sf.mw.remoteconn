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

/**
 @file
 @publishedPartner
 @released
*/

#ifndef RMTPCLIENT_H
#define RMTPCLIENT_H

#include <e32std.h>

/**
Implements the MTP client API. This API provides a control interface to start 
and stop MTP services. 
@publishedPartner
@released
*/
class RMTPClient : public RSessionBase
    {
public:

    IMPORT_C RMTPClient();
    IMPORT_C void Close();
    IMPORT_C TInt Connect();
    IMPORT_C TInt StartTransport(TUid aTransport);
    IMPORT_C TInt StartTransport(TUid aTransport, const TDesC8& aParameter);
    IMPORT_C TInt StopTransport(TUid aTransport);
    IMPORT_C const TVersion& Version() const;
	IMPORT_C TInt  IsAvailable(TUid aTransport);
	IMPORT_C TInt  IsProcessRunning();
private: // Owned

    /**
    The minimum MTP daemon version with which the MTP client API is compatible.
    */
    TVersion iVersion;
    };

/**
Implements the MTP operational mode Publish & Subscribe key.
@publishedPartner
@released
*/
static const TInt32 KUidMTPModeKeyValue(0x1028304B);

/**
Implements the MTP Connection State Publish & Subscribe Category.
@publishedPartner
@released
*/
const TUid KMTPPublishConnStateCat={0x102827A2};

/**
Implements the MTP Connection StatePublish & Subscribe key.
@publishedPartner
@released
*/
enum TMTPConnTypeKey
{
	EMTPConnStateKey,
	EMTPVersionInfo,	
};

/**
  These are the different connection states,This events are used for publishing
  @publishedPartner
  @released 
  */
enum TMTPConnStateType
{
	EConnectedToHost = 0,
	ESendingDataToHost,
	EReceiveDataFromHost,
	EDisconnectedFromHost
};

/**
  The struct is used for MTP over Bluetooth.
  @publishedPartner
  @released
  */
struct TMTPBTRemoteDevice
    {
    TInt64   iDeviceAddr;
    TUint16  iDeviceServicePort;
    };

/**
  The struct is used for MTP over Bluetooth.
  @publishedPartner
  @released
  */
typedef TPckgBuf<TMTPBTRemoteDevice> TMTPBTRemoteDeviceBuf;

/**
  Used for MTP over Bluetooth Parameter Verification.
  @publishedPartner
  @released
  */
const TUint8 KMTPBTRemoteDeviceBufLength( 0x0a );

#endif // RMTPCLIENT_H
