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
* Description:  Controls asynchronous requests between LOCOD and DUN server
*
*/


#include "dunactive.h"
#include "dunplugin.h"
#include "DunDebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunActive* CDunActive::NewL( CDunPlugin* aPlugin,
                              TLocodBearer aBearer,
                              TBool aBearerStatus)
    {
    CDunActive* self = CDunActive::NewLC( aPlugin, aBearer, aBearerStatus );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunActive* CDunActive::NewLC( CDunPlugin* aPlugin,
                               TLocodBearer aBearer,
                               TBool aBearerStatus )
    {
    CDunActive* self = new (ELeave) CDunActive( aPlugin,
                                                aBearer,
                                                aBearerStatus );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunActive::~CDunActive()
    {
    FTRACE(FPrint( _L("CDunActive::~CDunActive()") ));
    Cancel();
    FTRACE(FPrint( _L("CDunActive::~CDunActive() complete") ));
    }

// ---------------------------------------------------------------------------
// Request function
// ---------------------------------------------------------------------------
//
TInt CDunActive::ServiceRequest()
    {
    FTRACE(FPrint( _L("CDunActive::ServiceRequest()") ));
    TInt err = KErrNone;

    iStatus = KRequestPending;
    err = (iPlugin->Server()).ManageService( this, iStatus );
    SetActive();
    FTRACE(FPrint( _L("CDunActive::ServiceRequest() complete") ));
    return err;
    }

// ---------------------------------------------------------------------------
// CDunActive::CDunActive
// ---------------------------------------------------------------------------
//
CDunActive::CDunActive( CDunPlugin* aPlugin,
                        TLocodBearer aBearer,
                        TBool aBearerStatus )
    : CActive( EPriorityStandard ),
    iPlugin( aPlugin ),
    iBearer( aBearer ),
    iBearerStatus( aBearerStatus )
    {
    }

// ---------------------------------------------------------------------------
// CDunActive::ConstructL
// ---------------------------------------------------------------------------
//
void CDunActive::ConstructL()
    {
    FTRACE(FPrint( _L("CDunActive::ConstructL()") ));
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunActive::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Cancel current activity.
// ---------------------------------------------------------------------------
//
void CDunActive::DoCancel()
    {
    }

// ---------------------------------------------------------------------------
// From class CActive
// Called when read or write operation is ready.
// ---------------------------------------------------------------------------
//
void CDunActive::RunL()
    {
    FTRACE(FPrint( _L("CDunActive::RunL()") ));
    TInt retVal = iStatus.Int();
    iPlugin->ServiceCompleted( this, retVal );
    FTRACE(FPrint( _L("CDunActive::RunL() complete (%d)"), retVal ));
    }

// ---------------------------------------------------------------------------
// From class MDunActive.
// Bearer
// ---------------------------------------------------------------------------
//
TLocodBearer CDunActive::Bearer()
    {
    FTRACE(FPrint( _L("CDunActive::Bearer()") ));
    FTRACE(FPrint( _L("CDunActive::Bearer() complete") ));
    return iBearer;
    }

// ---------------------------------------------------------------------------
// From class MDunActive.
// Bearer status
// ---------------------------------------------------------------------------
//
TBool CDunActive::BearerStatus()
    {
    FTRACE(FPrint( _L("CDunActive::BearerStatus()") ));
    FTRACE(FPrint( _L("CDunActive::BearerStatus() complete") ));
    return iBearerStatus;
    }
