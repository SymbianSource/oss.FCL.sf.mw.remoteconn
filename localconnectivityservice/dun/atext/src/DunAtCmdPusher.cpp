/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AT command pusher for downstream
*
*/

/*
 * Filtering categories for multiple commands on one line (DunAtCmdPusher.cpp)
 * (here "OTHER" reply means a reply which is something else than "OK" and "ERROR")
 * One reply:     OK           -> OK
 * One reply:     OTHER        -> OTHER
 * One reply:     ERROR        -> ERROR
 * Two replies:   OK, OK       -> OK
 * Two replies:   OTHER, OTHER -> OTHER, OTHER
 * Two replies:   OK, OTHER    -> OTHER
 * Two replies:   OTHER, OK    -> OTHER
 * Two replies:   OK, ERROR    -> ERROR
 * Two replies:   OTHER, ERROR -> OTHER, ERROR
 * Note: "OK" replies are skipped. The "OK" string is stripped from the "OTHER"
 * replies and manually added the the downstream as the last operation if either
 * "OK" or "OTHER" was received before.
 */

#include "DunAtCmdPusher.h"
#include "DunDownstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdPusher* CDunAtCmdPusher::NewL(
    RATExt* aAtCmdExt,
    MDunAtCmdPusher* aCallback,
    MDunStreamManipulator* aDownstream,
    TDesC8* aOkBuffer )
    {
    CDunAtCmdPusher* self = NewLC( aAtCmdExt,
                                   aCallback,
                                   aDownstream,
                                   aOkBuffer );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdPusher* CDunAtCmdPusher::NewLC(
    RATExt* aAtCmdExt,
    MDunAtCmdPusher* aCallback,
    MDunStreamManipulator* aDownstream,
    TDesC8* aOkBuffer )
    {
    CDunAtCmdPusher* self = new (ELeave) CDunAtCmdPusher( aAtCmdExt,
                                                          aCallback,
                                                          aDownstream,
                                                          aOkBuffer );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdPusher::~CDunAtCmdPusher()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::~CDunAtCmdPusher()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtCmdPusher::~CDunAtCmdPusher() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtCmdPusher::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Starts AT command handling
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdPusher::IssueRequest( TDesC8& aInput, TBool aNormalMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::IssueRequest()") ));
    FTRACE(FPrint( _L("CDunAtCmdPusher::IssueRequest() send ATEXT:") ));
    FTRACE(FPrintRaw(aInput) );
    if ( iAtPushState!=EDunStateIdle && aNormalMode )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    if ( iDownstream->IsDataInQueue(&iRecvBuffer) )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::IssueRequest() (in queue!) complete") ));
        return KErrGeneral;
        }
    iStatus = KRequestPending;
    iAtPushState = EDunStateAtCmdPushing;
    iAtCmdExt->HandleCommand( iStatus,
                              aInput,
                              iRecvBuffer,
                              iReplyLeftPckg,
                              iReplyTypePckg );
    SetActive();
    FTRACE(FPrint( _L("CDunAtCmdPusher::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops AT command handling
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdPusher::Stop()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::Stop()") ));
    SetEndOfCmdLine();
    if ( iAtPushState != EDunStateAtCmdPushing )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    // As the EDunStateAtCmdHandling can be set even when the actual request
    // has completed (when replying with NotifyDataPushComplete() and setting
    // idle eventually), cancel the actual operation in DoCancel()
    Cancel();
    iAtPushState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunAtCmdPusher::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Manages request to abort command handling
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdPusher::ManageAbortRequest()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageAbortRequest()") ));
    if ( iAtPushState != EDunStateAtCmdPushing )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageAbortRequest() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( iCmdAbort )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::ManageAbortRequest() (already exists) complete") ));
        return KErrAlreadyExists;
        }
    TInt retTemp = iAtCmdExt->ReportHandleCommandAbort( iStop );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::ManageAbortRequest() (ERROR) complete") ));
        return retTemp;
        }
    iCmdAbort = ETrue;
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageAbortRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets end of command line marker on for the possible series of AT commands.
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::SetEndOfCmdLine()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::SetEndOfCmdLine()") ));
    iNoErrorReceived = EFalse;
    iLastOkPush = EFalse;
    iCmdAbort = EFalse;
    iStop = EFalse;
    iEditorMode = EFalse;
    FTRACE(FPrint( _L("CDunAtCmdPusher::SetEndOfCmdLine() complete") ));
    }

// ---------------------------------------------------------------------------
// Gets the editor mode status
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdPusher::EditorMode()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::EditorMode()") ));
    FTRACE(FPrint( _L("CDunAtCmdPusher::EditorMode() complete") ));
    return iEditorMode;
    }

// ---------------------------------------------------------------------------
// CDunAtCmdPusher::CDunAtCmdPusher
// ---------------------------------------------------------------------------
//
CDunAtCmdPusher::CDunAtCmdPusher( RATExt* aAtCmdExt,
                                  MDunAtCmdPusher* aCallback,
                                  MDunStreamManipulator* aDownstream,
                                  TDesC8* aOkBuffer ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iCallback( aCallback ),
    iDownstream( aDownstream ),
    iOkBuffer( aOkBuffer ),
    iReplyLeftPckg( iReplyBytesLeft ),
    iReplyTypePckg( iReplyType )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtCmdPusher::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ConstructL()") ));
    if ( !iAtCmdExt || !iCallback || !iDownstream || !iOkBuffer )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunAtCmdPusher::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::Initialize()
    {
    // Don't initialize iAtCmdExt here (it is set through NewL)
    // Don't initialize iCallback here (it is set through NewL)
    // Don't initialize iDownstream here (it is set through NewL)
    // Don't initialize iOkBuffer here (it is set through NewL)
    iAtPushState = EDunStateIdle;
    iReplyBytesLeft = 0;
    iReplyType = EReplyTypeUndefined;
    SetEndOfCmdLine();
    }

// ---------------------------------------------------------------------------
// Sets state to idle and notifies about subcommand handling completion
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::SetToIdleAndNotifyEnd( TInt aError )
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::SetToIdleAndNotifyEnd()") ));
    iCmdAbort = EFalse;
    iAtPushState = EDunStateIdle;
    iCallback->NotifyEndOfProcessing( aError );
    FTRACE(FPrint( _L("CDunAtCmdPusher::SetToIdleAndNotifyEnd() complete") ));
    }

// ---------------------------------------------------------------------------
// Checks if "OK" (verbose) or "0" (numeric) string or exists at the end of
// buffer and removes it
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdPusher::CheckAndRemoveOkString()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::CheckAndRemoveOkString()") ));
    TInt recvBufferLength = iRecvBuffer.Length();
    TInt okBufferLength = iOkBuffer->Length();
    // Skip the removal if removing not possible, if removal results in zero
    // length (plugin should have used KErrReplyTypeOk) or if string to be
    // removed is zero.
    // Note also that if plugin sends a final reply when quiet mode is on, DUN
    // can't remove the possibly existing result code as it is different from
    // iOkReply (zero length).
    if ( recvBufferLength<=okBufferLength || okBufferLength<=0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::CheckAndRemoveOkString() (skip) complete") ));
        return KErrGeneral;
        }
    TInt lengthWithNoOk = recvBufferLength - okBufferLength;
    TPtr8 recvBufferDes( &iRecvBuffer[lengthWithNoOk], okBufferLength, okBufferLength );
    if ( recvBufferDes.Compare(*iOkBuffer) != 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdPusher::CheckAndRemoveOkString() (not found) complete") ));
        return KErrNotFound;
        }
    iRecvBuffer.SetLength( lengthWithNoOk );
    FTRACE(FPrint( _L("CDunAtCmdPusher::CheckAndRemoveOkString() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sends reply data to downstream
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::SendReplyData( TBool aRecvBuffer )
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::SendReplyData()") ));
    TDesC8* sendBuffer = iOkBuffer;
    if ( aRecvBuffer )
        {
        sendBuffer = &iRecvBuffer;
        // Check if last block of long push and remove "OK" if exists
        if ( iReplyType==EReplyTypeOther && iReplyBytesLeft==0 )
            {
            CheckAndRemoveOkString();
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdPusher::SendReplyData() send reply:") ));
    FTRACE(FPrintRaw(*sendBuffer) );
    iDownstream->NotifyDataPushRequest( sendBuffer, this );
    FTRACE(FPrint( _L("CDunAtCmdPusher::SendReplyData() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages change in reply type to EReplyTypeOther
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ManageReplyTypeChangeToOther()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToOther()") ));
    iNoErrorReceived = ETrue;
    SendReplyData();
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToOther() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages change in reply type to EReplyTypeOk
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ManageReplyTypeChangeToOk()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToOk()") ));
    // Skip the "OK" replies if not last. Only push the "OK" reply at the end.
    // iStop changes it so that the we have to send the "OK" immediately and
    // only stop with NotifyDataPushComplete()
    TBool found = iCallback->NotifyNextCommandPeekRequest();
    if ( !found || iStop )
        {
        SendReplyData();
        }
    else
        {
        iNoErrorReceived = ETrue;
        SetToIdleAndNotifyEnd( KErrNone );
        }
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToOk() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages change in reply type to EReplyTypeError
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ManageReplyTypeChangeToError()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToError()") ));
    if ( iNoErrorReceived )
       {
       iAtCmdExt->ReportExternalHandleCommandError();
       }
    SendReplyData();
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToError() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages change in reply type to EReplyTypeEditor
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ManageReplyTypeChangeToEditor()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToEditor()") ));
    if ( !iEditorMode )
        {
        // First change to editor mode: manage it as EReplyTypeOther (prompt)
        iEditorMode = ETrue;
        ManageReplyTypeChangeToOther();
        FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToEditor() (editor) complete") ));
        return;
        }
    // The same reply to editor mode as before: no reply, only notification for
    // echo/forwarding purposes
    iCallback->NotifyEditorModeReply();
    // Do nothing after notifying. The next ForwardEditorModeInput() triggers
    // the next call of this function.
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChangeToEditor() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages change in reply type
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::ManageReplyTypeChange()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange()") ));
    switch ( iReplyType )
        {
        case EReplyTypeOther:
            {
            FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() EReplyTypeOther") ));
            iEditorMode = EFalse;
            ManageReplyTypeChangeToOther();
            }
            break;
        case EReplyTypeOk:
            {
            FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() EReplyTypeOk") ));
            iEditorMode = EFalse;
            ManageReplyTypeChangeToOk();
            }
            break;
        case EReplyTypeError:
            {
            FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() EReplyTypeError") ));
            iEditorMode = EFalse;
            ManageReplyTypeChangeToError();
            }
            break;
        case EReplyTypeEditor:
            FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() EReplyTypeEditor") ));
            ManageReplyTypeChangeToEditor();
            break;
        default:
            {
            FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() EReplyTypeUndefined") ));
            iEditorMode = EFalse;
            SetToIdleAndNotifyEnd( KErrNone );
            }
            break;
        }
    FTRACE(FPrint( _L("CDunAtCmdPusher::ManageReplyTypeChange() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when AT command handled
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::RunL()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::RunL()") ));
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        SetToIdleAndNotifyEnd( retTemp );
        FTRACE(FPrint( _L("CDunAtCmdPusher::RunL() (ERROR) complete (%d)"), retTemp));
        return;
        }
    ManageReplyTypeChange();
    FTRACE(FPrint( _L("CDunAtCmdPusher::RunL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::DoCancel()
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::DoCancel()") ));
    iAtCmdExt->CancelHandleCommand();
    FTRACE(FPrint( _L("CDunAtCmdPusher::DoCancel() complete") ));
    }

// ---------------------------------------------------------------------------
// From class MDunCompletionReporter.
// Gets called when data push is complete
// ---------------------------------------------------------------------------
//
void CDunAtCmdPusher::NotifyDataPushComplete( TBool /*aAllPushed*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdPusher::NotifyDataPushComplete()") ));
    // First check if error or stop condition detected
    if ( iReplyType==EReplyTypeError || iStop )
        {
        SetEndOfCmdLine();
        iAtPushState = EDunStateIdle;
        iCallback->NotifyEndOfCmdLineProcessing();
        FTRACE(FPrint( _L("CDunAtCmdPusher::NotifyDataPushComplete() (error reply) complete") ));
        return;
        }
    // Secondly check only the case where push restart is required
    if ( iReplyType==EReplyTypeOther && iReplyBytesLeft>0 )
        {
        iAtCmdExt->GetNextPartOfReply( iRecvBuffer, iReplyBytesLeft );
        SendReplyData();
        FTRACE(FPrint( _L("CDunAtCmdPusher::NotifyDataPushComplete() (push restart) complete") ));
        return;
        }
    // Next check the case where other than "OK" and "ERROR" reply is received
    // and that is the last one in the command line. Then just send "OK".
    if ( !iLastOkPush && iReplyType==EReplyTypeOther )
        {
        TBool found = iCallback->NotifyNextCommandPeekRequest();
        if ( !found )
            {
            // Force iReplyType here to match the correct one in NotifyDataPushComplete()
            iReplyType = EReplyTypeOk;
            iLastOkPush = ETrue;
            SendReplyData( EFalse );
            FTRACE(FPrint( _L("CDunAtCmdPusher::NotifyDataPushComplete() (last OK) complete") ));
            return;
            }
        // Now the next command was found so just fall through
        }
    // As a last step just set to idle
    SetToIdleAndNotifyEnd( KErrNone );
    FTRACE(FPrint( _L("CDunAtCmdPusher::NotifyDataPushComplete() complete") ));
    }
