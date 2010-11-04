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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpcontrollertimerTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPCONTROLLERTIMER_DES_ENTRY );
    OstTraceFunctionExit0( CMTPCONTROLLERTIMER_DES_EXIT );
    }

CMTPControllerTimer::CMTPControllerTimer( RMTPClient& aMTPClient, CMTPOperator& aMTPOperator ):
    CTimer( CActive::EPriorityStandard ), iMTPClient(aMTPClient)
    {
    iMTPOperator = &aMTPOperator;
    }

void CMTPControllerTimer::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPCONTROLLERTIMER_CONSTRUCTL_ENTRY );
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    iStopTransport = EFalse;
    OstTraceFunctionExit0( CMTPCONTROLLERTIMER_CONSTRUCTL_EXIT );
    }

void CMTPControllerTimer::RunL()
    {
    OstTraceFunctionEntry0( CMTPCONTROLLERTIMER_RUNL_ENTRY );
    if (KErrNone == iMTPClient.IsProcessRunning() && !iStopTransport)
        {
         OstTrace0( TRACE_NORMAL, CMTPCONTROLLERTIMER_RUNL, "Stop transport to shut down mtp server" );
        TInt error = iMTPClient.StopTransport(KMTPBtTransportUid);
        iMTPClient.Close();
        iStopTransport = ETrue;
        OstTrace1( TRACE_NORMAL, DUP1_CMTPCONTROLLERTIMER_RUNL, "The return value of stop transport is: %d", error );
        iMTPOperator->StartTimer(KStartMTPSeconds);
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP2_CMTPCONTROLLERTIMER_RUNL, "Start transport to launch mtp server" );
        
		TInt err = iMTPClient.Connect();
		if ( err == KErrNone )
			{
			iMTPClient.StartTransport(KMTPBtTransportUid);
	        iStopTransport = EFalse;
		   iMTPOperator->SubscribeConnState();
			}
		else
			{
			OstTrace1( TRACE_NORMAL, DUP3_CMTPCONTROLLERTIMER_RUNL, "connect to mtp server failed! error code %d", err );
			}
        
        }
    OstTraceFunctionExit0( CMTPCONTROLLERTIMER_RUNL_EXIT );
    }
   
