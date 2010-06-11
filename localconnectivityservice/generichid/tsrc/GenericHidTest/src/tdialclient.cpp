/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialhandling class for hid test application
*
*/


#include "tdialclient.h"

// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CDialClient* CDialClient::NewL()
    {
    CDialClient* self = CDialClient::NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// ?description_if_needed
// ---------------------------------------------------------------------------
//
CDialClient* CDialClient::NewLC()
    {
    CDialClient* self = new( ELeave ) CDialClient;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CDialClient::CDialClient()
: CActive(EPriorityNormal), iCallParamsPckg(iCallParams),
  iCallStarted(EFalse)
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CDialClient::ConstructL()
    {
    iTelephony = CTelephony::NewL();
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDialClient::~CDialClient()
    {
    Cancel();
    delete iTelephony;
    }

// ---------------------------------------------------------------------------
// RunL
// ---------------------------------------------------------------------------
//
void CDialClient::RunL()
    {
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    }

// ---------------------------------------------------------------------------
// DoCancel()
// ---------------------------------------------------------------------------
//
void CDialClient::DoCancel()
    {
        
    }
// ---------------------------------------------------------------------------
// CreateCall()
// ---------------------------------------------------------------------------
//
void CDialClient::CreateCall(const TDesC& aNumber)
    {
    if ( IsActive() )
        {
        return;
        }
    CTelephony::TTelNumber telNumber(aNumber);       
    iCallParams.iIdRestrict = CTelephony::ESendMyId;        
    iTelephony->DialNewCall(iStatus, iCallParamsPckg, telNumber, iCallId);
    
    SetActive();    
    iSyncWaiter.Start();    
    }

// ---------------------------------------------------------------------------
// HangUp()
// ---------------------------------------------------------------------------
//
void CDialClient::HangUp()
    {    
    iTelephony->Hangup( iStatus , iCallId );    
    }
