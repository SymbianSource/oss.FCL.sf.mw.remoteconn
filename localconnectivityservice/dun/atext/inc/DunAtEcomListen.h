/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ECom plugin install/uninstall/version listener
*
*/


#ifndef C_CDUNATECOMLISTEN_H
#define C_CDUNATECOMLISTEN_H

#include <atext.h>
#include "DunTransporter.h"

/**
 *  Notification interface class for ECOM plugin interface status changes
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunAtEcomListen )
    {

public:

    /**
     * Notifies about new plugin installation
     *
     * @since S60 5.0
     * @param aPluginUid UID of installed plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginInstallation( TUid& aPluginUid ) = 0;

    /**
     * Notifies about existing plugin uninstallation
     *
     * @since S60 5.0
     * @param aPluginUid UID of uninstalled plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyPluginUninstallation( TUid& aPluginUid ) = 0;

    };

/**
 *  Class for ECom plugin install/uninstall/version listener
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunAtEcomListen ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to mode status change
     * @return Instance of self
     */
	static CDunAtEcomListen* NewL( RATExt* aAtCmdExt,
                                   MDunAtEcomListen* aCallback );

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to mode status change
     * @return Instance of self
     */
    static CDunAtEcomListen* NewLC( RATExt* aAtCmdExt,
                                    MDunAtEcomListen* aCallback );
	
    /**
    * Destructor.
    */
    virtual ~CDunAtEcomListen();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Starts waiting for ECom plugin install/uninstall/version status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops waiting for Ecom plugin install/uninstall/version status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunAtEcomListen( RATExt* aAtCmdExt,
                      MDunAtEcomListen* aCallback );

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
     * Gets called when plugin installed, uninstalled or changed
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
     * AT command extension
     * Not own.
     */
    RATExt* iAtCmdExt;

    /**
     * Callback to call when plugin installed, uninstalled or changed
     * Not own.
     */
    MDunAtEcomListen* iCallback;

    /**
     * Current state of ECom interface listening: active or inactive
     */
    TDunState iEcomListenState;

    /**
     * UID of the installed, uninstalled or changed plugin
     */
    TUid iPluginUid;

    /**
     * Package for plugin UID
     */
    TPckg<TUid> iPluginUidPckg;

    /**
     * Package for ECOM type
     */
    TPckg<TATExtensionEcomType> iEcomTypePckg;

    };

#endif  // C_CDUNATECOMLISTEN_H
