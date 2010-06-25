/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definitions needed for common stream functionality
*
*/


#ifndef C_CDUNSTREAM_H
#define C_CDUNSTREAM_H

#include "DunTransporter.h"

/**
 *  Class for common stream functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunStream ) : public CActive
    {

public:

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Adds error code to consider as "no error" to either endpoint
     *
     * @since S60 3.2
     * @param aError Error code to add
     * @param aOperationType Type of operation for which to add error
     *                       (read or write)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddSkippedError( TInt aError, TDunOperationType aOperationType );

    /**
     * Adds callback for line status change controlling
     * The callback will be called when serious read error is detected
     *
     * @since S60 3.2
     * @param aCallback Callback to call when read error occurs
     * @param aOperationType Type of operation for which to add error
     *                       (read or write)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddConnMonCallback( MDunConnMon* aCallback,
                             TDunOperationType aOperationType );

    /**
     * Sets buffering for this stream
     *
     * @since S60 3.2
     * @param aBufferPtr Pointer to the buffer
     * @return KErrGeneral if buffer pointer null, KErrNone otherwise
     */
    TInt SetBuffering( TPtr8* aBufferPtr );

    /**
     * Sets media to be used for this endpoint
     *
     * @since S60 3.2
     * @param aComm RComm pointer to use as the endpoint
     * @param aMediaContext Media context to use for this endpoint, either
     *                      local or network
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RComm* aComm, TDunMediaContext aMediaContext );

    /**
     * Sets media to be used for this endpoint
     *
     * @since S60 3.2
     * @param aSocket RSocket pointer to use as the endpoint
     * @param aMediaContext Media context to use for this endpoint, either
     *                      local or network
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RSocket* aSocket, TDunMediaContext aMediaContext );

    /**
     * Gets media context
     *
     * @since S60 3.2
     * @param aStreamType Stream type
     * @return Media context in use
     */
    TDunMediaContext GetMediaContext( TDunStreamType aStreamType );

protected:

    CDunStream();

    /**
    * Destructor.
    */
    virtual ~CDunStream();

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
     * Gets called when endpoint data read/write complete
     *
     * @since S60 3.2
     * @return None
     */
    virtual void RunL() {};

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 3.2
     * @return None
     */
    virtual void DoCancel() {};

private:

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

protected:  // data

    /**
     * Pointer to stream's data buffer: upstream or downstream
     * Not own.
     */
    TPtr8* iBufferPtr;

    /**
     * Callback(s) to call when notification(s) via MDunConnMon to be made
     * These callbacks are called on read failures
     */
    RPointerArray<MDunConnMon> iCallbacksR;

    /**
     * Callback(s) to call when notification(s) via MDunConnMon to be made
     * These callbacks are called on write failures
     */
    RPointerArray<MDunConnMon> iCallbacksW;

    /**
     * Read length set when RSocket::RecvOneOrMore() request completes
     */
    TSockXfrLength iReadLengthSocket;

    /**
     * Type of current operation: read or write
     */
    TDunOperationType iOperationType;

    /**
     * Current state of transfer: active or inactive
     */
    TDunState iTransferState;

    /**
     * Direction of data transfer
     * This is set after iStreamType and iOperationType are known
     */
    TDunDirection iDirection;

    /**
     * Array of error codes that will be skipped for read operations
     */
    RArray<TInt> iOkErrorsR;

    /**
     * Array of error codes that will be skipped for write operations
     */
    RArray<TInt> iOkErrorsW;

    /**
     * RComm object of network side
     * Not own.
     */
    RComm* iNetwork;

    /**
     * RSocket object of local media side
     * If this is set then iComm is not used
     * Not own.
     */
    RSocket* iSocket;

    /**
     * RComm object of local media side
     * If this is set then iSocket is not used
     * Not own.
     */
    RComm* iComm;

    };

#endif  // C_CDUNSTREAM_H
