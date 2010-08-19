// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file
 @internalComponent
*/

#include "cmtpcontrollertimer.h"
#include "cmtpoperator.h"

__FLOG_STMT( _LIT8( KComponent, "mtpConTimer" ); )

const TUid KMTPBtTransportUid = { 0x10286FCB };
const TInt KStartMTPSeconds = 7;

CMTPControllerTimer* CMTPControllerTimer::NewLC( RMTPClient& aMTPClient, CMTPOperator & aMTPOperator )
    {
    CMTPControllerTimer* self = new(ELeave) CMTPControllerTimer( aMTPClient, aMTPOperator );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CMTPControllerTimer* CMTPControllerTimer::NewL( RMTPClient& aMTPClient, CMTPOperator & aMTPOperator )
    {
    CMTPControllerTimer* self = NewLC( aMTPClient, aMTPOperator );
    CleanupStack::Pop( self );
    return self;
    }

void CMTPControllerTimer::Start( TInt aTimeOut )
    {
    CTimer::After( aTimeOut * ETimerMultiplier );
    }

TBool CMTPControllerTimer::GetStopTransportStatus()
    {
    return iStopTransport;
    }

CMTPControllerTimer::~CMTPControllerTimer()
    {
    __FLOG( _L8("CMPTControllerTimer destruction") );
    __FLOG_CLOSE;
    }

CMTPControllerTimer::CMTPControllerTimer( RMTPClient& aMTPClient, CMTPOperator& aMTPOperator ):
    CTimer( CActive::EPriorityStandard ), iMTPClient(aMTPClient)
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    iMTPOperator = &aMTPOperator;
    }

void CMTPControllerTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    iStopTransport = EFalse;
    __FLOG( _L8("CMPTControllerTimer construction") );
    }

void CMTPControllerTimer::RunL()
    {
    if (KErrNone == iMTPClient.IsProcessRunning() && !iStopTransport)
        {
        __FLOG( _L8("Stop transport to shut down mtp server") );
        TInt error = iMTPClient.StopTransport(KMTPBtTransportUid);
        iMTPClient.Close();
        iStopTransport = ETrue;
        __FLOG_1( _L8("The return value of stop transport is: %d"), error );
        iMTPOperator->StartTimer(KStartMTPSeconds);
        }
    else
        {
        __FLOG( _L8("Start transport to launch mtp server") );
        
        User::LeaveIfError(iMTPClient.Connect());
        iMTPClient.StartTransport(KMTPBtTransportUid);
        iStopTransport = EFalse;
        iMTPOperator->SubscribeConnState();
        }
    }
