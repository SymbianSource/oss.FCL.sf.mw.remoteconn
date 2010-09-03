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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpfsentrycacheTraces.h"
#endif



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
    OstTraceFunctionEntry0( CMTPFSENTRYCACHE_CMTPFSENTRYCACHE_DES_ENTRY );
    OstTraceFunctionExit0( CMTPFSENTRYCACHE_CMTPFSENTRYCACHE_DES_EXIT );
    }

EXPORT_C TBool CMTPFSEntryCache::IsOnGoing() const
    {
    return iIsOngoing;
    }

EXPORT_C void CMTPFSEntryCache::SetOnGoing(TBool aOnGoing)
    {
    OstTraceFunctionEntry0( CMTPFSENTRYCACHE_SETONGOING_ENTRY );
    iIsOngoing = aOnGoing;
    OstTraceFunctionExit0( CMTPFSENTRYCACHE_SETONGOING_EXIT );
    }

EXPORT_C TUint32 CMTPFSEntryCache::TargetHandle() const
    {
    return iTargetHandle;
    }

EXPORT_C void CMTPFSEntryCache::SetTargetHandle(TUint32 aHandle)
    {
    OstTraceFunctionEntry0( CMTPFSENTRYCACHE_SETTARGETHANDLE_ENTRY );
    iTargetHandle = aHandle;
    OstTraceFunctionExit0( CMTPFSENTRYCACHE_SETTARGETHANDLE_EXIT );
    }

EXPORT_C TEntry& CMTPFSEntryCache::FileEntry()
    {
    return iFileEntry;
    }

EXPORT_C void CMTPFSEntryCache::SetFileEntry(const TEntry& aEntry)
    {
    OstTraceFunctionEntry0( CMTPFSENTRYCACHE_SETFILEENTRY_ENTRY );
    iFileEntry = aEntry;
    OstTraceFunctionExit0( CMTPFSENTRYCACHE_SETFILEENTRY_EXIT );
    }

CMTPFSEntryCache::CMTPFSEntryCache():iIsOngoing(EFalse), iTargetHandle(KMTPHandleNone)
    {
    }

void CMTPFSEntryCache::ConstructL()
    {    
    OstTraceFunctionEntry0( CMTPFSENTRYCACHE_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPFSENTRYCACHE_CONSTRUCTL_EXIT );
    }
