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
* Description:  DUN Infrared plugin
*
*/


#ifndef C_CDUNIRPLUGIN_H
#define C_CDUNIRPLUGIN_H

#include <e32base.h>
#include <c32comm.h>
#include "DunTransporter.h"
#include "DunPlugin.h"

/**
 *  DUN Infrared plugin's main class
 *  This class creates new transporter channel as soon as listening is
 *  required by DUN server
 *
 *  @lib dunir.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunIrPlugin ) : public CBase,
                                    public MDunLocalMediaPlugin,
                                    public MDunConnMon
    {

public:

    CDunIrPlugin();

    /**
    * Destructor.
    */
    virtual ~CDunIrPlugin();

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
    TInt ConstructListener();

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
     * Initializes one infrared port with role DCE
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt InitPort();

    /**
     * Allocates a channel
     *
     * @since S60 3.2
     * @return None
     */
    void AllocateChannelL();

    /**
     * Uninitializes this plugin
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Uninitialize();

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

private:  // Data

    /**
     * Callback to call when notification MDunServerCallback via to be made
     * Not own.
     */
    MDunServerCallback* iServer;

    /**
     * Communications server used to load IRCOMM and open IR port
     */
    RCommServ iCommServer;

    /**
     * RComm port for infrared local media side
     */
    RComm iIrPort;

    /**
     * Transporter to use for network side communication
     * Not own.
     */
    CDunTransporter* iTransporter;

    };

#endif  // C_CDUNIRPLUGIN_H
