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


#ifndef C_CDUNDOWNSTREAM_H
#define C_CDUNDOWNSTREAM_H

#include "DunTransUtils.h"
#include "DunStream.h"
#include "DunAtCmdHandler.h"
#include "DunDataPusher.h"

class MDunCompletionReporter;
class MDunAtCmdHandler;

/**
 *  Class used for storing data related to data pushing
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunPushData )
    {

public:

    /**
     * Flag for command mode notifier's MDunCmdModeMonitor callback
     * This flag is needed to mark command mode start/end
     */
    TBool iDataMode;

    /**
     * Data pusher for stream manipulation
     */
    CDunDataPusher* iDataPusher;

    /**
     * AT command handling related functionality for CDunDownstream
     * Not own.
     */
    MDunAtCmdHandler* iAtCmdHandler;

    };

/**
 *  Class for manipulating existing stream's contents from outside
 *  (outside of class CDunDownstream)
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunStreamManipulator )
    {

public:

    /**
     * Gets called when outside party wants to push data to the existing stream
     *
     * @since S60 5.0
     * @param aPushedData Data to push to the stream (not copied)
     * @param aCallback Callback to call when data is processed by the stream
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyDataPushRequest(
        const TDesC8 *aPushedData,
        MDunCompletionReporter* aCallback ) = 0;

    };

/**
 *  Class for data transmission of one "stream" from network to local media
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunDownstream ) : public CDunStream,
                                      public MDunStreamManipulator,
                                      public MDunCompletionReporter,
                                      public MDunCmdModeMonitor
    {

    friend class CDunDataPusher;

public:

    /**
     * Two-phased constructor.
     * @param aUtility Pointer to common utility class
     * @return Instance of self
     */
	static CDunDownstream* NewL( MDunTransporterUtilityAux* aUtility );

    /**
    * Destructor.
    */
    virtual ~CDunDownstream();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Starts downstream by issuing read request
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StartStream();

    /**
     * Stops transfer for read or write endpoints
     *
     * @since S60 3.2
     * @param aStopMplex ETrue if multiplexer stop, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop( TBool aStopMplex=ETrue );

    /**
     * Initializes this stream for data pushing
     *
     * @since S60 3.2
     * @param aAtCmdHandler AT command handling related functionality
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitializeForDataPushing( MDunAtCmdHandler* aAtCmdHandler );

    /**
     * Adds data to event queue and starts sending if needed
     *
     * @since S60 5.0
     * @param aPushedData Data to push to the stream (not copied)
     * @param aCallback Callback to call when data is processed by the stream
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddToQueueAndSend( const TDesC8 *aPushedData,
                            MDunCompletionReporter* aCallback );

private:

    CDunDownstream( MDunTransporterUtilityAux* aUtility );

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

// from base class CActive

    /**
     * From CActive.
     * Gets called when endpoint data read complete
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

// from base class MDunStreamManipulator

    /**
     * Gets called when outside party wants to push data to the existing stream
     *
     * @since S60 3.2
     * @param aPushedData Data to push to the stream (not copied)
     * @param aCallback Callback to call when data is processed by the stream
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyDataPushRequest( const TDesC8 *aPushedData,
                                MDunCompletionReporter* aCallback );

// from base class MDunCompletionReporter

    /**
     * Gets called when data push is complete
     *
     * @since S60 5.0
     * @param aAllPushed ETrue if all in the queue were pushed, EFalse otherwise
     * @return None
     */
    void NotifyDataPushComplete( TBool aAllPushed );

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

private:  // data

    /**
     * Pointer to common utility class
     * Not own.
     */
    MDunTransporterUtilityAux* iUtility;

    /**
     * Data related to data pushing
     */
    TDunPushData iPushData;

    };

#endif  // C_CDUNDOWNSTREAM_H
