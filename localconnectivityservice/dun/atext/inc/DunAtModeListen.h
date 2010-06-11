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
* Description:  Modem's mode listener and notifier
*
*/


#ifndef C_CDUNATMODELISTEN_H
#define C_CDUNATMODELISTEN_H

#include <e32base.h>
#include <atextcommon.h>
#include "DunTransporter.h"

/**
 *  Notification interface class for modem's mode listener
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunAtModeListen )
    {

public:

    /**
     * Notifies about mode status change
     *
     * @since S60 5.0
     * @param aMode Current modem modes
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyModeStatusChange( TUint aMode ) = 0;

    };

/**
 *  Class for modem's mode listener and notifier
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunAtModeListen ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * aAtCmdExtCommand AT command extension to common functionality
     * aCallback Callback to mode status change
     * @return Instance of self
     */
	static CDunAtModeListen* NewL( RATExtCommon* aAtCmdExtCommon,
	                               MDunAtModeListen* aCallback );

    /**
     * Two-phased constructor.
     * aAtCmdExtCommand AT command extension to common functionality
     * aCallback Callback to mode status change
     * @return Instance of self
     */
    static CDunAtModeListen* NewLC( RATExtCommon* aAtCmdExtCommon,
                                    MDunAtModeListen* aCallback );

    /**
    * Destructor.
    */
    virtual ~CDunAtModeListen();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Issues request to start monitoring for mode status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops monitoring for mode status changes
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunAtModeListen( RATExtCommon* iAtCmdExtCommon,
                      MDunAtModeListen* aCallback );

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
     * Gets called when mode changes
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
     * AT command extension to common functionality
     * Not own.
     */
    RATExtCommon* iAtCmdExtCommon;

    /**
     * Callback to call when mode status changes
     * Not own.
     */
    MDunAtModeListen* iCallback;

    /**
     * Current state of mode listening: active or inactive
     */
    TDunState iModeListenState;

    /**
     * Stored mode status change
     */
    TUint iMode;

    /**
     * Package for mode status changes
     */
    TPckg<TUint> iModePckg;

    };

#endif  // C_CDUNATMODELISTEN_H
