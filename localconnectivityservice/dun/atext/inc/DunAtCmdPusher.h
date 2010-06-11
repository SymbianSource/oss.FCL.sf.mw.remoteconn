/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AT command pusher for downstream
*
*/

#ifndef C_CDUNATCMDPUSHER_H
#define C_CDUNATCMDPUSHER_H

#include <atext.h>
#include <e32base.h>
#include "DunDataPusher.h"

class MDunStreamManipulator;

/**
 *  Notification interface class for data pushing status changes
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunAtCmdPusher )
    {

public:

    /**
     * Notifies about end of AT command processing.
     * This is after all reply data for an AT command is multiplexed to the
     * downstream.
     *
     * @since S60 5.0
     * @param aError Error code of command processing completion
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyEndOfProcessing( TInt aError ) = 0;

    /**
     * Notifies about request to stop AT command handling for the rest of the
     * command line data
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyEndOfCmdLineProcessing() = 0;

    /**
     * Notifies about request to peek for the next command
     *
     * @since S60 5.0
     * @return ETrue if next command exists, EFalse otherwise
     */
    virtual TBool NotifyNextCommandPeekRequest() = 0;

    /**
     * Notifies about editor mode reply
     *
     * @since TB9.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyEditorModeReply() = 0;

    };

/**
 *  Class for AT command URC handler
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunAtCmdPusher ) : public CActive,
                                       public MDunCompletionReporter
    {

public:

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to AT command handler
     * @param aDownstream Callback to downstream,
     * @param aOkBuffer Buffer for ok reply
     * @return Instance of self
     */
	static CDunAtCmdPusher* NewL( RATExt* aAtCmdExt,
	                              MDunAtCmdPusher* aCallback,
	                              MDunStreamManipulator* aDownstream,
	                              TDesC8* aOkBuffer );

    /**
     * Two-phased constructor.
     * @param aAtCmdExt Pointer to AT command extension
     * @param aCallback Callback to AT command handler
     * @param aDownstream Callback to downstream
     * @param aOkBuffer Buffer for ok reply
     * @return Instance of self
     */
    static CDunAtCmdPusher* NewLC( RATExt* aAtCmdExt,
                                   MDunAtCmdPusher* aCallback,
                                   MDunStreamManipulator* aDownstream,
                                   TDesC8* aOkBuffer );

    /**
    * Destructor.
    */
    virtual ~CDunAtCmdPusher();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    void ResetData();

    /**
     * Starts AT command handling
     *
     * @since S60 5.0
     * @param aCommand AT command or editor mode input to handle
     * @param aNormalMode ETrue if request issue for normal mode
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest( TDesC8& aCommand, TBool aNormalMode=ETrue );

    /**
     * Stops AT command handling
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

    /**
     * Manages request to abort command handling
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ManageAbortRequest();

    /**
     * Sets end of command line marker on for the possible series of AT
     * commands.
     *
     * @since S60 5.0
     * @param aPushLast ETrue to push last reply, EFalse otherwise
     * @return None
     */
    void SetEndOfCmdLine();

    /**
     * Gets the editor mode status
     *
     * @since TB9.2
     * @return ETrue if in editor mode, EFalse otherwise
     */
    TBool EditorMode();

private:

    CDunAtCmdPusher( RATExt* aAtCmdExt,
                     MDunAtCmdPusher* aCallback,
                     MDunStreamManipulator* aDownstream,
                     TDesC8* aOkBuffer );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 5.0
     * @return None
     */
    void Initialize();

    /**
     * Sets state to idle and notifies about subcommand handling completion
     *
     * @since S60 5.0
     * @param aError Error code for completion
     * @return None
     */
    void SetToIdleAndNotifyEnd( TInt aError );

    /**
     * Checks if "OK" (verbose) or "0" (numeric) string or exists at the end of
     * buffer and removes it
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt CheckAndRemoveOkString();

    /**
     * Sends reply data to downstream
     *
     * @since S60 5.0
     * @param aRecvBuffer ETrue if using receive buffer
     *                    EFalse if using "OK" buffer
     * @return None
     */
    void SendReplyData( TBool aRecvBuffer=ETrue );

    /**
     * Manages change in reply type to EReplyTypeOther
     *
     * @since TB9.2
     * @return None
     */
    void ManageReplyTypeChangeToOther();

    /**
     * Manages change in reply type to EReplyTypeOk
     *
     * @since TB9.2
     * @return None
     */
    void ManageReplyTypeChangeToOk();

    /**
     * Manages change in reply type to EReplyTypeError
     *
     * @since TB9.2
     * @return None
     */
    void ManageReplyTypeChangeToError();

    /**
     * Manages change in reply type to EReplyTypeEditor
     *
     * @since TB9.2
     * @return None
     */
    void ManageReplyTypeChangeToEditor();

    /**
     * Manages change in reply type
     *
     * @since S60 5.0
     * @return None
     */
    void ManageReplyTypeChange();

// from base class CActive

    /**
     * From CActive.
     * Gets called when AT command handled
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

// from base class MDunCompletionReporter

    /**
     * From MDunCompletionReporter.
     * Gets called when data push is complete
     *
     * @since S60 5.0
     * @param aAllPushed ETrue if all in the queue were pushed, EFalse otherwise
     * @return None
     */
    void NotifyDataPushComplete( TBool aAllPushed );

private:  // data

    /**
     * AT command extension
     * Not own.
     */
    RATExt* iAtCmdExt;

    /**
     * Notification interface class for data pushing status changes
     * Not own.
     */
    MDunAtCmdPusher* iCallback;

    /**
     * Callback to call when data to push
     * Not own.
     */
    MDunStreamManipulator* iDownstream;

    /**
     * Buffer for ok reply
     * Not own.
     */
    TDesC8* iOkBuffer;

    /**
     * Current state of AT command handling: active or inactive
     */
    TDunState iAtPushState;

    /**
     * Buffer for receiving
     */
    TBuf8<KDefaultCmdBufLength> iRecvBuffer;

    /**
     * Indicates how many bytes of reply left
     */
    TInt iReplyBytesLeft;

    /**
     * Package for reply bytes left
     */
    TPckg<TInt> iReplyLeftPckg;

    /**
     * Reply type for the handled AT command
     */
    TATExtensionReplyType iReplyType;

    /**
     * Package for reply type for the handled AT command
     */
    TPckg<TATExtensionReplyType> iReplyTypePckg;

    /**
     * Flag indicating if other than "ERROR" reply already received in the
     * command line
     */
    TBool iNoErrorReceived;

    /**
     * Flag indicating if the last "OK" reply push is started
     */
    TBool iLastOkPush;

    /**
     * Flag indicating if abort already tried for one command
     */
    TBool iCmdAbort;

    /**
     * Flag indicating if stop needed after the next reply
     */
    TBool iStop;

    /**
     * Flag indicating if in editor mode
     */
    TBool iEditorMode;

    };

#endif  // C_CDUNATCMDPUSHER_H
