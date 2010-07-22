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
* Description:  ECom plugin install/uninstall/version listener
*
*/


#include "DunAtEcomListen.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtEcomListen* CDunAtEcomListen::NewL(
    RATExt* aAtCmdExt,
    MDunAtEcomListen* aCallback )
    {
    CDunAtEcomListen* self = NewLC( aAtCmdExt, aCallback );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtEcomListen* CDunAtEcomListen::NewLC(
    RATExt* aAtCmdExt,
    MDunAtEcomListen* aCallback )
    {
    CDunAtEcomListen* self = new (ELeave) CDunAtEcomListen( aAtCmdExt,
                                                            aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtEcomListen::~CDunAtEcomListen()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::~CDunAtEcomListen()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtEcomListen::~CDunAtEcomListen() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtEcomListen::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtEcomListen::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Starts waiting for ECom plugin install/uninstall/version status changes
// ---------------------------------------------------------------------------
//
TInt CDunAtEcomListen::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::IssueRequest()") ));
    if ( iEcomListenState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunAtEcomListen::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    iStatus = KRequestPending;
    iAtCmdExt->ReceiveEcomPluginChange( iStatus, iPluginUidPckg, iEcomTypePckg );
    SetActive();
    iEcomListenState = EDunStateEcomListening;
    FTRACE(FPrint( _L("CDunAtEcomListen::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops waiting for Ecom plugin install/uninstall/version status changes
// ---------------------------------------------------------------------------
//
TInt CDunAtEcomListen::Stop()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::Stop()") ));
    if ( iEcomListenState != EDunStateEcomListening )
        {
        FTRACE(FPrint( _L("CDunAtEcomListen::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iAtCmdExt->CancelReceiveEcomPluginChange();
    Cancel();
    iEcomListenState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunAtEcomListen::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunAtEcomListen::CDunAtEcomListen
// ---------------------------------------------------------------------------
//
CDunAtEcomListen::CDunAtEcomListen( RATExt* aAtCmdExt,
                                    MDunAtEcomListen* aCallback ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iCallback( aCallback ),
    iPluginUidPckg( iPluginUid ),
    iEcomTypePckg( EEcomTypeUninstall )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtEcomListen::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtEcomListen::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::ConstructL()") ));
    if ( !iAtCmdExt || !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunAtEcomListen::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtEcomListen::Initialize()
    {
    // Don't initialize iAtCmdExt here (it is set through NewL)
    // Don't initialize iCallback here (it is set through NewL)
    iEcomListenState = EDunStateIdle;
    iPluginUid = TUid::Null();
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when plugin installed, uninstalled or changed
// ---------------------------------------------------------------------------
//
void CDunAtEcomListen::RunL()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::RunL()") ));
    iEcomListenState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtEcomListen::RunL() (ERROR) complete (%d)"), retTemp ));
        return;
        }
    IssueRequest();
    // Now the operation is either unintall or install of plugin
    // Notify parent and ATEXT
    if ( iEcomTypePckg() == EEcomTypeUninstall )
        {
        iCallback->NotifyPluginUninstallation( iPluginUid );
        }
    else if ( iEcomTypePckg() == EEcomTypeInstall )
        {
        iCallback->NotifyPluginInstallation( iPluginUid );
        }
    else
        {
        FTRACE(FPrint( _L("CDunAtEcomListen::RunL() (not supported) complete") ));
        }
    FTRACE(FPrint( _L("CDunAtEcomListen::RunL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunAtEcomListen::DoCancel()
    {
    FTRACE(FPrint( _L("CDunAtEcomListen::DoCancel()") ));
    FTRACE(FPrint( _L("CDunAtEcomListen::DoCancel() complete") ));
    }
