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
* Description:  Monitors for new data in RComm local media side
*
*/


#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#else
#include <d32comm.h>
#endif
#include "DunTransporter.h"
#include "DunSignalWaiter.h"
#include "DunSignalCopy.h"
#include "DunDebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunSignalWaiter* CDunSignalWaiter::NewL( MDunChannelAllocator* aChannelCallback )
    {
    CDunSignalWaiter* self = new (ELeave) CDunSignalWaiter( aChannelCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunSignalWaiter::~CDunSignalWaiter()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::~CDunSignalWaiter()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunSignalWaiter::~CDunSignalWaiter() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunSignalWaiter::ResetData()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // AddConnMonCallbackL()
    iCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunSignalWaiter::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for RunL error controlling
// The callback will be called when error is detected in asynchronous
// operation
// ---------------------------------------------------------------------------
//
TInt CDunSignalWaiter::AddCallback( MDunConnMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::AddCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::AddCallback() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::AddCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::AddCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunSignalWaiter::AddCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunSignalWaiter::SetMedia( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::SetMedia(RComm) (addr=0x%08X)" ), aComm));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::SetMedia() (aComm) not initialized!" )));
        return KErrGeneral;
        }
    TCommCaps2 caps;
    aComm->Caps( caps );
    if ( !(caps().iNotificationCaps & KNotifySignalsChangeSupported) )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::SetMedia() (RComm) (not supported) complete" )));
        return KErrNotSupported;
        }
    iComm = aComm;
    FTRACE(FPrint( _L("CDunSignalWaiter::SetMedia() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues request to start waiting for new data in RComm
// ---------------------------------------------------------------------------
//
TInt CDunSignalWaiter::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::IssueRequest()" )));
    if ( iSignalWaiterState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::IssueRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iComm )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::IssueRequest() (iComm) not initialized!" ) ));
        return KErrGeneral;
        }
    iStatus = KRequestPending;
    iSignalWaiterState = EDunStateSignalWaiting;
    iComm->NotifySignalChange( iStatus, iSignals, KSignalDCEInputs );
    SetActive();
    FTRACE(FPrint( _L("CDunSignalWaiter::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops monitoring for new data
// ---------------------------------------------------------------------------
//
TInt CDunSignalWaiter::Stop()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::Stop()" )));
    if ( iSignalWaiterState != EDunStateSignalWaiting )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iComm )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::Stop() (iComm) not initialized!" )));
        return KErrGeneral;
        }
    iComm->NotifySignalChangeCancel();
    Cancel();
    iSignalWaiterState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunSignalWaiter::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunSignalWaiter::CDunSignalWaiter
// ---------------------------------------------------------------------------
//
CDunSignalWaiter::CDunSignalWaiter( MDunChannelAllocator* aChannelCallback ) :
    CActive( EPriorityHigh ),
    iChannelCallback( aChannelCallback )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunSignalWaiter::ConstructL
// ---------------------------------------------------------------------------
//
void CDunSignalWaiter::ConstructL()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::ConstructL()" ) ));
    if ( !iChannelCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunSignalWaiter::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunSignalWaiter::Initialize()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::Initialize()" ) ));
    // Don't initialize iChannelCallback here (it is set through NewL)
    iSignalWaiterState = EDunStateIdle;
    iSignals = 0;
    iComm = NULL;
    FTRACE(FPrint( _L("CDunSignalWaiter::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when signals changed in RComm object
// ---------------------------------------------------------------------------
//
void CDunSignalWaiter::RunL()
    {
    FTRACE(FPrint( _L("CDunSignalWaiter::RunL()" ) ));
    iSignalWaiterState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunSignalWaiter::RunL() (failed) complete (%d)"), retTemp));
        TInt count = iCallbacks.Count();
        TDunConnectionReason connReason;
        connReason.iReasonType = EDunReasonTypeRunL;
        connReason.iContext = EDunMediaContextLocal;
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = EDunDirectionUndefined;
        connReason.iErrorCode = retTemp;
        for ( TInt i=0; i<count; i++ )
            {
            TRAP_IGNORE(
                iCallbacks[i]->NotifyProgressChangeL( iComm, connReason ) );
            }
        return;
        }
    if ( iChannelCallback )
        {
        iChannelCallback->NotifyNewChannelRequest( iComm );
        }
    FTRACE(FPrint( _L("CDunSignalWaiter::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunSignalWaiter::DoCancel()
    {
    }
