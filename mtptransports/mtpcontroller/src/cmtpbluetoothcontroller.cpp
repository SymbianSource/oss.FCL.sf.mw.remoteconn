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

#include "cmtpbluetoothcontroller.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpbluetoothcontrollerTraces.h"
#endif

LOCAL_D const TUid KMTPBtTransportUid = { 0x10286FCB };

CMTPBluetoothController* CMTPBluetoothController::NewL( CMTPBearerMonitor& aMon )
    {
    CMTPBluetoothController* self = new( ELeave ) CMTPBluetoothController( aMon );
    return self;
    }

CMTPBluetoothController::~CMTPBluetoothController()
    {
    OstTraceFunctionEntry0( CMTPBLUETOOTHCONTROLLER_DES_ENTRY );
    delete iMTPOperator;
    OstTraceFunctionExit0( CMTPBLUETOOTHCONTROLLER_DES_EXIT );
    }

void CMTPBluetoothController::ManageService( TBool aStatus )
    {
    OstTraceFunctionEntry0( CMTPBLUETOOTHCONTROLLER_MANAGESERVICE_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPBLUETOOTHCONTROLLER_MANAGESERVICE, "The status is %d", aStatus );
    iStat = aStatus;
    TInt err = KErrNone;
    if ( !iMTPOperator )
        {
        TRAP( err, iMTPOperator = CMTPOperator::NewL( *this ) );
        }
    if ( KErrNone != err )
        {
        Monitor().ManageServiceCompleted( Bearer(), iStat, err );
        OstTraceFunctionExit0( CMTPBLUETOOTHCONTROLLER_MANAGESERVICE_EXIT );
        return;
        }
    
    if ( aStatus )
        {
        iMTPOperator->StartTransport( KMTPBtTransportUid );
        }
    else
        {
        iMTPOperator->StopTransport( KMTPBtTransportUid );
        }
    OstTraceFunctionExit0( DUP1_CMTPBLUETOOTHCONTROLLER_MANAGESERVICE_EXIT );
    }

void CMTPBluetoothController::HandleStartTrasnportCompleteL( TInt aError )
    {
    OstTraceFunctionEntry0( CMTPBLUETOOTHCONTROLLER_HANDLESTARTTRASNPORTCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPBLUETOOTHCONTROLLER_HANDLESTARTTRASNPORTCOMPLETEL, "The error is %d", aError );
    switch( aError )
        {
        case KErrServerBusy:// Another transport is running, keep observing the status of the transport bearer
            aError = KErrNone;
            break;
        default:
            break;
        }
    Monitor().ManageServiceCompleted( Bearer(), iStat, aError );
    OstTraceFunctionExit0( CMTPBLUETOOTHCONTROLLER_HANDLESTARTTRASNPORTCOMPLETEL_EXIT );
    }

void CMTPBluetoothController::HandleStopTrasnportCompleteL( TInt aError )
    {
    OstTraceFunctionEntry0( CMTPBLUETOOTHCONTROLLER_HANDLESTOPTRASNPORTCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPBLUETOOTHCONTROLLER_HANDLESTOPTRASNPORTCOMPLETEL, "The error is %d", aError );
    Monitor().ManageServiceCompleted( Bearer(), iStat, aError );
    OstTraceFunctionExit0( CMTPBLUETOOTHCONTROLLER_HANDLESTOPTRASNPORTCOMPLETEL_EXIT );
    }

CMTPBluetoothController::CMTPBluetoothController( CMTPBearerMonitor& aMon ):
    CMTPControllerBase( aMon, ELocodBearerBT ),
    iStat( EFalse )
    {
    OstTraceFunctionEntry0( CMTPBLUETOOTHCONTROLLER_CONS_ENTRY );
    OstTraceFunctionExit0( CMTPBLUETOOTHCONTROLLER_CONS_EXIT );
    }

