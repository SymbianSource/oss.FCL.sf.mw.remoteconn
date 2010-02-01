// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMEDIASYNCSERVERDEF_H
#define CMEDIASYNCSERVERDEF_H

#include <comms-infras/commsdebugutility.h>

__FLOG_STMT(_LIT8(KMSSSubsystem, "MSS");)

_LIT(KFinderMSSName, "mediasyncserver*");
_LIT(KMediaSyncServerName, "mediasyncserver");
 
_LIT(KMssDbName, "mediasync.db");
_LIT(KImageTableName, "ImageStore");
_LIT(KSQLCombinedIndex, "CombinedIndex");
_LIT(KMssLockName, "mss.lock");

_LIT(KMediaSyncClientPanicCategory, "MediaSyncServ-Client");

// MIME definition
_LIT(KJpegMime, "image/jpeg");

const TInt KCustomSqlMaxLength = 512;

const TInt KMediaSyncServerVersionMinor = 0;
const TInt KMediaSyncServerVersionMajor = 1;

/**
The Media Sync Server process UID3.
*/
const TUid KMediaSyncServerUid3 = {0x20024331};

enum TMediaSyncPanicsClient
    {
    ECannotStartServer,
    EBadRequest,
    ERequestPending
    };

enum TMediaSyncClientMessage 
    {
    EMediaSyncClientGetGSHHandle,
    EMediaSyncClientGetChanges,    
    EMediaSyncClientRemoveAllRecords,
    EMediaSyncClientEnableMonitor,
    EMediaSyncClientDisableMonitor,
    EMediaSyncClientNeedFullSync,
    EMediaSyncClientClearFullSync,
    EMediaSyncClientShutdown,
    EMediaSyncClientNotSupported
    };

const TUint KMssRemoval     = 1;
const TUint KMssAddition    = 2;
const TUint KMssChange      = 3;
const TUint KMssPresent     = 4;
const TUint KMssNotPresent  = 5;

class TDataHeaderInfo
    {
public:
    TInt iCount;
    };

#endif /* CMEDIASYNCSERVERDEF_H */
