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
* Description:  Copies signals changes between network and local media
*
*/


#ifndef C_CDUNSIGNALCOPY_H
#define C_CDUNSIGNALCOPY_H

#include "DunTransporter.h"
#include "DunTransUtils.h"
#include "DunAtCmdHandler.h"

/**
 *  Class for copying signal changes between to endpoints
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunSignalCopy ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @return Instance of self
     */
	static CDunSignalCopy* NewL();

    /**
    * Destructor.
    */
    virtual ~CDunSignalCopy();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Adds callback for line status change controlling
     * The callback will be called when RunL error is detected
     *
     * @since S60 3.2
     * @param aCallback Callback to call when line status changes
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddCallback( MDunConnMon* aCallback );

    /**
     * Sets media to use for this endpoint monitor
     *
     * @since S60 3.2
     * @param aComm RComm pointer to local media side
     * @param aNetwork RComm pointer to network side
     * @param aStreamType Stream type for this endpoint
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RComm* aComm,
                   RComm* aNetwork,
                   TDunStreamType aStreamType );

    /**
     * Issues request to start monitoring the endpoint for line status change
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops monitoring the endpoint for line status change
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunSignalCopy();

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Manages upstream signal changes
     *
     * @since S60 3.2
     * @return None
     */
    void ManageSignalChange();

    /**
     * Manages signal changes
     *
     * @since S60 3.2
     * @return None
     */
    void ManageSignalChangeUpstream();

    /**
     * Manages downstream signal changes
     *
     * @since S60 3.2
     * @return None
     */
    void ManageSignalChangeDownstream();

    /**
     * Changes upstream signal
     *
     * @since S60 5.0
     * @param aSetMask Set the handshaking lines in the mask
     * @param aClearMask Clear the handshaking lines in the mask
     * @return None
     */
    void ChangeUpstreamSignal( TUint aSetMask, TUint aClearMask );

    /**
     * Changes downstream signal
     *
     * @since S60 3.2
     * @param aSetMask Set the handshaking lines in the mask
     * @param aClearMask Clear the handshaking lines in the mask
     * @return None
     */
    void ChangeDownstreamSignal( TUint aSetMask, TUint aClearMask );

// from base class CActive

    /*
     * From CActive.
     * Gets called when line status changes
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
     * Callback(s) to call when command mode starts or ends
     * Usually two needed: one for upstream and second for downstream
     */
    RPointerArray<MDunCmdModeMonitor> iCmdCallbacks;

    /**
     * Used media context: network or local
     */
    TDunMediaContext iContextInUse;

    /**
     * Used stream type: upstream or downstream
     */
    TDunStreamType iStreamType;

    /**
     * Current state of connection monitoring: active or inactive
     */
    TDunState iSignalCopyState;

    /**
     * Signal to listen with RComm::NotifySignalChange()
     */
    TUint iListenSignals;

    /**
     * Signals set when RComm::NotifySignalChange() request completes
     */
    TUint iSignals;

    /**
     * RComm object of network side
     * Not own.
     */
    RComm* iNetwork;

    /**
     * RComm object of local media side
     * Not own.
     */
    RComm* iComm;

    };

#endif  // C_CDUNSIGNALCOPY_H
