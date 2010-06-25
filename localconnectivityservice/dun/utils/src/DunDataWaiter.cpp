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


#include "DunTransporter.h"
#include "DunDataWaiter.h"
#include "DunSignalCopy.h"
#include "DunDebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunDataWaiter* CDunDataWaiter::NewL( MDunChannelAllocator* aChannelCallback )
    {
    CDunDataWaiter* self = new (ELeave) CDunDataWaiter( aChannelCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunDataWaiter::~CDunDataWaiter()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::~CDunDataWaiter()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunDataWaiter::~CDunDataWaiter() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunDataWaiter::ResetData()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::ResetData()") ));
    Stop();
    // APIs affecting this:
    // AddConnMonCallbackL()
    iCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunDataWaiter::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for RunL error controlling
// The callback will be called when error is detected in asynchronous
// operation
// ---------------------------------------------------------------------------
//
TInt CDunDataWaiter::AddCallback( MDunConnMon* aCallback )
    {
    FTRACE(FPrint( _L("CDunDataWaiter::AddCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::AddCallback() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::AddCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::AddCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunDataWaiter::AddCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to use for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunDataWaiter::SetMedia( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunDataWaiter::SetMedia(RComm) (addr=0x%08X)" ), aComm));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::SetMedia() (aComm) not initialized!" )));
        return KErrGeneral;
        }
    TCommCaps2 caps;
    aComm->Caps( caps );
    if ( !(caps().iNotificationCaps & KNotifyDataAvailableSupported) )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::SetMedia() (RComm) (not supported) complete" )));
        return KErrNotSupported;
        }
    iComm = aComm;
    FTRACE(FPrint( _L("CDunDataWaiter::SetMedia() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues request to start waiting for new data in RComm
// ---------------------------------------------------------------------------
//
TInt CDunDataWaiter::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::IssueRequest()" )));
    if ( iDataWaiterState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::IssueRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iComm )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::IssueRequest() (iComm) not initialized!" ) ));
        return KErrGeneral;
        }
    iStatus = KRequestPending;
    iComm->ResetBuffers();
    iComm->NotifyDataAvailable( iStatus );
    SetActive();
    iDataWaiterState = EDunStateDataWaiting;
    FTRACE(FPrint( _L("CDunDataWaiter::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops monitoring for new data
// ---------------------------------------------------------------------------
//
TInt CDunDataWaiter::Stop()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::Stop()" )));
    if ( iDataWaiterState != EDunStateDataWaiting )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( !iComm )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::Stop() (iComm) not initialized!" )));
        return KErrGeneral;
        }
    iComm->NotifyDataAvailableCancel();
    Cancel();
    iDataWaiterState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunDataWaiter::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunDataWaiter::CDunDataWaiter
// ---------------------------------------------------------------------------
//
CDunDataWaiter::CDunDataWaiter( MDunChannelAllocator* aChannelCallback ) :
    CActive( EPriorityHigh ),
    iChannelCallback( aChannelCallback )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunDataWaiter::ConstructL
// ---------------------------------------------------------------------------
//
void CDunDataWaiter::ConstructL()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::ConstructL()" ) ));
    if ( !iChannelCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunDataWaiter::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunDataWaiter::Initialize()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::Initialize()" ) ));
    // Don't initialize iChannelCallback here (it is set through NewL)
    iDataWaiterState = EDunStateIdle;
    iComm = NULL;
    FTRACE(FPrint( _L("CDunDataWaiter::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when new data in RComm object
// ---------------------------------------------------------------------------
//
void CDunDataWaiter::RunL()
    {
    FTRACE(FPrint( _L("CDunDataWaiter::RunL()" ) ));
    iDataWaiterState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDataWaiter::RunL() (failed) complete (%d)"), retTemp));
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
    // Don't check other iMediaInUse values here
    // They can't be other than EDunMediaRComm (checked in IssueRequest())
    FTRACE(FPrint( _L("CDunDataWaiter::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunDataWaiter::DoCancel()
    {
    }
