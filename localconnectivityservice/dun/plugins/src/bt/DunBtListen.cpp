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


#include <btsdp.h>
#include <e32std.h>
#include <bt_sock.h>
#include <btengdiscovery.h>
#include "DunPlugin.h"
#include "DunBtListen.h"
#include "DunBtPlugin.h"
#include "DunDebug.h"

const TInt KListenQueSize   = 1;
const TInt KDunFixedChannel = 22;  // Hack/kludge for Apple Bug ID 6527598

//Service Class Bits supported by DUN
static const TUint16 KCoDDunServiceClass = EMajorServiceTelephony | EMajorServiceNetworking;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunBtListen* CDunBtListen::NewL( MDunServerCallback* aServer,
                                  MDunListenCallback* aParent,
                                  CDunTransporter* aTransporter,
                                  TBtPortEntity& aEntity )
    {
    CDunBtListen* self = new (ELeave) CDunBtListen( aServer,
                                                    aParent,
                                                    aTransporter,
                                                    aEntity );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunBtListen::~CDunBtListen()
    {
    FTRACE(FPrint( _L("CDunBtListen::~CDunBtListen()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunBtListen::~CDunBtListen() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunBtListen::ResetData()
    {
    // APIs affecting this:
    // IssueRequestL()
    Stop();
    StopServiceAdvertisement();
    // NewL()
    iTransporter->FreeAdvertisementMonitor( KDunBtPluginUid, this );
    delete iDiscovery;
    iDiscovery = NULL;
    if ( iSockServer.Handle() != KNullHandle )
        {
        iSockServer.Close();
        }
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Registers itself to SDP and BT manager, opens a socket
// and starts to listen it.
// ---------------------------------------------------------------------------
//
void CDunBtListen::IssueRequestL()
    {
    FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL()" ) ));

    if ( iListenState == EBtListenStateListening )
        {
        FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL() (already active) complete" ) ));
        User::Leave( KErrNotReady );
        }

    TBool advertise = iTransporter->AdvertisementStatus();
    if ( !advertise )
        {
        // Return silently here as CDunTransporter will notify later
        return;
        }

    TBool inUse = EFalse;
    TInt numOfChans = 0;
    TInt retTemp = StartServiceAdvertisement( inUse );
    if ( retTemp != KErrNone )
        {
        if ( inUse )
            {
            numOfChans = iTransporter->GetNumberOfAllocatedChannelsByUid(
                KDunBtPluginUid );
            if ( numOfChans == 0)
                {
                // No channels so parent can't reissue requests of this object
                // This is fatal case -> leave.
                // NOTE: To add full support for this case a poller (timer) is
                // needed that polls for free RFCOMM channel by given interval.
                User::Leave( retTemp );
                }
            // If in use and parent has channels then just fail silently.
            // Let this object to wait until parent finds new resources.
            FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL() complete" ) ));
            return;
            }
        FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL() (failed!) complete" ) ));
        User::Leave( retTemp );
        }

    // Not already active here so start listening
    // First open blank data socket
    retTemp = iEntity.iBTPort.Open( iSockServer );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL() (ERROR) complete (%d)" ), retTemp));
        User::Leave( retTemp );
        }
    iStatus = KRequestPending;
    iListenSocket.Accept( iEntity.iBTPort, iStatus );
    SetActive();
    iListenState = EBtListenStateListening;

    FTRACE(FPrint( _L( "CDunBtListen::IssueRequestL() complete") ));
    }

// ---------------------------------------------------------------------------
// Stops listening
// ---------------------------------------------------------------------------
//
TInt CDunBtListen::Stop()
    {
    FTRACE(FPrint( _L( "CDunBtListen::Stop()") ));
    if ( iListenState != EBtListenStateListening )
        {
        FTRACE(FPrint( _L("CDunBtListen::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iListenSocket.CancelAccept();
    Cancel();
    iListenState = EBtListenStateIdle;
    FTRACE(FPrint( _L( "CDunBtListen::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunBtListen::CDunBtListen
// ---------------------------------------------------------------------------
//
CDunBtListen::CDunBtListen( MDunServerCallback* aServer,
                            MDunListenCallback* aParent,
                            CDunTransporter* aTransporter,
                            TBtPortEntity& aEntity ) :
    CActive( EPriorityStandard ),
    iServer( aServer ),
    iParent( aParent ),
    iTransporter( aTransporter ),
    iEntity( aEntity )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunBtListen::ConstructL
// ---------------------------------------------------------------------------
//
void CDunBtListen::ConstructL()
    {
    FTRACE(FPrint(_L("CDunBtListen::ConstructL()")));
    if ( !iServer || !iParent || !iTransporter )
        {
        User::Leave( KErrGeneral );
        }

    CBTEngDiscovery* discovery = CBTEngDiscovery::NewLC();
    FTRACE(FPrint(_L("CDunBtListen::ConstructL: iSockServer.Connect")));
    User::LeaveIfError( iSockServer.Connect() );

    // Set advertisement monitor
    iTransporter->SetAdvertisementMonitorL( KDunBtPluginUid, this );

    // Then we are ready to start listening and accepting incoming connection
    // requests.
    CleanupStack::Pop( discovery );
    iDiscovery = discovery;
    CActiveScheduler::Add( this );
    FTRACE(FPrint(_L("CDunBtListen::ConstructL() complete")));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunBtListen::Initialize()
    {
    // Don't initialize iServer here (it is set through NewL)
    // Don't initialize iParent here (it is set through NewL)
    // Don't initialize iTransporter here (it is set through NewL)
    // Don't initialize iEntity here (it is set through NewL)
    iListenState = EBtListenStateIdle;
    iDiscovery = NULL;
    iChannelNum = 0;
    iSDPHandleDun = 0;
    }

// ---------------------------------------------------------------------------
// Starts dialup service advertisement
// ---------------------------------------------------------------------------
//
TInt CDunBtListen::StartServiceAdvertisement( TBool& aInUse )
    {
    FTRACE(FPrint( _L( "CDunBtListen::StartServiceAdvertisement()" ) ));

    TInt retTemp = ReserveLocalChannel( iSockServer,
                                        iListenSocket,
                                        iChannelNum,
                                        aInUse );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtListen::StartServiceAdvertisement() (ERROR) complete" ) ));
        return retTemp;
        }

    // Now RFCOMM channel number of the next data socket must be the same as
    // the current listener's RFCOMM channel number. Set that now.
    iEntity.iChannelNum = iChannelNum;

    // Register SDP record
    iSDPHandleDun = 0;
    retTemp = iDiscovery->RegisterSdpRecord( KDialUpNetworkingUUID,
                                             iChannelNum,
                                             iSDPHandleDun );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtListen::StartServiceAdvertisement() (failed!) complete (%d)" ), retTemp));
        return retTemp;
        }
    FTRACE(FPrint( _L( "CDunBtListen::StartServiceAdvertisement() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops dialup service advertisement
// ---------------------------------------------------------------------------
//
TInt CDunBtListen::StopServiceAdvertisement()
    {
    FTRACE(FPrint( _L( "CDunBtListen::StopServiceAdvertisement()" ) ));
    if ( !iDiscovery )
        {
        FTRACE(FPrint( _L( "CDunBtListen::StopServiceAdvertisement() (iDiscovery) not initialized!" ) ));
        return KErrGeneral;
        }
    if ( iSDPHandleDun != 0 )
        {
        TInt retTemp = iDiscovery->DeleteSdpRecord( iSDPHandleDun );
        FTRACE(FPrint( _L( "CDunBtListen::StopServiceAdvertisement() record closed (%d)" ), retTemp ));
        iSDPHandleDun = 0;
        }
    if ( iListenSocket.SubSessionHandle() )
        {
        iListenSocket.Close();
        }
    FTRACE(FPrint( _L( "CDunBtListen::StopServiceAdvertisement() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Method which reserves local RFCOMM channel (from possible channels 1-30)
// and returns it to client.
// ---------------------------------------------------------------------------
//
TInt CDunBtListen::ReserveLocalChannel( RSocketServ& aSocketServ,
                                        RSocket& aListenSocket,
                                        TUint& aChannelNum,
                                        TBool& aInUse )
    {
    FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel()")));
    aInUse = EFalse;
    if ( aListenSocket.SubSessionHandle() )
        {
        FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() (open socket!) complete")));
        return KErrArgument;
        }
    TInt retTemp;
    TProtocolDesc pInfo;
    retTemp = aSocketServ.FindProtocol( TProtocolName(KRFCOMMDesC), pInfo );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() (FindProtocol failed) complete (%d)"), retTemp));
        return retTemp;
        }
    retTemp = aListenSocket.Open( aSocketServ,
                                  pInfo.iAddrFamily,
                                  pInfo.iSockType,
                                  pInfo.iProtocol );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() (Open failed) complete (%d)"), retTemp));
        return retTemp;
        }
    TRfcommSockAddr addr;
    TBTServiceSecurity sec;
    sec.SetAuthentication( ETrue );
    sec.SetAuthorisation( ETrue );
    sec.SetEncryption( ETrue );
    sec.SetPasskeyMinLength( 0 );
    addr.SetSecurity( sec );
    addr.SetPort( KRfcommPassiveAutoBind );
    // When fix from Apple, replace the following with
    // "retTemp = aListenSocket.Bind( addr );"
    retTemp = DoExtendedBind( aListenSocket, addr );
    if ( retTemp != KErrNone )
        {
        aListenSocket.Close();
        aInUse = ETrue;
        FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() Bind() complete (%d)"), retTemp));
        return KErrInUse;
        }
    aChannelNum = aListenSocket.LocalPort();

    // We try to set the Telephony and Networking bits in our service class.  If this fails we
    // ignore it, as it's better to carry on without it than to fail to start listening.
    aListenSocket.SetOpt(KBTRegisterCodService, KSolBtRFCOMM, KCoDDunServiceClass);

    retTemp = aListenSocket.Listen( KListenQueSize );
    if ( retTemp != KErrNone )
        {
        aListenSocket.Close();
        FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() Listen() complete (%d)"), retTemp));
        return retTemp;
        }
    FTRACE(FPrint(_L("CDunBtListen::ReserveLocalChannel() complete (%d)"), aChannelNum));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Tries to bind to a fixed port and if that fails with KRfcommPassiveAutoBind.
// This is for spec breaking solutions like the OSX Leopard.
// ---------------------------------------------------------------------------
//
TInt CDunBtListen::DoExtendedBind( RSocket& aListenSocket,
                                   TRfcommSockAddr& aSockAddr )
    {
    FTRACE(FPrint(_L("CDunBtListen::DoExtendedBind()")));
    if ( !aListenSocket.SubSessionHandle() )
        {
        FTRACE(FPrint(_L("CDunBtListen::DoExtendedBind() (closed socket!) complete")));
        return KErrGeneral;
        }
    TRfcommSockAddr fixedAddr = aSockAddr;
    fixedAddr.SetPort( KDunFixedChannel );
    TInt retTemp = aListenSocket.Bind( fixedAddr );
    if ( retTemp == KErrNone )
        {
        FTRACE(FPrint(_L("CDunBtListen::DoExtendedBind() complete")));
        return KErrNone;
        }
    TInt retVal = aListenSocket.Bind( aSockAddr );
    FTRACE(FPrint(_L("CDunBtListen::DoExtendedBind() complete")));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called when a service is requested via BT.
// ---------------------------------------------------------------------------
//
void CDunBtListen::RunL()
    {
    FTRACE(FPrint( _L( "CDunBtListen::RunL()" ) ));
    iListenState = EBtListenStateIdle;

    StopServiceAdvertisement();

    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtListen::RunL() (ERROR) complete (%d)" ), retTemp));
        iServer->NotifyPluginCloseRequest( KDunBtPluginUid, ETrue );
        return;
        }
    // Notify new connection
    TBool noFreeChans = EFalse;
    retTemp = iParent->NotifyChannelAllocate( noFreeChans );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunBtListen::RunL() channel allocation failed! (%d)" ), retTemp));
        // Other error than no free channels, close plugin now
        if ( !noFreeChans )
            {
            iServer->NotifyPluginCloseRequest( KDunBtPluginUid, ETrue );
            }
        return;
        }

    // Don't restart listening here. Request is issued via
    // NotifyAdvertisementStart()

    FTRACE(FPrint( _L( "CDunBtListen::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Cancel current activity.
// ---------------------------------------------------------------------------
//
void CDunBtListen::DoCancel()
    {
    }

// ---------------------------------------------------------------------------
// From class MDunServAdvMon.
// Gets called when advertisement status changes to start.
// ---------------------------------------------------------------------------
//
void CDunBtListen::NotifyAdvertisementStart( TBool aCreation )
    {
    FTRACE(FPrint( _L( "CDunBtListen::NotifyAdvertisementStart()" ) ));
    // Remove the "if" below when fix comes from Apple
    if ( !aCreation )
        {
        TRAP_IGNORE( IssueRequestL() );
        }
    FTRACE(FPrint( _L( "CDunBtListen::NotifyAdvertisementStart() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunServAdvMon.
// Gets called when advertisement status changes to end.
// ---------------------------------------------------------------------------
//
void CDunBtListen::NotifyAdvertisementEnd()
    {
    FTRACE(FPrint( _L( "CDunBtListen::NotifyAdvertisementEnd()" ) ));
    Stop();
    StopServiceAdvertisement();
    FTRACE(FPrint( _L( "CDunBtListen::NotifyAdvertisementEnd() complete" ) ));
    }
