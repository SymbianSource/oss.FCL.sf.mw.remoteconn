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
    iProperty.Close();
    delete iTimer;
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

void CMTPOperator::StartTimer(TInt aSecond)
    {
    __FLOG(_L8("StartTimer in cmtpoperator!"));
    iTimer->Start(aSecond);    
    }

void CMTPOperator::DoCancel()
    {
    __FLOG( _L8("+/-DoCancel") );
    iProperty.Cancel();
    iConSubscribed = EFalse;
    }

void CMTPOperator::RunL()
    {
    __FLOG( _L8("+RunL") );
    
    iConSubscribed = EFalse;
    TInt count = iPendingOperations.Count();
    
    TInt connState = KInitialValue;
    
    if ( count > 0 )
        {
        TOperation& operation = iPendingOperations[0];
        TRAP_IGNORE( HandleOperationL( operation ) );
        iPendingOperations.Remove( 0 );
        }
    else
        {
        //this will go on to get the updated connection status.
        SubscribeConnState();


        TInt error = iProperty.Get(KMTPPublishConnStateCat, EMTPConnStateKey, connState);
        __FLOG_2(_L8("Before, the iConnState is %d and connState is %d"), iConnState, connState);
        if ( KErrNotFound == error )
            {
            iConnState = KInitialValue;
            __FLOG( _L8("The key is deleted and mtp server shut down!") );
            }
        else
            {
            if (iTimer->IsActive() && !iTimer->GetStopTransportStatus())
                {
                __FLOG( _L8("Timer is cancelled!") );
                iTimer->Cancel();
                }
            //if the disconnect is not set, set the disconnect
            //else if the connState is disconnect, launch the timer to restart the server to unload dps.
            if ( KInitialValue == iConnState )
                {
                iConnState = connState;
                __FLOG( _L8("the first time to launch mtp") );
                }
            else
                {
                if (EDisconnectedFromHost == connState)
                    {
                    iConnState = connState;
                    if (!iTimer->IsActive())
                        {
                        iTimer->Start(KStopMTPSeconds);
                        }
                    __FLOG( _L8("Timer is launched.") );
                    }
                else
                    {

                    iConnState = connState;
                    }
                }
            }
        __FLOG_2(_L8("After, the iConnState is %d and connState is %d"), iConnState, connState);
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
    //if the server is running, the first disconnction shows the conection is down!
    if(KErrNone == iMTPClient.IsProcessRunning())
        {
        iConnState = EDisconnectedFromHost;
        }
    else
        {
        iConnState = KInitialValue;
        }
    __FLOG_1( _L8("The connstate is set to %d"), iConnState );
    User::LeaveIfError( iMTPClient.Connect() );
    User::LeaveIfError(iProperty.Attach(KMTPPublishConnStateCat, EMTPConnStateKey));
    iTimer = CMTPControllerTimer::NewL(iMTPClient, *this);
    
    iConSubscribed = EFalse;
    __FLOG( _L8("-ConstructL") );
    }

TInt CMTPOperator::AppendOperation( TOperationType aType, TUid aTransport )
    {
    TOperation operation = { aType, aTransport };
    TInt err = iPendingOperations.Append( operation );
    __FLOG_1( _L8("+AppendOperation returns %d"), err );
    if ( ( KErrNone == err ) && !IsActive() )
        {
        Schedule( KErrNone );
        }
    else
        {
        if (iConSubscribed)
            {
            Cancel();
            if (KErrNone == err)
                {
                Schedule( KErrNone );
                }
            }
        }
    __FLOG( _L8("-AppendOperation") );
    return err;
    }

void CMTPOperator::Schedule( TInt aError )
    {
    __FLOG_1( _L8("+/-Schedule( %d )"), aError );
    if(iTimer->IsActive())
        {
        iTimer->Cancel();
        }
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
            SubscribeConnState();
            break;
        default:
            __ASSERT_DEBUG( ( EStopTransport == aOperation.iType ), User::Invariant() );
            if(!iTimer->GetStopTransportStatus())
                {
                err = iMTPClient.StopTransport( aOperation.iTransport );
                }
         
            iNotifier.HandleStopTrasnportCompleteL( err );
            break;
        }
    __FLOG( _L8("-HandleOperationL") );
    }

void CMTPOperator::SubscribeConnState()
    {
    if(!IsActive())
        {
        __FLOG( _L8("Subscribe connection state changed)") );
        iProperty.Subscribe(iStatus);
        iConSubscribed = ETrue;
        SetActive();
        }
  
    }

            
