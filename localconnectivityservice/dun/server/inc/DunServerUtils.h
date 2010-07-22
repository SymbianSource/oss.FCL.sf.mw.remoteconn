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


#ifndef C_CDUNSERVERUTILS_H
#define C_CDUNSERVERUTILS_H

#include <e32base.h>
#include "DunPlugin.h"

class CDunServer;

/**
 *  Utility accessor class for CDunServer class
 *  Basically a collection of miscellaneous helper methods
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunServerUtility )
    {

public:

    /**
     * From MDunServerUtility.
     * Closes plugins with state marked as zombie
     *
     * @since S60 3.2
     * @return KErrGeneral = close on any of the plugins failed,
               KErrNone otherwise
     */
    virtual TInt RemoveZombiePlugins() = 0;

    /**
     * From MDunServerUtility.
     * Loads local media module
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media module to load
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt CreateNewPlugin( TUid aPluginUid ) = 0;

    /**
     * From MDunServerUtility.
     * Constructs local media module
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media module to construct
     * @param aDllName Name of the DLL to the constructed local media module
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt ConstructLocalMediaModule( const TUid& aPluginUid,
                                            const TPtrC& aDllName ) = 0;

    /**
     * From MDunServerUtility.
     * Clears queued UIDs
     *
     * @since S60 3.2
     * @param aPluginUid UID of local media plugin to remove
     * @param aClearClosed ETrue if clear from closed queued wanted
     * @param aClearQueued ETrue if clear from plugin queue wanted
     * @return ETrue if plugin UID was removed, EFalse otherwise
     */
    virtual TBool ClearQueuedUIDs( TUid aPluginUid,
                                   TBool aClearClosed,
                                   TBool aClearQueued ) = 0;

    /**
     * From MDunServerUtility.
     * Tries to close loaded local media plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @param aClientClose ETrue if close originator is client
     * @param aSelfClose ETrue if plugin closes itself, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt TryClosePlugin( TInt aIndex,
                                 TBool aDequeue,
                                 TBool aClientClose,
                                 TBool aSelfClose ) = 0;

    /**
     * From MDunServerUtility.
     * Closes a plugin directly without uninitializing it
     *
     * @since S60 5.0
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @param aClientClose ETrue if close originator is client
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoClosePlugin( TInt aIndex,
                                TBool aDequeue,
                                TBool aClientClose ) = 0;

    /**
     * From MDunServerUtility.
     * Does immediate close of plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt DoImmediatePluginClose( TInt aIndex,
                                         TBool aDequeue ) = 0;

    /**
     * From MDunServerUtility.
     * Tries uninitialization and after that listening state switch on a plugin
     *
     * @since S60 3.2
     * @param aPluginUid UID of the plugin to initialize to listening state
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt TryInitializeToListening( TUid aPluginUid ) = 0;

    /**
     * From MDunServerUtility.
     * Tries listening state switch on a plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @return KErrNotFound = plugin not found,
               KErrNotReady = wrong state,
               KErrNone otherwise
     */
    virtual TInt TryListening( TInt aIndex ) = 0;

    /**
     * From MDunServerUtility.
     * Tries uninitialization of a plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @return KErrNotFound = plugin not found, KErrNone otherwise
     */
    virtual TInt TryUninitialize( TInt aIndex ) = 0;

    };

/**
 *  DUN server's utility class
 *
 *  @since S60 v3.2
 */
