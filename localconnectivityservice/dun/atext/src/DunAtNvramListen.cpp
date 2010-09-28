/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AT NVRAM status change listener and notifier
*
*/


#include "DunAtNvramListen.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtNvramListen* CDunAtNvramListen::NewL( RATExt* aAtCmdExt,
                                            RATExtCommon* aAtCmdExtCommon )
    {
    CDunAtNvramListen* self = NewLC( aAtCmdExt, aAtCmdExtCommon );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtNvramListen* CDunAtNvramListen::NewLC( RATExt* aAtCmdExt,
                                             RATExtCommon* aAtCmdExtCommon )
    {
    CDunAtNvramListen* self = new (ELeave) CDunAtNvramListen(
        aAtCmdExt,
        aAtCmdExtCommon );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtNvramListen::~CDunAtNvramListen()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::~CDunAtNvramListen()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtNvramListen::~CDunAtNvramListen() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtNvramListen::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtNvramListen::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Starts waiting for NVRAM status changes
// ---------------------------------------------------------------------------
//
TInt CDunAtNvramListen::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::IssueRequest()") ));
    if ( iNvramHandleState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunAtNvramListen::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    if ( !iStarted )
        {
        iAtCmdExtCommon->GetNvramStatus( iNvramBuffer );
        iAtCmdExt->BroadcastNvramStatusChange( iNvramBuffer );
        iStarted = ETrue;
        }
    iStatus = KRequestPending;
    iNvramHandleState = EDunStateNvramListening;
    iAtCmdExtCommon->ReceiveNvramStatusChange( iStatus, iNvramBuffer );
    SetActive();
    FTRACE(FPrint( _L("CDunAtNvramListen::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops waiting for NVRAM status changes
// ---------------------------------------------------------------------------
//
TInt CDunAtNvramListen::Stop()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::Stop()") ));
    if ( iNvramHandleState != EDunStateNvramListening )
        {
        FTRACE(FPrint( _L("CDunAtNvramListen::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iAtCmdExtCommon->CancelReceiveNvramStatusChange();
    Cancel();
    iNvramHandleState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunAtNvramListen::Stop() complete") ));
    // Note: Don't mark iStarted to EFalse here!
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunAtNvramListen::CDunAtNvramListen
// ---------------------------------------------------------------------------
//
CDunAtNvramListen::CDunAtNvramListen( RATExt* aAtCmdExt,
                                      RATExtCommon* aAtCmdExtCommon ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iAtCmdExtCommon( aAtCmdExtCommon )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtNvramListen::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtNvramListen::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::ConstructL()") ));
    if ( !iAtCmdExt || !iAtCmdExtCommon )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunAtNvramListen::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtNvramListen::Initialize()
    {
    // Don't initialize iAtCmdExt here (it is set through NewL)
    // Don't initialize iAtCmdExtCommon here (it is set through NewL)
    iNvramHandleState = EDunStateIdle;
    iStarted = EFalse;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when NVRAM has changed
// ---------------------------------------------------------------------------
//
void CDunAtNvramListen::RunL()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::RunL()") ));
    iNvramHandleState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtNvramListen::RunL() (ERROR) complete (%d)"), retTemp ));
        return;
        }
    iAtCmdExt->BroadcastNvramStatusChange( iNvramBuffer );
    IssueRequest();
    FTRACE(FPrint( _L("CDunAtNvramListen::RunL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunAtNvramListen::DoCancel()
    {
    FTRACE(FPrint( _L("CDunAtNvramListen::DoCancel()") ));
    FTRACE(FPrint( _L("CDunAtNvramListen::DoCancel() complete") ));
    }
