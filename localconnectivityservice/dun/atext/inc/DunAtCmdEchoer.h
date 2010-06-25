/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Echoer for AT commands
*
*/

#ifndef C_CDUNATCMDECHOER_H
#define C_CDUNATCMDECHOER_H

#include "DunDataPusher.h"

class MDunStreamManipulator;

/**
 *  Notification interface class for text mode
 *
 *  @lib dunatext.lib
 *  @since TB9.2
 */
NONSHARABLE_CLASS( MDunAtCmdEchoer )
    {

public:

    /**
     * Notifies about completed echo in text mode
     *
     * @since TB9.2
     * @return None
     */
    virtual void NotifyEchoComplete() = 0;

    };

/**
 *  Class for AT command echoer
 *
 *  @lib dunatext.lib
 *  @since TB9.2
 */
NONSHARABLE_CLASS( CDunAtCmdEchoer ) : public CBase,
                                       public MDunCompletionReporter
    {

public:

    /**
     * Two-phased constructor.
     * @param aStreamCallback Callback to stream
     * @return Instance of self
     */
    static CDunAtCmdEchoer* NewL( MDunStreamManipulator* aStreamCallback );

    /**
    * Destructor.
    */
    virtual ~CDunAtCmdEchoer();

    /**
     * Resets data to initial values
     *
     * @since TB9.2
     * @return None
     */
    void ResetData();

    /**
     * Sends a character to be echoed
     *
     * @since TB9.2
     * @param aInput Input to echo
     * @param aCallback Callback to echo request completions
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SendEchoCharacter( const TDesC8* aInput, MDunAtCmdEchoer* aCallback );

private:

    CDunAtCmdEchoer( MDunStreamManipulator* aStreamCallback );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since TB9.2
     * @return None
     */
    void Initialize();

// from base class MDunCompletionReporter

    /**
     * From MDunCompletionReporter.
     * Gets called when data push is complete
     *
     * @since TB9.2
     * @param aAllPushed ETrue if all in the queue were pushed, EFalse otherwise
     * @return None
     */
    void NotifyDataPushComplete( TBool aAllPushed );

private:  // data

    /**
     * Notification interface class for echo request completions
     * Not own.
     */
    MDunAtCmdEchoer* iCallback;

    /**
     * Callback to call when data to push
     * Not own.
     */
    MDunStreamManipulator* iStreamCallback;

    /**
     * Current state of echoing: active or inactive
     */
    TDunState iAtEchoState;

    };

#endif  // C_CDUNATCMDECHOER_H
