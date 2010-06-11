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


#include <bt_sock.h>
#include <c32comm.h>
#include "DunPlugin.h"
#include "DunBtListen.h"
#include "DunBtPlugin.h"
#include "DunDebug.h"
#include "DunTransporter.h"

_LIT( KBtChannelName, "DUNBT::"  );

const TInt KCharactersInTInt = 10;  // For "2147483648"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDunBtPlugin::CDunBtPlugin()
// ---------------------------------------------------------------------------
//
CDunBtPlugin::CDunBtPlugin() :
    iServer( NULL ),
    iBTListen( NULL ),
    iTransporter( NULL )
    {
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunBtPlugin::~CDunBtPlugin()
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::~CDunBtPlugin()" ) ));
    Uninitialize();
    FTRACE(FPrint( _L( "CDunBtPlugin::~CDunBtPlugin() complete" ) ));
    }

// ---------------------------------------------------------------------------
// State of this plugin
// ---------------------------------------------------------------------------
//
TDunPluginState CDunBtPlugin::PluginState()
    {
    return iServer->GetPluginStateByUid( KDunBtPluginUid );
    }

// ---------------------------------------------------------------------------
// Constructs a listener object for this plugin
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::ConstructListenerL()
    {
    FTRACE(FPrint(_L("CDunBtPlugin::ConstructListenerL()")));
    if ( PluginState() != EDunStateLoaded )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ConstructListenerL() (not ready) complete")));
        User::Leave( KErrNotReady );
        }
    ReportStateChangeUp( EDunStateTryListen );
    if ( iBTListen )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ConstructListenerL() (already exists) complete")));
        User::Leave( KErrAlreadyExists );
        }
    CDunBtListen* listen = CDunBtListen::NewL( iServer,
                                               this,
                                               iTransporter,
                                               iEntity );
    CleanupStack::PushL( listen );
    listen->IssueRequestL();
    CleanupStack::Pop( listen );
    iBTListen = listen;
    ReportStateChangeUp( EDunStateListening );
    FTRACE(FPrint(_L("CDunBtPlugin::ConstructListenerL() complete")));
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one more than the old state
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::ReportStateChangeUp( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeUp()")));
    TInt retTemp = iServer->NotifyPluginStateChangeUp( aPluginState,
                                                       KDunBtPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeUp() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeUp() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets new state
// New state must be one less than the old state
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::ReportStateChangeDown( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeDown()")));
    TInt retTemp = iServer->NotifyPluginStateChangeDown( aPluginState,
                                                         KDunBtPluginUid );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeDown() (ERROR) complete")));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunBtPlugin::ReportStateChangeDown() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Allocates a free channel
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::AllocateChannelL( TBool& aNoFreeChans,
                                     TBtCleanupInfo& aCleanupInfo )
    {
    FTRACE(FPrint(_L("CDunBtPlugin::AllocateChannelL()")));
    // iDataSocket has new data socket information so copy it to iBTPorts
    TBtPortEntity* foundEntity = NULL;
    TInt foundIndex = GetFirstFreePort( foundEntity );
    if ( !foundEntity )  // free not found so add new
        {
        TBtPortEntity newEntity;
        iBTPorts.AppendL( newEntity );
        aCleanupInfo.iNewEntity = ETrue;
        aCleanupInfo.iEntityIndex = iBTPorts.Count() - 1;
        foundEntity = &iBTPorts[ aCleanupInfo.iEntityIndex ];
        }
    else  // free found so change array
        {
        aCleanupInfo.iNewEntity = EFalse;
        aCleanupInfo.iEntityIndex = foundIndex;
        foundEntity = &iBTPorts[ foundIndex ];
        }
    foundEntity->iChannelNum = iEntity.iChannelNum;
    foundEntity->iBTPort = iEntity.iBTPort;
    RSocket* socket = &foundEntity->iBTPort;
    HBufC8* channelName = HBufC8::NewMaxLC( KBtChannelName().Length() +
                          KCharactersInTInt );
    TPtr8 channelNamePtr = channelName->Des();
    channelNamePtr.Copy( KBtChannelName );
    channelNamePtr.AppendNum( iEntity.iChannelNum );
    iTransporter->AllocateChannelL( socket,
                                    KDunBtPluginUid,
                                    channelNamePtr,
                                    EFalse,
                                    aNoFreeChans );
    iTransporter->AddConnMonCallbackL( socket,
                                       this,
                                       EDunReaderUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( socket,
                                       this,
                                       EDunWriterUpstream,
                                       EFalse );
    iTransporter->AddConnMonCallbackL( socket,
                                       this,
                                       EDunReaderDownstream,
                                       ETrue );
    iTransporter->AddConnMonCallbackL( socket,
                                       this,
                                       EDunWriterDownstream,
                                       EFalse );
    iTransporter->IssueTransferRequestsL( socket );
    CleanupStack::PopAndDestroy( channelName );
    FTRACE(FPrint(_L("CDunBtPlugin::AllocateChannelL() complete")));
    }

// ---------------------------------------------------------------------------
// Frees an existing channel
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::FreeChannels()
    {
    FTRACE(FPrint(_L("CDunBtPlugin::FreeChannels()")));
    if ( PluginState() != EDunStateTryUninitialize )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::FreeChannels() (not ready) complete")));
        return KErrNotReady;
        }
    TInt i;
    TInt count = iBTPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iBTPorts[i].iBTPort.SubSessionHandle() )
            {
            iTransporter->FreeChannel( &iBTPorts[i].iBTPort );
            iBTPorts[i].iBTPort.Close();
            // All channels freed and this is for Uninitialize() so don't touch
            // advertisement monitor here!
            }
        iBTPorts[i].iChannelNum = KErrNotFound;
        }
    iBTPorts.Close();
    FTRACE(FPrint(_L("CDunBtPlugin::FreeChannels() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Uninitializes this plugin
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::Uninitialize()
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::Uninitialize()" ) ));
    ReportStateChangeDown( EDunStateTryUninitialize );
    // Free channels (ignore errors)
    FreeChannels();
    // Delete listening object (also advertisement monitor)
    delete iBTListen;
    iBTListen = NULL;
    // Set state back to loaded
    ReportStateChangeUp( EDunStateUninitialized );
    ReportStateChangeUp( EDunStateTryLoad );
    ReportStateChangeUp( EDunStateLoaded );
    FTRACE(FPrint( _L( "CDunBtPlugin::Uninitialize() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Gets port's index and entity by connection ID
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::GetPortByConnId( TConnId aConnId, TBtPortEntity*& aEntity )
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::GetPortByConnId()")) );
    TInt i;
    TInt count = iBTPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( &iBTPorts[i].iBTPort == aConnId )
            {
            aEntity = &iBTPorts[i];
            FTRACE(FPrint( _L( "CDunBtPlugin::GetPortByConnId() complete")) );
            return i;
            }
        }
    aEntity = NULL;
    FTRACE(FPrint( _L( "CDunBtPlugin::GetPortByConnId() (not found) complete")) );
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Gets first free port's index and entity
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::GetFirstFreePort( TBtPortEntity*& aEntity )
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::GetFirstFreePort()")) );
    TInt i;
    TInt count = iBTPorts.Count();
    for ( i=0; i<count; i++ )
        {
        if ( !iBTPorts[i].iBTPort.SubSessionHandle() )
            {
            aEntity = &iBTPorts[i];
            FTRACE(FPrint( _L( "CDunBtPlugin::GetFirstFreePort() complete")) );
            return i;
            }
        }
    aEntity = NULL;
    FTRACE(FPrint( _L( "CDunBtPlugin::GetFirstFreePort() (not found) complete")) );
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Sets modem's MSC (Modem Status Command)
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::SetRFCOMMStatusCommand( TBtPortEntity& aEntity,
                                           TUint8 aSignal,
                                           TBool aSignalOn )
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::SetRFCOMMStatusCommand()" ) ));
    // Get existing Modem Status Command (MSC)
    // Ref.: 3GPP TS 07.10 V7.2.0 (2002-03)
    // Table 6,7, (5.4.6.3.7)
    TUint8 modemStatus = 0;
    TPckgBuf<TUint8> pkg( modemStatus );
    TInt retTemp = aEntity.iBTPort.GetOpt( KRFCOMMLocalModemStatus,
                                           KSolBtRFCOMM,
                                           pkg );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtPlugin::SetRFCOMMStatusCommand() (GetOpt failed!) complete" ) ));
        return retTemp;
        }
    modemStatus = pkg();
    FTRACE(FPrint( _L( "CDunBtPlugin::SetRFCOMMStatusCommand() signals are: 0x%02X" ), modemStatus));
    TBool changed = EFalse;
    TUint8 signal = modemStatus & aSignal;
    if ( aSignalOn )
        {
        if ( !signal )
            {
            modemStatus |= aSignal;
            changed = ETrue;
            }
        }
    else
        {
        if ( signal )
            {
            modemStatus &= ( ~aSignal );
            changed = ETrue;
            }
        }
    if ( changed )
        {
        pkg = modemStatus;
        retTemp = aEntity.iBTPort.SetOpt( KRFCOMMLocalModemStatus,
                                          KSolBtRFCOMM,
                                          pkg );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunBtPlugin::SetRFCOMMStatusCommand() (SetOpt failed!) complete" ) ));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L( "CDunBtPlugin::SetRFCOMMStatusCommand() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Manages advertiser for channel free operation
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::ManageAdvertiserFreeOperationL()
    {
    FTRACE(FPrint(_L("CDunBtPlugin::ManageAdvertiserFreeOperationL()")));
    TInt numOfChans = iTransporter->NumberOfAllocatedChannels();
    // Remove of last CDunTransporter channel removes also the
    // advertisement monitor so set it now if necessary
    if ( numOfChans == 0 )
        {
        iTransporter->SetAdvertisementMonitorL( KDunBtPluginUid, iBTListen );
        }
    FTRACE(FPrint(_L("CDunBtPlugin::ManageAdvertiserFreeOperationL() complete")));
    }

// ---------------------------------------------------------------------------
// Cleans partial created channel data based on TATExtCleanupInfo
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::CleanPartialChanneldata( TBtCleanupInfo& aCleanupInfo )
    {
    FTRACE(FPrint(_L("CDunBtPlugin::CleanPartialChanneldata()")));
    RSocket* socket = &iBTPorts[aCleanupInfo.iEntityIndex].iBTPort;
    iTransporter->FreeChannel( socket );
    iBTPorts[aCleanupInfo.iEntityIndex].iChannelNum = KErrNotFound;
    socket->Close();
    if ( aCleanupInfo.iNewEntity )
        {
        iBTPorts.Remove( aCleanupInfo.iEntityIndex );
        }
    FTRACE(FPrint(_L("CDunBtPlugin::CleanPartialChanneldata() complete")));
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// CDunBtPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::ConstructL( MDunServerCallback* aServer,
                               CDunTransporter* aTransporter )
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::ConstructL()" ) ));
    if ( !aServer || !aTransporter )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ConstructL() not initialized!")));
        User::Leave( KErrBadHandle );
        }
    iServer = aServer;
    iTransporter = aTransporter;
    FTRACE(FPrint( _L( "CDunBtPlugin::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server changes a plugin's state
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::NotifyServerStateChange( TDunPluginState aPluginState )
    {
    FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange()")));
    TInt retTemp;
    switch ( aPluginState )
        {
        case EDunStateTryListen:
            if ( PluginState() != EDunStateLoaded )
                {
                FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Change to listening mode
            TRAPD( retTrap, ConstructListenerL() );
            if ( retTrap != KErrNone )
                {
                FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() (ERROR) complete (%d)"), retTrap));
                return retTrap;
                }
            break;
        case EDunStateTryUninitialize:
            if ( PluginState() == EDunStateUninitialized )
                {
                FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() (not ready) complete")));
                return KErrNotReady;
                }
            // Uninitialize
            retTemp = Uninitialize();
            if ( retTemp != KErrNone )
                {
                FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() (not ready) complete (%d)"), retTemp));
                return KErrNotReady;
                }
            break;
        default:
            FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() (unknown state) complete")));
            return KErrNotSupported;
        }
    FTRACE(FPrint(_L("CDunBtPlugin::NotifyServerStateChange() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunLocalMediaPlugin.
// Gets called when server needs to know the active connection
// ---------------------------------------------------------------------------
//
TConnId CDunBtPlugin::ActiveConnection()
    {
    FTRACE(FPrint(_L("CDunBtPlugin::ActiveConnection()")));
    if ( iBTPorts.Count() >= 1 )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::ActiveConnection() complete")));
        return &iBTPorts[0];
        }
    FTRACE(FPrint(_L("CDunBtPlugin::ActiveConnection() (not found) complete")));
    return NULL;
    }

// ---------------------------------------------------------------------------
// From class MDunListenCallback.
// Gets called when new channel must be created
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::NotifyChannelAllocate( TBool& aNoFreeChans )
    {
    // Now state can be either EDunStateListening (no channels) or
    // EDunStateChanneled (one or more channels). Support both states
    TDunPluginState startState = PluginState();
    if ( startState!=EDunStateListening && startState!=EDunStateChanneled )
        {
        FTRACE(FPrint(_L("CDunBtPlugin::NotifyChannelAllocate() (not ready) complete")));
        return KErrNotReady;
        }
    if ( startState == EDunStateListening )
        {
        ReportStateChangeUp( EDunStateTryChannel );
        }
    TBtCleanupInfo cleanupInfo;
    TRAPD( retTrap, AllocateChannelL(aNoFreeChans,cleanupInfo) );
    if ( retTrap != KErrNone )
        {
        CleanPartialChanneldata( cleanupInfo );
        FTRACE(FPrint(_L("CDunBtPlugin::NotifyChannelAllocate() (trapped!) complete")));
        return retTrap;
        }
    if ( startState == EDunStateListening )
        {
        ReportStateChangeUp( EDunStateChanneled );
        }
    FTRACE(FPrint(_L("CDunBtPlugin::NotifyChannelAllocate() complete")));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunListenCallback.
// Gets called when an existing channel must be freed
// ---------------------------------------------------------------------------
//
TInt CDunBtPlugin::NotifyChannelFree()
    {
    // No implementation needed here
    return KErrNotSupported;
    }

// ---------------------------------------------------------------------------
// From class MDunConnMon.
// Gets called when line status changes or when any type of error is detected
// ---------------------------------------------------------------------------
//
void CDunBtPlugin::NotifyProgressChangeL( TConnId aConnId,
                                          TDunConnectionReason aConnReason )
    {
    FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL()" ) ));
    // Find matching failed ID
    TBtPortEntity* foundEntity = NULL;
    TInt foundIndex = GetPortByConnId( aConnId, foundEntity );
    if ( !foundEntity )
        {
        FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL() (not found) complete")) );
        User::Leave( KErrNotFound );
        }
    if ( aConnReason.iReasonType == EDunReasonTypeSignal )
        {
        if ( aConnReason.iContext != EDunMediaContextNetwork )
            {
            // Should never come here as other signals are for RComm
            FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL() (ERROR) complete")) );
            User::Leave( KErrGeneral );
            }
        // Signal change detected on network side -> process change
        if ( aConnReason.iSignalType == KSignalDCD )
            {
            SetRFCOMMStatusCommand( *foundEntity,
                                    KModemSignalDV,
                                    aConnReason.iSignalHigh );
            FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL() DV changed")) );
            }
        else if ( aConnReason.iSignalType == KSignalRNG )
            {
            SetRFCOMMStatusCommand( *foundEntity,
                                    KModemSignalIC,
                                    aConnReason.iSignalHigh );
            FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL() IC changed")) );
            }
        // Omit other signals
        }
    else
        {
        // All other cases are down indications from local media side
        if ( foundEntity->iBTPort.SubSessionHandle() )
            {
            iTransporter->FreeChannel( &foundEntity->iBTPort );
            // CDunTransporter will notify the listener about advertisement
            // status change after FreeChannel() so no need to do
            // IssueRequestL() for CDunBtListen here after this.
            foundEntity->iBTPort.Close();
            }
        ManageAdvertiserFreeOperationL();
        // Now resources are freed so command server to reopen possibly
        // existing queued plugins
        iServer->NotifyPluginReopenRequest();
        }
    FTRACE(FPrint( _L( "CDunBtPlugin::NotifyProgressChangeL() complete")) );
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLocalPluginL implements factory construction for
// the class CDunBtPlugin.
// The function is exported at ordinal 1.
// ---------------------------------------------------------------------------
//
EXPORT_C MDunLocalMediaPlugin* NewLocalPluginL()
    {
    return new (ELeave) CDunBtPlugin;
    }
