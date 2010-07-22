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
* Description:  HID Heaset plugin timeouttimer
*
*/


#include "timer.h"

// ======== MEMBER FUNCTIONS ========
// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CWaitTimer* CWaitTimer::NewL( TTimeIntervalMicroSeconds32 aTimeOutTime )
    {
    CWaitTimer* self = CWaitTimer::NewLC( aTimeOutTime );
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//    
CWaitTimer* CWaitTimer::NewLC( TTimeIntervalMicroSeconds32 aTimeOutTime )
    {
    CWaitTimer* self = new (ELeave) CWaitTimer();
    CleanupStack::PushL(self);
    self->ConstructL( aTimeOutTime );
    return self;
    }

// ---------------------------------------------------------------------------
// CTimeOutTimer()
// ---------------------------------------------------------------------------
//
CWaitTimer::CWaitTimer():
    CTimer( EPriorityStandard )    
    {
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CWaitTimer::~CWaitTimer()
    {
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CWaitTimer::ConstructL( TTimeIntervalMicroSeconds32 aTimeOutTime  )
    {
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    After( aTimeOutTime );
    iSyncWaiter.Start();
    }

// ---------------------------------------------------------------------------
// From class CActive
// RunL()
// ---------------------------------------------------------------------------
//
void CWaitTimer::RunL()
    {
    // Timer request has completed, so notify the timer's owner
    iSyncWaiter.AsyncStop();
    }
