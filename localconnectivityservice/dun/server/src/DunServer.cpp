/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN server
*
*/


#include <e32property.h>
#include "DunServer.h"
#include "DunServerUtils.h"
#include "DunSession.h"
#include "DunDebug.h"
#include "DunServerSecurityPolicy.h"
#include "dundomainpskeys.h"
#include "DunTransporter.h"

// PubSub security settings constants
_LIT_SECURITY_POLICY_PASS( KConnStatusReadPolicy );
_LIT_SECURITY_POLICY_C2( KConnStatusWritePolicy,
                         ECapabilityLocalServices,
                         ECapabilityWriteDeviceData );

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// StartServerL Create Active Scheduler and CDunServer.
// ---------------------------------------------------------------------------
//
static void StartServerL()
    {
    FTRACE(FPrint( _L("CDunServer::StartServerL()") ));
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );

    CDunServer* server = CDunServer::NewL();
    CleanupStack::PushL( server );

    User::RenameThread( KDialupServerName );
    RProcess::Rendezvous( KErrNone );
    FTRACE(FPrint( _L("CDunServer::StartServerL() Initialization done. Server is running.") ));

    CActiveScheduler::Start();  // run the server

    CleanupStack::PopAndDestroy( server );
    CleanupStack::PopAndDestroy( scheduler );
    FTRACE(FPrint( _L("CDunServer::StartServerL() complete") ));
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunServer* CDunServer::NewL()
    {
    CDunServer* self = new (ELeave) CDunServer( EPriorityStandard );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunServer::~CDunServer()
    {
    FTRACE(FPrint(_L("CDunServer::~CDunServer()")));
    // Destroy iConnData
    TInt i;
    for ( i=iConnData.Count()-1; i>=0; i-- )
        {
        if ( iUtility )
            {
            iUtility->TryClosePlugin( i, EFalse, EFalse, EFalse );
            }
        }
    if ( iConnData.Count() == 0 )
        {
        iConnData.Close();
        }
    else
        {
        FTRACE(FPrint(_L("CDunServer::~CDunServer() (ERROR) complete")));
        }
    // Destroy iPluginQueue
    iPluginQueue.Close();
    // Destroy iClosedQueue
    iClosedQueue.Close();
    // Destroy transporter
    delete iTransporter;
    iTransporter = NULL;
    // Destroy utility
    CDunServerUtils* utility = static_cast<CDunServerUtils*>( iUtility );
    delete utility;
    iUtility = NULL;
    // Destroy close waiter
    delete iCloseWait;
    iCloseWait = NULL;
    // Remove key
    RProperty::Delete( KPSUidDialupConnStatus, KDialupConnStatus );
    FTRACE(FPrint(_L("CDunServer::~CDunServer() complete")));
    }

// ---------------------------------------------------------------------------
// Notification to manage local media On request
// This comes from client
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyMediaOpenRequest( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyMediaOpenRequest()")));
    iUtility->ClearQueuedUIDs( aPluginUid, ETrue, ETrue );
    iUtility->RemoveZombiePlugins();
    TInt retVal = OpenMediaByUid( aPluginUid, ETrue );
    FTRACE(FPrint(_L("CDunServer::NotifyMediaOpenRequest() complete (%d)"), retVal));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Notification to manage local media Off request
// This comes from client
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyMediaCloseRequest( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyMediaCloseRequest()")));
    // If plugin can be found with iEnqueue or iClosed marked ETrue,
    // Close also them in the same way (no special processing needed)
    TInt retVal = CloseMediaByUid( aPluginUid, ETrue, ETrue, ETrue );
    FTRACE(FPrint(_L("CDunServer::NotifyMediaCloseRequest() complete (%d)"), retVal));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Current active connection
// This comes from client
// ---------------------------------------------------------------------------
//
TConnId CDunServer::ActiveConnection()
    {
    FTRACE(FPrint(_L("CDunServer::ActiveConnection()")));
    if ( iConnData.Count()<1 || iConnData.Count()>1 )
        {
        FTRACE(FPrint(_L("CDunServer::ActiveConnection() (unknown plugin amount) complete")));
        return NULL;
        }
    TDunConnectionData& plugin = iConnData[0];
    if ( !plugin.iLocalModulePtr )
        {
        FTRACE(FPrint(_L("CDunServer::ActiveConnection() (no instance) complete")));
        return NULL;
        }
    TConnId connId = plugin.iLocalModulePtr->ActiveConnection();
    FTRACE(FPrint(_L("CDunServer::ActiveConnection() complete")));
    return connId;
    }

// ---------------------------------------------------------------------------
// Increases reference count made by sessions
// This comes from client
// ---------------------------------------------------------------------------
//
void CDunServer::IncRefCount()
    {
    FTRACE(FPrint(_L("CDunServer::IncRefCount()")));
    iRefCount++;
    FTRACE(FPrint(_L("CDunServer::IncRefCount() complete")));
    }

// ---------------------------------------------------------------------------
// Decreases reference count made by sessions
// This comes from client
// ---------------------------------------------------------------------------
//
void CDunServer::DecRefCount()
    {
    FTRACE(FPrint(_L("CDunServer::DecRefCount()")));
    if ( iRefCount > 0 )
        {
        iRefCount--;
        FTRACE(FPrint(_L("CDunServer::DecRefCount() reference count decreased")));
        if ( iRefCount == 0 )
            {
            FTRACE(FPrint(_L("CDunServer::DecRefCount() stopping activescheduler...")));
            CActiveScheduler::Stop();
            }
        }
    FTRACE(FPrint(_L("CDunServer::DecRefCount() complete")));
    }

// ---------------------------------------------------------------------------
// From class CServer2 (CPolicyServer -> CServer2).
// Creates a server-side client session object.
// ---------------------------------------------------------------------------
//
CSession2* CDunServer::NewSessionL( const TVersion& aVersion,
                                    const RMessage2& /*aMessage*/ ) const
    {
    FTRACE(FPrint(_L("CDunServer::NewSessionL()")));
    TVersion version(KDunServerMajorVersionNumber,
                     KDunServerMinorVersionNumber,
                     KDunServerBuildVersionNumber );
    TBool supported = User::QueryVersionSupported( version, aVersion );
    if ( !supported )
        {
        FTRACE(FPrint(_L("CDunServer::NewSessionL() (not supported) complete")));
        User::Leave( KErrNotSupported );
        }
    CDunSession* session = new (ELeave) CDunSession(const_cast<CDunServer*>( this ));
    FTRACE(FPrint(_L("CDunServer::NewSessionL() complete")));
    return session;
    }

// ---------------------------------------------------------------------------
// CDunServer::CDunServer
// ---------------------------------------------------------------------------
//
CDunServer::CDunServer( TInt aPriority ) :
    // aPriority: priority of AO
    // KDunServerPolicy: TPolicy structure of DUN
    CPolicyServer( aPriority, KDunServerPolicy ),
    iTransporter( NULL ),
    iConnStatus( EDialupUndefined ),
    iRefCount( 0 )
    {
    FTRACE( FPrint( _L( "CDunServer::CDunServer()" )));
    }

// ---------------------------------------------------------------------------
// CDunServer::ConstructL
// ---------------------------------------------------------------------------
//
void CDunServer::ConstructL()
    {
    FTRACE( FPrint( _L( "CDunServer::ConstructL()" )));
    StartL( KDialupServerName );
    // Initialize pubsub key properties
    User::LeaveIfError(
        RProperty::Define( KPSUidDialupConnStatus,
                           KDialupConnStatus,
                           RProperty::EInt,
                           KConnStatusReadPolicy,
                           KConnStatusWritePolicy) );
    // Set key to initial value
    RProperty::Set( KPSUidDialupConnStatus,
                    KDialupConnStatus,
                    EDialupInactive );
    // Create waiter for closing self-destruct plugins
    CDunCloseWait* closeWait = CDunCloseWait::NewL( this );
    CleanupStack::PushL( closeWait );
    // Create transporter
    CDunTransporter* transporter = CDunTransporter::NewL( this );
    CleanupStack::PushL( transporter );
    // Create utility
    CDunServerUtils* utility = CDunServerUtils::NewL( *this );
    iUtility = static_cast<MDunServerUtility*>( utility );
    iTransporter = transporter;
    iCloseWait = closeWait;
    CleanupStack::Pop( transporter );
    CleanupStack::Pop( closeWait );
    FTRACE( FPrint( _L( "CDunServer::ConstructL() complete")));
    }

// ---------------------------------------------------------------------------
// Opens local media plugin by UID
// ---------------------------------------------------------------------------
//
TInt CDunServer::OpenMediaByUid( TUid aPluginUid, TBool aDequeue )
    {
    FTRACE(FPrint(_L("CDunServer::OpenMediaByUid()")));
    TInt i;
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() (already exists) complete")));
            return KErrAlreadyExists;
            }
        }
    FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() creating new plugin 0x%08X"), aPluginUid.iUid));
    TInt retTemp = iUtility->CreateNewPlugin( aPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() (ERROR) complete")));
        return retTemp;
        }
    TInt pluginIndex = iConnData.Count() - 1;
    FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() trying listen at index %d"), pluginIndex));
    retTemp = iUtility->TryListening( pluginIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() trying close at index %d"), pluginIndex));
        iUtility->TryClosePlugin( pluginIndex, aDequeue, EFalse, EFalse );
        // Ignore error(s), TryClosePlugin()'s TryUninitialize already sets to Zombie if error(s)
        FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() (ERROR) complete")));
        return retTemp;
        }
    // Clear queued also from queued plugin UIDs, just to be safe
    iUtility->ClearQueuedUIDs( aPluginUid, ETrue, ETrue );
    FTRACE(FPrint(_L("CDunServer::OpenMediaByUid() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Closes local media plugin by UID
// ---------------------------------------------------------------------------
//
TInt CDunServer::CloseMediaByUid( TUid aPluginUid,
                                  TBool aClearQueued,
                                  TBool aClientClose,
                                  TBool aSelfDelete )
    {
    FTRACE(FPrint(_L("CDunServer::CloseMediaByUid()")));
    TInt i;
    TBool cleared = EFalse;
    if ( aClearQueued )
        {
        cleared = iUtility->ClearQueuedUIDs( aPluginUid, ETrue, ETrue );
        }
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L("CDunServer::CloseMediaByUid() (not found) complete")));
        return KErrNotFound;
        }
    FTRACE(FPrint(_L("CDunServer::CloseMediaByUid() trying close at index %d"), i));
    // Plugin found from main list, now close
    TInt retTemp = iUtility->TryClosePlugin( i,
                                             ETrue,
                                             aClientClose,
                                             aSelfDelete );
    if ( retTemp!=KErrNone && !cleared )
        {
        FTRACE(FPrint(_L("CDunServer::CloseMediaByUid() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServer::CloseMediaByUid() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Reopens plugins from plugin queue
// ---------------------------------------------------------------------------
//
TInt CDunServer::ReopenQueuedPlugins()
    {
    FTRACE(FPrint(_L("CDunServer::ReopenQueuedPlugins() (%d)"), iPluginQueue.Count()));
    TInt i = iPluginQueue.Count() - 1;
    while ( i >= 0 )
        {
        TInt retTemp = OpenMediaByUid( iPluginQueue[i], EFalse );
        if ( retTemp == KErrNone )
            {
            // Open succeeded so plugin UID was removed by ClearQueuedUIDs().
            // Recalculate i because this method can remove more than one
            // match.
            i = iPluginQueue.Count() - 1;
            }
        else
            {
            // Open failed so the plugin UID was not removed from queue.
            // Just decrease index (avoid forever loop).
            i--;
            }
        }
    FTRACE(FPrint(_L("CDunServer::ReopenQueuedPlugin()")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerCallback.
// Searches for plugin in connection data and returns its state
// ---------------------------------------------------------------------------
//
TDunPluginState CDunServer::GetPluginStateByUid( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::GetPluginStateByUid()")));
    TInt i;
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L("CDunServer::GetPluginStateByUid() (not found) complete")));
        return EDunStateNone;
        }
    FTRACE(FPrint(_L("CDunServer::GetPluginStateByUid() state at index %d is %d"), i, iConnData[i].iPluginState));
    FTRACE(FPrint(_L("CDunServer::GetPluginStateByUid() complete")));
    return iConnData[i].iPluginState;
    }

// ---------------------------------------------------------------------------
// From class MDunServerCallback.
// Notification about request to change plugin state up by one level
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginStateChangeUp( TDunPluginState aPluginState,
                                            TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp()")));
    TInt i;
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp() (not found) complete")));
        return KErrNotFound;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp() found at index %d"), i));
    TDunPluginState pluginState = iConnData[i].iPluginState;
    if ( pluginState<EDunStateTryUninitialize || aPluginState-pluginState!=1 )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp() (unknown state) complete (%d/%d)"), pluginState, aPluginState));
        return KErrNotSupported;
        }
    iConnData[i].iPluginState = aPluginState;
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp() state changed to %d(%d)"), aPluginState, pluginState));
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeUp() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerCallback.
// Notification about request to change plugin state down by one level
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginStateChangeDown( TDunPluginState aPluginState,
                                              TUid aPluginUid)
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown()")));
    TInt i;
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() (not found) complete")));
        return KErrNotFound;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() found at index %d"), i));
    TDunPluginState pluginState = iConnData[i].iPluginState;
    if ( aPluginState >= pluginState )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() (unknown state) complete (%d/%d)"), pluginState, aPluginState));
        return KErrNotSupported;
        }
    // Next catch the cases where state is not "try"
    if ( aPluginState != EDunStateTryUninitialize &&
         aPluginState != EDunStateTryLoad         &&
         aPluginState != EDunStateTryListen       &&
         aPluginState != EDunStateTryChannel )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() (unknown state) complete (%d/%d)"), pluginState, aPluginState));
        return KErrNotSupported;
        }
    iConnData[i].iPluginState = aPluginState;
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() state changed to %d(%d)"), aPluginState, pluginState));
    FTRACE(FPrint(_L("CDunServer::NotifyPluginStateChangeDown() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerCallback.
// Notification about plugin restart request
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginRestart( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginRestart()")));
    TInt retTemp = iUtility->TryInitializeToListening( aPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginRestart() (ERROR) initialize failed")));
        return retTemp;
        }
    // Restarting may have freed channel(s),
    // try to reopen queued plugins (ignore error(s))
    ReopenQueuedPlugins();
    FTRACE(FPrint(_L("CDunServer::NotifyPluginRestart() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerCallback.
// Notification about plugin restart request
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginReopenRequest()
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginReopenRequest()")));
    TInt retVal = ReopenQueuedPlugins();
    FTRACE(FPrint(_L("CDunServer::NotifyPluginReopenRequest() complete")));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunPluginManager (MDunServerCallback->MDunPluginManager).
// Notification about request to enqueue a plugin to plugin queue
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginEnqueueRequest( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest()")));
    TInt i;
    TInt count = iConnData.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iConnData[i].iLocalModuleUid == aPluginUid )
            {
            break;
            }
        }
    if ( i >= count )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() (not found) complete")));
        return KErrNotFound;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() found at index %d"), i));
    // Plugin exists, now try to find it from plugin queue
    count = iPluginQueue.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iPluginQueue[i] == aPluginUid )
            {
            FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() (already exists) complete")));
            return KErrAlreadyExists;
            }
        }
    // New plugin found, queue
    TInt retTemp = CloseMediaByUid( aPluginUid, EFalse, EFalse, ETrue );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() (close failed) complete")));
        return KErrGeneral;
        }
    retTemp = iPluginQueue.Append( aPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() (append failed!) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() appended to index %d"), iPluginQueue.Count()-1));
    FTRACE(FPrint(_L("CDunServer::NotifyPluginEnqueueRequest() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunPluginManager (MDunServerCallback->MDunPluginManager).
// Notification about request to dequeue a plugin from plugin queue
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginDequeueRequest( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginDequeueRequest()")));
    TBool cleared = iUtility->ClearQueuedUIDs( aPluginUid, ETrue, ETrue );
    if ( !cleared )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginDequeueRequest() (not found) complete")));
        return KErrNotFound;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginDequeueRequest() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunPluginManager (MDunServerCallback->MDunPluginManager).
// Notification about request to close a plugin
// ---------------------------------------------------------------------------
//
TInt CDunServer::NotifyPluginCloseRequest( TUid aPluginUid, TBool aSelfClose )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginCloseRequest()")));
    TInt retTemp = CloseMediaByUid( aPluginUid, ETrue, EFalse, aSelfClose );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServer::NotifyPluginCloseRequest() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginCloseRequest() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From MDunCloseWait.
// Notification about request to close a plugin after wait
// ---------------------------------------------------------------------------
//
void CDunServer::NotifyPluginCloseAfterWait(
    RPointerArray<MDunLocalMediaPlugin>& aPluginsToClose )
    {
    FTRACE(FPrint(_L("CDunServer::NotifyPluginCloseAfterWait()")));
    TInt i;
    TInt j;
    TInt closeCount = aPluginsToClose.Count();
    for ( i=0; i<closeCount; i++ )
        {
        // keep iConnData.Count() on the line below as it changes!
        for ( j=0; j<iConnData.Count(); j++ )
            {
            if ( iConnData[j].iLocalModulePtr == aPluginsToClose[i] )
                {
                iUtility->DoClosePlugin( j, ETrue, EFalse );
                }
            }
        }
    FTRACE(FPrint(_L("CDunServer::NotifyPluginCloseAfterWait() complete")));
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// WinsMain/E32Main implements main function for DUN
// exe.
// Create Cleanup Stack, initialise server and start Active
// Scheduler.
// Returns: Error value: General error value
// ---------------------------------------------------------------------------
//
TInt E32Main()
    {
    FTRACE(FPrint( _L("CDunServer::Main() Process started.") ));
__UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt retVal = KErrNoMemory;

    if ( cleanup )
        {
        TRAP( retVal, StartServerL() ); // perform all other initialisation
        delete cleanup;
        }
__UHEAP_MARKEND;

    FTRACE(FPrint( _L("CDunServer::Main() Process exit code = %d"), retVal));
    return retVal;
    }
