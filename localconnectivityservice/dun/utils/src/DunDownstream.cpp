/*
* Copyright (c) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Definitions needed for one "stream" of CDunTransporter
*
*/


/*
 * TODO: When local media is of type RComm, listening on it is started with
 * RComm::NotifyDataAvailable() call. Check that USB ACM port and Irda RCOMM
 * (and any other new media in the future) behaves correctly so that when
 * RComm::ReadOneOrMore() is issued, the read is issued immediately without
 * checking for new data. If waiting for new data happens in this
 * NotifyDataAvailable/ReadOneOrMore combination, raise a defect to Symbian.
 */

#include "DunTransporter.h"
#include "DunDownstream.h"
#include "DunUpstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunDownstream* CDunDownstream::NewL( MDunTransporterUtilityAux* aUtility )
    {
    CDunDownstream* self = new (ELeave) CDunDownstream( aUtility );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunDownstream::~CDunDownstream()
    {
    FTRACE(FPrint( _L("CDunDownstream::~CDunDownstream()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunDownstream::~CDunDownstream() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunDownstream::ResetData()
    {
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // InitializeForDataPushing()
    delete iPushData.iDataPusher;
    iPushData.iDataPusher = NULL;
    // AddConnMonCallbackL()
    iCallbacksR.Close();
    iCallbacksW.Close();
    // AddSkippedErrorL()
    iOkErrorsR.Close();
    iOkErrorsW.Close();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Initializes this stream for AT command notifications
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::InitializeForDataPushing(
    MDunAtCmdHandler* aAtCmdHandler )
    {
    FTRACE(FPrint( _L("CDunDownstream::InitializeForDataPushing()" ) ));
    if ( iPushData.iDataPusher || iPushData.iAtCmdHandler )  // optional
        {
        FTRACE(FPrint( _L("CDunDownstream::InitializeForDataPushing() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    if ( !aAtCmdHandler )
        {
        FTRACE(FPrint( _L("CDunDownstream::InitializeForDataPushing() (aAtCmdHandler) not initialized!" ) ));
        return KErrGeneral;
        }
    CDunDataPusher* dataPusher = NULL;
    TRAPD( retTrap, dataPusher = CDunDataPusher::NewL(*this,this) );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDownstream::InitializeForDataPushing() (trapped!) complete" ) ));
        return retTrap;
        }
    if ( iComm )
        {
        dataPusher->SetMedia( iComm );
        }
    else if ( iSocket )
        {
        dataPusher->SetMedia( iSocket );
        }
    iPushData.iDataPusher = dataPusher;
    iPushData.iAtCmdHandler = aAtCmdHandler;
    FTRACE(FPrint( _L("CDunDownstream::InitializeForDataPushing() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Checks if data is in queue
// ---------------------------------------------------------------------------
//
TBool CDunDownstream::IsDataInQueue( const TDesC8* aDataToPush )
    {
    FTRACE(FPrint( _L("CDunDownstream::IsDataInQueue()" ) ));
    if ( !iPushData.iDataPusher )
        {
        FTRACE(FPrint( _L("CDunDownstream::IsDataInQueue() (iPushData.iDataPusher not initialized!) complete" )));
        return EFalse;
        }
    TInt foundIndex = iPushData.iDataPusher->FindEventFromQueue( aDataToPush );
    FTRACE(FPrint( _L("CDunDownstream::IsDataInQueue() complete" ) ));
    return ( foundIndex >= 0 ) ? ETrue : EFalse;
    }

// ---------------------------------------------------------------------------
// Adds data to event queue and starts sending if needed
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::AddToQueueAndSend( const TDesC8* aDataToPush,
                                        MDunCompletionReporter* aCallback )
    {
    FTRACE(FPrint( _L("CDunDownstream::AddToQueueAndSend()" ) ));
    if ( !iPushData.iDataPusher )
        {
        FTRACE(FPrint( _L("CDunDownstream::AddToQueueAndSend() (iPushData.iDataPusher not initialized!) complete" )));
        return KErrGeneral;
        }
    // Add to event queue. If something went wrong, just return
    TInt retTemp = iPushData.iDataPusher->AddToEventQueue( aDataToPush, aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDownstream::AddToQueueAndSend() (ERROR) complete" )));
        return retTemp;
        }
    // Now push the data. If already active, push will start later, if not
    // active it will start immediately.
    iPushData.iDataPusher->SendQueuedData();
    FTRACE(FPrint( _L("CDunDownstream::AddToQueueAndSend() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Starts downstream by issuing read request
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::StartStream()
    {
    FTRACE(FPrint( _L("CDunDownstream::StartStream()" ) ));
    FTRACE(FPrint( _L("CDunDownstream::StartStream() (buffer=0x%08X)" ), iBufferPtr ));
    // Note: only start URC here.
    // The downstream read request is started when command mode ends.
    // This is done to make the data arrive in the correct order (reply vs.
    // data) with "ATD" command.
    TInt retVal = KErrNone;
    if ( iPushData.iAtCmdHandler )
        {
        retVal = iPushData.iAtCmdHandler->StartUrc();
        }
    FTRACE(FPrint( _L("CDunDownstream::StartStream() complete" ) ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Stops transfer for read or write endpoints
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::Stop( TBool aStopMplex )
    {
    FTRACE(FPrint( _L("CDunDownstream::Stop() (Dir=%d)" ), iDirection));
    if ( !iPushData.iDataPusher )
        {
        FTRACE(FPrint( _L("CDunDownstream::Stop() (iPushData.iDatapusher not initialized!) complete" )));
        return KErrGeneral;
        }
    // Stop the downstream related AT command handling functionality
    if ( aStopMplex )  // optional
        {
        if ( iPushData.iAtCmdHandler )
            {
            iPushData.iAtCmdHandler->StopAtCmdHandling();
            }
        // Stop the multiplexer separately
        iPushData.iDataPusher->Stop();
        }
    if ( iTransferState != EDunStateTransferring )
        {
        FTRACE(FPrint( _L("CDunDownstream::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    // Stop only current operation
    if ( iOperationType == EDunOperationTypeRead )
        {
        if ( iNetwork )
            {
            iNetwork->ReadCancel();
            Cancel();
            FTRACE(FPrint( _L("CDunDownstream::Stop() (Network) cancelled" )));
            }
        }
    else if ( iOperationType == EDunOperationTypeWrite )
        {
        iPushData.iDataPusher->StopOneEvent( iBufferPtr );
        }
    iTransferState = EDunStateIdle;
    iOperationType = EDunOperationTypeUndefined;
    FTRACE(FPrint( _L("CDunDownstream::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunDownstream::CDunDownstream
// ---------------------------------------------------------------------------
//
CDunDownstream::CDunDownstream( MDunTransporterUtilityAux* aUtility ) :
    iUtility( aUtility )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunDownstream::ConstructL
// ---------------------------------------------------------------------------
//
void CDunDownstream::ConstructL()
    {
    FTRACE(FPrint( _L("CDunDownstream::ConstructL()" ) ));
    if ( !iUtility )
        {
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunDownstream::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunDownstream::Initialize()
    {
    // Don't initialize iUtility here (it is set through NewL)
    iPushData.iDataMode = EFalse;
    iPushData.iDataPusher = NULL;
    iPushData.iAtCmdHandler = NULL;
    }

// ---------------------------------------------------------------------------
// Issues transfer request for this stream
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::IssueRequest()
    {
    // Set direction
    iDirection = static_cast<TDunDirection>( EDunStreamTypeDownstream | iOperationType );

    FTRACE(FPrint( _L("CDunDownstream::IssueRequest() (Dir=%d)" ), iDirection));
    if ( !iPushData.iDataPusher )
        {
        FTRACE(FPrint( _L("CDunDownstream::IssueRequest() (iPushData.iDataPusher not initialized!) complete" ) ));
        return KErrGeneral;
        }

    if ( iTransferState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunDownstream::IssueRequest() (not ready) complete" ) ));
        return KErrNotReady;
        }

    if ( iOperationType == EDunOperationTypeRead )
        {
        iBufferPtr->SetLength( iBufferPtr->MaxLength() );
        FTRACE(FPrint( _L("CDunDownstream::IssueRequest() trying to read %d bytes... (Dir=%d)" ), iBufferPtr->Length(), iDirection));
        }
    else // iOperationType == EDunOperationTypeWrite
        {
        FTRACE(FPrint( _L("CDunDownstream::IssueRequest() writing %d bytes... (Dir=%d)" ), iBufferPtr->Length(), iDirection));
        }

    switch ( iDirection )
        {
        case EDunReaderDownstream:
            iStatus = KRequestPending;
            iTransferState = EDunStateTransferring;
            iNetwork->ReadOneOrMore( iStatus, *iBufferPtr );
            SetActive();
            FTRACE(FPrint( _L("CDunDownstream::IssueRequest() RComm ReadOneOrMore() requested" ) ));
            break;
        case EDunWriterDownstream:
            iTransferState = EDunStateTransferring;
            AddToQueueAndSend( iBufferPtr, this );
            break;
        default:
            FTRACE(FPrint( _L("CDunDownstream::IssueRequest() (ERROR) complete" ) ));
            return KErrGeneral;
        }

    FTRACE(FPrint( _L("CDunDownstream::IssueRequest() (Dir=%d) complete" ), iDirection));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when endpoint data read complete
// ---------------------------------------------------------------------------
//
void CDunDownstream::RunL()
    {
    FTRACE(FPrint( _L("CDunDownstream::RunL() (Dir=%d)" ), iDirection));
    iTransferState = EDunStateIdle;
    if ( iOperationType != EDunOperationTypeRead )
        {
        FTRACE(FPrint( _L("CDunDownstream::RunL() (wrong operation type!) complete" )));
        return;
        }

    TBool isError;
    TInt retTemp = iStatus.Int();
    TInt stop = ProcessErrorCondition( retTemp, isError );

    if ( !stop )  // no real error detected -> continue
        {
        if ( !isError )
            {
            // Operation type was read so now set to write
            iOperationType = EDunOperationTypeWrite;
            }  // if ( !isError )

        IssueRequest();

        }  // if ( !stop )
    else  // stop -> tear down connection
        {
        // Now CDunDataPusher notifies to write case so here we just notify the
        // read case.
        TDunConnectionReason connReason;
        connReason.iReasonType = EDunReasonTypeRW;
        connReason.iContext = GetMediaContext( EDunStreamTypeDownstream );
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = iDirection;
        connReason.iErrorCode = retTemp;
        iUtility->DoNotifyConnectionNotOk( iComm,
                                           iSocket,
                                           connReason,
                                           iCallbacksR );
        FTRACE(FPrint( _L("CDunDownstream::RunL() stop" )));
        }  // else

    FTRACE(FPrint( _L("CDunDownstream::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunDownstream::DoCancel()
    {
    FTRACE(FPrint( _L("CDunDownstream::DoCancel()" )));
    FTRACE(FPrint( _L("CDunDownstream::DoCancel() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunStreamManipulator.
// Gets called when outside party wants to push data to the existing stream
// ---------------------------------------------------------------------------
//
TInt CDunDownstream::NotifyDataPushRequest( const TDesC8* aDataToPush,
                                            MDunCompletionReporter* aCallback )
    {
    FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushRequest()" )));
    // If in data mode push the reply anyway as "CONNECT" or "NO CARRIER"
    // reply could arrive before/after the command mode information itself.
    TInt retVal = AddToQueueAndSend( aDataToPush, aCallback );
    FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushRequest() complete" )));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunCompletionReporter.
// Gets called when data push is complete
// ---------------------------------------------------------------------------
//
void CDunDownstream::NotifyDataPushComplete( TBool aAllPushed )
    {
    FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete()" )));
    // Next just skip the notifications of atomic operations because also this
    // class initiates the pushing of atomic data.
    if ( !aAllPushed )
        {
        FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete() (continue) complete" )));
        return;
        }
    if ( !iPushData.iDataPusher )
        {
        FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete() (iPushData.iDataPusher not initialized!) complete" )));
        return;
        }
    // Now the multiplexer might have pushed the contained data in this class
    // or it might have pushed only the external data. If the pushed data
    // contains this classes data then reissue request, otherwise just clear
    // the queue.
    TInt foundIndex = iPushData.iDataPusher->FindEventFromQueue( iBufferPtr );
    iPushData.iDataPusher->SignalCompletionAndClearQueue();
    FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete() (find event)" )));
    if ( foundIndex >= 0 )
        {
        iTransferState = EDunStateIdle;
        iOperationType = EDunOperationTypeUndefined;
        FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete() (issue request)" )));
        if ( iPushData.iDataMode )
            {
            iOperationType = EDunOperationTypeRead;
            IssueRequest();
            }
        }
    FTRACE(FPrint( _L("CDunDownstream::NotifyDataPushComplete() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunCmdModeMonitor.
// Notifies about command mode start
// ---------------------------------------------------------------------------
//
void CDunDownstream::NotifyCommandModeStart()
    {
    FTRACE(FPrint( _L("CDunDownstream::NotifyCommandModeStart()" )));
    iPushData.iDataMode = EFalse;
    // Now the data mode has ended. If read operation then cancel it.
    // Check for iTransferState here to minimize logging
    if ( iTransferState==EDunStateTransferring &&
         iOperationType==EDunOperationTypeRead )
        {
        Stop( EFalse );
        }
    FTRACE(FPrint( _L("CDunDownstream::NotifyCommandModeStart() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunCmdModeMonitor.
// Notifies about command mode end
// ---------------------------------------------------------------------------
//
void CDunDownstream::NotifyCommandModeEnd()
    {
    FTRACE(FPrint( _L("CDunDownstream::NotifyCommandModeEnd()" )));
    iPushData.iDataMode = ETrue;
    // Command mode ends here so start reading from Dataport only if generic
    // transferring state is not EDunStateIdle. This is a reduced form of having
    // read pending -> reissue not needed OR write pending -> reissue not needed
    // (NotifyDataPushComplete() will reissue).
    if ( iTransferState == EDunStateIdle )
        {
        iOperationType = EDunOperationTypeRead;  // just in case
        IssueRequest();
        }
    FTRACE(FPrint( _L("CDunDownstream::NotifyCommandModeEnd() complete" )));
    }
