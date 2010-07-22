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


#include "DunIrPlugin.h"
#include "DunUtils.h"
#include "DunDebug.h"

_LIT( KIrdaCsy,       "IRCOMM"    );
_LIT( KIrdaCsy0,      "IRCOMM::0" );
_LIT( KIrChannelName, "DUNIR::0"  );

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDunIrPlugin::CDunIrPlugin
// ---------------------------------------------------------------------------
//
CDunIrPlugin::CDunIrPlugin() :
    iServer( NULL ),
    iTransporter( NULL )
    {
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunIrPlugin::~CDunIrPlugin()
    {
    FTRACE(FPrint( _L( "CDunIrPlugin::~CDunIrPlugin()" ) ));
    Uninitialize();
    FTRACE(FPrint( _L( "CDunIrPlugin::~CDunIrPlugin() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Gets state of this plugin
// ---------------------------------------------------------------------------
//
TDunPluginState CDunIrPlugin::PluginState()
    {
    return iServer->GetPluginStateByUid( KDunIrPluginUid );
    }

// ---------------------------------------------------------------------------
// Constructs a listener object for this plugin
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::ConstructListener()
    {
    FTRACE(FPrint(_L("CDunIrPlugin::ConstructListenerL()")));
    if ( PluginState() != EDunStateLoaded )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::ConstructListenerL() (not ready) complete")));
        return KErrNotReady;
        }
    ReportStateChangeUp( EDunStateTryListen );
    TInt retTemp = InitPort();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::ConstructListenerL() (ERROR) complete")));
        return retTemp;
        }
    ReportStateChangeUp( EDunStateListening );
    ReportStateChangeUp( EDunStateTryChannel );
    TRAPD( retTrap, AllocateChannelL() );
    if ( retTrap != KErrNone )
        {
        iTransporter->FreeChannel( &iIrPort );
        FTRACE(FPrint(_L("CDunIrPlugin::ConstructListenerL() (trapped!) complete")));
        return retTrap;
        }
    ReportStateChangeUp( EDunStateChanneled );
    FTRACE(FPrint(_L("CDunIrPlugin::ConstructListenerL() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one more than the old state
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::ReportStateChangeUp( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeUp()")));
    TInt retTemp = iServer->NotifyPluginStateChangeUp( aPluginState,
                                                       KDunIrPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeUp() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeUp() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one less than the old state
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::ReportStateChangeDown( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeDown()")));
    TInt retTemp = iServer->NotifyPluginStateChangeDown( aPluginState,
                                                         KDunIrPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeDown() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunIrPlugin::ReportStateChangeDown() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Initializes one infrared port with role DCE
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::InitPort()
    {
    FTRACE(FPrint( _L( "CDunIrPlugin::InitPort()" ) ));
    TInt retTemp;
    retTemp = CDunUtils::ConnectCommsServer( iCommServer );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunIrPlugin::InitPort() (ERROR) complete" ) ));
        return retTemp;
        }
    retTemp = iCommServer.LoadCommModule( KIrdaCsy );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::InitPort() (load module) failed!") ));
        return retTemp;
        }
    if ( iIrPort.SubSessionHandle() )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::InitPort (already exists!) complete") ));
        return KErrAlreadyExists;
        }
    retTemp = iIrPort.Open( iCommServer,
                            KIrdaCsy0,
                            ECommExclusive,
                            ECommRoleDCE );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::InitPort() (open) failed!") ));
        return retTemp;
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Allocates a channel
// ---------------------------------------------------------------------------
//
void CDunIrPlugin::AllocateChannelL()
    {
    FTRACE(FPrint(_L("CDunIrPlugin::AllocateChannelL()")));
    HBufC8* channelName = HBufC8::NewMaxLC( KIrChannelName().Length() );
    TPtr8 channelNamePtr = channelName->Des();
    channelNamePtr.Copy( KIrChannelName );
    iTransporter->AllocateChannelL( &iIrPort,
                                    KDunIrPluginUid,
                                    channelNamePtr,
                                    ETrue );
    iTransporter->AddConnMonCallbackL( &iIrPort,
                                       this,
                                       EDunReaderUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &iIrPort,
                                       this,
                                       EDunWriterUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &iIrPort,
                                       this,
                                       EDunReaderDownstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &iIrPort,
                                       this,
                                       EDunWriterDownstream,
                                       EFalse );
    iTransporter->IssueTransferRequestsL( &iIrPort );
    CleanupStack::PopAndDestroy( channelName );
    FTRACE(FPrint(_L("CDunIrPlugin::AllocateChannelL() complete")));
    }

// ---------------------------------------------------------------------------
// Uninitializes this plugin
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::Uninitialize()
    {
    FTRACE(FPrint(_L("CDunIrPlugin::Uninitialize()" )));
    ReportStateChangeDown( EDunStateTryUninitialize );
    if ( iIrPort.SubSessionHandle() )
        {
        iTransporter->FreeChannel( &iIrPort );
        iIrPort.SetSignals( 0, KSignalDCEOutputs );
        iIrPort.Close();
        }
    if ( iCommServer.Handle() )
        {
        iCommServer.UnloadCommModule( KIrdaCsy );
        iCommServer.Close();
        }
    ReportStateChangeUp( EDunStateUninitialized );
    ReportStateChangeUp( EDunStateTryLoad );
    ReportStateChangeUp( EDunStateLoaded );
    FTRACE(FPrint(_L("CDunIrPlugin::Uninitialize() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// CDunIrPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CDunIrPlugin::ConstructL( MDunServerCallback* aServer,
                               CDunTransporter* aTransporter )
    {
    FTRACE(FPrint(_L("CDunIrPlugin::ConstructL()")));
    if ( !aServer || !aTransporter )
        {
        FTRACE(FPrint(_L("CDunIrPlugin::ConstructL() not initialized!")));
        User::Leave( KErrGeneral );
        }
    iServer = aServer;
    iTransporter = aTransporter;
    FTRACE(FPrint(_L("CDunIrPlugin::ConstructL() complete")));
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server changes a plugin's state
// ---------------------------------------------------------------------------
//
TInt CDunIrPlugin::NotifyServerStateChange( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange()")));
    TInt retTemp;
    switch ( aPluginState )
        {
        case EDunStateTryListen:
            if ( PluginState() != EDunStateLoaded )
                {
                FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Change to listening mode
            retTemp = ConstructListener();
            if ( retTemp != KErrNone )
                {
                FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() (ERROR) complete (%d)"), retTemp));
                return retTemp;
                }
            break;
        case EDunStateTryUninitialize:
            if ( PluginState() == EDunStateUninitialized )
                {
                FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Uninitialize
            retTemp = Uninitialize();
            if ( retTemp != KErrNone )
                {
                FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() (ERROR) complete (%d)"), retTemp));
                return retTemp;
                }
            break;
        default:
            FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() (unknown state) complete")));
            return KErrNotSupported;
        }
    FTRACE(FPrint(_L("CDunIrPlugin::NotifyServerStateChange() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server needs to know the active connection
// ---------------------------------------------------------------------------
//
TConnId CDunIrPlugin::ActiveConnection()
    {
    FTRACE(FPrint(_L("CDunIrPlugin::ActiveConnection()")));
    FTRACE(FPrint(_L("CDunIrPlugin::ActiveConnection() (not found) complete")));
    return NULL;
    }

// ---------------------------------------------------------------------------
// From class MDunConnMon.
// Gets called when line status changes or when any type of error is detected
// ---------------------------------------------------------------------------
//
void CDunIrPlugin::NotifyProgressChangeL(
    TConnId aConnId,
    TDunConnectionReason /*aConnReason*/ )
    {
    FTRACE(FPrint( _L( "CDunIrPlugin::NotifyProgressChangeL()" ) ));
    RComm* irConn = static_cast<RComm*>( aConnId );
    if ( &iIrPort != irConn )
        {
        FTRACE(FPrint( _L( "CDunIrPlugin::NotifyProgressChangeL() (not found) complete")) );
        User::Leave( KErrNotFound );
        }
    // Now indications are down indications from local media side
    FTRACE(FPrint( _L( "CDunIrPlugin::NotifyProgressChangeL() restart plugin" ) ));
    iServer->NotifyPluginRestart( KDunIrPluginUid );
    FTRACE(FPrint( _L( "CDunIrPlugin::NotifyProgressChangeL() complete")) );
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLocalPluginL implements factory construction for
// the class CDunIrPlugin.
// The function is exported at ordinal 1.
// Returns: Pointer: The new instance of CDunIrPlugin
// ---------------------------------------------------------------------------
//
EXPORT_C MDunLocalMediaPlugin* NewLocalPluginL()
    {
    return new (ELeave) CDunIrPlugin;
    }
