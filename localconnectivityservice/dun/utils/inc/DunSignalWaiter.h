/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Monitors for signal change in RComm local media side
*
*/


#ifndef C_CDUNSIGNALWAITER_H
#define C_CDUNSIGNALWAITER_H

#include "DunChanMan.h"

/**
 *  Class to monitor for signal change in RComm local media side
 *  This class is needed to avoid unnecessary resource allocation of Dataport
 *  side when number of supported Dataport ports for DUN is less than the
 *  number of local media side plugins that use RComm
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunSignalWaiter ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aChannelCallback Callback to call when notification via
     *                         MDunChannelAllocator to be made
     * @return Instance of self
     */
	static CDunSignalWaiter* NewL( MDunChannelAllocator* aChannelCallback );

    /**
    * Destructor.
    */
    virtual ~CDunSignalWaiter();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Adds callback for RunL error controlling
     * The callback will be called when error is detected in asynchronous
     * operation
     *
     * @since S60 3.2
     * @param aCallback Callback to call when line status changes
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddCallback( MDunConnMon* aCallback );

    /**
     * Sets media to use for this endpoint
     *
     * @since S60 3.2
     * @param aComm RComm pointer to use as the endpoint
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RComm* aComm );

    /**
    * Issues request to start waiting for new data in RComm
    *
    * @since S60 3.2
    * @return Symbian error code on error, KErrNone otherwise
    */
    TInt IssueRequest();

    /**
     * Stops monitoring for new data
     *
     * @since S60 3.2
     * @return None
     */
    void Stop();

private:

    CDunSignalWaiter( MDunChannelAllocator* aChannelCallback );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

// from base class CActive

    /**
     * From CActive.
     * Gets called when new data in RComm object
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
     * Callback(s) to call when notification(s) via MDunConnMon to be made
     * Normally contains only one callback
     */
    RPointerArray<MDunConnMon> iCallbacks;

    /**
     * Callback to call when notification via MDunChannelAllocator to be made
     * Not own.
     */
    MDunChannelAllocator* iChannelCallback;

    /**
     * Signals set when RComm::NotifySignalChange() request completes
     */
    TUint iSignals;

    /**
     * RComm object of local media side
     * Not own.
     */
    RComm* iComm;

    };

#endif  // C_CDUNSIGNALWAITER_H
