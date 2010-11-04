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
* Description:  Modem's mode listener and notifier
*
*/


#include "DunAtModeListen.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtModeListen* CDunAtModeListen::NewL( RATExtCommon* aAtCmdExtCommon,
                                          MDunAtModeListen* aCallback )
    {
    CDunAtModeListen* self = NewLC( aAtCmdExtCommon, aCallback );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtModeListen* CDunAtModeListen::NewLC( RATExtCommon* aAtCmdExtCommon,
                                           MDunAtModeListen* aCallback )
    {
    CDunAtModeListen* self = new (ELeave) CDunAtModeListen( aAtCmdExtCommon,
                                                            aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtModeListen::~CDunAtModeListen()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::~CDunAtModeListen()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtModeListen::~CDunAtModeListen() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtModeListen::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Issues request to start monitoring for mode status changes
// ---------------------------------------------------------------------------
//
TInt CDunAtModeListen::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::IssueRequest()" )));
    if ( IsActive() )
        {
        FTRACE(FPrint( _L("CDunAtModeListen::IssueRequest() (not ready) complete" ) ));
        return KErrNotReady;
        }
    iAtCmdExtCommon->ReceiveModeStatusChange( iStatus, iModePckg );
    SetActive();
    FTRACE(FPrint( _L("CDunAtModeListen::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops monitoring for mode status changes
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::Stop()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::Stop()" )));
    Cancel();
    FTRACE(FPrint( _L("CDunAtModeListen::Stop() complete" )));
    }

// ---------------------------------------------------------------------------
// CDunAtModeListen::CDunAtModeListen
// ---------------------------------------------------------------------------
//
CDunAtModeListen::CDunAtModeListen( RATExtCommon* aAtCmdExtCommon,
                                    MDunAtModeListen* aCallback ) :
    CActive( EPriorityHigh ),
    iAtCmdExtCommon( aAtCmdExtCommon ),
    iCallback( aCallback ),
    iModePckg( iMode )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtModeListen::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::ConstructL()") ));
    if ( !iAtCmdExtCommon || !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunAtModeListen::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::Initialize()
    {
    // Don't initialize iAtCmdExtCommon here (it is set through NewL)
    // Don't initialize iCallback here (it is set through NewL)
    iMode = 0;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when mode changes
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::RunL()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::RunL()") ));
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtModeListen::RunL() (ERROR) complete (%d)"), retTemp ));
        return;
        }
    FTRACE(FPrint( _L("CDunAtModeListen::RunL() managing 0x%08X" ), iMode ));
    iCallback->NotifyModeStatusChange( iMode );
    IssueRequest();
    FTRACE(FPrint( _L("CDunAtModeListen::RunL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunAtModeListen::DoCancel()
    {
    FTRACE(FPrint( _L("CDunAtModeListen::DoCancel()") ));
    iAtCmdExtCommon->CancelReceiveModeStatusChange();
    FTRACE(FPrint( _L("CDunAtModeListen::DoCancel() complete") ));
    }
