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
* Description:  Common waiter for closing self-destruct plugins
*
*/

#include "DunCloseWait.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunCloseWait* CDunCloseWait::NewL( MDunCloseWait* aCallback )
    {
    CDunCloseWait* self = new (ELeave) CDunCloseWait( aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunCloseWait::~CDunCloseWait()
    {
    FTRACE(FPrint( _L("CDunCloseWait::~CDunCloseWait()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunCloseWait::~CDunCloseWait() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunCloseWait::ResetData()
    {
    // IssueRequest()
    Stop();
    // AddPluginToClose()
    iCloseList.Close();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Adds a new object to be destructed to the list
// ---------------------------------------------------------------------------
//
TInt CDunCloseWait::AddPluginToClose( MDunLocalMediaPlugin* aPluginToClose )
    {
    FTRACE(FPrint( _L("CDunCloseWait::AddPluginToClose()" ) ));
    if ( !aPluginToClose )
        {
        FTRACE(FPrint( _L("CDunCloseWait::AddPluginToClose() (ERROR) complete" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCloseList.Find( aPluginToClose );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunCloseWait::AddPluginToClose() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCloseList.Append( aPluginToClose );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunCloseWait::AddPluginToClose() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunCloseWait::AddPluginToClose() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Issues request to start closing the objects in the close list
// ---------------------------------------------------------------------------
//
TInt CDunCloseWait::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunCloseWait::IssueRequest()" )));
    if ( IsActive() )
        {
        FTRACE(FPrint( _L("CDunCloseWait::IssueRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* requestStatus = &iStatus;
    User::RequestComplete( requestStatus, KErrNone );
    FTRACE(FPrint( _L("CDunCloseWait::IssueRequest() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops closing the objects in the close list
// ---------------------------------------------------------------------------
//
TInt CDunCloseWait::Stop()
    {
    FTRACE(FPrint( _L("CDunCloseWait::Stop()" )));
    if ( !IsActive() )
        {
        FTRACE(FPrint( _L("CDunCloseWait::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    Cancel();
    FTRACE(FPrint( _L("CDunCloseWait::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunCloseWait::CDunCloseWait
// ---------------------------------------------------------------------------
//
CDunCloseWait::CDunCloseWait( MDunCloseWait* aCallback ) :
    CActive( EPriorityHigh ),
    iCallback( aCallback )
    {
    }

// ---------------------------------------------------------------------------
// CDunCloseWait::ConstructL
// ---------------------------------------------------------------------------
//
void CDunCloseWait::ConstructL()
    {
    FTRACE(FPrint( _L("CDunCloseWait::ConstructL()" ) ));
    if ( !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunCloseWait::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunCloseWait::Initialize()
    {
    // Don't initialize iCallback here (it is set through NewL)
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when closing of the plugins should be done
// ---------------------------------------------------------------------------
//
void CDunCloseWait::RunL()
    {
    FTRACE(FPrint( _L("CDunCloseWait::RunL()" )));
    iCallback->NotifyPluginCloseAfterWait( iCloseList );
    iCloseList.Reset();
    FTRACE(FPrint( _L("CDunCloseWait::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunCloseWait::DoCancel()
    {
    }
