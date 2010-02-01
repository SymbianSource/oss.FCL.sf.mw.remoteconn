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

__FLOG_STMT( _LIT8( KComponent, "mtpbtcontroller" ); )
LOCAL_D const TUid KMTPBtTransportUid = { 0x10286FCB };

CMTPBluetoothController* CMTPBluetoothController::NewL( CMTPBearerMonitor& aMon )
    {
    CMTPBluetoothController* self = new( ELeave ) CMTPBluetoothController( aMon );
    return self;
    }

CMTPBluetoothController::~CMTPBluetoothController()
    {
    delete iMTPOperator;
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

void CMTPBluetoothController::ManageService( TBool aStatus )
    {
    __FLOG_1( _L8("+/-ManageService( %d )"), aStatus );
    iStat = aStatus;
    TInt err = KErrNone;
    if ( !iMTPOperator )
        {
        TRAP( err, iMTPOperator = CMTPOperator::NewL( *this ) );
        }
    if ( KErrNone != err )
        {
        Monitor().ManageServiceCompleted( Bearer(), iStat, err );
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
    }

void CMTPBluetoothController::HandleStartTrasnportCompleteL( TInt aError )
    {
    __FLOG_1( _L8("+HandleStartTrasnportCompleteL( %d )"), aError );
    switch( aError )
        {
        case KErrServerBusy:// Another transport is running, keep observing the status of the transport bearer
            aError = KErrNone;
            break;
        default:
            break;
        }
    Monitor().ManageServiceCompleted( Bearer(), iStat, aError );
    __FLOG( _L8("-HandleStartTrasnportCompleteL") );
    }

void CMTPBluetoothController::HandleStopTrasnportCompleteL( TInt aError )
    {
    __FLOG_1( _L8("+HandleStopTrasnportCompleteL( %d )"), aError );
    Monitor().ManageServiceCompleted( Bearer(), iStat, aError );
    __FLOG( _L8("-HandleStopTrasnportCompleteL") );
    }

CMTPBluetoothController::CMTPBluetoothController( CMTPBearerMonitor& aMon ):
    CMTPControllerBase( aMon, ELocodBearerBT ),
    iStat( EFalse )
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }

