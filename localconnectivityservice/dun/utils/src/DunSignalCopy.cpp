/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Copies signals changes between network and local media
*
*/


#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#else
#include <d32comm.h>
#endif
#include "DunTransporter.h"
#include "DunSignalCopy.h"
#include "DunDebug.h"

const TUint KDunSixLowestBitsMask = 0x3F;  // Symbian magic

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunSignalCopy* CDunSignalCopy::NewL()
    {
    CDunSignalCopy* self = new (ELeave) CDunSignalCopy();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDunSignalCopy::~CDunSignalCopy()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::~CDunSignalCopy()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunSignalCopy::~CDunSignalCopy() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ResetData()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // AddCallback()
    iCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunSignalCopy::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for line status change controlling
// The callback will be called when RunL error is detected
// ---------------------------------------------------------------------------
//
TInt CDunSignalCopy::AddCallback( MDunConnMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunSignalCopy::AddCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::AddCallback() (aCallback) not initialized!" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::AddCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::AddCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunSignalCopy::AddCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint monitor
// ---------------------------------------------------------------------------
//
TInt CDunSignalCopy::SetMedia( RComm* aComm,
                               RComm* aNetwork,
                               TDunStreamType aStreamType )
    {
    FTRACE(FPrint( _L("CDunSignalCopy::SetMedia()" )));
    if ( !aNetwork )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::SetMedia() (aComm) not initialized!" )));
        return KErrGeneral;
        }
    RComm* comm = NULL;
    if ( aStreamType == EDunStreamTypeUpstream )
        {
        iContextInUse = EDunMediaContextLocal;
        comm = aComm;
        }
    else if ( aStreamType == EDunStreamTypeDownstream )
        {
        iContextInUse = EDunMediaContextNetwork;
        comm = aNetwork;
        }
    else
        {
        FTRACE(FPrint( _L("CDunSignalCopy::SetMedia() (aStreamtype) not initialized!" )));
        return KErrGeneral;
        }
    // No possibility to do iUtility->GetLocalId() here as aComm is not yet
    // in channel data; use aComm directly
    TCommCaps2 caps;
    comm->Caps( caps );
    if ( !(caps().iNotificationCaps & KNotifySignalsChangeSupported) )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::SetMedia() (RComm) (not supported) complete" )));
        return KErrNotSupported;
        }
    iComm = aComm;
    iNetwork = aNetwork;
    iStreamType = aStreamType;
    FTRACE(FPrint( _L("CDunSignalCopy::SetMedia() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues request to start monitoring the endpoint for line status change
// ---------------------------------------------------------------------------
//
TInt CDunSignalCopy::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest()" )));
    if ( iSignalCopyState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() (not ready) complete" ) ));
        return KErrNotReady;
        }
    RComm* comm = NULL;
    if ( iStreamType == EDunStreamTypeUpstream )
        {
        iListenSignals = KSignalDCEInputs;
        comm = iComm;
        }
    else if ( iStreamType == EDunStreamTypeDownstream )
        {
        iListenSignals = KSignalDTEInputs;
        comm = iNetwork;
        }
    else
        {
        FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TUint currentSignals = comm->Signals( iListenSignals );
    TUint oldSignalsLow = iSignals & KDunSixLowestBitsMask;
    TUint newSignalsLow = currentSignals & KDunSixLowestBitsMask;
    TUint oldSignalsHigh = oldSignalsLow * KSignalChanged;
    TUint newSignalsHigh = newSignalsLow * KSignalChanged;
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() oldSignalsLow=0x%08X" ), oldSignalsLow ));
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() newSignalsLow=0x%08X" ), newSignalsLow ));
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() oldSignalsHigh=0x%08X" ), oldSignalsHigh ));
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() newSignalsHigh=0x%08X" ), newSignalsHigh ));
    if ( newSignalsLow != oldSignalsLow )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() signals already set..." ) ));
        iSignals = newSignalsLow | (oldSignalsHigh ^ newSignalsHigh);
        ManageSignalChange();
        }
    else
        {
        FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() start waiting for change..." ) ));
        iStatus = KRequestPending;
        iSignalCopyState = EDunStateSignalCopy;
        comm->NotifySignalChange( iStatus, iSignals, iListenSignals );
        SetActive();
        }
    FTRACE(FPrint( _L("CDunSignalCopy::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops monitoring the endpoint for line status change
// ---------------------------------------------------------------------------
//
TInt CDunSignalCopy::Stop()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::Stop()" )));
    if ( iSignalCopyState != EDunStateSignalCopy )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( iStreamType == EDunStreamTypeUpstream )
        {
        iComm->NotifySignalChangeCancel();
        }
    else if ( iStreamType == EDunStreamTypeDownstream )
        {
        iNetwork->NotifySignalChangeCancel();
        }
    else
        {
        FTRACE(FPrint( _L("CDunSignalCopy::Stop() (ERROR) complete" )));
        return KErrGeneral;
        }
    Cancel();
    iSignalCopyState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunSignalCopy::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunSignalCopy::CDunSignalCopy
// ---------------------------------------------------------------------------
//
CDunSignalCopy::CDunSignalCopy() :
    CActive( EPriorityHigh )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunSignalCopy::ConstructL
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ConstructL()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ConstructL()" ) ));
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunSignalCopy::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::Initialize()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::Initialize()" ) ));
    iContextInUse = EDunMediaContextUndefined;
    iStreamType = EDunStreamTypeUndefined;
    iSignalCopyState = EDunStateIdle;
    iListenSignals = 0;
    iSignals = 0;
    iNetwork = NULL;
    iComm = NULL;
    FTRACE(FPrint( _L("CDunSignalCopy::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages signal changes
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ManageSignalChange()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChange()" ) ));

    if ( iStreamType == EDunStreamTypeUpstream )
        {
        // Manage upstream case
        ManageSignalChangeUpstream();
        }
    else  // iStreamType == EDunStreamTypeDownstream
        {
        // Manage downstream case
        ManageSignalChangeDownstream();
        }

    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages upstream signal changes
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ManageSignalChangeUpstream()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream()" ) ));
    // For upstream the following signals are set for local media:
    // KSignalDCEInputs = KSignalRTS | KSignalDTR
    if ( iSignals & KRTSChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() checking RTS..." ) ));
        if ( iSignals & KSignalRTS )  // RTS changed to high
            {
            ChangeUpstreamSignal( KSignalRTS, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() RTS changed high" ) ));
            }
        else  // RTS changed to low
            {
            ChangeUpstreamSignal( 0, KSignalRTS );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() RTS changed low" ) ));
            }
        }
    if ( iSignals & KDTRChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() checking DTR..." ) ));
        if ( iSignals & KSignalDTR )  // DTR changed to high
            {
            ChangeUpstreamSignal( KSignalDTR, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() DTR changed high" ) ));
            }
        else  // DTR changed to low
            {
            ChangeUpstreamSignal( 0, KSignalDTR );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() DTR changed low" ) ));
            }
        }
    IssueRequest();
    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeUpstream() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages downstream signal changes
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ManageSignalChangeDownstream()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream()" ) ));
    // For downstream the following signals are set for network:
    // KSignalDTEInputs = KSignalCTS | KSignalDSR | KSignalDCD | KSignalRNG
    if ( iSignals & KCTSChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() checking CTS..." ) ));
        if ( iSignals & KSignalCTS )  // CTS changed to high or initially high
            {
            ChangeDownstreamSignal( KSignalCTS, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() CTS changed high" ) ));
            }
        else  // CTS changed to low
            {
            ChangeDownstreamSignal( 0, KSignalCTS );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() CTS changed low" ) ));
            }
        }
    if ( iSignals & KDSRChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() checking DSR..." ) ));
        if ( iSignals & KSignalDSR )  // DSR changed to high
            {
            ChangeDownstreamSignal( KSignalDSR, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() DSR changed high" ) ));
            }
        else  // DSR changed to low
            {
            ChangeDownstreamSignal( 0, KSignalDSR );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() DSR changed low" ) ));
            }
        }
    if ( iSignals & KDCDChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() checking DCD..." ) ));
        if ( iSignals & KSignalDCD )  // DCD changed to high
            {
            ChangeDownstreamSignal( KSignalDCD, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() DCD changed high" ) ));
            }
        else  // DCD changed to low
            {
            ChangeDownstreamSignal( 0, KSignalDCD );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() DCD changed low" ) ));
            }
        }
    if ( iSignals & KRNGChanged )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() checking RNG..." ) ));
        if ( iSignals & KSignalRNG )  // RNG changed to high
            {
            ChangeDownstreamSignal( KSignalRNG, 0 );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() RNG changed high" ) ));
            }
        else  // RNG changed to low
            {
            ChangeDownstreamSignal( 0, KSignalRNG );
            FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() RNG changed low" ) ));
            }
        }
    IssueRequest();
    FTRACE(FPrint( _L("CDunSignalCopy::ManageSignalChangeDownstream() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Changes upstream signal
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ChangeUpstreamSignal( TUint aSetMask, TUint aClearMask )
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ChangeUpstreamSignal()" ) ));
    if ( iNetwork )
        {
        iNetwork->SetSignals( aSetMask, aClearMask );
        }
    // Add other cases here if necessary
    FTRACE(FPrint( _L("CDunSignalCopy::ChangeUpstreamSignal() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Changes downstream signal
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::ChangeDownstreamSignal( TUint aSetMask, TUint aClearMask )
    {
    FTRACE(FPrint( _L("CDunSignalCopy::ChangeDownstreamSignal()" ) ));
    if ( iComm )
        {
        iComm->SetSignals( aSetMask, aClearMask );
        }
    // Add other cases here if necessary
    FTRACE(FPrint( _L("CDunSignalCopy::ChangeDownstreamSignal() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when line status changes
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::RunL()
    {
    FTRACE(FPrint( _L("CDunSignalCopy::RunL()" ) ));
    iSignalCopyState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunSignalCopy::RunL() (failed) complete (%d)"), retTemp));
        TInt i;
        TInt count = iCallbacks.Count();
        TDunConnectionReason connReason;
        connReason.iReasonType = EDunReasonTypeRunL;
        connReason.iContext = iContextInUse;
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = EDunDirectionUndefined;
        connReason.iErrorCode = retTemp;
        for ( i=0; i<count; i++ )
            {
            TRAP_IGNORE(
                iCallbacks[i]->NotifyProgressChangeL( iComm, connReason ) );
            }
        return;
        }
    FTRACE(FPrint( _L("CDunSignalCopy::RunL() managing signal 0x%08X" ), iSignals ));
    ManageSignalChange();
    FTRACE(FPrint( _L("CDunSignalCopy::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunSignalCopy::DoCancel()
    {
    }
