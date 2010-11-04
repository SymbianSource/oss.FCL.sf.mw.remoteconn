/*
* Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definitions needed for one "stream" of CDunTransporter
*
*/


#ifndef C_CDUNUPSTREAM_H
#define C_CDUNUPSTREAM_H

#include "DunTransUtils.h"
#include "DunStream.h"
#include "DunAtCmdHandler.h"
#include "DunAtCmdEchoer.h"
#include "DunDataPusher.h"

class MDunCmdModeMonitor;

/**
 *  Class used for storing data related to activity monitoring
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunActivityData )
    {

public:

    /**
     * Callback to call when notification via MDunActivityManager to be made
     * Not own.
     */
    MDunActivityManager* iActivityCallback;

    /**
     * Flag for data read activity's MDunActivityManager callback
     * This flag is needed to prevent multiple notifications of same activity
     */
    TBool iDataRead;

    /**
     * Used as a flag for the first notification of read activity
     * This flag is needed to keep MDunActivityManager notifications in sync
     * (inactivity notification done only if activity notification done first)
     */
    TBool iNotified;

    };

/**
 *  Class used for storing data related to AT command parsing
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunParseData )
    {

public:

    /**
     * Flag for command mode notifier's MDunCmdModeMonitor callback
     * This flag is needed to mark command mode start/end for parse start/end
     */
    TBool iDataMode;

    /**
     * AT Command handler.
     */
    CDunAtCmdHandler* iAtCmdHandler;

    };

/**
 *  Accessor class for AT command handling related functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunAtCmdHandler )
    {

public:

    /**
     * Starts URC message handling
     * This is an accessor for CDunDownstream's StartStream()
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt StartUrc() = 0;

    /**
     * Stops AT command handling downstream related activity (also URC)
     * This is an accessor for CDunDownstream's Stop()
     *
     * @since S60 3.2
     * @return None
     */
    virtual void StopAtCmdHandling() = 0;

    };

/**
 *  Class for data transmission of one "stream" from local media to network
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunUpstream ) : public CDunStream,
                                    public MDunAtCmdHandler,
                                    public MDunCmdModeMonitor,
                                    public MDunAtCmdStatusReporter,
                                    public MDunAtCmdEchoer
    {

public:

    /**
     * Two-phased constructor.
     * @param aUtility Pointer to common utility class
     * @return Instance of self
     */
	static CDunUpstream* NewL( MDunTransporterUtilityAux* aUtility );

    /**
    * Destructor.
    */
    virtual ~CDunUpstream();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Sets activity callback for this stream
     *
     * @since S60 3.2
     * @param aActivityCallback Pointer to activity callback
     * @return KErrGeneral if callback null, KErrNone otherwise
     */
    TInt SetActivityCallback( MDunActivityManager* aActivityCallback );

    /**
     * Initializes this stream for AT command notifications
     *
     * @since S60 5.0
     * @param aStreamCallback Pointer to stream callback
     * @param aConnectionName Connection identifier name
     * @param aCallbackUp Upstream callback to call when command mode starts or
     *                    ends
     * @param aCallbackDown Downstream callback to call when command mode starts
     *                    or ends
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitializeForAtParsing( MDunStreamManipulator* aStreamCallback,
                                 const TDesC8* aConnectionName,
                                 MDunCmdModeMonitor* aCallbackUp,
                                 MDunCmdModeMonitor* aCallbackDown );

    /**
     * Starts upstream by issuing read request
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StartStream();

    /**
     * Stops transfer for read & write endpoints
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

    /**
     * Data transmission state (if read completed)
     *
     * @since S60 3.2
     * @return ETrue if data successfully read, EFalse otherwise
     */
    TBool DataReadStatus();

private:

    CDunUpstream( MDunTransporterUtilityAux* aUtility );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Issues transfer request for this stream
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Processes data that was read
     *
     * @since S60 5.0
     * @return ETrue if request to be reissued, EFalse otherwise
     */
    TBool ProcessReadData();

    /**
     * Manages activity in a channel
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ManageChannelActivity();

// from base class CActive

    /**
     * From CActive.
     * Gets called when endpoint data read/write complete
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();

// from base class MDunAtCmdStatusReporter

    /**
     * Notifies about parser's need to get more data
     *
     * @since TB9.2
     * @return None
     */
    void NotifyParserNeedsMoreData();

    /**
     * Notifies about editor mode reply
     *
     * @since TB9.2
     * @param aStart ETrue if start of editor mode, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    void NotifyEditorModeReply( TBool aStart );

// from base class MDunAtCmdHandler

    /**
     * Starts URC message handling
     * This is an accessor for CDunDownstream's StartStream()
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StartUrc();

    /**
     * Stops AT command handling downstream related activity (also URC)
     * This is an accessor for CDunDownstream's Stop()
     *
     * @since S60 3.2
     * @return None
     */
    void StopAtCmdHandling();

// from base class MDunCmdModeMonitor

    /**
     * Notifies about command mode start
     *
     * @since S60 5.0
     * @return None
     */
    void NotifyCommandModeStart();

    /**
     * Notifies about command mode end
     *
     * @since S60 5.0
     * @return None
     */
    void NotifyCommandModeEnd();

// from base class MDunAtCmdEchoer

    /**
     * Notifies about completed echo in text mode
     *
     * @since TB9.2
     * @return None
     */
    void NotifyEchoComplete();

private:  // data

    /**
     * Pointer to common utility class
     * Not own.
     */
    MDunTransporterUtilityAux* iUtility;

    /**
     * Data related to activity monitoring
     */
    TDunActivityData iActivityData;

    /**
     * Data related to AT command parsing
     */
    TDunParseData iParseData;

    };

#endif  // C_CDUNUPSTREAM_H
