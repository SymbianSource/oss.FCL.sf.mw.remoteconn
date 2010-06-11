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
* Description:  DUN Bluetooth plugin's listener
*
*/


#ifndef C_CDUNBTLISTEN_H
#define C_CDUNBTLISTEN_H

#include <btengdiscovery.h>
#include "DunPlugin.h"
#include "DunTransporter.h"

class TBtPortEntity;

enum TBtListenState
    {
    EBtListenStateIdle,
    EBtListenStateListening
    };

/**
 *  DUN Bluetooth plugin's listener class
 *  This class starts listening on a created BT RFComm channel and notifies
 *  parent (CDunBtPlugin) when new connection is detected. Also registers and
 *  unregisters DUN SDP record.
 *
 *  @lib dunbt.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunBtListen ) : public CActive,
                                    public MDunServAdvMon
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Server
     * @param aParent Callback interface to parent
     * @param aTransporter Transporter
     * @param aEntity Data socket entity
     * @return Instance of self
     */
    static CDunBtListen* NewL( MDunServerCallback* aServer,
                               MDunListenCallback* aParent,
                               CDunTransporter* aTransporter,
                               TBtPortEntity& aEntity );

    /**
    * Destructor.
    */
    virtual ~CDunBtListen();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Registers DUN and starts to listen.
     * Registers itself to SDP and BT manager, opens a socket
     * and starts to listen it.
     *
     * @since S60 3.2
     * @return None
     */
    void IssueRequestL();

    /**
     * Stops listening
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunBtListen( MDunServerCallback* aServer,
                  MDunListenCallback* aParent,
                  CDunTransporter* aTransporter,
                  TBtPortEntity& aEntity );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Starts dialup service advertisement
     *
     * @since S60 3.2
     * @param aInUse ETrue is returned if all RFCOMM channels in use.
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StartServiceAdvertisement( TBool& aInUse );

    /**
     * Stops dialup service advertisement
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt StopServiceAdvertisement();

    /**
     * Method which reserves local RFCOMM channel (from possible channels 1-30)
     * and returns it to client.
     *
     * @since S60 3.2
     * @param aSocketServer Handle to connected socket server.
     * @param aListenSocket Socket which is initiated for listening.
     * @param aChannelNum RFCOMM channel which is reserved for listening.
     * @param aInUse ETrue is returned if all RFCOMM channels in use.
     * @return KErrNone if OK, else value indicating error situation.
     */
    TInt ReserveLocalChannel( RSocketServ& aSocketServ,
                              RSocket& aListenSocket,
                              TUint& aChannelNum,
                              TBool& aInUse );

    /**
     * Tries to bind to a fixed port and if that fails with
     * KRfcommPassiveAutoBind. This is for spec breaking solutions like the
     * OSX Leopard.
     *
     * @since S60 5.0
     * @param aListenSocket Listen socket for Bind()
     * @param aSockAddr Address for the socket
     *                 (must be set for KRfcommPassiveAutoBind)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoExtendedBind( RSocket& aListenSocket, TRfcommSockAddr& aSockAddr );

// from base class CActive

    /**
     * From CActive.
     * Called when a service is requested via BT.
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Cancel current activity.
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();

// from base class MDunServAdvMon

    /**
     * From MDunServAdvMon.
     * Gets called when advertisement status changes to start
     *
     * @since S60 5.0
     * @param aCreation ETrue if channel creation
     *                  EFalse if channel free
     * @return None
     */
    void NotifyAdvertisementStart( TBool aCreation );

    /**
     * From MDunServAdvMon.
     * Gets called when advertisement status changes to end
     *
     * @since S60 5.0
     * @return None
     */
    void NotifyAdvertisementEnd();

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
     * Transporter to use for network side communication
     * Not own.
     */
    CDunTransporter* iTransporter;

    /**
     * Data socket entity for actual data transfer
     * This will be valid after CDunBtListen reacts to new data in
     * listener socket
     */
    TBtPortEntity& iEntity;

    /**
     * Current state of listening: active or inactive
     */
    TBtListenState iListenState;

    /**
     * Listener socket that will listen for activity in RFComm channel
     */
    RSocket iListenSocket;

    /**
     * Socket server used to reserve RFComm channel and creating data socket
     */
    RSocketServ iSockServer;

    /**
     * BT engine's Bluetooth discovery functionality
     * Used for registering/unregistering SDP record
     * Own.
     */
    CBTEngDiscovery* iDiscovery;

    /**
     * Local BT channel number
     * Set to reserved RFComm channel and used to register SDP record
     */
    TUint iChannelNum;

    /**
     * Handle to the reserver SDP channel
     * Set when SDP record registered
     */
    TSdpServRecordHandle iSDPHandleDun;

    };

#endif  // C_CDUNBTLISTEN_H
