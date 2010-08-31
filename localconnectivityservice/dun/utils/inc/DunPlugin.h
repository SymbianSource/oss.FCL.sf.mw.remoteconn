/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Common definitions and classes needed by DUN plugins
*
*/


#ifndef DUN_PLUGIN_H
#define DUN_PLUGIN_H

#include <e32base.h>

typedef TAny* TConnId;  // supported now: RSocket*, RComm*

enum TDunPluginState
    {
    EDunStateNone = KErrNotFound,  // -1
    EDunStateZombie = 0,           //  0
    EDunStateTryUninitialize,      //  1
    EDunStateUninitialized,        //  2
    EDunStateTryLoad,              //  3
    EDunStateLoaded,               //  4
    EDunStateTryListen,            //  5
    EDunStateListening,            //  6
    EDunStateTryChannel,           //  7
    EDunStateChanneled             //  8
    };

const TInt KDunLocalMediaPluginInterfaceUidValue = 0x101F6E2D;
const TUid KDunLocalMediaPluginInterfaceUid      = { KDunLocalMediaPluginInterfaceUidValue };

const TInt KDunBtPluginUidValue   = 0x101F6E2B;
const TUid KDunBtPluginUid        = { KDunBtPluginUidValue };
const TInt KDunIrPluginUidValue   = 0x101FBAEB;
const TUid KDunIrPluginUid        = { KDunIrPluginUidValue };
const TInt KDunUsbPluginUidValue  = 0x101F6E2F;
const TUid KDunUsbPluginUid       = { KDunUsbPluginUidValue };

class CDunTransporter;

/**
 *  Notification interface class to report request for plugin enqueue
 *  This reqeust is made when there are not enough network resources for a
 *  certain plugin's needs
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunPluginManager )
    {

public:

    /**
     * Gets called when old plugin should be enqueued
     *
     * @since S60 3.2
     * @param aPluginUid UID of the plugin that should be enqueued
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginEnqueueRequest( TUid aPluginUid ) = 0;

     /**
     * Gets called when new plugin should be dequeued
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to dequeue
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginDequeueRequest( TUid aPluginUid ) = 0;

    /**
     * Get callen when plugin should be closed
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media plugin to close
     * @param aSelfClose ETrue if plugin closes itself, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginCloseRequest( TUid aPluginUid,
                                           TBool aSelfClose ) = 0;

    };

/**
 *  Interface class for accessing CDunServer's functionality
 *  Shares basic interface for all plugins to use when plugin needs to access
 *  server's functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunServerCallback ) : public MDunPluginManager
    {

public:

    /**
     * Callback from plugins to server to get a plugin's state
     *
     * @since S60 3.2
     * @param aPluginUid Plugin's UID which state to get
     * @return Plugin's state
     */
    virtual TDunPluginState GetPluginStateByUid( TUid aPluginUid ) = 0;

    /**
     * Callback from plugins to server to set a new state
     * New state must be one more than the old state
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a plugin
     * @param aPluginUid Plugin's UID for which to change state
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginStateChangeUp( TDunPluginState aPluginState,
                                            TUid aPluginUid ) = 0;

    /**
     * Callback from plugins to server to set a new state
     * New state must be one less than the old state
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a plugin
     * @param aPluginUid Plugin's UID for which to change state
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginStateChangeDown( TDunPluginState aPluginState,
                                              TUid aPluginUid ) = 0;

    /**
     * Callback from plugins to server to restart a plugin
     * Restarting must be done by uninitialization->listening switch since
     * context is in plugin
     *
     * @since S60 3.2
     * @param aPluginUid Plugin's UID to restart
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginRestart( TUid aPluginUid ) = 0;

    /**
     * Callback from plugins to server to reopen a plugin from queue
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginReopenRequest() = 0;

    };

/**
 *  Interface class for accessing plugins' functionality
 *  Shares basic interface for server to use when server needs to access
 *  plugin's functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunLocalMediaPlugin )
    {

public:

    /**
     * Destructor. Derived class must have virtual destructor also as a plugin
     * module is always destructed via M-class.
     */
    inline virtual ~MDunLocalMediaPlugin() {};

    /**
     * Symbian 2nd phase constructor.
     */
    virtual void ConstructL( MDunServerCallback* aServer,
                             CDunTransporter* aTransporter ) = 0;

    /**
     * Callback from server to plugins to notify when server changes a
     * plugin's state
     *
     * @since S60 3.2
     * @param aPluginState New changed state
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyServerStateChange( TDunPluginState aPluginState ) = 0;

    /**
     * Callback from server to plugins to ask for current active connection
     * (For testing purposes only)
     *
     * @since S60 5.0
     * @return Active connection, NULL otherwise
     */
    virtual TConnId ActiveConnection() = 0;

    };

/**
 *  Interface class for accessing plugin's main class when its listener
 *  reports that channel can be created. Can be used also to notify plugin's
 *  main class when channel can be deleted.
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunListenCallback )
    {

public:

    /**
     * Callback from plugin's listener to plugin to notify about need to
     * allocate new channel
     *
     * @since S60 3.2
     * @param aNoFreeChan ETrue if no free channels, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyChannelAllocate( TBool& aNoFreeChans ) = 0;

    /**
     * Callback from plugin's listener to plugin to notify about need to free
     * an existing channel
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyChannelFree() = 0;

    };

#endif // DUN_PLUGIN_H
