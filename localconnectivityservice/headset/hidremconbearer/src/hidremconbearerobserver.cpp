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
* Description:  Remcon bearer observer implementation
 *
*/


//INCLUDE
#include <e32def.h>
#include <e32cmn.h>
#include <coreapplicationuisdomainpskeys.h>

#include "hidremconbearerinternalpskeys.h"
#include "hidremconbearerobserver.h"
#include "debug.h"
// ======== MEMBER FUNCTIONS ========
//
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::NewL()
// Constructs a new entry with given values.
// ---------------------------------------------------------------------------
//
CHidRemconBearerObserver* CHidRemconBearerObserver::NewL(
        MCallBackReceiver& aCallback, TInt aKeyType )
    {
    CHidRemconBearerObserver* self = new ( ELeave ) CHidRemconBearerObserver(
            aCallback );
    CleanupStack::PushL( self );
    self->ConstructL( aKeyType );
    CleanupStack::Pop();
    return self;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::CHidRemconBearerObserver()
// C++ constructor
// ---------------------------------------------------------------------------
// 
CHidRemconBearerObserver::CHidRemconBearerObserver(
        MCallBackReceiver& aCallback ) :
    CActive( EPriorityStandard ), iCallback( aCallback )
    {
    //Pass        
    TRACE_FUNC
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::CHidRemconBearerObserver()
// Destructor
// ---------------------------------------------------------------------------
//
CHidRemconBearerObserver::~CHidRemconBearerObserver()
    {
    TRACE_FUNC
    Stop();
    if ( iProperty.Handle() != KNullHandle )
        {
        iProperty.Close();
        }
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::ConstructL()
// Symbian OS default constructor
// ---------------------------------------------------------------------------
// 
void CHidRemconBearerObserver::ConstructL( TInt aKeyType )
    {
    TRACE_FUNC
    iKeyType = aKeyType;
    // Add this active object to the scheduler.
    CActiveScheduler::Add( this );
    switch ( iKeyType )
        {
        case EMediaKeys:
            User::LeaveIfError( iProperty.Attach( KPSUidHidEventNotifier,
                    KHidControlKeyEvent ) );
            break;
        case EAccessoryVolumeKeys:
            User::LeaveIfError( iProperty.Attach( KPSUidHidEventNotifier,
                    KHidAccessoryVolumeEvent ) );
            break;
        case EMuteKey:
            User::LeaveIfError( iProperty.Attach( KPSUidHidEventNotifier,
                    KHidMuteKeyEvent ) );
            break;
        case EHookKeys:
            User::LeaveIfError( iProperty.Attach( KPSUidHidEventNotifier,
                    KHidHookKeyEvent ) );
            break;
        default:
            User::Leave( KErrArgument );
        }
    Start();
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::Start()
// Starts listening KUidCurrentCall event  
// ---------------------------------------------------------------------------
//
TInt CHidRemconBearerObserver::Start()
    {
    TRACE_FUNC
    if ( IsActive() )
        {
        return KErrInUse;
        }
    iStatus = KRequestPending;
    iProperty.Subscribe( iStatus );
    SetActive();
    return KErrNone;
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::Stop()
// Stops listening KUidCurrentCall event 
// ---------------------------------------------------------------------------
//
void CHidRemconBearerObserver::Stop()
    {
    TRACE_FUNC
    Cancel();
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::RunL()
// ---------------------------------------------------------------------------
// 
void CHidRemconBearerObserver::RunL()
    {
    TInt scanCode;
    TInt ret = 0;
    ret = iProperty.Get( scanCode );
    if ( ret == KErrNone )
        {
        // If this Active Object is for receiving the USB MTP status, the
        // scanCode varibale contains the status whether transfer is 
        // happening now(active) or not(not active).    
        iCallback.ReceivedKeyEvent( scanCode, iKeyType );
        }
    }
// ---------------------------------------------------------------------------
// CHidRemconBearerObserver::DoCancel()
// Cancels event listening
// ---------------------------------------------------------------------------
// 
void CHidRemconBearerObserver::DoCancel()
    {
    iProperty.Cancel();
    }
