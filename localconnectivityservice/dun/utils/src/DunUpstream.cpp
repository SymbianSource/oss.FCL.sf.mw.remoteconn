/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "DunUpstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunUpstream* CDunUpstream::NewL( MDunTransporterUtilityAux* aUtility )
    {
    CDunUpstream* self = new (ELeave) CDunUpstream( aUtility );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunUpstream::~CDunUpstream()
    {
    FTRACE(FPrint( _L("CDunUpstream::~CDunUpstream()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunUpstream::~CDunUpstream() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunUpstream::ResetData()
    {
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // InitializeForAtParsing()
    delete iParseData.iAtCmdHandler;
    iParseData.iAtCmdHandler = NULL;
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
// Sets activity callback for this stream
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::SetActivityCallback(
    MDunActivityManager* aActivityCallback )
    {
    if ( !aActivityCallback )
        {
        FTRACE(FPrint( _L("CDunUpstream::SetActivityCallback() (aActivityCallback) not initialized!" ) ));
        return KErrGeneral;
        }
    if ( iActivityData.iActivityCallback )
        {
        FTRACE(FPrint( _L("CDunUpstream::SetActivityCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    iActivityData.iActivityCallback = aActivityCallback;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Initializes this stream for AT command notifications
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::InitializeForAtParsing(
    MDunStreamManipulator* aStreamCallback,
    const TDesC8* aConnectionName,
    MDunCmdModeMonitor* aCallbackUp,
    MDunCmdModeMonitor* aCallbackDown )
    {
    FTRACE(FPrint( _L("CDunUpstream::InitializeForAtParsing()" ) ));
    if ( iParseData.iAtCmdHandler )
        {
        FTRACE(FPrint( _L("CDunUpstream::InitializeForAtParsing() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    TInt retTrap = KErrNone;
    CDunAtCmdHandler* atCmdHandler = NULL;
    TRAP( retTrap, atCmdHandler = CDunAtCmdHandler::NewL(this,
                                                         aStreamCallback,
                                                         aConnectionName) );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunUpstream::InitializeForAtParsing() (trapped!) complete" ) ));
        return retTrap;
        }
    atCmdHandler->AddCmdModeCallback( aCallbackUp );
    atCmdHandler->AddCmdModeCallback( aCallbackDown );
    iParseData.iDataMode = EFalse;
    iParseData.iParseNeeded = ETrue;
    iParseData.iHandling = EFalse;
    iParseData.iAtCmdHandler = atCmdHandler;
    FTRACE(FPrint( _L("CDunUpstream::InitializeForAtParsing() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Starts upstream by issuing read request
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::StartStream()
    {
    FTRACE(FPrint( _L("CDunUpstream::StartStream()" ) ));
    if ( !iNetwork )
        {
        FTRACE(FPrint( _L("CDunUpstream::StartStream() (iNetwork) not initialized!" ) ));
        return KErrGeneral;
        }
    if ( !iComm && !iSocket )
        {
        FTRACE(FPrint( _L("CDunUpstream::StartStream() (iComm&iSocket) not initialized!" ) ));
        return KErrGeneral;
        }
    iOperationType = EDunOperationTypeRead;
    TInt retVal = IssueRequest();
    FTRACE(FPrint( _L("CDunUpstream::StartStream() complete" ) ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Stops transfer for read or write endpoints
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::Stop()
    {
    FTRACE(FPrint( _L("CDunUpstream::Stop() (Dir=%d)" ), iDirection));
    // Don't stop CDunAtCmdHandler here as it is downstream related!
    if ( iTransferState != EDunStateTransferring )
        {
        FTRACE(FPrint( _L("CDunUpstream::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    // Stop only current operation
    if ( iOperationType == EDunOperationTypeRead )
        {
        if ( iComm )
            {
            iComm->ReadCancel();
            FTRACE(FPrint( _L("CDunUpstream::Stop() (RComm) cancelled" )));
            }
        else if ( iSocket )
            {
            iSocket->CancelRecv();
            FTRACE(FPrint( _L("CDunUpstream::Stop() (RSocket) cancelled" )));
            }
        }
    else if ( iOperationType == EDunOperationTypeWrite )
        {
        if ( iNetwork )
            {
            iNetwork->WriteCancel();
            FTRACE(FPrint( _L("CDunUpstream::Stop() (Network) cancelled" )));
            }
        }
    Cancel();
    iTransferState = EDunStateIdle;
    // Notify parent about inactivity
    if ( iActivityData.iActivityCallback && iActivityData.iNotified )
        {
        iActivityData.iActivityCallback->NotifyChannelInactivity();
        iActivityData.iNotified = EFalse;
        }
    FTRACE(FPrint( _L("CDunUpstream::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Data transmission state (if read completed)
// ---------------------------------------------------------------------------
//
TBool CDunUpstream::DataReadStatus()
    {
    return iActivityData.iDataRead;
    }

// ---------------------------------------------------------------------------
// CDunUpstream::CDunUpstream
// ---------------------------------------------------------------------------
//
CDunUpstream::CDunUpstream( MDunTransporterUtilityAux* aUtility ) :
    iUtility( aUtility )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunUpstream::ConstructL
// ---------------------------------------------------------------------------
//
void CDunUpstream::ConstructL()
    {
    FTRACE(FPrint( _L("CDunUpstream::ConstructL()" ) ));
    if ( !iUtility )
        {
        User::Leave( KErrGeneral );
        }
    FTRACE(FPrint( _L("CDunUpstream::ConstructL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunUpstream::Initialize()
    {
    // Don't initialize iUtility here (it is set through NewL)
    iActivityData.iActivityCallback = NULL;
    iActivityData.iDataRead = EFalse;
    iActivityData.iNotified = EFalse;
    iParseData.iDataMode = EFalse;
    iParseData.iParseNeeded = EFalse;
    iParseData.iHandling = EFalse;
    iParseData.iAtCmdHandler = NULL;
    }

// ---------------------------------------------------------------------------
// Issues transfer request for this stream
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::IssueRequest()
    {
    // Set direction
    iDirection = static_cast<TDunDirection>( EDunStreamTypeUpstream | iOperationType );

    FTRACE(FPrint( _L("CDunUpstream::IssueRequest() (Dir=%d)" ), iDirection));

    if ( iTransferState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunUpstream::IssueRequest() (not ready) complete" ) ));
        return KErrNotReady;
        }

    if ( iOperationType == EDunOperationTypeRead )
        {
        iBufferPtr->SetLength( iBufferPtr->MaxLength() );
        FTRACE(FPrint( _L("CDunUpstream::IssueRequest() trying to read %d bytes... (Dir=%d)" ), iBufferPtr->Length(), iDirection));
        }
    else // iOperationType == EDunOperationTypeWrite
        {
        FTRACE(FPrint( _L("CDunUpstream::IssueRequest() writing %d bytes... (Dir=%d)" ), iBufferPtr->Length(), iDirection));
        }

    switch ( iDirection )
        {
        case EDunReaderUpstream:
            if ( iComm )
                {
                iStatus = KRequestPending;
                iComm->ReadOneOrMore( iStatus, *iBufferPtr );
                FTRACE(FPrint( _L("CDunUpstream::IssueRequest() RComm ReadOneOrMore() requested" ) ));
                }
            else if ( iSocket )
                {
                iStatus = KRequestPending;
                iSocket->RecvOneOrMore( *iBufferPtr, 0, iStatus, iReadLengthSocket );
                FTRACE(FPrint( _L("CDunUpstream::IssueRequest() RSocket RecvOneOrMore() requested" ) ));
                }
            else
                {
                FTRACE(FPrint( _L("CDunUpstream::IssueRequest() (ERROR) complete" ) ));
                return KErrGeneral;
                }
            break;
        case EDunWriterUpstream:
            iStatus = KRequestPending;
            iNetwork->Write( iStatus, *iBufferPtr );
            FTRACE(FPrint( _L("CDunUpstream::IssueRequest() RComm Write() requested" ) ));
            break;
        default:
            FTRACE(FPrint( _L("CDunUpstream::IssueRequest() (ERROR) complete" ) ));
            return KErrGeneral;
        }

    SetActive();
    iTransferState = EDunStateTransferring;

    FTRACE(FPrint( _L("CDunUpstream::IssueRequest() (Dir=%d) complete" ), iDirection));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Processes data that was read
// ---------------------------------------------------------------------------
//
TBool CDunUpstream::ProcessReadData()
    {
    FTRACE(FPrint( _L("CDunUpstream::ProcessReadData()" )));
    // The following will be transferred to Dataport
    if ( iParseData.iDataMode || !iParseData.iParseNeeded )
        {
        iOperationType = EDunOperationTypeWrite;
        FTRACE(FPrint( _L("CDunUpstream::ProcessReadData() (next write) complete" )));
        return ETrue;
        }
    if ( !iParseData.iAtCmdHandler )  // optional
        {
        FTRACE(FPrint( _L("CDunUpstream::ProcessReadData() (no handler) complete" )));
        return ETrue;
        }
    // The following will be transferred to parser
    TInt retTemp = KErrNone;
    TBool partialInput = EFalse;
    retTemp = iParseData.iAtCmdHandler->ParseCommand( *iBufferPtr,
                                                      partialInput );
    if ( retTemp!=KErrNone || !partialInput )
        {
        FTRACE(FPrint( _L("CDunUpstream::ProcessReadData() (no reissue) complete" )));
        return EFalse;
        }
    FTRACE(FPrint( _L("CDunUpstream::ProcessReadData() (reissue) complete" )));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Manages activity in a channel
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::ManageChannelActivity()
    {
    FTRACE(FPrint( _L("CDunUpstream::ManageChannelActivity()" )));
    if ( iActivityData.iDataRead )
        {
        FTRACE(FPrint( _L("CDunUpstream::ManageChannelActivity() (not ready) complete" )));
        return KErrNotReady;
        }
    iActivityData.iDataRead = ETrue;
    if ( iActivityData.iActivityCallback && !iActivityData.iNotified )
        {
        iActivityData.iActivityCallback->NotifyChannelActivity();
        iActivityData.iNotified = ETrue;
        }
    FTRACE(FPrint( _L("CDunUpstream::ManageChannelActivity() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when endpoint data read/write complete
// ---------------------------------------------------------------------------
//
void CDunUpstream::RunL()
    {
    FTRACE(FPrint( _L("CDunUpstream::RunL() (Dir=%d)" ), iDirection));
    iTransferState = EDunStateIdle;

    TBool isError;
    TInt retTemp = iStatus.Int();
    TInt stop = ProcessErrorCondition( retTemp, isError );

    if ( !stop )  // no real error detected -> continue
        {
        TBool reIssue = ETrue;
        if ( !isError )
            {
            if ( iOperationType == EDunOperationTypeRead )
                {
                ManageChannelActivity();
                reIssue = ProcessReadData();
                }  // if ( iOperationType == EDunOperationTypeRead )
            else // iOperationType == EDunOperationTypeWrite
                {
                iOperationType = EDunOperationTypeRead;
                }
            }  // if ( !isError )

        if ( reIssue )
            {
            IssueRequest();
            }

        }  // if ( !stop )
    else  // stop -> tear down connection
        {
        FTRACE(FPrint( _L("CDunUpstream::RunL() stop" )));
        TDunConnectionReason connReason;
        connReason.iReasonType = EDunReasonTypeRW;
        connReason.iContext = GetMediaContext( EDunStreamTypeUpstream );
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = iDirection;
        connReason.iErrorCode = retTemp;
        if ( iOperationType == EDunOperationTypeRead )
            {
            iUtility->DoNotifyConnectionNotOk( iComm,
                                               iSocket,
                                               connReason,
                                               iCallbacksR );
            }
        else  // iOperationType == EDunOperationTypeWrite
            {
            iUtility->DoNotifyConnectionNotOk( iComm,
                                               iSocket,
                                               connReason,
                                               iCallbacksW );
            }
        }  // else

    FTRACE(FPrint( _L("CDunUpstream::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunUpstream::DoCancel()
    {
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdStatusReporter
// Notifies about AT command handling start
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyAtCmdHandlingStart()
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingStart()" )));
    if ( iParseData.iHandling )
        {
        FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingStart() (already set!)" )));
        }
    iParseData.iHandling = ETrue;
    FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingStart() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdStatusReporter
// Notifies about AT command handling end
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyAtCmdHandlingEnd( TInt aStartIndex )
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingEnd()" )));
    if ( !iParseData.iHandling )
        {
        FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingEnd() (already set!)" )));
        }
    iParseData.iHandling = EFalse;
    // Next check the offset to the next command inside this block
    TInt length = iBufferPtr->Length();
    if ( aStartIndex < 0 )
        {
        // Start of the next command not found so here we need to just reissue
        // the read request and not clear the input buffer.
        iParseData.iAtCmdHandler->SetEndOfCmdLine( EFalse );
        IssueRequest();  // iOperationType must be read here (don't set)
        FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingEnd() (not found) complete" )));
        return;
        }
    // Here the start of next command was found so try to directly handle that
    // command using ProcessReadData() for the next subblock.
    iParseData.iAtCmdHandler->SetEndOfCmdLine( ETrue );
    TInt maxLength = iBufferPtr->MaxLength();
    iBufferPtr->Set( &(*iBufferPtr)[aStartIndex], length-aStartIndex, maxLength );
    TBool reIssue = ProcessReadData();
    if ( reIssue )
        {
        // Note: should come here only if something went wrong
        IssueRequest();
        }
    FTRACE(FPrint( _L("CDunUpstream::NotifyAtCmdHandlingEnd() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdStatusReporter
// Notifies about editor mode reply
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyEditorModeReply( TBool aStart )
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyEditorModeReply()" )));
    if ( iParseData.iDataMode )
        {
        FTRACE(FPrint( _L("CDunUpstream::NotifyEditorModeReply() (not ready) complete" )));
        return;
        }
    // If start of editor mode then just reissue the read request
    // If continuation then echo and reissue the read request
    if ( aStart )
        {
        IssueRequest();
        FTRACE(FPrint( _L("CDunUpstream::NotifyEditorModeReply() (start) complete" )));
        return;
        }
    iParseData.iAtCmdHandler->SendEchoCharacter( iBufferPtr, this );
    FTRACE(FPrint( _L("CDunUpstream::NotifyEditorModeReply() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdHandler
// Starts URC message handling
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::StartUrc()
    {
    FTRACE(FPrint( _L("CDunUpstream::StartUrc()" )));
    TInt retVal = KErrNone;
    if ( iParseData.iAtCmdHandler )  // optional
        {
        retVal = iParseData.iAtCmdHandler->StartUrc();
        }
    FTRACE(FPrint( _L("CDunUpstream::StartUrc() complete" )));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdHandler
// Stops AT command handling downstream related activity (also URC)
// ---------------------------------------------------------------------------
//
TInt CDunUpstream::StopAtCmdHandling()
    {
    FTRACE(FPrint( _L("CDunUpstream::StopAtCmdHandling()" )));
    TInt retVal = KErrNone;
    if ( iParseData.iAtCmdHandler )  // optional
        {
        retVal = iParseData.iAtCmdHandler->StopUrc();
        if ( retVal != KErrNone )
            {
            iParseData.iAtCmdHandler->Stop();
            FTRACE(FPrint( _L("CDunUpstream::StopAtCmdHandling() (iAtCmdHandler) complete" )));
            return retVal;
            }
        retVal = iParseData.iAtCmdHandler->Stop();
        }
    FTRACE(FPrint( _L("CDunUpstream::StopAtCmdHandling() complete" )));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunCmdModeMonitor.
// Notifies about command mode start
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyCommandModeStart()
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyCommandModeStart()" )));
    iParseData.iDataMode = EFalse;
    // Stop processing (just to be sure).
    // This will stop any possibly pending operations of
    // CDunAtCmdHandler and CDunAtUrcHandler. CDunDownstream will take care of
    // clearing (and stopping) non-callback write queues.
    StopAtCmdHandling();
    // Also restart the URC handling after the data mode
    StartUrc();
    FTRACE(FPrint( _L("CDunUpstream::NotifyCommandModeStart() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunCmdModeMonitor.
// Notifies about command mode end
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyCommandModeEnd()
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyCommandModeEnd()" )));
    iParseData.iDataMode = ETrue;
    // Stop processing (mandatory).
    // This will stop any possibly pending operations of
    // CDunAtCmdHandler and CDunAtUrcHandler. CDunDownstream will take care of
    // clearing (and stopping) non-callback write queues.
    StopAtCmdHandling();
    // The follow is needed because stopping the AT command handling here
    // prevents the subsequent AT command handling notification to reach the
    // NotifyAtCmdHandlingEnd() in this class (the notification starts from
    // CDunAtCmdPusher's SetToIdleAndNotifyEnd()).
    // So here we have to do the block "if ( aStartIndex < 0 )" in function
    // NotifyAtCmdHandlingEnd().
    IssueRequest();
    FTRACE(FPrint( _L("CDunUpstream::NotifyCommandModeEnd() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdEchoer.
// Notifies about command mode end
// ---------------------------------------------------------------------------
//
void CDunUpstream::NotifyEchoComplete()
    {
    FTRACE(FPrint( _L("CDunUpstream::NotifyEchoComplete()" )));
    IssueRequest();
    FTRACE(FPrint( _L("CDunUpstream::NotifyEchoComplete() complete" )));
    }
