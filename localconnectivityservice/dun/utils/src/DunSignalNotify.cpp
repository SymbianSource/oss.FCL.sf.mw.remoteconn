/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Monitors signal changes on network side and reports changes
*
*/


#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#else
#include <d32comm.h>
#endif
#include "DunTransporter.h"
#include "DunSignalNotify.h"
#include "DunDebug.h"

const TUint KDunSixLowestBitsMask = 0x3F;  // Symbian magic

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunSignalNotify* CDunSignalNotify::NewL( MDunTransporterUtilityAux* aUtility )
    {
    CDunSignalNotify* self = new (ELeave) CDunSignalNotify( aUtility );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDunSignalNotify::~CDunSignalNotify()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::~CDunSignalNotify()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunSignalNotify::~CDunSignalNotify() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::ResetData()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // AddCallback()
    iCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunSignalNotify::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for line status change controlling
// The callback will be called when line status change is detected in
// endpoint
// ---------------------------------------------------------------------------
//
TInt CDunSignalNotify::AddCallback( MDunConnMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunSignalNotify::AddCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::AddCallback() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::AddCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::AddCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunSignalNotify::AddCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint monitor
// ---------------------------------------------------------------------------
//
TInt CDunSignalNotify::SetMedia( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunSignalNotify::SetMedia()" )));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::SetMedia() (aComm) not initialized!" )));
        return KErrGeneral;
        }
    // As it's not possible to listen for RFCOMM signal changes the RTS and DTR
    // signals should be anyway set high to Dataport
    aComm->SetSignals( KSignalRTS|KSignalDTR, 0 );
    // Next check if signal change notification from Dataport supported
    TCommCaps2 caps;
    aComm->Caps( caps );
    if ( !(caps().iNotificationCaps & KNotifySignalsChangeSupported) )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::SetMedia() (RComm) (not supported) complete" )));
        return KErrNotSupported;
        }
    iListenSignals = KSignalDTEInputs;
    iNetwork = aComm;
    FTRACE(FPrint( _L("CDunSignalNotify::SetMedia() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues request to start monitoring the endpoint for line status change
// ---------------------------------------------------------------------------
//
TInt CDunSignalNotify::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest()" )));
    if ( iSignalNotifyState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iNetwork )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() (iNetwork) not initialized!" ) ));
        return KErrGeneral;
        }
    TUint currentSignals = iNetwork->Signals( iListenSignals );
    TUint oldSignalsLow = iSignals & KDunSixLowestBitsMask;
    TUint newSignalsLow = currentSignals & KDunSixLowestBitsMask;
    TUint oldSignalsHigh = oldSignalsLow * KSignalChanged;
    TUint newSignalsHigh = newSignalsLow * KSignalChanged;
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() oldSignalsLow=0x%08X" ), oldSignalsLow ));
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() newSignalsLow=0x%08X" ), newSignalsLow ));
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() oldSignalsHigh=0x%08X" ), oldSignalsHigh ));
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() newSignalsHigh=0x%08X" ), newSignalsHigh ));
    if ( newSignalsLow != oldSignalsLow )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() signals already set..." ) ));
        iSignals = newSignalsLow | (oldSignalsHigh ^ newSignalsHigh);
        ManageSignalChange();
        }
    else
        {
        FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() start waiting for change..." ) ));
        iStatus = KRequestPending;
        iNetwork->NotifySignalChange( iStatus, iSignals, iListenSignals );
        SetActive();
        iSignalNotifyState = EDunStateSignalNotify;
        }
    FTRACE(FPrint( _L("CDunSignalNotify::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops monitoring the endpoint for line status change
// ---------------------------------------------------------------------------
//
TInt CDunSignalNotify::Stop()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::Stop()" )));
    if ( iSignalNotifyState != EDunStateSignalNotify )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iNetwork )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::Stop() (iNetwork) not initialized!" )));
        return KErrGeneral;
        }
    iNetwork->NotifySignalChangeCancel();
    Cancel();
    iSignalNotifyState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunSignalNotify::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunSignalNotify::CDunSignalNotify
// ---------------------------------------------------------------------------
//
CDunSignalNotify::CDunSignalNotify( MDunTransporterUtilityAux* aUtility ) :
    CActive( EPriorityHigh ),
    iUtility( aUtility )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunSignalNotify::ConstructL
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::ConstructL()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::ConstructL()" ) ));
    if ( !iUtility )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunSignalNotify::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::Initialize()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::Initialize()" ) ));
    // Don't initialize iUtility here (it is set through NewL)
    iSignalNotifyState = EDunStateIdle;
    iListenSignals = 0;
    iSignals = 0;
    iNetwork = NULL;
    FTRACE(FPrint( _L("CDunSignalNotify::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages signal changes
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::ManageSignalChange()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange()" ) ));
    // The following signals are input signals from network side:
    // KSignalDTEInputs = KSignalCTS | KSignalDSR | KSignalDCD | KSignalRNG
    if ( iSignals & KCTSChanged )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() checking CTS..." ) ));
        if ( iSignals & KSignalCTS )  // CTS changed to high
            {
            ReportSignalChange( KSignalCTS, 0 );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() CTS changed high" ) ));
            }
        else  // CTS changed to low
            {
            ReportSignalChange( 0, KSignalCTS );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() CTS changed low" ) ));
            }
        }
    if ( iSignals & KDSRChanged )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() checking DSR..." ) ));
        if ( iSignals & KSignalDSR )  // DSR changed to high
            {
            ReportSignalChange( KSignalDSR, 0 );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() DSR changed high" ) ));
            }
        else  // DSR changed to low
            {
            ReportSignalChange( 0, KSignalDSR );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() DSR changed low" ) ));
            }
        }
    if ( iSignals & KDCDChanged )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() checking DCD..." ) ));
        if ( iSignals & KSignalDCD )  // DCD changed to high
            {
            ReportSignalChange( KSignalDCD, 0 );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() DCD changed high" ) ));
            }
        else  // DCD changed to low
            {
            ReportSignalChange( 0, KSignalDCD );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() DCD changed low" ) ));
            }
        }
    if ( iSignals & KRNGChanged )
        {
        FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() checking RNG..." ) ));
        if ( iSignals & KSignalRNG )  // RNG changed to high
            {
            ReportSignalChange( KSignalRNG, 0 );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() RNG changed high" ) ));
            }
        else  // RNG changed to low
            {
            ReportSignalChange( 0, KSignalRNG );
            FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() RNG changed low" ) ));
            }
        }
    IssueRequest();
    FTRACE(FPrint( _L("CDunSignalNotify::ManageSignalChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Reports signal change
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::ReportSignalChange( TUint aSetMask, TUint aClearMask )
    {
    FTRACE(FPrint( _L("CDunSignalNotify::ReportSignalChange()" ) ));

    TUint signalType = aClearMask;
    TUint signalHigh = EFalse;
    if ( aSetMask != 0 )
        {
        signalType = aSetMask;
        signalHigh = ETrue;
        }
    TInt i;
    TInt count = iCallbacks.Count();
    TDunConnectionReason connReason;
    TConnId localId = iUtility->GetLocalId( iNetwork );
    connReason.iReasonType = EDunReasonTypeSignal;
    connReason.iContext = EDunMediaContextNetwork;
    connReason.iSignalType = signalType;
    connReason.iSignalHigh = signalHigh;
    connReason.iDirection = EDunDirectionUndefined;
    connReason.iErrorCode = KErrNone;
    for ( i=0; i<count; i++ )
        {
        TRAP_IGNORE(
            iCallbacks[i]->NotifyProgressChangeL( localId, connReason ) );
        }

    FTRACE(FPrint( _L("CDunSignalNotify::ReportSignalChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when line status changes
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::RunL()
    {
    FTRACE(FPrint( _L("CDunSignalNotify::RunL()" ) ));
    iSignalNotifyState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        TInt i;
        TInt count = iCallbacks.Count();
        TDunConnectionReason connReason;
        TConnId localId = iUtility->GetLocalId( iNetwork );
        connReason.iReasonType = EDunReasonTypeRunL;
        connReason.iContext = EDunMediaContextNetwork;
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = EDunDirectionUndefined;
        connReason.iErrorCode = retTemp;
        for ( i=0; i<count; i++ )
            {
            TRAP_IGNORE(
                iCallbacks[i]->NotifyProgressChangeL( localId, connReason ) );
            }
        return;
        }
    FTRACE(FPrint( _L("CDunSignalNotify::RunL() managing signal 0x%08X" ), iSignals ));
    ManageSignalChange();
    FTRACE(FPrint( _L("CDunSignalNotify::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunSignalNotify::DoCancel()
    {
    }
