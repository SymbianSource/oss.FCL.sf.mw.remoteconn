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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpoperatorTraces.h"
#endif

CMTPOperator* CMTPOperator::NewL( MMTPOperatorNotifier& aNotifier )
    {
    CMTPOperator* self = new( ELeave ) CMTPOperator( aNotifier );
    self->ConstructL();
    return self;
    }

CMTPOperator::~CMTPOperator()
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_DES_ENTRY );
    Cancel();
    iPendingOperations.Reset();
    iPendingOperations.Close();
    iMTPClient.Close();
    iProperty.Close();
    delete iTimer;
    OstTraceFunctionExit0( CMTPOPERATOR_DES_EXIT );
    }

void CMTPOperator::StartTransport( TUid aTransport )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_STARTTRANSPORT_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPOPERATOR_STARTTRANSPORT, "The transport uid is 0x%08X", aTransport.iUid );
    TInt err = AppendOperation( EStartTransport, aTransport );
    if ( KErrNone != err )
        {
        iNotifier.HandleStartTrasnportCompleteL( err );
        }
    OstTraceFunctionExit0( CMTPOPERATOR_STARTTRANSPORT_EXIT );
    }

void CMTPOperator::StopTransport( TUid aTransport )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_STOPTRANSPORT_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPOPERATOR_STOPTRANSPORT, "The transport uid is 0x%08X", aTransport.iUid );
    TInt err = AppendOperation( EStopTransport, aTransport );
    if ( KErrNone != err )
        {
        iNotifier.HandleStartTrasnportCompleteL( err );
        }
    OstTraceFunctionExit0( CMTPOPERATOR_STOPTRANSPORT_EXIT );
    }

void CMTPOperator::StartTimer(TInt aSecond)
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_STARTTIMER_ENTRY );
    iTimer->Start(aSecond);    
    OstTraceFunctionExit0( CMTPOPERATOR_STARTTIMER_EXIT );
    }

void CMTPOperator::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_DOCANCEL_ENTRY );
    iProperty.Cancel();
    iConSubscribed = EFalse;
    OstTraceFunctionExit0( CMTPOPERATOR_DOCANCEL_EXIT );
    }

void CMTPOperator::RunL()
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_RUNL_ENTRY );
    
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
        OstTraceExt2( TRACE_NORMAL, CMTPOPERATOR_RUNL, "Before, the iConnState is %d and connState is %d", iConnState, connState );
        if ( KErrNotFound == error )
            {
            iConnState = KInitialValue;
            OstTrace0( TRACE_NORMAL, DUP1_CMTPOPERATOR_RUNL, "The key is deleted and mtp server shut down!" );
            }
        else
            {
            if (iTimer->IsActive() && !iTimer->GetStopTransportStatus())
                {
                OstTrace0( TRACE_NORMAL, DUP2_CMTPOPERATOR_RUNL, "Timer is cancelled!" );
                iTimer->Cancel();
                }
            //if the disconnect is not set, set the disconnect
            //else if the connState is disconnect, launch the timer to restart the server to unload dps.
            if ( KInitialValue == iConnState )
                {
                iConnState = connState;
                OstTrace0( TRACE_NORMAL, DUP3_CMTPOPERATOR_RUNL, "the first time to launch mtp" );
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
                    OstTrace0( TRACE_NORMAL, DUP4_CMTPOPERATOR_RUNL, "Timer is launched." );
                    }
                else
                    {

                    iConnState = connState;
                    }
                }
            }
        OstTraceExt2( TRACE_NORMAL, DUP5_CMTPOPERATOR_RUNL, "After, the iConnState is %d and connState is %d", iConnState, connState );
        }
    OstTraceFunctionExit0( CMTPOPERATOR_RUNL_EXIT );
    }

CMTPOperator::CMTPOperator( MMTPOperatorNotifier& aNotifier ):
    CActive( EPriorityStandard ),
    iNotifier( aNotifier )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_CONS_ENTRY );
    OstTraceFunctionExit0( CMTPOPERATOR_CONS_EXIT );
    }

void CMTPOperator::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_CONSTRUCTL_ENTRY );
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
    OstTrace1( TRACE_NORMAL, CMTPOPERATOR_CONSTRUCTL, "The connstate is set to %d", iConnState );
    
    LEAVEIFERROR( iMTPClient.Connect(),
            OstTrace0( TRACE_ERROR, DUP1_CMTPOPERATOR_CONSTRUCTL, "Leave when the client connects to mtp server" ));
    
    LEAVEIFERROR(iProperty.Attach(KMTPPublishConnStateCat, EMTPConnStateKey),
            OstTrace0( TRACE_ERROR, DUP2_CMTPOPERATOR_CONSTRUCTL, "iProperty attached failed." ));
    iTimer = CMTPControllerTimer::NewL(iMTPClient, *this);
    
    iConSubscribed = EFalse;
    OstTraceFunctionExit0( CMTPOPERATOR_CONSTRUCTL_EXIT );
    }

TInt CMTPOperator::AppendOperation( TOperationType aType, TUid aTransport )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_APPENDOPERATION_ENTRY );
    TOperation operation = { aType, aTransport };
    TInt err = iPendingOperations.Append( operation );
    OstTrace1( TRACE_NORMAL, CMTPOPERATOR_APPENDOPERATION, "The return value is %d", err );
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

    OstTraceFunctionExit0( CMTPOPERATOR_APPENDOPERATION_EXIT );
    return err;
    }

void CMTPOperator::Schedule( TInt aError )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_SCHEDULE_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPOPERATOR_SCHEDULE, "The error value is %d", aError );
    if(iTimer->IsActive())
        {
        iTimer->Cancel();
        }
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, aError );
    SetActive();
    OstTraceFunctionExit0( CMTPOPERATOR_SCHEDULE_EXIT );
    }

void CMTPOperator::HandleOperationL( const TOperation& aOperation )
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_HANDLEOPERATIONL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPOPERATOR_HANDLEOPERATIONL, "The transport id is 0x%08X and the operation is 0x%08X", (TInt)aOperation.iTransport.iUid, aOperation.iType );
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
    OstTraceFunctionExit0( CMTPOPERATOR_HANDLEOPERATIONL_EXIT );
    }

void CMTPOperator::SubscribeConnState()
    {
    OstTraceFunctionEntry0( CMTPOPERATOR_SUBSCRIBECONNSTATE_ENTRY );
    if(!IsActive())
        {
        iProperty.Subscribe(iStatus);
        iConSubscribed = ETrue;
        SetActive();
        }
  
    OstTraceFunctionExit0( CMTPOPERATOR_SUBSCRIBECONNSTATE_EXIT );
    }

            
