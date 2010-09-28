/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Echoer for AT commands
*
*/

#include "DunAtCmdEchoer.h"
#include "DunDownstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdEchoer* CDunAtCmdEchoer::NewL(
    MDunStreamManipulator* aStreamCallback )
    {
    CDunAtCmdEchoer* self = new (ELeave) CDunAtCmdEchoer( aStreamCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdEchoer::~CDunAtCmdEchoer()
    {
    FTRACE(FPrint( _L("CDunAtCmdEchoer::~CDunAtCmdEchoer()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtCmdEchoer::~CDunAtCmdEchoer() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtCmdEchoer::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtCmdEchoer::ResetData()") ));
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtCmdEchoer::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Sends a character to be echoed
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdEchoer::SendEchoCharacter(
    const TDesC8* aInput,
    MDunAtCmdEchoer* aCallback )
    {
    FTRACE(FPrint( _L("CDunAtCmdEchoer::SendEchoCharacter()") ));
    if ( iAtEchoState!=EDunStateIdle || iCallback )
        {
        FTRACE(FPrint( _L("CDunAtCmdEchoer::SendEchoCharacter() (not ready) complete") ));
        return KErrNotReady;
        }
    iAtEchoState = EDunStateAtCmdEchoing;
    iStreamCallback->NotifyDataPushRequest( aInput, this );
    iCallback = aCallback;
    FTRACE(FPrint( _L("CDunDownstream::SendEchoCharacter() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunAtCmdEchoer::CDunAtCmdEchoer
// ---------------------------------------------------------------------------
//
CDunAtCmdEchoer::CDunAtCmdEchoer( MDunStreamManipulator* aStreamCallback ) :
    iStreamCallback( aStreamCallback )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtCmdEchoer::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtCmdEchoer::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtCmdEchoer::ConstructL()") ));
    if ( !iStreamCallback )
        {
        FTRACE(FPrint( _L("CDunAtCmdEchoer::ConstructL() complete") ));
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunAtCmdEchoer::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdEchoer::Initialize()
    {
    // Don't initialize iStreamCallback here (it is set through NewL)
    iCallback = NULL;
    iAtEchoState = EDunStateIdle;
    }

// ---------------------------------------------------------------------------
// From MDunCompletionReporter.
// Gets called when data push is complete
// ---------------------------------------------------------------------------
//
void CDunAtCmdEchoer::NotifyDataPushComplete( TBool /*aAllPushed*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdEchoer::NotifyDataPushComplete()") ));
    MDunAtCmdEchoer* callback = iCallback;
    iCallback = NULL;
    iAtEchoState = EDunStateIdle;
    callback->NotifyEchoComplete();
    FTRACE(FPrint( _L("CDunAtCmdEchoer::NotifyDataPushComplete() complete") ));
    }
