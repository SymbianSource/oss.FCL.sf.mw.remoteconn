/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include "keypresstimer.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========
// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CKeyPressTimer* CKeyPressTimer::NewL( MTimerNotifier* aTimeOutNotify,
        TTimeIntervalMicroSeconds32 aTimeOutTime, TTimerType aTimerType )
    {
    CKeyPressTimer* self = CKeyPressTimer::NewLC( aTimeOutNotify,
            aTimeOutTime, aTimerType );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//    
CKeyPressTimer* CKeyPressTimer::NewLC( MTimerNotifier* aTimeOutNotify,
        TTimeIntervalMicroSeconds32 aTimeOutTime, TTimerType aTimerType )
    {
    CKeyPressTimer* self = new ( ELeave ) CKeyPressTimer( aTimeOutNotify,
            aTimerType );
    CleanupStack::PushL( self );
    self->ConstructL( aTimeOutTime );
    return self;
    }

// ---------------------------------------------------------------------------
// CTimeOutTimer()
// ---------------------------------------------------------------------------
//
CKeyPressTimer::CKeyPressTimer( MTimerNotifier* aTimeOutNotify,
        TTimerType aTimerType ) :
    CTimer( EPriorityStandard ), iNotify( aTimeOutNotify ), iTimerType(
            aTimerType )
    {
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CKeyPressTimer::~CKeyPressTimer()
    {
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CKeyPressTimer::ConstructL( TTimeIntervalMicroSeconds32 aTimeOutTime )
    {
    TRACE_FUNC
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    After( aTimeOutTime );
    }

// ---------------------------------------------------------------------------
// From class CActive
// RunL()
// ---------------------------------------------------------------------------
//
void CKeyPressTimer::RunL()
    {
    TRACE_FUNC
    // Timer request has completed, so notify the timer's owner
    if ( iNotify )
        {
        iNotify->TimerExpired( iTimerType );
        }
    }
