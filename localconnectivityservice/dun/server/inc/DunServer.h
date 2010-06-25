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


#ifndef C_CDUNSERVER_H
#define C_CDUNSERVER_H

#include <e32base.h>
#include "DunPlugin.h"
#include "DunCloseWait.h"
#include "dunserverdef.h"
#include "dundomainpskeys.h"

_LIT( KDunPluginBt,   "dunbt.dll"  );
_LIT( KDunPluginIrda, "dunir.dll"  );
_LIT( KDunPluginUsb,  "dunusb.dll" );

class MDunSession;
class MDunServerUtility;

/**
 *  Class used to store data needed for each separate plugin
 *  One TDunConnectionData data contains objects needed for one plugin
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunConnectionData )
    {

public:

    /**
     * Pointer to plugin DLL
     * Not own.
     */
    MDunLocalMediaPlugin* iLocalModulePtr;

    /**
     * Current state of plugin
     */
    TDunPluginState iPluginState;

    /**
     * DLL library server needed to open/close existing plugin
     */
    RLibrary iModuleLibrary;

    /**
     * Loaded plugin's UID
     */
    TUid iLocalModuleUid;

    };

/**
 *  Main DUN server's class
 *
 *  @since S60 v3.2
 */
class CDunServer : public CPolicyServer,
                   public MDunServerCallback,
                   public MDunCloseWait
    {

    friend class CDunServerUtils;

public:

    /**
     * Two-phased constructor.
     * @return Instance of self
     */
    static CDunServer* NewL();

    /**
    * Destructor.
    */
    virtual ~CDunServer();

    /**
     * Notification to manage local media On request
     * This comes from client
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to open
     * @return None
     */
    TInt NotifyMediaOpenRequest( TUid aPluginUid );

    /**
     * Notification to manage local media Off request
     * This comes from client
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to close
     * @return None
     */
    TInt NotifyMediaCloseRequest( TUid aPluginUid );

    /**
     * Current active connection
     * (For testing purposes only)
     *
     * @since S60 5.0
     * @return Active connection, NULL otherwise
     */
    TConnId ActiveConnection();

    /**
     * Increases reference count made by sessions
     * This comes from client
     *
     * @since S60 3.2
     * @return None
     */
    void IncRefCount();

    /**
     * Decreases reference count made by sessions
     * This comes from client
     *
     * @since S60 3.2
     * @return None
     */
    void DecRefCount();

// from base class CServer2 (CPolicyServer -> CServer2)

    /**
     * From CServer2 (CPolicyServer derived from CServer2).
     * Creates a server-side client session object.
     *
     * @since S60 3.2
     * @param aVersion Version information supplied by the client
     * @return A pointer to the newly created server-side client session
     */
    CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& ) const;

private:

    CDunServer( TInt aPriority );

    void ConstructL();

    /**
     * Opens local media plugin by UID
     *
     * @since S60 3.2
     * @param aPluginUid UID of local media plugin to open
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also TryClosePlugin()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt OpenMediaByUid( TUid aPluginUid, TBool aDequeue );

    /**
     * Closes local media plugin by UID
     *
     * @since S60 3.2
     * @param aPluginUid UID of local media plugin to close
     * @param aClearQueued ETrue if clear from plugin queue wanted,
     *                     EFalse otherwise
     * @param aClientClose ETrue if close originator is client
     * @param aSelfDelete ETrue if plugin closes itself, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt CloseMediaByUid( TUid aPluginUid,
                          TBool aClearQueued,
                          TBool aClientClose,
                          TBool aSelfDelete );

    /**
     * Reopens plugins from plugin queue
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ReopenQueuedPlugins();

// from base class MDunServerCallback

    /**
     * From MDunServerCallback.
     * Searches for plugin in connection data and returns its state
     *
     * @since S60 3.2
     * @param aPluginUid UID of local media plugin to search for
     * @return State of plugin
     */
    TDunPluginState GetPluginStateByUid( TUid aPluginUid );

    /**
     * From MDunServerCallback.
     * Notification about request to change plugin state up by one level
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a local media plugin
     * @param aPluginUid UID of the local media plugin whose state to change
     * @return KErrNotFound if plugin not found by UID,
               KErrNotSupported if unknown state,
               KErrNone otherwise
     */
    TInt NotifyPluginStateChangeUp( TDunPluginState aPluginState,
                                    TUid aPluginUid );

    /**
     * From MDunServerCallback.
     * Notification about request to change plugin state down by one level
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a local media plugin
     * @param aPluginUid UID of the local media plugin whose state to change
     * @return KErrNotFound if plugin not found by UID,
               KErrNotSupported if unknown state,
               KErrNone otherwise
     */
    TInt NotifyPluginStateChangeDown( TDunPluginState aPluginState,
                                      TUid aPluginUid );

    /**
     * From MDunServerCallback.
     * Notification about plugin restart request
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to restart
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyPluginRestart( TUid aPluginUid );

    /**
     * From MDunServerCallback.
     * Notification about plugin reopen request
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyPluginReopenRequest();

// from base class MDunPluginManager (MDunServerCallback->MDunPluginManager).

    /**
     * From MDunPluginManager (MDunServerCallback->MDunPluginManager).
     * Notification about request to enqueue a plugin to plugin queue
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to enqueue
     * @return KErrNotFound = plugin not found by UID,
               KErrAlreadyExists = plugin already in queue,
               KErrNone otherwise
     */
    TInt NotifyPluginEnqueueRequest( TUid aPluginUid );

    /**
     * From MDunPluginManager (MDunServerCallback->MDunPluginManager).
     * Notification about request to dequeue a plugin from plugin queue
     * Just removes a found UID from the queue
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to dequeue
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyPluginDequeueRequest( TUid aPluginUid );

    /**
     * From MDunPluginManager (MDunServerCallback->MDunPluginManager).
     * Notification about request to close a plugin
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to close
     * @param aSelfClose ETrue if plugin closes itself, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyPluginCloseRequest( TUid aPluginUid,
                                   TBool aSelfClose );

// from base class MDunCloseWait

    /**
     * From MDunCloseWait.
     * Notification about request to close a plugin after wait
     *
     * @since S60 5.0
     * @param aPluginsToClose Plugins to be closed
     * @return Symbian error code on error, KErrNone otherwise
     */
    void NotifyPluginCloseAfterWait(
        RPointerArray<MDunLocalMediaPlugin>& aPluginsToClose );

private:  // data

    /**
     * Pointer to common utility class
     * Own.
     */
    MDunServerUtility* iUtility;

    /**
     * Transporter to use for network side communication
     * Own.
     */
    CDunTransporter* iTransporter;

    /**
     * Waiter for closing self-destruct plugins
     * Own.
     */
    CDunCloseWait* iCloseWait;

    /**
     * Array of TDunConnectionData needed for each loaded plugin
     */
    RArray<TDunConnectionData> iConnData;

    /**
     * Array of plugin UIDs
     * Used for queuing plugins that have no resources to use
     * Used for dequeuing plugins when free resources detected
     */
    RArray<TUid> iPluginQueue;

    /**
     * Array of plugin UIDs
     * Used for queuing plugins that were successfully closed outside
     * this server. This is to avoid returning KErrNotFound to LOCOD
     * when NotifyMediaCloseRequest() is caught
     */
    RArray<TUid> iClosedQueue;

    /**
     * Connection status of dialup: active or inactive
     * Used for updating KPSUidDialupConnStatus Pub&Sub key
     */
    TDialupConnStatus iConnStatus;

    /**
     * Reference count of client sides
     */
    TInt iRefCount;

    };

#endif  // C_CDUNSERVER_H
