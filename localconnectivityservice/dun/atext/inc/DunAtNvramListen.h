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
* Description:  AT NVRAM status change listener and notifier
*
*/

#ifndef C_CDUNATNVRAMLISTEN_H
#define C_CDUNATNVRAMLISTEN_H

#include <atext.h>
#include <atextcommon.h>
#include "DunTransporter.h"

/**
 *  Class for AT NVRAM status change listener
 *
 *  @lib dunatext.lib
 *  @since TB9.2
 */
NONSHARABLE_CLASS( CDunAtNvramListen ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aAtCmdExtCommon Pointer to AT command extension to common
     *                        functionality
     * @return Instance of self
     */
	static CDunAtNvramListen* NewL( RATExt* aAtCmdExt,
	                                RATExtCommon* aAtCmdExtCommon );

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aAtCmdExtCommon Pointer to AT command extension to common
     *                        functionality
     * @return Instance of self
     */
    static CDunAtNvramListen* NewLC( RATExt* aAtCmdExt,
                                     RATExtCommon* aAtCmdExtCommon );

    /**
    * Destructor.
    */
    virtual ~CDunAtNvramListen();

    /**
     * Resets data to initial values
     *
     * @since TB9.2
     * @return None
     */
    void ResetData();

    /**
     * Starts waiting for NVRAM status changes
     *
     * @since TB9.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops waiting for NVRAM status changes
     *
     * @since TB9.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunAtNvramListen( RATExt* aAtCmdExt,
                       RATExtCommon* aAtCmdExtCommon );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since TB9.2
     * @return None
     */
    void Initialize();

// from base class CActive

    /**
     * From CActive.
     * Gets called when NVRAM has changed
     *
     * @since TB9.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since TB9.2
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
     * AT command extension to common functionality
     * Not own.
     */
    RATExtCommon* iAtCmdExtCommon;

    /**
     * Current state of NVRAM status change listening; active or inactive
     */
    TDunState iNvramHandleState;

    /**
     * Buffer for NVRAM
     */
    TBuf8<KDefaultNvramBufLength> iNvramBuffer;

    /**
     * Flag to indicate start of receiving
     */
    TBool iStarted;

    };

#endif  // C_CDUNATNVRAMLISTEN_H
