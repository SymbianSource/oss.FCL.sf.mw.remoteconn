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

#include "cmtpoperator.h"

__FLOG_STMT( _LIT8( KComponent, "mtpoperator" ); )

CMTPOperator* CMTPOperator::NewL( MMTPOperatorNotifier& aNotifier )
    {
    CMTPOperator* self = new( ELeave ) CMTPOperator( aNotifier );
    self->ConstructL();
    return self;
    }

CMTPOperator::~CMTPOperator()
    {
    Cancel();
    iPendingOperations.Reset();
    iPendingOperations.Close();
    iMTPClient.Close();
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

void CMTPOperator::StartTransport( TUid aTransport )
    {
    __FLOG_1( _L8("+/-StartTransport( 0x%08X )"), aTransport.iUid );
    TInt err = AppendOperation( EStartTransport, aTransport );
    if ( KErrNone != err )
        {
        iNotifier.HandleStartTrasnportCompleteL( err );
        }
    }

void CMTPOperator::StopTransport( TUid aTransport )
    {
    __FLOG_1( _L8("+/-StopTransport( 0x%08X )"), aTransport.iUid );
    TInt err = AppendOperation( EStopTransport, aTransport );
    if ( KErrNone != err )
        {
        iNotifier.HandleStartTrasnportCompleteL( err );
        }
    }

void CMTPOperator::DoCancel()
    {
    __FLOG( _L8("+/-DoCancel") );
    }

void CMTPOperator::RunL()
    {
    __FLOG( _L8("+RunL") );
    
    TInt count = iPendingOperations.Count();
    if ( count > 0 )
        {
        TOperation& operation = iPendingOperations[0];
        TRAP_IGNORE( HandleOperationL( operation ) );
        iPendingOperations.Remove( 0 );
        }
    
    __FLOG( _L8("-RunL") );
    }

CMTPOperator::CMTPOperator( MMTPOperatorNotifier& aNotifier ):
    CActive( EPriorityStandard ),
    iNotifier( aNotifier )
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }

void CMTPOperator::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    CActiveScheduler::Add( this );
    User::LeaveIfError( iMTPClient.Connect() );
    __FLOG( _L8("-ConstructL") );
    }

TInt CMTPOperator::AppendOperation( TOperationType aType, TUid aTransport )
    {
    TOperation operation = { aType, aTransport };
    TInt err = iPendingOperations.Append( operation );
    if ( ( KErrNone == err ) && !IsActive() )
        {
        Schedule( KErrNone );
        }
    __FLOG_1( _L8("+/-AppendOperation returns %d"), err );
    return err;
    }

void CMTPOperator::Schedule( TInt aError )
    {
    __FLOG_1( _L8("+/-Schedule( %d )"), aError );
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, aError );
    SetActive();
    }

void CMTPOperator::HandleOperationL( const TOperation& aOperation )
    {
    __FLOG_2( _L8("+HandleOperationL( 0x%08X, 0x%08X )"), aOperation.iTransport.iUid, aOperation.iType );
    TInt err = KErrNone;
    switch ( aOperation.iType )
        {
        case EStartTransport:
            err = iMTPClient.StartTransport( aOperation.iTransport );
            iNotifier.HandleStartTrasnportCompleteL( err );
            break;
        default:
            __ASSERT_DEBUG( ( EStopTransport == aOperation.iType ), User::Invariant() );
            err = iMTPClient.StopTransport( aOperation.iTransport );
            iNotifier.HandleStopTrasnportCompleteL( err );
            break;
        }
    __FLOG( _L8("-HandleOperationL") );
    }