class CDunServerUtils : public CBase,
                        public MDunServerUtility
    {

public:

    /**
     * Two-phased constructor.
     * @param aParent Parent class owning this friend class (CDunServer)
     * @return Instance of self
     */
    static CDunServerUtils* NewL( CDunServer& aParent );

    /**
    * Destructor.
    */
    virtual ~CDunServerUtils();

private:

    CDunServerUtils( CDunServer& aParent );

    void ConstructL();

// from base class MDunServerUtility

    /**
     * From MDunServerUtility.
     * Closes plugins with state marked as zombie
     *
     * @since S60 3.2
     * @return KErrGeneral = close on any of the plugins failed,
               KErrNone otherwise
     */
    TInt RemoveZombiePlugins();

    /**
     * From MDunServerUtility.
     * Loads local media module
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media module to load
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt CreateNewPlugin( TUid aPluginUid );

    /**
     * From MDunServerUtility.
     * Constructs local media module
     *
     * @since S60 3.2
     * @param aPluginUid UID of the local media module to construct
     * @param aDllName Name of the DLL to the constructed local media module
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ConstructLocalMediaModule( const TUid& aPluginUid,
                                    const TPtrC& aDllName );

    /**
     * From MDunServerUtility.
     * Clears queued UIDs
     *
     * @since S60 3.2
     * @param aPluginUid UID of local media plugin to remove
     * @param aClearClosed ETrue if clear from closed queued wanted
     * @param aClearQueued ETrue if clear from plugin queue wanted
     * @return ETrue if plugin UID was removed, EFalse otherwise
     */
    TBool ClearQueuedUIDs( TUid aPluginUid,
                           TBool aClearClosed,
                           TBool aClearQueued );

    /**
     * From MDunServerUtility.
     * Tries to close loaded local media plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @param aClientClose ETrue if close originator is client
     * @param aSelfClose ETrue if plugin closes itself, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt TryClosePlugin( TInt aIndex,
                         TBool aDequeue,
                         TBool aClientClose,
                         TBool aSelfClose );

    /**
     * From MDunServerUtility.
     * Closes a plugin directly without uninitializing it
     *
     * @since S60 5.0
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @param aClientClose ETrue if close originator is client
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoClosePlugin( TInt aIndex,
                        TBool aDequeue,
                        TBool aClientClose );

    /**
     * From MDunServerUtility.
     * Does immediate close of plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @param aDequeue ETrue if restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (normal case, see also OpenMediaByUid()),
     *                 EFalse if no restarting of queued plugins and plugin
     *                 dequeue wanted after successful plugin close
     *                 (in this case caller is reopener itself, see also
     *                 ReopenQeueuedPlugins())
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoImmediatePluginClose( TInt aIndex,
                                 TBool aDequeue );

    /**
     * From MDunServerUtility.
     * Tries uninitialization and after that listening state switch on a plugin
     *
     * @since S60 3.2
     * @param aPluginUid UID of the plugin to initialize to listening state
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt TryInitializeToListening( TUid aPluginUid );

    /**
     * From MDunServerUtility.
     * Tries listening state switch on a plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @return KErrNotFound = plugin not found,
               KErrNotReady = wrong state,
               KErrNone otherwise
     */
    TInt TryListening( TInt aIndex );

    /**
     * From MDunServerUtility.
     * Tries uninitialization of a plugin
     *
     * @since S60 3.2
     * @param aIndex Index of plugin in connectiondata
     * @return KErrNotFound = plugin not found, KErrNone otherwise
     */
    TInt TryUninitialize( TInt aIndex );

private:  // data

    /**
     * Parent class owning this friend class (CDunServer)
     */
    CDunServer& iParent;

    /**
     * Transporter to use for network side communication
     * (from parent iParent)
     */
    CDunTransporter*& iTransporter;

    /**
     * Waiter for closing self-destruct plugins
     * (from parent iParent)
     */
    CDunCloseWait*& iCloseWait;

    /**
     * Array of TDunConnectionData needed for each loaded plugin
     * (from parent iParent)
     */
    RArray<TDunConnectionData>& iConnData;

    /**
     * Array of plugin UIDs (from parent iParent)
     * Used for queuing plugins that have no resources to use
     * Used for dequeuing plugins when free resources detected
     */
    RArray<TUid>& iPluginQueue;

    /**
     * Array of plugin UIDs (from parent iParent)
     * Used for queuing plugins that were successfully closed outside
     * this server. This is to avoid returning KErrNotFound to LOCOD
     * when NotifyMediaCloseRequest() is caught
     */
    RArray<TUid>& iClosedQueue;

    };

#endif  // C_CDUNSERVERUTILS_H
