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
* Description:  Utility class for CDunServer
*
*/


#include <e32uid.h>
#include "DunServer.h"
#include "DunServerUtils.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunServerUtils* CDunServerUtils::NewL( CDunServer& aParent )
    {
    CDunServerUtils* self = new (ELeave) CDunServerUtils( aParent );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunServerUtils::~CDunServerUtils()
    {
    FTRACE(FPrint(_L("CDunServerUtils::~CDunServerUtils()")));
    FTRACE(FPrint(_L("CDunServerUtils::~CDunServerUtils() complete")));
    }

// ---------------------------------------------------------------------------
// CDunServerUtils::CDunServerUtils
// ---------------------------------------------------------------------------
//
CDunServerUtils::CDunServerUtils( CDunServer& aParent ) :
    iParent( aParent ),
    iTransporter( aParent.iTransporter ),
    iCloseWait( aParent.iCloseWait ),
    iConnData( aParent.iConnData ),
    iPluginQueue( aParent.iPluginQueue ),
    iClosedQueue( aParent.iClosedQueue )
    {
    }

// ---------------------------------------------------------------------------
// CDunServerUtils::ConstructL
// ---------------------------------------------------------------------------
//
void CDunServerUtils::ConstructL()
    {
    FTRACE(FPrint( _L("CDunServerUtils::ConstructL()" ) ));
    FTRACE(FPrint( _L("CDunServerUtils::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Closes plugins with state marked as zombie
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::RemoveZombiePlugins()
    {
    FTRACE(FPrint(_L("CDunServerUtils::RemoveZombiePlugins()")));
    TInt i;
    TInt retVal = KErrNone;
    for ( i=iConnData.Count()-1; i>=0; i-- )
        {
        if ( iConnData[i].iPluginState == EDunStateZombie )
            {
            // Following closes and removes if remove ok
            FTRACE(FPrint(_L("CDunServerUtils::RemoveZombiePlugins() trying close at index %d"), i));
            TInt retTemp = TryClosePlugin( i, ETrue, EFalse, EFalse );
            if ( retTemp != KErrNone )
                {
                retVal = KErrGeneral;
                }
            }
        }
    FTRACE(FPrint(_L("CDunServerUtils::RemoveZombiePlugins() complete")));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Loads local media module
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::CreateNewPlugin( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServerUtils::CreateNewPlugin()")));
    // plugin not constructed, construct now
    TFileName pluginFile;
    switch ( aPluginUid.iUid )
        {
        case KDunBtPluginUidValue:
            pluginFile.Copy( KDunPluginBt );
            break;
        case KDunIrPluginUidValue:
            pluginFile.Copy( KDunPluginIrda );
            break;
        case KDunUsbPluginUidValue:
            pluginFile.Copy( KDunPluginUsb );
            break;
        default:
            FTRACE(FPrint(_L("CDunServerUtils::CreateNewPlugin() (not supported) complete")));
            return KErrNotSupported;
        }
    TDunConnectionData emptyConn;
    emptyConn.iLocalModulePtr = NULL;
    emptyConn.iLocalModuleUid = TUid::Null();
    emptyConn.iPluginState = EDunStateNone;
    TInt retTemp = iConnData.Append( emptyConn );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServerUtils::CreateNewPlugin() (append failed!) complete")));
        return retTemp;
        }
    retTemp = ConstructLocalMediaModule( aPluginUid, pluginFile );
    if ( retTemp != KErrNone )
        {
        iConnData.Remove( iConnData.Count()-1 );
        FTRACE(FPrint(_L("CDunServerUtils::CreateNewPlugin() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServerUtils::CreateNewPlugin() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Constructs local media module
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::ConstructLocalMediaModule( const TUid& aPluginUid,
                                                 const TPtrC& aDllName )
    {
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule()")));
    // Create a new library object
    if ( iConnData.Count() == 0 )
        {
        FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() (not ready) complete")));
        return KErrNotReady;
        }
    TInt index = iConnData.Count() - 1;
    iConnData[index].iLocalModuleUid = aPluginUid;
    iConnData[index].iPluginState = EDunStateTryLoad;
    // Load the DLL containing the plug-in
    TUidType uidType( KDynamicLibraryUid,
                      KDunLocalMediaPluginInterfaceUid,
                      aPluginUid );
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() loading")));
    TInt retTemp = iConnData[index].iModuleLibrary.Load( aDllName, uidType );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() plugin 0x%08X loaded"), aPluginUid.iUid));
    // Create the plugin object.
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() looking up")));
    TLibraryFunction factoryFunction = iConnData[index].iModuleLibrary.Lookup( 1 );
    if ( !factoryFunction )
        {
        iConnData[index].iModuleLibrary.Close();
        FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() (ERROR) complete")));
        return KErrNotFound;
        }
    MDunLocalMediaPlugin* localPlugin =
        reinterpret_cast<MDunLocalMediaPlugin*>( factoryFunction() );
    if ( !localPlugin )
        {
        iConnData[index].iModuleLibrary.Close();
        FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() (ERROR) complete")));
        return KErrGeneral;
        }
    iConnData[index].iLocalModulePtr = localPlugin;
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() constructing")));
    TRAPD( retTrap, localPlugin->ConstructL(&iParent, iTransporter) );
    if ( retTrap != KErrNone )
        {
        delete iConnData[index].iLocalModulePtr;
        iConnData[index].iLocalModulePtr = NULL;
        iConnData[index].iModuleLibrary.Close();
        FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() (ERROR) complete")));
        return retTrap;
        }
    iConnData[index].iPluginState = EDunStateLoaded;
    FTRACE(FPrint(_L("CDunServerUtils::ConstructLocalMediaModule() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Clears queued UIDs
// ---------------------------------------------------------------------------
//
TBool CDunServerUtils::ClearQueuedUIDs( TUid aPluginUid,
                                        TBool aClearClosed,
                                        TBool aClearQueued )
    {
    FTRACE(FPrint(_L("CDunServerUtils::ClearQueuedUIDs()")));
    TInt i;
    TBool cleared = EFalse;
    if ( aClearClosed )
        {
        for ( i=iClosedQueue.Count()-1; i>=0; i-- )
            {
            if ( iClosedQueue[i] == aPluginUid )
                {
                iClosedQueue.Remove( i );
                cleared = ETrue;
                FTRACE(FPrint(_L("CDunServerUtils::ClearQueuedUIDs() removed from closed queue at %d"), i));
                }
            }
        }
    if ( aClearQueued )
        {
        for ( i=iPluginQueue.Count()-1; i>=0; i-- )
            {
            if ( iPluginQueue[i] == aPluginUid )
                {
                iPluginQueue.Remove( i );
                cleared = ETrue;
                FTRACE(FPrint(_L("CDunServerUtils::ClearQueuedUIDs() removed from plugin queue at %d"), i));
                }
            }
        }
    FTRACE(FPrint(_L("CDunServerUtils::ClearQueuedUIDs() complete")));
    return cleared;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Tries to close loaded local media plugin
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::TryClosePlugin( TInt aIndex,
                                      TBool aDequeue,
                                      TBool aClientClose,
                                      TBool aSelfClose )
    {
    FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin()")));
    if ( aIndex < 0 ||
         aIndex >= iConnData.Count() )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() (not found) complete")));
        return KErrNotFound;
        }
    TInt retTemp = TryUninitialize( aIndex );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() (ERROR) complete")));
        return retTemp;
        }
    if ( !aSelfClose )
        {
        retTemp = DoClosePlugin( aIndex, aDequeue, aClientClose );
        FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() complete")));
        return retTemp;
        }
    retTemp = iCloseWait->AddPluginToClose( iConnData[aIndex].iLocalModulePtr );
    if ( retTemp != KErrNone )
        {
        iConnData[aIndex].iPluginState = EDunStateZombie;
        FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() state changed to %d"), EDunStateZombie));
        return retTemp;
        }
    retTemp = iCloseWait->IssueRequest();
    if ( retTemp != KErrNone )
        {
        iConnData[aIndex].iPluginState = EDunStateZombie;
        FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() state changed to %d"), EDunStateZombie));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryClosePlugin() (waiting) complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Closes a plugin directly without uninitializing it
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::DoClosePlugin(
    TInt aIndex,
    TBool aDequeue,
    TBool aClientClose )
    {
    FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin()")));
    if ( aIndex < 0 ||
         aIndex >= iConnData.Count() )
        {
        FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin() (not found) complete")));
        return KErrNotFound;
        }
    TUid pluginUid = iConnData[aIndex].iLocalModuleUid;
    DoImmediatePluginClose( aIndex, aDequeue );
    if ( !aClientClose )
        {
        // Plugin was closed by something else than client
        // Enqueue it to closed queue
        TInt i;
        TInt count = iClosedQueue.Count();
        for ( i=0; i<count; i++ )
            {
            if ( iClosedQueue[i] == pluginUid )
                {
                FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin() (already exists) complete")));
                return KErrAlreadyExists;
                }
            }
        TInt retTemp = iClosedQueue.Append( pluginUid );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin() (append failed!) complete")));
            return retTemp;
            }
        FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin() appended to index %d"), iClosedQueue.Count()-1));
        }
    FTRACE(FPrint(_L("CDunServerUtils::DoClosePlugin() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Does immediate close of plugin
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::DoImmediatePluginClose( TInt aIndex, TBool aDequeue )
    {
    FTRACE(FPrint(_L("CDunServerUtils::DoImmediatePluginClose()")));
    if ( aIndex < 0 ||
         aIndex >= iConnData.Count() )
        {
        FTRACE(FPrint(_L("CDunServerUtils::DoImmediatePluginClose() (not found) complete")));
        return KErrNotFound;
        }
    delete iConnData[aIndex].iLocalModulePtr;
    iConnData[aIndex].iLocalModulePtr = NULL;
    if ( iConnData[aIndex].iModuleLibrary.Handle() != KNullHandle )
        {
        iConnData[aIndex].iModuleLibrary.Close();
        }
    iConnData.Remove( aIndex );
    // Now, the following check is needed to avoid recursion by:
    // ReopenQueuedPlugins()->OpenMediaByUid()->TryClosePlugin()->
    // DoImmediatePluginClose()->ReopenQueuedPlugins()
    if ( aDequeue )
        {
        iParent.ReopenQueuedPlugins();
        }
    // Ignore error(s); this function must only report it's own operation status
    FTRACE(FPrint(_L("CDunServerUtils::DoImmediatePluginClose() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Tries uninitialization and after that listening state switch on a plugin
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::TryInitializeToListening( TUid aPluginUid )
    {
    FTRACE(FPrint(_L("CDunServerUtils::TryInitializeToListening()")));
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
        FTRACE(FPrint(_L("CDunServerUtils::TryInitializeToListening() (not found) complete")));
        return KErrNotFound;
        }
    TInt retTemp = TryUninitialize( i );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryInitializeToListening() (uninitialize error) complete")));
        return retTemp;
        }
    // Change from Uninitialized to Loaded because listening mode needs it
    // (plugin is already loaded anyway)
    iConnData[i].iPluginState = EDunStateLoaded;
    retTemp = TryListening( i );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryInitializeToListening() (listening error) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryInitializeToListening() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Tries listening state switch on a plugin
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::TryListening( TInt aIndex )
    {
    FTRACE(FPrint(_L("CDunServerUtils::TryListening()")));
    if ( aIndex < 0 ||
         aIndex >= iConnData.Count() )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryListening() (not found) complete")));
        return KErrNotFound;
        }
    if ( iConnData[aIndex].iPluginState != EDunStateLoaded )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryListening() (not ready) complete")));
        return KErrNotReady;
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryListening() notifying server state change (%d)"), EDunStateTryListen));
    TInt retTemp =
        iConnData[aIndex].iLocalModulePtr->NotifyServerStateChange( EDunStateTryListen );
    if ( retTemp != KErrNone )
        {
        iConnData[aIndex].iPluginState = EDunStateZombie;
        FTRACE(FPrint(_L("CDunServerUtils::TryListening() state changed to %d"), EDunStateZombie));
        FTRACE(FPrint(_L("CDunServerUtils::TryListening() (ERROR) complete")));
        return retTemp;
        }
    // Plugin could have changed state, only change state if possible
    // This can happen if plugin has no real listening and switches directly
    // from listening mode to channeled mode
    if ( iConnData[aIndex].iPluginState == EDunStateTryListen )
        {
        iConnData[aIndex].iPluginState = EDunStateListening;
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryListening() state changed to %d"), EDunStateListening));
    FTRACE(FPrint(_L("CDunServerUtils::TryListening() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunServerUtility.
// Tries uninitialization of a plugin
// ---------------------------------------------------------------------------
//
TInt CDunServerUtils::TryUninitialize( TInt aIndex )
    {
    FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize()")));
    if ( aIndex < 0 ||
         aIndex >= iConnData.Count() )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() (not found) complete")));
        return KErrNotFound;
        }
    if ( iConnData[aIndex].iPluginState == EDunStateUninitialized )
        {
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() (already uninitialized) complete")));
        return KErrNotReady;
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() notifying server state change (%d)"), EDunStateTryUninitialize));
    TInt retTemp =
        iConnData[aIndex].iLocalModulePtr->NotifyServerStateChange( EDunStateTryUninitialize );
    if ( retTemp != KErrNone )
        {
        iConnData[aIndex].iPluginState = EDunStateZombie;
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() state changed to %d"), EDunStateZombie));
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() (ERROR) complete")));
        return retTemp;
        }
    // Plugin state must be EDunStateLoaded after uninitialization
    if ( iConnData[aIndex].iPluginState == EDunStateLoaded )
        {
        iConnData[aIndex].iPluginState = EDunStateUninitialized;
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() state changed to %d"), EDunStateUninitialized));
        }
    else
        {
        // Should never come here
        iConnData[aIndex].iPluginState = EDunStateZombie;
        FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() state changed to %d"), EDunStateZombie));
        }
    FTRACE(FPrint(_L("CDunServerUtils::TryUninitialize() complete")));
    return KErrNone;
    }
