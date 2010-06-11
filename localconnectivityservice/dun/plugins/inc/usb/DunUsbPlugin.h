/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN USB plugin
*
*/


#ifndef C_CDUNUSBPLUGIN_H
#define C_CDUNUSBPLUGIN_H

#include <e32base.h>
#include <usbman.h>
#include "DunTransporter.h"
#include "DunUsbListen.h"
#include "DunUsbConfig.h"
#include "DunPlugin.h"

/**
 *  DUN USB plugin's port entity
 *  This class is used to keep track of N number of USB ports and port
 *  numbers associated to them (for CDunUsbConfig's purposes).
 *
 *  @lib dunusb.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TUsbPortEntity )
    {

public:

    /**
     * Port number for USB local media side
     */
    TInt iPortNum;

    /**
     * Port for USB local media side
     */
    RComm iUsbPort;

    };

/**
 *  DUN USB plugin's main class
 *  This class creates new transporter channel(s) when configured USB is
 *  detected by CDunUsbListen class and acts on transporter's state changes.
 *  Also acts on DUN server's requests.
 *
 *  @lib dunusb.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunUsbPlugin ) : public CBase,
                                     public MDunLocalMediaPlugin,
                                     public MDunListenCallback,
                                     public MDunBufferCorrection,
                                     public MDunConnMon,
                                     public MDunUsbConfig
    {

public:

    CDunUsbPlugin();

    /**
    * Destructor.
    */
    virtual ~CDunUsbPlugin();

private:

    /**
     * State of this plugin
     *
     * @since S60 3.2
     * @return State of plugin
     */
    TDunPluginState PluginState();

    /**
     * Constructs a listener object for this plugin
     *
     * @since S60 3.2
     * @return None
     */
    void ConstructListenerL();

    /**
     * Initializes USB by loading LDD
     *
     * @since S60 3.2
     * @return None
     */
    void InitUsbL();

    /**
     * Initializes all usable USB ports for DUN
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitPorts();

    /**
     * Creates empty usable USB ports that can be used by DUN
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt CreateAllPorts();

    /**
     * Initializes one USB port for DUN
     *
     * @since S60 3.2
     * @param aEntity Pointer to port entity
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitOnePort( TUsbPortEntity* aEntity );

    /**
     * Sets channel for one USB port
     *
     * @since S60 3.2
     * @param aEntity Pointer to port entity
     * @return None
     */
    void SetChannelL( TUsbPortEntity* aEntity );

    /**
     * Adds skipped error code to Transporter
     * The skipped error is added to local media's read and write operations
     *
     * @since S60 3.2
     * @param aError Skipped error code to add
     * @param aComm RComm object for which to add the error code
     * @return None
     */
    void AddSkippedErrorL( TInt aError, RComm* aComm );

    /**
     * Sets new state
     * New state must be one more than the old state
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ReportStateChangeUp( TDunPluginState aPluginState );

    /**
     * Sets new state
     * New state must be one less than the old state
     *
     * @since S60 3.2
     * @param aPluginState New state to set for a plugin
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ReportStateChangeDown( TDunPluginState aPluginState );

    /**
     * Frees existing channels
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt FreeChannels();

    /**
     * Uninitializes this plugin
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Uninitialize();

    /**
     * Gets port's index and entity by connection ID
     *
     * @since S60 3.2
     * @param aConnId Connection ID of the connection to find
     * @param aEntity Returned port entity
     * @return KErrNotFound if entity not found, found index otherwise
     */
    TInt GetEntityByConnId( TConnId aConnId, TUsbPortEntity*& aEntity );

    /**
     * Gets port's entity by port number
     *
     * @since S60 3.2
     * @param aPortNum Port number of the connection to find
     * @param aEntity Returned port entity
     * @return KErrNotFound if entity not found, found index otherwise
     */
    TInt GetEntityByPortNumber( TInt aPortNum, TUsbPortEntity*& aEntity );

    /**
     * Gets first free port's index and entity
     *
     * @since S60 3.2
     * @param aEntity Returned port entity
     * @return KErrNotFound if entity not found, found index otherwise
     */
    TInt GetFirstFreeEntity( TUsbPortEntity*& aEntity );

// from base class MDunLocalMediaPlugin

    void ConstructL( MDunServerCallback* aServer,
                     CDunTransporter* aTransporter );

    /**
     * From MDunLocalMediaPlugin.
     * Gets called when server changes a plugin's state
     *
     * @since S60 3.2
     * @param aPluginState New changed state
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyServerStateChange( TDunPluginState aPluginState );

    /**
     * From MDunLocalMediaPlugin.
     * Gets called when server needs to know the active connection
     * (For testing purposes only)
     *
     * @since S60 5.0
     * @return Active connection, NULL otherwise
     */
    TConnId ActiveConnection();

// from base class MDunListenCallback

    /**
     * From MDunListenCallback.
     * Gets called when new channel must be created
     *
     * @since S60 3.2
     * @param aNoFreeChans ETrue if no free channels, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyChannelAllocate( TBool& aNoFreeChans );

    /**
     * From MDunListenCallback.
     * Gets called when an existing channel must be freed
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyChannelFree();

// from base class MDunConnMon

    /**
     * From MDunConnMon.
     * Gets called when line status changes or when any type of error is
     * detected
     *
     * @since S60 3.2
     * @param aConnId Connection ID for callback
     * @param aConnReason Reason for progress change
     * @return None
     */
    void NotifyProgressChangeL( TConnId aConnId,
                                TDunConnectionReason aConnReason );

// from base class MDunBufferCorrection

    /**
     * Gets called when request to change local media's buffer size
     *
     * @since S60 3.2
     * @param aLength Suggested buffer length that will be used if no
     *                correction done
     * @return New (corrected) buffer length
     */
    TInt NotifyBufferCorrection( TInt aLength );

// from base class MDunUsbConfig

    /**
     * Gets called when one or more ACM configurations are added
     *
     * @since S60 3.2
     * @param aIndex Index (also port number) of added ACM configuration
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyConfigAddition( TInt aIndex );

    /**
     * Gets called when one or more ACM configurations are removed
     *
     * @since S60 3.2
     * @param aIndex Index (also port number) of removed ACM configuration
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyConfigRemoval( TInt aIndex );

private:  // data

    /**
     * Callback to call when notification via MDunServerCallback to be made
     * Not own.
     */
    MDunServerCallback* iServer;

    /**
     * Listener for detecting attached USB cable and configured ACM
     * Notifies this class to create new channel
     * Own.
     */
    CDunUsbListen* iUsbListen;

    /**
     * Listener and accessor for detecting USB ACM configuration change
     * Own.
     */
    CDunUsbConfig* iUsbConfig;

    /**
     * Communications server used to load ECACM and open USB port(s)
     * Also used to get port information to calculate number of supported ACMs
     */
    RCommServ iCommServer;

    /**
     * USB server (USB manager) needed for device state notifications
     * Device states needed to create transporter channel
     */
    RUsb iUsbServer;

    /**
     * Array of port(s) for USB local media side
     */
    RArray<TUsbPortEntity> iUsbPorts;

    /**
     * Transporter to use for network side communication
     * Not own.
     */
    CDunTransporter* iTransporter;

    /**
     * Flag to be set on if CDunUsbPlugin is waiting for shutdown.
     * This flag is used to skip subsequent higher-level error codes after
     * driver based error occurred.
     */
    TBool iShutdown;

    };

#endif  // C_CDUNUSBPLUGIN_H
