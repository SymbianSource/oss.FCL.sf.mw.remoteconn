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
* Description:  DUN Bluetooth plugin
*
*/


#ifndef C_CDUNBTPLUGIN_H
#define C_CDUNBTPLUGIN_H

#include <e32base.h>
#include <es_sock.h>
#include "DunTransporter.h"
#include "DunPlugin.h"

/**
 *  Cleanup information for AllocateChannelL().
 *  This data is needed to clean up partially constructed data in
 *  NotifyChannelAllocate() when a leave occurs in channel creation.
 *
 *  @lib dunbt.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( TBtCleanupInfo )
    {

public:

    /**
     * Flag to indicate whether or not a new entity was created
     */
    TBool iNewEntity;

    /**
     * Index to the position of the found entity
     */
    TInt iEntityIndex;

    };

/**
 *  DUN BT plugin's port entity
 *  This class is used to keep track of N number of BT ports and RFCOMM
 *  channel numbers associated to them.
 *
 *  @lib dunbt.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TBtPortEntity )
    {

public:

    /**
     * RFCOMM channel number for BT local media side
     */
    TInt iChannelNum;

    /**
     * Port for BT local media side
     */
    RSocket iBTPort;

    };

/**
 *  DUN Bluetooth plugin's main class
 *  This class creates new transporter channel when new data is detected by
 *  CDunBtListen class and acts on transporter's state changes. Also acts on
 *  DUN server's requests.
 *
 *  @lib dunbt.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunBtPlugin ) : public CBase,
                                    public MDunLocalMediaPlugin,
                                    public MDunListenCallback,
                                    public MDunConnMon
    {

public:

    CDunBtPlugin();

    /**
    * Destructor.
    */
    virtual ~CDunBtPlugin();

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
     * Allocates a free channel
     *
     * @since S60 3.2
     * @param aNoFreeChans ETrue if no free channels, EFalse otherwise
     * @param aCleanupInfo Cleanup information
     * @return None
     */
    void AllocateChannelL( TBool& aNoFreeChans, TBtCleanupInfo& aCleanupInfo );

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
    TInt GetPortByConnId( TConnId aConnId, TBtPortEntity*& aEntity );

    /**
     * Gets first free port's index and entity
     *
     * @since S60 3.2
     * @param aEntity Returned port entity
     * @return KErrNotFound if entity not found, found index otherwise
     */
    TInt GetFirstFreePort( TBtPortEntity*& aEntity );

    /**
     * Sets modem's MSC (Modem Status Command)
     *
     * @since S60 3.2
     * @param aEntity Port entity for which to change signal
     * @param aSignal Signal to set to high or low
     * @param aSignalOn ETrue if signal wanted high, otherwise low
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetRFCOMMStatusCommand( TBtPortEntity& aEntity,
                                 TUint8 aSignal,
                                 TBool aSignalOn );

    /**
     * Manages advertiser for channel free operation
     *
     * @since S60 5.0
     * @return None
     */
    void ManageAdvertiserFreeOperationL();

    /**
     * Cleans partial created channel data based on TATExtCleanupInfo
     *
     * @since S60 5.0
     * @param aCleanupInfo Cleanup information
     * @return None
     */
    void CleanPartialChanneldata( TBtCleanupInfo& aCleanupInfo );
    
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

private:  // data

    /**
     * Callback to call when notification via MDunServerCallback to be made
     * Not own.
     */
    MDunServerCallback* iServer;

    /**
     * Listener for detecting new data in RFComm channel
     * Notifies this class about new data
     * Own.
     */
    CDunBtListen* iBTListen;

    /**
     * Data socket entity for actual data transfer
     * This will be valid after CDunBtListen reacts to new data in
     * listener socket
     */
    TBtPortEntity iEntity;

    /**
     * Array of RSocket port(s) entities for BT local media side
     */
    RArray<TBtPortEntity> iBTPorts;

    /**
     * Transporter to use for network side communication
     * Not own.
     */
    CDunTransporter* iTransporter;

    };

#endif  // C_CDUNBTPLUGIN_H
