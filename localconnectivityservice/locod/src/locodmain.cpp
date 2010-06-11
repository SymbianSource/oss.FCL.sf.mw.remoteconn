/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Global functions for Local Connectivity Daemon.
*
*/


#include "locodaemon.h"
#include "debug.h"

_LIT( KLocodName, "LocalConnectivityDaemon" );

static void StartDaemonL();

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// E32Main.
// Entry-point for LCD.
// ---------------------------------------------------------------------------
//
TInt E32Main()
    {
    TInt ret;
    User::RenameThread( KLocodName );

    __UHEAP_MARK;
    
    // create clean-up stack
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TRAP( ret, StartDaemonL() );
    delete cleanup; // destroy clean-up stack
    __UHEAP_MARKEND;

    return ret;
    }

// ----------------------------------------------------------------------------
// StartDaemonL().
// Constructs and installs the active scheduler, constructs Daemon object.
// ----------------------------------------------------------------------------
//
static void StartDaemonL()
    {
    TRACE_FUNC
    
    // Construct and install the active scheduler
    CActiveScheduler *myScheduler = new ( ELeave ) CActiveScheduler();

    // Push onto the cleanup stack
    CleanupStack::PushL( myScheduler );

    // Install as the active scheduler
    CActiveScheduler::Install( myScheduler );

    CLocoDaemon* daemon = NULL;
    daemon = CLocoDaemon::NewLC();
    CActiveScheduler::Start();

    CleanupStack::PopAndDestroy( daemon );
    CleanupStack::PopAndDestroy( myScheduler );
    }

