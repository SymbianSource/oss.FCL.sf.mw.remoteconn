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
* Description:  DUN USB plugin's listener
*
*/


#include <e32std.h>
#include "DunUtils.h"
#include "DunUsbListen.h"
#include "DunDebug.h"

const TUint KDunUsbDeviceStateMask = 0x00ff;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunUsbListen* CDunUsbListen::NewL( MDunServerCallback* aServer,
                                    MDunListenCallback* aParent,
                                    RUsb& aUsbServer )
    {
    CDunUsbListen* self = new (ELeave) CDunUsbListen( aServer,
                                                      aParent,
                                                      aUsbServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunUsbListen::~CDunUsbListen()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::~CDunUsbListen()" ) ));
    ResetData();
    FTRACE(FPrint( _L( "CDunUsbListen::~CDunUsbListen() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunUsbListen::ResetData()
    {
    // APIs affecting this:
    // IssueRequestL()
    Stop();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// IssueRequest to USB server for device state change notifications
// ---------------------------------------------------------------------------
//
TInt CDunUsbListen::IssueRequestL()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::IssueRequestL()" )));

    if ( iUsbServer.Handle() == KNullHandle )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::IssueRequestL() (iUsbServer) complete" ) ));
        User::Leave( KErrGeneral );
        }

    TUsbDeviceState usbDeviceState;
    User::LeaveIfError( iUsbServer.GetDeviceState(usbDeviceState) );
    FTRACE(FPrint( _L( "CDunUsbListen::IssueRequestL() Usb device state = %X" ), usbDeviceState));
    if ( iDeviceState == EUsbDeviceStateUndefined )
        {
        iDeviceState = usbDeviceState;
        }

    // USB device state now set; if configured already, notify parent
    // If not yet configured, start listening

    if ( usbDeviceState == EUsbDeviceStateConfigured )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::IssueRequestL() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }

    Activate();

    FTRACE(FPrint( _L( "CDunUsbListen::IssueRequestL() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops listening
// ---------------------------------------------------------------------------
//
TInt CDunUsbListen::Stop()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::Stop()" ) ));
    if ( iListenState != EUsbListenStateListening )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::Stop() (not ready) complete" ) ));
        return KErrNotReady;
        }
    iUsbServer.DeviceStateNotificationCancel();
    Cancel();
    iListenState = EUsbListenStateIdle;
    iDeviceState = EUsbDeviceStateUndefined;
    iDeviceStatePrev = EUsbDeviceStateUndefined;
    FTRACE(FPrint( _L( "CDunUsbListen::Stop() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunUsbListen::CDunUsbListen
// ---------------------------------------------------------------------------
//
CDunUsbListen::CDunUsbListen( MDunServerCallback* aServer,
                              MDunListenCallback* aParent,
                              RUsb& aUsbServer ) :
    CActive( EPriorityStandard ),
    iServer( aServer ),
    iParent( aParent ),
    iUsbServer( aUsbServer )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunUsbListen::ConstructL
// ---------------------------------------------------------------------------
//
void CDunUsbListen::ConstructL()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::ConstructL()" ) ));
    if ( !iServer || !iParent )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L( "CDunUsbListen::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunUsbListen::Initialize()
    {
    // Don't initialize iServer here (it is set through NewL)
    // Don't initialize iParent here (it is set through NewL)
    // Don't initialize iUsbServer here (it is set through NewL)
    iDeviceState = EUsbDeviceStateUndefined;
    iDeviceStatePrev = EUsbDeviceStateUndefined;
    }

// ---------------------------------------------------------------------------
// Activates listening request
// ---------------------------------------------------------------------------
//
TInt CDunUsbListen::Activate()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::Activate()" ) ));

    if ( iListenState != EUsbListenStateIdle )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::Activate() (not ready) complete" ) ));
        return KErrNotReady;
        }
    iDeviceStatePrev = iDeviceState;
    iStatus = KRequestPending;
    iListenState = EUsbListenStateListening;
    iUsbServer.DeviceStateNotification( KDunUsbDeviceStateMask,
                                        iDeviceState,
                                        iStatus );
    SetActive();
    FTRACE(FPrint( _L( "CDunUsbListen::Activate() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called when read or write operation is ready.
// ---------------------------------------------------------------------------
//
void CDunUsbListen::RunL()
    {
    FTRACE(FPrint( _L( "CDunUsbListen::RunL() iStatus=%d"), iStatus.Int() ));
    iListenState = EUsbListenStateIdle;

    if ( iStatus.Int() != KErrNone )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::RunL() (ERROR) complete" )));
        iServer->NotifyPluginCloseRequest( KDunUsbPluginUid, ETrue );
        return;
        }

    FTRACE(FPrint( _L( "CDunUsbListen::RunL() Usb device state =%X, Issue request" ), iDeviceState));

    TDunPluginState parentState = iServer->GetPluginStateByUid( KDunUsbPluginUid );

    TInt retTemp = KErrNone;
    if ( iDeviceState == EUsbDeviceStateConfigured &&
         iDeviceStatePrev != EUsbDeviceStateConfigured &&
         parentState != EDunStateChanneled )
        {
        // USB has been connected&configured and we are in PC Suite mode
        FTRACE(FPrint( _L( "CDunUsbListen::RunL() DeviceState is configured -> open connection" ) ));
        TBool noFreeChans = EFalse;
        // noFreeChans will be omitted (not needed to set to RComm)
        retTemp = iParent->NotifyChannelAllocate( noFreeChans );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunUsbListen::RunL() channel allocation failed!" ) ));
            iServer->NotifyPluginCloseRequest( KDunUsbPluginUid, ETrue );
            return;
            }
        }
    else if ( iDeviceState < EUsbDeviceStateConfigured &&
              iDeviceStatePrev >= EUsbDeviceStateConfigured &&
              parentState == EDunStateChanneled )
        {
        FTRACE(FPrint( _L( "CDunUsbListen::RunL() DeviceState is not configured -> close connection" ) ));
        retTemp = iParent->NotifyChannelFree();
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L( "CDunUsbListen::RunL() channel free failed!" ) ));
            iServer->NotifyPluginCloseRequest( KDunUsbPluginUid, ETrue );
            return;
            }
        }

    // Start listening again
    Activate();

    FTRACE(FPrint( _L( "CDunUsbListen::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Cancel current activity.
// ---------------------------------------------------------------------------
//
void CDunUsbListen::DoCancel()
    {
    }
