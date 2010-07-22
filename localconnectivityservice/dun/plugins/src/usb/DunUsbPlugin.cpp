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
* Description:  DUN USB plugin
*
*/


#include <c32comm.h>
#include <d32usbc.h>
#include "DunPlugin.h"
#include "DunUtils.h"
#include "DunUsbPlugin.h"
#include "DunDebug.h"

_LIT( KUsbCsyName,     "ECACM"    );
_LIT( KUsbPortName,    "ACM"      );
_LIT( KUsbPortPort,    "::"       );
_LIT( KUsbLddName,     "EUSBC"    );
_LIT( KUsbChannelName, "DUNUSB::" );

const TInt   KCharactersInTInt      = 10;    // For "2147483648"
const TUint8 KDefaultAcmProtocolNum = 0x01;  // Hayes compatible modem

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDunUsbPlugin::CDunUsbPlugin
// ---------------------------------------------------------------------------
//
CDunUsbPlugin::CDunUsbPlugin() :
    iServer( NULL ),
    iUsbListen( NULL ),
    iUsbConfig( NULL ),
    iTransporter( NULL )
    {
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunUsbPlugin::~CDunUsbPlugin()
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::~CDunUsbPlugin()" ) ));
    Uninitialize();
    FTRACE(FPrint( _L( "CDunUsbPlugin::~CDunUsbPlugin() complete" ) ));
    }

// ---------------------------------------------------------------------------
// State of this plugin
// ---------------------------------------------------------------------------
//
TDunPluginState CDunUsbPlugin::PluginState()
    {
    return iServer->GetPluginStateByUid( KDunUsbPluginUid );
    }

// ---------------------------------------------------------------------------
// Constructs a listener object for this plugin
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::ConstructListenerL()
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::ConstructListenerL()")));
    if ( PluginState() != EDunStateLoaded )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::ConstructListenerL() (not ready) complete")));
        User::Leave( KErrNotReady );
        }
    ReportStateChangeUp( EDunStateTryListen );
    if ( iUsbListen )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::ConstructListenerL() (already exists) complete")));
        User::Leave( KErrAlreadyExists );
        }
    InitUsbL();
    CDunUsbListen* listen = CDunUsbListen::NewL( iServer, this, iUsbServer );
    CleanupStack::PushL( listen );
    TInt retTemp = listen->IssueRequestL();
    CleanupStack::Pop( listen );
    iUsbListen = listen;
    // Here return value of KErrAlreadyExists means the device is already
    // configured
    // In this case we have to switch directly to channeled mode
    ReportStateChangeUp( EDunStateListening );
    if ( retTemp == KErrAlreadyExists )
        {
        TBool noFreeChans = EFalse;
        // noFreeChans will be omitted (not needed to set to RComm)
        NotifyChannelAllocate( noFreeChans );  // Create channel and change state
        }
    FTRACE(FPrint(_L("CDunUsbPlugin::ConstructListenerL() complete")));
    }

