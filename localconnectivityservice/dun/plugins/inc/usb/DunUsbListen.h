/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN USB plugin's listener
*
*/


#ifndef C_CDUNUSBLISTEN_H
#define C_CDUNUSBLISTEN_H

#include <e32std.h>
#include <c32comm.h>
#include <usbman.h>
#include "DunPlugin.h"

enum TUsbListenState
    {
    EUsbListenStateIdle,
    EUsbListenStateListening
    };

/**
 *  DUN USB plugin's listener class
 *  This class starts listening for USB device state to change to configured.
 *  When change to configured is detected, parent (CDunUsbPlugin) is notified
 *  to create a channel. Also when device state is no longer configured,
 *  parent (CDunUsbPlugin) is notified to remove a channel.
 *
 *  @lib dunusb.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunUsbListen ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Server
     * @param aParent Creator of this instance
     * @param aUsbServer USB server
     * @return Instance of self
     */
    static CDunUsbListen* NewL( MDunServerCallback* aServer,
                                MDunListenCallback* aParent,
                                RUsb& aUsbServer );

    /**
    * Destructor.
    */
    virtual ~CDunUsbListen();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * IssueRequest to USB server for device state change notifications
     *
     * @return KErrAlreadyExists if device state configured,
     *         KErrNone otherwise
     */
    TInt IssueRequestL();

    /**
     * Stops listening
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunUsbListen( MDunServerCallback* aServer,
                   MDunListenCallback* aParent,
                   RUsb& aUsbServer );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Activates listening request
     *
     * @since S60 3.2
     * @return KErrNotReady if already active, KErrNone otherwise
     */
    TInt Activate();

// from base class CActive

    /**
     * From CActive.
     * Called when read or write operation is ready.
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Cancel current activity.
     *
     * @return None
     */
    void DoCancel();

private:  // data

    /**
     * Callback to call when notification via MDunServerCallback to be made
     * Not own.
     */
    MDunServerCallback* iServer;

    /**
     * Callback to call when notification via MDunListenCallback to be made
     * Not own.
     */
    MDunListenCallback* iParent;

    /**
     * Current state of listening: active or inactive
     */
    TUsbListenState iListenState;

    /**
     * USB server (USB manager) needed for device state notifications
     * Device states needed to create transporter channel
     */
    RUsb& iUsbServer;

    /**
     * Device state of USB set when RUsb::DeviceStateNotification completes
     */
    TUsbDeviceState iDeviceState;

    /**
     * Previous state of USB set when RUsb::DeviceStateNotification completes
     */
    TUsbDeviceState iDeviceStatePrev;

    };

#endif  // C_CDUNUSBLISTEN_H
