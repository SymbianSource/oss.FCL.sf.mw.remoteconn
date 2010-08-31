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
* Description:  Common waiter for closing self-destruct plugins
*
*/

#ifndef C_CDUNCLOSEWAIT_H
#define C_CDUNCLOSEWAIT_H

#include <e32base.h>

class MDunLocalMediaPlugin;

/**
 *  Notification interface class to report requst for closing a plugin
 *
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunCloseWait )
    {

public:

    /**
     * Gets called when closing of plugins should be done
     *
     * @since S60 5.0
     * @param aPluginsToClose Plugins to be closed
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual void NotifyPluginCloseAfterWait(
        RPointerArray<MDunLocalMediaPlugin>& aPluginsToClose ) = 0;

    };

/**
 *  Common waiter for closing self-destruct plugins
 *
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunCloseWait ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aCallback Callback to call when notification via MDunCloseWait
     *                  to be made
     * @return Instance of self
     */
	static CDunCloseWait* NewL( MDunCloseWait* aCallback );

    /**
    * Destructor.
    */
    virtual ~CDunCloseWait();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Adds a new plugin to be closed to the list
     *
     * @since S60 5.0
     * @param aPluginToClose The plugin to be closed
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddPluginToClose( MDunLocalMediaPlugin* aPluginToClose );

    /**
     * Issues request to start closing the objects in the close list
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops closing the objects in the close list
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunCloseWait( MDunCloseWait* aCallback );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 5.0
     * @return None
     */
    void Initialize();

// from base class CActive

    /**
     * From CActive.
     * Gets called when closing of the plugins should be done
     *
     * @since S60 5.0
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 5.0
     * @return None
     */
    void DoCancel();

private:  // data

    /**
     * Callback to call when notification via MDunCloseWait to be made
     */
    MDunCloseWait* iCallback;

    /**
     * List of plugins to be closed
     * Not own.
     */
    RPointerArray<MDunLocalMediaPlugin> iCloseList;

};

#endif  // C_CDUNCLOSEWAIT_H
