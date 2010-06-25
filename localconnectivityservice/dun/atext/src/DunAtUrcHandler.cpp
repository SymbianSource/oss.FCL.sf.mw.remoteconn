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
* Description:  AT command URC handler
*
*/


#include <atext.h>
#include "DunAtCmdHandler.h"
#include "DunAtUrcHandler.h"
#include "DunDownstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler* CDunAtUrcHandler::NewL(
    RATExt* aAtCmdExt,
    MDunStreamManipulator* aStreamCallback )
    {
    CDunAtUrcHandler* self = NewLC( aAtCmdExt, aStreamCallback );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler* CDunAtUrcHandler::NewLC(
    RATExt* aAtCmdExt,
    MDunStreamManipulator* aStreamCallback )
    {
    CDunAtUrcHandler* self = new (ELeave) CDunAtUrcHandler( aAtCmdExt,
                                                            aStreamCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler::~CDunAtUrcHandler()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::~CDunAtUrcHandler()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtUrcHandler::~CDunAtUrcHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Don't close iAtCmdExt here (it is done in CDunAtCmdHandler)!
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtUrcHandler::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Starts waiting for an incoming URC message
// ---------------------------------------------------------------------------
//
TInt CDunAtUrcHandler::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::IssueRequest()") ));
    if ( iUrcHandleState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunAtUrcHandler::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    iStatus = KRequestPending;
    iAtCmdExt->ReceiveUnsolicitedResult( iStatus, iRecvBuffer, iOwnerUidPckg );
    SetActive();
    iUrcHandleState = EDunStateAtUrcHandling;
    // Next mark ownership
    if ( !iStarted )
        {
        TInt retTemp = iAtCmdExt->MarkUrcHandlingOwnership( iOwnerUid );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L("CDunAtUrcHandler::IssueRequest() (not owned) complete") ));
            return KErrGeneral;
            }
        }
    iStarted = ETrue;
    FTRACE(FPrint( _L("CDunAtUrcHandler::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops waiting for an incoming URC message
// ---------------------------------------------------------------------------
//
TInt CDunAtUrcHandler::Stop()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::Stop()") ));
    if ( iUrcHandleState != EDunStateAtUrcHandling )
        {
        FTRACE(FPrint( _L("CDunAtUrcHandler::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iAtCmdExt->CancelReceiveUnsolicitedResult( iOwnerUid );
    Cancel();
    iUrcHandleState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunAtUrcHandler::Stop() complete") ));
    // Note: Don't mark iStarted to EFalse here as it is used to get the UID
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// UID of the owning plugin
// ---------------------------------------------------------------------------
//
TUid CDunAtUrcHandler::OwnerUid()
    {
    return iOwnerUid;
    }

// ---------------------------------------------------------------------------
// CDunAtUrcHandler::CDunAtUrcHandler
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler::CDunAtUrcHandler( RATExt* aAtCmdExt,
                                    MDunStreamManipulator* aStreamCallback ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iStreamCallback( aStreamCallback ),
    iOwnerUidPckg( iOwnerUid )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtUrcHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::ConstructL()") ));
    if ( !iAtCmdExt || !iStreamCallback  )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunAtUrcHandler::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::Initialize()
    {
    // Don't initialize iStreamCallback here (it is set through NewL)
    iUrcHandleState = EDunStateIdle;
    iOwnerUid = TUid::Null();
    iStarted = EFalse;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when URC command received
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::RunL()
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::RunL()") ));
    iUrcHandleState = EDunStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtUrcHandler::RunL() (ERROR) complete (%d)"), retTemp));
        return;
        }
    if ( iRecvBuffer.Length() == 0 )
        {
        FTRACE(FPrint( _L("CDunAtUrcHandler::RunL() (empty buffer) complete") ));
        return;
        }

    // Send received URC message

    iStreamCallback->NotifyDataPushRequest( &iRecvBuffer, this );

    FTRACE(FPrint( _L("CDunAtUrcHandler::RunL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::DoCancel()
    {
    }

// ---------------------------------------------------------------------------
// From class MDunCompletionReporter.
// Gets called when data push is complete
// ---------------------------------------------------------------------------
//
void CDunAtUrcHandler::NotifyDataPushComplete( TBool /*aAllPushed*/ )
    {
    FTRACE(FPrint( _L("CDunAtUrcHandler::NotifyDataPushComplete()") ));
    IssueRequest();
    FTRACE(FPrint( _L("CDunAtUrcHandler::NotifyDataPushComplete() complete") ));
    }
