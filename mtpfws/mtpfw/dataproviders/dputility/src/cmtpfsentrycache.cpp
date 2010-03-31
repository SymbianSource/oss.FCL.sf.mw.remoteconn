// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/mtpprotocolconstants.h>

#include "cmtpfsentrycache.h"


__FLOG_STMT(_LIT8(KComponent,"MTPFSEntryCache");)

// -----------------------------------------------------------------------------
// CMTPFSEntryCache::NewL
// Two-phase construction method
// -----------------------------------------------------------------------------
//
EXPORT_C CMTPFSEntryCache* CMTPFSEntryCache::NewL()
    {
    CMTPFSEntryCache* self = new (ELeave) CMTPFSEntryCache();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CMTPFSEntryCache::~CMTPFSEntryCache()
    {
    __FLOG(_L8("~CMTPFSEntryCache - Entry"));
    __FLOG(_L8("~CMTPFSEntryCache - Exit"));
    __FLOG_CLOSE;
    }

EXPORT_C TBool CMTPFSEntryCache::IsOnGoing() const
    {
    return iIsOngoing;
    }

EXPORT_C void CMTPFSEntryCache::SetOnGoing(TBool aOnGoing)
    {
    __FLOG(_L8("SetOnGoing - Entry"));
    iIsOngoing = aOnGoing;
    __FLOG(_L8("SetOnGoing - Exit"));
    }

EXPORT_C TUint32 CMTPFSEntryCache::TargetHandle() const
    {
    return iTargetHandle;
    }

EXPORT_C void CMTPFSEntryCache::SetTargetHandle(TUint32 aHandle)
    {
    __FLOG(_L8("SetTargetHandle - Entry"));
    iTargetHandle = aHandle;
    __FLOG(_L8("SetTargetHandle - Exit"));
    }

EXPORT_C TEntry& CMTPFSEntryCache::FileEntry()
    {
    return iFileEntry;
    }

EXPORT_C void CMTPFSEntryCache::SetFileEntry(const TEntry& aEntry)
    {
    __FLOG(_L8("SetFileEntry - Entry"));
    iFileEntry = aEntry;
    __FLOG(_L8("SetFileEntry - Exit"));
    }

CMTPFSEntryCache::CMTPFSEntryCache():iIsOngoing(EFalse), iTargetHandle(KMTPHandleNone)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPFSEntryCache - Entry"));
    __FLOG(_L8("CMTPFSEntryCache - Exit"));
    }

void CMTPFSEntryCache::ConstructL()
    {    
    }
