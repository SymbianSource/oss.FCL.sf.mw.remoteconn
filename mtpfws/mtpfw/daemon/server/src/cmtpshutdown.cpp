// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include <e32std.h>

#include "cmtpshutdown.h"
#include "cmtpdataprovidercontroller.h"


/**
CMTPShutdown factory method. 
@return A pointer to a new CMTPShutdown instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPShutdown* CMTPShutdown::NewL()
    {
    CMTPShutdown* self = new (ELeave) CMTPShutdown;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor.
*/
CMTPShutdown::~CMTPShutdown()
    {
    Cancel();
    iSingletons.Close();
    }
    
/**
Starts the shutdown timer.
*/   
void CMTPShutdown::Start()
    {
    // Shutdown delay, in microseconds.
    const TUint KMTPShutdownDelay = (1000000 * 5);
    After(KMTPShutdownDelay);
    }
    
void CMTPShutdown::RunL()
    {
    if ( iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted )
        {
        Start();
        }
    else
        {
        CActiveScheduler::Stop();
        }
    }
    
/** 
Constructor
*/
CMTPShutdown::CMTPShutdown() : 
    CTimer(EPriorityNormal)
    {
    
    }    

/**
Second phase constructor.
*/    
void CMTPShutdown::ConstructL()
    {
    CTimer::ConstructL();
    iSingletons.OpenL();
    CActiveScheduler::Add(this);
    }
