/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Manages note showing in UI
*
*/


#include "DunNoteHandler.h"
#include "DunDebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunNoteHandler* CDunNoteHandler::NewL()
    {
    CDunNoteHandler* self = new (ELeave) CDunNoteHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDunNoteHandler::~CDunNoteHandler()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::~CDunNoteHandler()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunNoteHandler::~CDunNoteHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::ResetData()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    delete iNote;
    iNote = NULL;
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunNoteHandler::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Issues request to start showing UI note
// ---------------------------------------------------------------------------
//
TInt CDunNoteHandler::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest()") ));
    if ( iNoteState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    TRAPD( retTrap, DoIssueRequestL() );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() (trapped!) complete (%d)"), retTrap));
        return retTrap;
        }
    iNoteState = EDunStateUiNoting;
    FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops showing UI note
// ---------------------------------------------------------------------------
//
TInt CDunNoteHandler::Stop()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::Stop()") ));
    if ( iNoteState != EDunStateUiNoting )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::Stop() (not ready) complete") ));
        return KErrNotReady;
        }
    if ( !iNote )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::Stop() (iNote not initialized!) complete") ));
        return KErrGeneral;
        }
    iNote->Close();
    iNoteState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunNoteHandler::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunNoteHandler::CDunNoteHandler
// ---------------------------------------------------------------------------
//
CDunNoteHandler::CDunNoteHandler()
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunNoteHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::ConstructL()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::ConstructL()") ));
    FTRACE(FPrint( _L("CDunNoteHandler::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::Initialize()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::Initialize()" ) ));
    iNote = NULL;
    iNoteState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunNoteHandler::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Issues request to start showing UI note
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::DoIssueRequestL()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL()") ));
    if ( iNote )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL() (ERROR) complete") ));
        User::Leave( KErrGeneral );
        }

    CHbDeviceMessageBoxSymbian* messageBox =
            CHbDeviceMessageBoxSymbian::NewL(
                    CHbDeviceMessageBoxSymbian::EWarning);
    CleanupStack::PushL(messageBox);
    //ToDo: Need to use localised strings.
    _LIT(KText, "Maximum number of dialup-connections. Dial-up failed.");
    messageBox->SetTextL(KText);
    messageBox->SetObserver(this);
    messageBox->SetTimeout(0);
    messageBox->ShowL();
    CleanupStack::Pop(messageBox);
    iNote = messageBox;
    FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class MHbDeviceMessageBoxObserver.
// Gets called on dialog close.
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::MessageBoxClosed(
    const CHbDeviceMessageBoxSymbian* /*aMessageBox*/,
    CHbDeviceMessageBoxSymbian::TButtonId /*aButton*/ )
    {
    FTRACE(FPrint( _L("CDunNoteHandler::MessageBoxClosed()" ) ));
    iNoteState = EDunStateIdle;
    delete iNote;
    iNote = NULL;
    FTRACE(FPrint( _L("CDunNoteHandler::MessageBoxClosed() complete" ) ));
    }