// ---------------------------------------------------------------------------
// Initializes USB by loading LDD
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::InitUsbL()
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitUsbL()") ));
    // Load Logical device driver for USB
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitUsbL() LoadLogicalDevice") ));
    TInt retTemp = User::LoadLogicalDevice( KUsbLddName );
    if ( retTemp!=KErrAlreadyExists && retTemp!=KErrNone )
        {
        User::Leave( retTemp );
        }
    // Connect to the USB Manager server
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitUsbL() Connect()") ));
    User::LeaveIfError( iUsbServer.Connect() );
    // Create USB configuration accessor and listener
    iUsbConfig = CDunUsbConfig::NewL( iServer, this, KDefaultAcmProtocolNum );
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitUsbL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes all usable USB ports for DUN
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::InitPorts()
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitPorts() (num=%d)" ), iUsbPorts.Count()));
    TInt i;
    TInt retTemp = CreateAllPorts();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::InitPorts() (ERROR) complete" )));
        return retTemp;
        }
    TInt count = iUsbPorts.Count();
    for ( i=0; i<count; i++ )
        {
        retTemp = InitOnePort( &iUsbPorts[i] );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunUsbPlugin::InitPorts() (not ready) complete" )));
            return KErrGeneral;
            }
        }
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitPorts() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Creates empty usable USB ports that can be used by DUN
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::CreateAllPorts()
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::CreateAllPorts()" ) ));
    // Connect to comms server
    TInt retTemp = CDunUtils::ConnectCommsServer( iCommServer );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::CreateAllPorts() (ERROR) complete" )));
        return retTemp;
        }
    // Load communications module
    retTemp = iCommServer.LoadCommModule( KUsbCsyName );
    if ( retTemp!=KErrNone && retTemp!=KErrAlreadyExists )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::CreateAllPorts() (LoadCommModule failed) complete" ) ));
        return retTemp;
        }
    // Stop waiter now (just to be sure) before explicitly fetching port data
    iUsbConfig->Stop();
    // Find out what ports can be supported and append them to array
    TBool validConfig;
    TUsbPortEntity entity;
    for ( TInt i=0;; i++ )
        {
        retTemp = iUsbConfig->GetConfigValidityByIndex( i, validConfig );
        if ( retTemp != KErrNone )
            {
            break;
            }
        if ( !validConfig )
            {
            continue;
            }
        // Valid config found, append it to array
        entity.iPortNum = i;
        retTemp = iUsbPorts.Append( entity );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunUsbPlugin::CreateAllPorts() (append failed!) complete" ) ));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L( "CDunUsbPlugin::CreateAllPorts() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Initializes one USB port for DUN
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::InitOnePort( TUsbPortEntity* aEntity )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitOnePort()" )));
    if ( !aEntity )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::InitOnePort() (not initialized!) complete" )));
        return KErrGeneral;
        }
    TInt retTemp;
    TName portName;
    portName.Copy( KUsbPortName );
    portName.Append( KUsbPortPort );
    portName.AppendNum( aEntity->iPortNum );
    retTemp = aEntity->iUsbPort.Open( iCommServer,
                                      portName,
                                      ECommExclusive,
                                      ECommRoleDCE );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::InitOnePort() Open failed (%d)" ), retTemp));
        return retTemp;
        }
    TRAPD( retTrap, SetChannelL(aEntity) );
    if ( retTrap != KErrNone )
        {
        iTransporter->FreeChannel( &aEntity->iUsbPort );
        FTRACE(FPrint( _L( "CDunUsbPlugin::InitOnePort() (trapped!) complete" )));
        return retTrap;
        }
    FTRACE(FPrint( _L( "CDunUsbPlugin::InitOnePort() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets channel for one USB port
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::SetChannelL( TUsbPortEntity* aEntity )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::SetChannelL()" )));
    if ( !aEntity )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::SetChannelL() (not initialized!) complete" )));
        User::Leave( KErrGeneral );
        }
    HBufC8* channelName = HBufC8::NewMaxLC( KUsbChannelName().Length() +
                          KCharactersInTInt );
    TPtr8 channelNamePtr = channelName->Des();
    channelNamePtr.Copy( KUsbChannelName );
    channelNamePtr.AppendNum( aEntity->iPortNum );
    iTransporter->AllocateChannelL( &aEntity->iUsbPort,
                                    KDunUsbPluginUid,
                                    channelNamePtr,
                                    ETrue,
                                    this );
    AddSkippedErrorL( KErrUsbInterfaceChange, &aEntity->iUsbPort );
    iTransporter->AddConnMonCallbackL( &aEntity->iUsbPort,
                                       this,
                                       EDunReaderUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &aEntity->iUsbPort,
                                       this,
                                       EDunWriterUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &aEntity->iUsbPort,
                                       this,
                                       EDunReaderDownstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( &aEntity->iUsbPort,
                                       this,
                                       EDunWriterDownstream,
                                       EFalse );
    iTransporter->IssueTransferRequestsL( &aEntity->iUsbPort );
    CleanupStack::PopAndDestroy( channelName );
    FTRACE(FPrint( _L( "CDunUsbPlugin::SetChannelL() complete" )));
    }

// ---------------------------------------------------------------------------
// Adds skipped error code to Transporter
// The skipped error is added to local media's read and write operations
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::AddSkippedErrorL( TInt aError, RComm* aComm )
    {
    iTransporter->AddSkippedErrorL( aError, aComm, EDunReaderUpstream );
    iTransporter->AddSkippedErrorL( aError, aComm, EDunWriterDownstream );
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one more than the old state
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::ReportStateChangeUp( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeUp()")));
    TInt retTemp = iServer->NotifyPluginStateChangeUp( aPluginState,
                                                       KDunUsbPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeUp() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeUp() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one less than the old state
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::ReportStateChangeDown( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeDown()")));
    TInt retTemp = iServer->NotifyPluginStateChangeDown( aPluginState,
                                                         KDunUsbPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeDown() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunUsbPlugin::ReportStateChangeDown() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Frees existing channels
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::FreeChannels()
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::FreeChannels()")));
    if ( PluginState() != EDunStateTryUninitialize )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::FreeChannels() (not ready) complete" ) ));
        return KErrNotReady;
        }
    TInt i;
    TInt count = iUsbPorts.Count();
    for ( i=0; i<count; i++ )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::FreeChannels() i=%d" ), i));
        if ( iUsbPorts[i].iUsbPort.SubSessionHandle() )
            {
            iTransporter->FreeChannel( &iUsbPorts[i].iUsbPort );
            iUsbPorts[i].iUsbPort.SetSignals( 0, KSignalDCEOutputs );
            iUsbPorts[i].iUsbPort.Close();
            }
        }
    iUsbPorts.Close();
    if ( iCommServer.Handle() != KNullHandle )
        {
        iCommServer.UnloadCommModule( KUsbCsyName );
        iCommServer.Close();
        }
    FTRACE(FPrint(_L("CDunUsbPlugin::FreeChannels() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Uninitializes this plugin
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::Uninitialize()
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::Uninitialize()" ) ));
    ReportStateChangeDown( EDunStateTryUninitialize );
    // Free channel(s), ignore errors
    FreeChannels();
    delete iUsbListen;  // delete before iUsbServer close
    iUsbListen = NULL;
    delete iUsbConfig;
    iUsbConfig = NULL;
    if ( iUsbServer.Handle() != KNullHandle )
        {
        iUsbServer.Close();
        }
    User::FreeLogicalDevice( KUsbLddName );
    ReportStateChangeUp( EDunStateUninitialized );
    ReportStateChangeUp( EDunStateTryLoad );
    ReportStateChangeUp( EDunStateLoaded );
    FTRACE(FPrint( _L( "CDunUsbPlugin::Uninitialize() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Gets port's index and entity by connection ID
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::GetEntityByConnId( TConnId aConnId,
                                       TUsbPortEntity*& aEntity )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByConnId()")) );
    TInt i;
    TInt count = iUsbPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( &iUsbPorts[i].iUsbPort == aConnId )
            {
            aEntity = &iUsbPorts[i];
            FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByConnId() complete")) );
            return i;
            }
        }
    aEntity = NULL;
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByConnId() (not found) complete")) );
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Gets port's entity by port number
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::GetEntityByPortNumber( TInt aPortNum,
                                           TUsbPortEntity*& aEntity )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByPortNumber()")) );
    TInt i;
    TInt count = iUsbPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iUsbPorts[i].iPortNum == aPortNum )
            {
            aEntity = &iUsbPorts[i];
            FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByPortNumber() complete")) );
            return i;
            }
        }
    aEntity = NULL;
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetEntityByPortNumber() (not found) complete")) );
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Gets first free port's index and entity
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::GetFirstFreeEntity( TUsbPortEntity*& aEntity )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetFirstFreeEntity()")) );
    TInt i;
    TInt count = iUsbPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iUsbPorts[i].iPortNum == KErrNotFound &&
             !iUsbPorts[i].iUsbPort.SubSessionHandle() )
            {
            aEntity = &iUsbPorts[i];
            FTRACE(FPrint( _L( "CDunUsbPlugin::GetFirstFreeEntity() complete")) );
            return i;
            }
        }
    aEntity = NULL;
    FTRACE(FPrint( _L( "CDunUsbPlugin::GetFirstFreeEntity() (not found) complete")) );
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// CDunUsbPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::ConstructL( MDunServerCallback* aServer,
                                CDunTransporter* aTransporter )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::ConstructL()" ) ));
    if ( !aServer || !aTransporter )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::ConstructL() not initialized!" ) ));
        User::Leave( KErrGeneral );
        }
    iServer = aServer;
    iTransporter = aTransporter;
    FTRACE(FPrint( _L( "CDunUsbPlugin::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server changes a plugin's state
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyServerStateChange( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange()")));
    TInt retTemp;
    switch ( aPluginState )
        {
        case EDunStateTryListen:
            if ( PluginState() != EDunStateLoaded )
                {
                FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Change to listening mode
            TRAPD( retTrap, ConstructListenerL() );
            if ( retTrap != KErrNone )
                {
                FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() (ERROR) complete (%d)"), retTrap));
                return retTrap;
                }
            break;
        case EDunStateTryUninitialize:
            if ( PluginState() == EDunStateUninitialized )
                {
                FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Uninitialize
            retTemp = Uninitialize();
            if ( retTemp != KErrNone )
                {
                FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() (not ready) complete (%d)"), retTemp));
                return retTemp;
                }
            break;
        default:
            FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() (unknown state) complete")));
            return KErrNotSupported;
        }
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyServerStateChange() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server needs to know the active connection
// ---------------------------------------------------------------------------
//
TConnId CDunUsbPlugin::ActiveConnection()
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::ActiveConnection()")));
    FTRACE(FPrint(_L("CDunUsbPlugin::ActiveConnection() (not found) complete")));
    return NULL;
    }

// ---------------------------------------------------------------------------
// From class MDunListenCallback.
// Gets called when new channel must be created
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyChannelAllocate( TBool& /*aNoFreeChans*/ )
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelAllocate()")));
    if ( PluginState() != EDunStateListening )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelAllocate() (not ready) complete")));
        return KErrNotReady;
        }
    ReportStateChangeUp( EDunStateTryChannel );
    TInt retTemp = InitPorts();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelAllocate() (ERROR) complete")));
        return retTemp;
        }
    // Channels allocated so start to wait for possible port config change
    retTemp = iUsbConfig->IssueRequest();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelAllocate() (ERROR) complete")));
        return retTemp;
        }
    iShutdown = EFalse;
    ReportStateChangeUp( EDunStateChanneled );
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelAllocate() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunListenCallback.
// Gets called when an existing channel must be freed
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyChannelFree()
    {
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelFree()")));
    if ( PluginState() != EDunStateChanneled )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelFree() (not ready) complete")));
        return KErrNotReady;
        }
    // Cable removed or PC sleep, hibernate or reset.
    // Just remove channels.
    ReportStateChangeDown( EDunStateTryUninitialize );
    TInt retTemp = FreeChannels();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelFree() (ERROR) complete")));
        return retTemp;
        }
    ReportStateChangeUp( EDunStateUninitialized );
    ReportStateChangeUp( EDunStateTryLoad );
    ReportStateChangeUp( EDunStateLoaded );
    ReportStateChangeUp( EDunStateTryListen );
    ReportStateChangeUp( EDunStateListening );
    FTRACE(FPrint(_L("CDunUsbPlugin::NotifyChannelFree() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunConnMon.
// Gets called when line status changes or when any type of error is detected
// ---------------------------------------------------------------------------
//
void CDunUsbPlugin::NotifyProgressChangeL(
    TConnId aConnId,
    TDunConnectionReason aConnReason )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL()")) );
    // Find matching failed ID
    TUsbPortEntity* foundEntity = NULL;
    TInt foundIndex = GetEntityByConnId( aConnId, foundEntity );
    if ( !foundEntity )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL() (not found) complete")) );
        User::Leave( KErrNotFound );
        }
    if ( aConnReason.iReasonType == EDunReasonTypeRW ||
         aConnReason.iReasonType == EDunReasonTypeRunL )
        {
        // The following check will make it possible for CDunUsbListen to react
        // to cable removal (<KErrUsbDriverBase) or to the case when personality
        // is changed from USBUI and LOCOD/USBWatcher is too slow to notify the
        // change (KErrAccessDenied).
        if ( aConnReason.iErrorCode < KErrUsbDriverBase ||
             aConnReason.iErrorCode == KErrAccessDenied )
            {
            FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL() (shutdown now) complete")) );
            iShutdown = ETrue;
            return;
            }
        }
    if ( iShutdown )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL() (shutdown) complete")) );
        return;
        }
    // Now indications are down indications from local media side
    if ( foundEntity->iUsbPort.SubSessionHandle() )
        {
        iTransporter->FreeChannel( &foundEntity->iUsbPort );
        foundEntity->iUsbPort.SetSignals( 0, KSignalDCEOutputs );
        foundEntity->iUsbPort.Close();
        }
    TInt retTemp = InitOnePort( foundEntity );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL() (ERROR) complete")) );
        User::Leave( KErrGeneral );
        }
    // Now this plugin was basically "restarted", but only for one
    // channel. Because transporter has channels with waiters, notify
    // server to reopen queued plugin(s)
    iServer->NotifyPluginReopenRequest();
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyProgressChangeL() complete")) );
    }

// ---------------------------------------------------------------------------
// From class MDunBufferCorrection.
// Gets called when request to change local media's buffer size
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyBufferCorrection( TInt aLength )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyBufferCorrection()")) );
    /*
     * This method modifies the default buffer length to match the maximum value
     * used by "CanHandleZLP=0" configuration option. This length is nearest length
     * divisible by 64 - 1. With slow USB compared to high speed HSDPA some products
     * can start to collect data to Dataport which results in full packet writes to
     * USB. However, the default full packet must not be divisible by 64, which results
     * in the ACM to send the full packet to LDD plus one extra packet with one byte
     * (disabling interrupts for a long time with current non-DMA USB driver).
     */
    TInt newLength = aLength;
    if ( newLength % 64 == 0 )
        {
        newLength = ((aLength >> 6) << 6) - 1;
        }
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyBufferCorrection() complete")) );
    return newLength;
    }

// ---------------------------------------------------------------------------
// From class MDunUsbConfig.
// Gets called when one or more ACM configurations are added
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyConfigAddition( TInt aIndex )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigAddition()")) );
    // Configuration added so aIndex is the port number to be added to array.
    // This port number must not already exist in the array.
    TUsbPortEntity* foundEntity = NULL;
    TInt foundIndex = GetEntityByPortNumber( aIndex, foundEntity );
    if ( foundEntity )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigAddition() (already exists) complete")) );
        return KErrAlreadyExists;
        }
    // Now either find port with KErrNotFound set as port number or if that is
    // not found then try to append to array
    foundIndex = GetFirstFreeEntity( foundEntity );
    if ( !foundEntity )  // free not found so add new
        {
        // Now append the new port to array
        TUsbPortEntity newEntity;
        newEntity.iPortNum = aIndex;
        TInt retTemp = iUsbPorts.Append( newEntity );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigAddition() (append failed!) complete")) );
            return KErrGeneral;
            }
        // entity not valid here so set now
        foundEntity = &iUsbPorts[iUsbPorts.Count()-1];
        }
    else  // free found so change array
        {
        foundEntity->iPortNum = aIndex;
        }
    // Open port and create channel for it
    TInt retTemp = InitOnePort( foundEntity );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigAddition() (ERROR) complete" )));
        return KErrGeneral;
        }
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigAddition() complete")) );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunUsbConfig.
// Gets called when one or more ACM configurations are removed
// ---------------------------------------------------------------------------
//
TInt CDunUsbPlugin::NotifyConfigRemoval( TInt aIndex )
    {
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigRemoval()")) );
    // Configuration removed so aIndex is the port number to be added to array.
    // This port number must already exist in the array.
    TUsbPortEntity* foundEntity = NULL;
    TInt foundIndex = GetEntityByPortNumber( aIndex, foundEntity );
    if ( !foundEntity )
        {
        FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigRemoval() (not found) complete")) );
        return KErrNotFound;
        }
    // Now free channel and mark as unused
    iTransporter->FreeChannel( &foundEntity->iUsbPort );
    foundEntity->iUsbPort.SetSignals( 0, KSignalDCEOutputs );
    foundEntity->iUsbPort.Close();
    iUsbPorts[foundIndex].iPortNum = KErrNotFound;
    FTRACE(FPrint( _L( "CDunUsbPlugin::NotifyConfigRemoval() complete")) );
    return KErrNone;
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLocalPluginL implements factory construction for
// the class CDunUsbPlugin.
// The function is exported at ordinal 1.
// Returns: Pointer: The new instance of CDunUsbPlugin
// ---------------------------------------------------------------------------
//
EXPORT_C MDunLocalMediaPlugin* NewLocalPluginL()
    {
    return new (ELeave) CDunUsbPlugin;
    }
