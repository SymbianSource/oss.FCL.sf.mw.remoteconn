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
* Description:  Pushes data to existing stream from outside
*
*/


#ifndef C_CDUNDATAPUSHER_H
#define C_CDUNDATAPUSHER_H

#include "DunTransporter.h"

class MDunCompletionReporter;

/**
 *  Class used for storing data related to data push
 *  (data coming from MDunStreamManipulator::NotifyDataPushRequest())
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunDataPush )
    {

public:

    /**
     * Data to push to the stream (not copied)
     */
    const TDesC8 *iPushedData;

    /**
     * Callback to call when data is processed by the stream
     * If this is NULL then no callback will be made
     */
    MDunCompletionReporter* iCallback;

    };

/**
 *  Class for notifications of stream manipulator's completion events
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunCompletionReporter )
    {

public:

    /**
     * Gets called when data from
     * MDunStreamManipulator::NotifyDataPushRequest() has been processed and
     * CDunDownstream is ready to process more data.
     *
     * @since S60 5.0
     * @param aAllPushed ETrue if all in the queue were pushed, EFalse otherwise
     * @return None
     */
    virtual void NotifyDataPushComplete( TBool aAllPushed ) = 0;

    };

/**
 *  Class for pushing data to existing CDunDownstream stream from outside
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunDataPusher ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aParent Parent class owning this friend class (CDunDownstream)
     * @param aStreamCallback Callback to call when events processed
     * @return Instance of self
     */
	static CDunDataPusher* NewL( CDunDownstream& aParent,
	                             MDunCompletionReporter* aStreamCallback );

    /**
    * Destructor.
    */
    virtual ~CDunDataPusher();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Sets media to be used for this endpoint
     *
     * @since S60 3.2
     * @param aComm RComm pointer to use as the endpoint
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RComm* aComm );

    /**
     * Sets media to be used for this endpoint
     *
     * @since S60 3.2
     * @param aSocket RSocket pointer to use as the endpoint
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RSocket* aSocket );

    /**
     * Adds event notification to queue
     *
     * @since S60 3.2
     * @param aPushedData Data to push to the stream (not copied)
     * @param aCallback Callback to call when data is processed by the stream
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddToEventQueue( const TDesC8 *aPushedData,
                          MDunCompletionReporter* aCallback );

    /**
     * Finds an event from queue
     *
     * @since S60 5.0
     * @param aPushedData Data to push to the stream (not copied)
     * @return Index of found event, Symbian error code otherwise
     */
    TInt FindEventFromQueue( const TDesC8 *aPushedData );

    /**
     * Stops one event in the event queue
     *
     * @since S60 5.0
     * @param aPushedData Data to push to the stream (not copied)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StopOneEvent( const TDesC8 *aPushedData );

    /**
     * Sends queued data in round robin
     *
     * @since S60 3.2
     * @return ETrue if sending started, EFalse if nothing to do
     */
    TBool SendQueuedData();

    /**
     * Stops sending for write endpoint
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

    /**
     * Stops sending for write endpoint and clears event queue
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StopAndClearQueue();

    /**
     * Signals completion status in round robin and clears event queue
     *
     * @return Symbian error code on error, KErrNone otherwise
     * @return None
     */
    TInt SignalCompletionAndClearQueue();

private:

    CDunDataPusher( CDunDownstream& aParent,
                    MDunCompletionReporter* aStreamCallback );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Manages one event's data push
     *
     * @since S60 3.2
     * @return None
     */
    TInt ManageOneEvent();

    /**
     * Check whether an error code is severe error or not
     *
     * @since S60 3.2
     * @param aError Error code to check for severity
     * @param aIsError ETrue if error code is error, EFalse if not error
     * @return ETrue if severe error detected, EFalse if not severe error
     */
    TBool ProcessErrorCondition( TInt aError, TBool& aIsError );

// from base class CActive

    /**
     * From CActive.
     * Gets called when endpoint data write complete
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

private:  // data

    /**
     * Parent class owning this friend class (CDunDownstream)
     */
    CDunDownstream& iParent;

    /**
     * Callback to call when notification via MDunCompletionReporter to be made
     * Not own.
     */
    MDunCompletionReporter* iStreamCallback;

    /**
     * Event queue for pushed data requests
     */
    RArray<TDunDataPush> iEventQueue;

    /**
     * Current state of data push: active or inactive
     */
    TDunState iPushState;

    /**
     * Index of current event to serve
     */
    TInt iEventIndex;

    /**
     * RSocket object of local media side
     * If this is set then iComm is not used
     * Not own.
     */
    RSocket* iSocket;

    /**
     * RComm object of local media or network side
     * If this is set then iSocket is not used
     * Not own.
     */
    RComm* iComm;

    };

#endif  // C_DUNDATAPUSHER_H
