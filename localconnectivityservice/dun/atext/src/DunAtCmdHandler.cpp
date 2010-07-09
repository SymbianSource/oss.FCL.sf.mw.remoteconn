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
* Description:  AT command handler and notifier
*
*/

/*
 * Points to consider:
 * - Each of the AT commands sent to ATEXT are converted to upper case form.
 *   Thus the ATEXT plugins don't need to check for case. The conversion to
 *   upper case form stops when carriage return or '=' character is found.
 */

/*
 * This file has the following functionality:
 * 1) Line buffer filler:
 *    AddDataForParsing(), ManagePartialCommand(), ExtractLineFromInputBuffer(),
 *    etc. This buffer is used for parsing. These functions are used for
 *    splitter-combiner logic described below. CDunAtCmdPusher is used
 *    separately for each element in the line buffer.
 * 2) Parser and splitter-combiner to handle the separater elements (subcommands)
 *    in the line buffer. When end of line is detected, iEndIndex is used to
 *    extract the next line in iInput to the line buffer (ManageEndOfCmdHandling()
 *    and ExtractLineFromInputBuffer()).
 * 3) When end of iEndIndex is found (ExtractLineFromInputBuffer()), more data
 *    is asked from CDunUpstream.
 * Note: There is separate handling for "one character input data" and "A/"
 * command handling which should be supported only for one line based data
 * (ManagePartialCommand()).
 */

/*
 * The AT command handling is splitted to two parts on high level:
 * 1) Splitter: splitting the sub-commands in a command line to multiple ones
 *    for ATEXT to process.
 * 2) Combiner: combining the replies coming from ATEXT using a filter
 *    (the filter categories are explained in DunAtCmdPusher.cpp)
 */

/*
 * Note1: This file uses AT command parsing based on heuristics.
 * Refer to test specification if planning to change the heuristic.
 * Note2: Input buffer management (ExtractLineFromInputBuffer()) can be tested
 * with non-line based terminals such as HyperTerminal or Realterm.
 */

#include "DunAtCmdHandler.h"
#include "DunAtUrcHandler.h"
#include "DunDownstream.h"
#include "DunDebug.h"

const TInt8 KDunCancel = 24;  // Used for line editing, cancel character
const TInt8 KDunEscape = 27;  // Used for editor ending, escape character

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
EXPORT_C CDunAtCmdHandler* CDunAtCmdHandler::NewL(
    MDunAtCmdStatusReporter* aUpstream,
    MDunStreamManipulator* aDownstream,
    const TDesC8* aConnectionName )
    {
    CDunAtCmdHandler* self = new (ELeave) CDunAtCmdHandler(
        aUpstream,
        aDownstream,
        aConnectionName );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdHandler::~CDunAtCmdHandler()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::~CDunAtCmdHandler()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtCmdHandler::~CDunAtCmdHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunAtCmdHandler::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // NewL()
    DeletePluginHandlers();
    delete iCmdEchoer;
    iCmdEchoer = NULL;
    delete iNvramListen;
    iNvramListen = NULL;
    delete iModeListen;
    iModeListen = NULL;
    delete iEcomListen;
    iEcomListen = NULL;
    delete iAtSpecialCmdHandler;
    iAtSpecialCmdHandler = NULL;
    if ( iAtCmdExtCommon.Handle() )
        {
        iAtCmdExtCommon.SynchronousClose();
        iAtCmdExtCommon.Close();
        }
    if ( iAtCmdExt.Handle() )
        {
        iAtCmdExt.SynchronousClose();
        iAtCmdExt.Close();
        }
    iSpecials.ResetAndDestroy();
    iSpecials.Close();
    // AddCmdModeCallback()
    iCmdCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for command mode notification
// The callback will be called when command mode starts or ends
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::AddCmdModeCallback( MDunCmdModeMonitor* aCallback )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (aCallback) not initialized!" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCmdCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCmdCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Adds data for parsing and parses if necessary
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::AddDataForParsing( TDesC8& aInput,
                                                   TBool& aMoreNeeded )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing()") ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing() received (%d bytes):"), aInput.Length() ));
    FTRACE(FPrintRaw(aInput) );
    TBool editorMode = iCmdPusher->EditorMode();
    if ( editorMode )
        {
        // Note: return here with "no more data needed" and some error to fool
        // CDunUpstream into not reissuing the read request.
        iCmdPusher->IssueRequest( aInput, EFalse );
        aMoreNeeded = EFalse;
        return KErrGeneral;
        }
    iInput = &aInput;  // iInput only for normal mode
    // Manage partial AT command
    TBool moreNeeded = ManagePartialCommand();
    if ( moreNeeded )
        {
        aMoreNeeded = ETrue;
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing() (more partial) complete") ));
        return KErrNone;
        }
    if ( iHandleState != EDunStateIdle )
        {
        aMoreNeeded = EFalse;
        ManageEndOfCmdHandling( EFalse, EFalse );
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing() (not ready) complete") ));
        return KErrNotReady;
        }
    TBool pushStarted = HandleASlashCommand();
    if ( pushStarted )
        {
        // Note: return here with "partial input" status to fool CDunUpstream
        // into reissuing the read request. The AT command has not really
        // started yet so this is necessary.
        aMoreNeeded = ETrue;
        ManageEndOfCmdHandling( EFalse, EFalse );
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing() (A/) complete") ));
        return KErrNone;
        }
    iHandleState = EDunStateAtCmdHandling;
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = 0;
    iDecodeInfo.iPrevExists = EFalse;
    iParseInfo.iLimit = KErrNotFound;
    iParseInfo.iSendBuffer.Zero();
    iEditorModeInfo.iContentFound = EFalse;
    HandleNextSubCommand();
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddDataForParsing() complete") ));
    aMoreNeeded = EFalse;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Manages request to abort command handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::ManageAbortRequest()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageAbortRequest()") ));
    // Just forward the request, do no other own processing
    TInt retVal = iCmdPusher->ManageAbortRequest();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageAbortRequest() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Sends a character to be echoed
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::SendEchoCharacter( const TDesC8* aInput,
                                                   MDunAtCmdEchoer* aCallback )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SendEchoCharacter()") ));
    TInt retVal = iCmdEchoer->SendEchoCharacter( aInput, aCallback );
    FTRACE(FPrint( _L("CDunAtCmdHandler::SendEchoCharacter() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Stops sending of AT command from parse buffer
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::Stop()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::Stop()") ));
    // Only stop iCmdPusher here, not iUrcHandlers!
    if ( iHandleState != EDunStateAtCmdHandling )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iCmdPusher->Stop();
    // The line below is used in the case when this function is called by
    // CDunUpstream as a result of "data mode ON" change notification.
    // In this case it is possible that HandleNextSubCommand() returns
    // without resetting the iSendBuffer because of the way it checks the
    // iHandleState.
    ManageEndOfCmdHandling( ETrue, EFalse );
    FTRACE(FPrint( _L("CDunAtCmdHandler::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Starts URC message handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::StartUrc()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc()") ));
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        TInt retTemp = iUrcHandlers[i]->IssueRequest();
        if ( retTemp!=KErrNone && retTemp!=KErrNotReady )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc() (ERROR) complete") ));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops URC message handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::StopUrc()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::StopUrc()") ));
    TInt i;
    TInt retVal = KErrNone;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        retVal = iUrcHandlers[i]->Stop();
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::StopUrc() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// CDunAtCmdHandler::CDunAtCmdHandler
// ---------------------------------------------------------------------------
//
CDunAtCmdHandler::CDunAtCmdHandler( MDunAtCmdStatusReporter* aUpstream,
                                    MDunStreamManipulator* aDownstream,
                                    const TDesC8* aConnectionName ) :
    iUpstream( aUpstream ),
    iDownstream( aDownstream ),
    iConnectionName( aConnectionName )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtCmdHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL()") ));
    if ( !iUpstream || !iDownstream || !iConnectionName )
        {
        User::Leave( KErrGeneral );
        }
    // Connect to AT command extension (must succeed)
    TInt retTemp = KErrNone;
    CleanupClosePushL( iAtCmdExt );
    retTemp = iAtCmdExt.Connect( EDunATExtension, *iConnectionName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() connect (%d)"), retTemp));
        User::Leave( retTemp );
        }
    CleanupClosePushL( iAtCmdExtCommon );
    retTemp = iAtCmdExtCommon.Connect( *iConnectionName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() common connect (%d)"), retTemp));
        User::Leave( retTemp );
        }
    // Create the array of special commands
    CreateSpecialCommandsL();
    // Create the plugin handlers
    CreatePluginHandlersL();
    // Create the echo handler
    iCmdEchoer = CDunAtCmdEchoer::NewL( iDownstream );
    // Create the listeners
    iEcomListen = CDunAtEcomListen::NewL( &iAtCmdExt, this );
    iModeListen = CDunAtModeListen::NewL( &iAtCmdExtCommon, this );
    iNvramListen = CDunAtNvramListen::NewL( &iAtCmdExt, &iAtCmdExtCommon );
    iAtSpecialCmdHandler = CDunAtSpecialCmdHandler::NewL();
    // Set the default modes (+report) and characters
    GetAndSetDefaultSettingsL();
    // Start listening
    iEcomListen->IssueRequest();
    iModeListen->IssueRequest();
    iNvramListen->IssueRequest();
    CleanupStack::Pop( &iAtCmdExtCommon );
    CleanupStack::Pop( &iAtCmdExt );
    FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::Initialize()
    {
    // Don't initialize iUpstream here (it is set through NewL)
    // Don't initialize iDownstream here (it is set through NewL)
    // Don't initialize iConnectionName here (it is set through NewL)
    iHandleState = EDunStateIdle;
    iCarriageReturn = 0;
    iLineFeed = 0;
    iBackspace = 0;
    iInput = NULL;
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = KErrNotFound;
    iDecodeInfo.iExtendedIndex = KErrNotFound;
    iDecodeInfo.iPrevChar = 0;
    iDecodeInfo.iPrevExists = EFalse;
    iDecodeInfo.iAssignFound = EFalse;
    iDecodeInfo.iInQuotes = EFalse;
    iDecodeInfo.iSpecialFound = EFalse;
    iDecodeInfo.iCmdsHandled = 0;
    iEditorModeInfo.iContentFound = EFalse;
    iCmdPusher = NULL;
    iEcomListen = NULL;
    iModeListen = NULL;
    iNvramListen = NULL;
    iDataMode = EFalse;
    iEchoOn = EFalse;
    iQuietOn = EFalse;
    iVerboseOn = EFalse;
    iEndIndex = KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Creates plugin handlers for this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::CreatePluginHandlersL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL()") ));
    if ( !iAtCmdExt.Handle() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL() complete") ));
        User::Leave( KErrGeneral );
        }
    // First create the command reply pusher
    CDunAtCmdPusher* cmdPusher = CDunAtCmdPusher::NewLC( &iAtCmdExt,
                                                         this,
                                                         iDownstream,
                                                         &iOkBuffer );
    // Next create the URC handlers
    TInt i;
    TInt numOfPlugins = iAtCmdExt.NumberOfPlugins();
    for ( i=0; i<numOfPlugins; i++ )
        {
        AddOneUrcHandlerL();
        }
    CleanupStack::Pop( cmdPusher );
    iCmdPusher = cmdPusher;
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL() complete") ));
    }

// ---------------------------------------------------------------------------
// Creates an array of special commands
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::CreateSpecialCommandsL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreateSpecialCommandsL()") ));
    TInt retTemp = KErrNone;
    TBool firstSearch = ETrue;
    for ( ;; )
        {
        // Let's borrow iLineBuffer for this purpose
        retTemp = iAtCmdExt.GetNextSpecialCommand( iLineBuffer, firstSearch );
        if ( retTemp != KErrNone )
            {
            break;
            }
        TInt lineLength = iLineBuffer.Length();
        HBufC8* specialCmd = HBufC8::NewMaxLC( lineLength );
        *specialCmd = iLineBuffer;
        iSpecials.AppendL( specialCmd );
        CleanupStack::Pop( specialCmd );
        }
    iLineBuffer.Zero();
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreateSpecialCommandsL() complete") ));
    }

// ---------------------------------------------------------------------------
// Recreates special command data.
// This is done when a plugin is installed or uninstalled.
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::RecreateSpecialCommands()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RecreateSpecialCommands()") ));
    iSpecials.ResetAndDestroy();
    TRAPD( retTrap, CreateSpecialCommandsL() );
    FTRACE(FPrint( _L("CDunAtCmdHandler::RecreateSpecialCommands() complete") ));
    return retTrap;
    }

// ---------------------------------------------------------------------------
// Gets default settings from RATExtCommon and sets them to RATExt
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::GetAndSetDefaultSettingsL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL()") ));
    // Note: Let's assume command mode is off by default
    TUint modeSet = GetCurrentModeL( KModeEcho | KModeQuiet | KModeVerbose );
    iEchoOn    = ( modeSet & KEchoModeBase    ) ? ETrue : EFalse;
    iQuietOn   = ( modeSet & KQuietModeBase   ) ? ETrue : EFalse;
    iVerboseOn = ( modeSet & KVerboseModeBase ) ? ETrue : EFalse;
    iCarriageReturn = GetCurrentModeL( KModeCarriage );
    iLineFeed = GetCurrentModeL( KModeLineFeed );
    iBackspace = GetCurrentModeL( KModeBackspace );
    iAtCmdExt.ReportQuietModeChange( iQuietOn );
    iAtCmdExt.ReportVerboseModeChange( iVerboseOn );
    iAtCmdExt.ReportCharacterChange( ECharTypeCarriage, iCarriageReturn );
    iAtCmdExt.ReportCharacterChange( ECharTypeLineFeed, iLineFeed );
    iAtCmdExt.ReportCharacterChange( ECharTypeBackspace, iBackspace );
    RegenerateReplyStrings();
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() settings: CR=%u, LF=%u, BS=%u"), iCarriageReturn, iLineFeed, iBackspace ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() complete") ));
    }

// ---------------------------------------------------------------------------
// Regenerates the reply strings based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateReplyStrings()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateReplyStrings()") ));
    TBool retVal = EFalse;
    retVal |= RegenerateOkReply();
    retVal |= RegenerateErrorReply();
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateReplyStrings() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Regenerates the ok reply based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateOkReply()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply()") ));
    if ( iDownstream->IsDataInQueue(&iOkBuffer) )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply() (in queue!) complete") ));
        return iQuietOn;
        }
    iOkBuffer.Zero();
    if ( iQuietOn )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply() (quiet) complete") ));
        return ETrue;
        }
    if ( iVerboseOn )
        {
        _LIT8( KVerboseOk, "OK" );
        iOkBuffer.Append( iCarriageReturn );
        iOkBuffer.Append( iLineFeed );
        iOkBuffer.Append( KVerboseOk );
        iOkBuffer.Append( iCarriageReturn );
        iOkBuffer.Append( iLineFeed );
        }
    else
        {
        _LIT8( KNumericOk, "0" );
        iOkBuffer.Append( KNumericOk );
        iOkBuffer.Append( iCarriageReturn );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Regenerates the error reply based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateErrorReply()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply()") ));
    if ( iDownstream->IsDataInQueue(&iErrorBuffer) )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply() (in queue!) complete") ));
        return iQuietOn;
        }
    iErrorBuffer.Zero();
    if ( iQuietOn )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply() (quiet) complete") ));
        return ETrue;
        }
    if ( iVerboseOn )
        {
        _LIT8( KVerboseError, "ERROR" );
        iErrorBuffer.Append( iCarriageReturn );
        iErrorBuffer.Append( iLineFeed );
        iErrorBuffer.Append( KVerboseError );
        iErrorBuffer.Append( iCarriageReturn );
        iErrorBuffer.Append( iLineFeed );
        }
    else
        {
        _LIT8( KNumericError, "4" );
        iErrorBuffer.Append( KNumericError );
        iErrorBuffer.Append( iCarriageReturn );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Gets current mode
// ---------------------------------------------------------------------------
//
TUint CDunAtCmdHandler::GetCurrentModeL( TUint aMask )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL()") ));
    TUint maskCheck = aMask & ( ~KSupportedModes );
    if ( maskCheck != 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() (not supported) complete") ));
        User::Leave( KErrNotSupported );
        }
    TUint newMode = 0;
    TInt retTemp = iAtCmdExtCommon.GetMode( aMask, newMode );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() (ERROR) complete") ));
        User::Leave( retTemp );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() complete") ));
    return newMode & (KModeChanged-1);
    }

// ---------------------------------------------------------------------------
// Instantiates one URC message handling class instance and adds it to the URC
// message handler array
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler* CDunAtCmdHandler::AddOneUrcHandlerL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddOneUrcHandlerL()") ));
    CDunAtUrcHandler* urcHandler = CDunAtUrcHandler::NewLC( &iAtCmdExt,
                                                            iDownstream );
    iUrcHandlers.AppendL( urcHandler );
    CleanupStack::Pop( urcHandler );
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddOneUrcHandlerL() complete") ));
    return urcHandler;
    }

// ---------------------------------------------------------------------------
// Deletes all instantiated URC message handlers
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::DeletePluginHandlers()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::DeletePluginHandlers()") ));
    delete iCmdPusher;
    iCmdPusher = NULL;
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        delete iUrcHandlers[i];
        iUrcHandlers[i] = NULL;
        }
    iUrcHandlers.Reset();
    iUrcHandlers.Close();
    FTRACE(FPrint( _L("CDunAtCmdHandler::DeletePluginHandlers() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages partial AT command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManagePartialCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand()") ));
    // Check one character (or unit) based input data
    if ( iInput->Length() == KDunChSetMaxCharLen )
        {
        EchoCommand();
        // Handle backspace and cancel characters
        TBool found = HandleSpecialCharacters();
        if ( found )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (special) complete") ));
            return ETrue;
            }
        }
    TBool moreNeeded = ExtractLineFromInputBuffer();
    if ( moreNeeded )
        {
        // More data is not needed with "A/" (no carriage return), check that
        // special case here, otherwise continue processing
        if ( !IsASlashCommand() )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (more) complete") ));
            return ETrue;
            }
        }
    // If something went wrong, do nothing (return consumed)
    if ( iLineBuffer.Length() <= 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (length) complete") ));
        return ETrue;
        }
    // For other commands, just return with consumed
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Echoes a command if echo is on
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::EchoCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand()") ));
    if ( iInput->Length() > KDunChSetMaxCharLen )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() (wrong length) complete") ));
        return EFalse;
        }
    if ( iEchoOn )
        {
        if ( iDownstream->IsDataInQueue(&iEchoBuffer) )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() (in queue!) complete") ));
            return EFalse;
            }
        iEchoBuffer.Copy( *iInput );
        iDownstream->NotifyDataPushRequest( &iEchoBuffer, NULL );
        FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() (not started) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles backspace and cancel characters
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleSpecialCharacters()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters()") ));
    if ( iInput->Length() != KDunChSetMaxCharLen )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (wrong length) complete") ));
        return EFalse;
        }
    if ( (*iInput)[0] == iBackspace )
        {
        TInt lineLength = iLineBuffer.Length();
        if ( lineLength > 0 )
            {
            iLineBuffer.SetLength( lineLength-1 );
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (backspace) complete") ));
        return ETrue;
        }
    if ( (*iInput)[0] == KDunCancel )
        {
        ManageEndOfCmdHandling( EFalse, EFalse );
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (cancel) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Extracts line from input buffer to line buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ExtractLineFromInputBuffer()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer()") ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() before (%d bytes):"), iLineBuffer.Length() ));
    FTRACE(FPrintRaw(iLineBuffer) );
    // Case1: If no data in iLineBuffer and end-of-line character in iInputBuffer[start]:
    //     - Skip end-of-line characters, find start-of-line condition, find end-of-line character
    //     - If partial line found (start-of-line condition and no end-of-line character):
    //           - Save partial line to iLineBuffer
    //           - Set iEndIndex to end of iInputBuffer
    //     - If full line found (start-of-line condition and end-of-line character):
    //           - Save full line to iLineBuffer
    //           - Skip multiple end-of-line characters until next start-of-line
    //             condition or end of iInputBuffer -> save this position to iEndIndex
    // Case2: If no data in iLineBuffer and non-end-of-line character in iInputBuffer[start]:
    //     - Find end-of-line character
    //     - If partial line found (no end-of-line character):
    //           - Save partial line to iLineBuffer
    //           - Set iEndIndex to end of iLineBuffer
    //     - If full line found (end-of-line character):
    //           - Save full line to iLineBuffer
    //           - Skip multiple end-of-line characters until next start-of-line
    //             condition or end of iInputBuffer -> save this position to iEndIndex
    // Case3: If data in iLineBuffer and end-of-line character in iInputBuffer[start]:
    //     - Skip end-of-line characters
    //     - Keep string currently in iLineBuffer
    //     - Skip end-of-line characters until non-end-of-line or end of
    //       iInputBuffer -> save this position to iEndIndex
    // Case4: If data in iLineBuffer and non-end-of-line character in iInputBuffer[start]:
    //     - Processed the same way as Case1, however "Skip end-of-line characters" does
    //       not have any effect
    if ( iInput->Length() <= 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() (wrong length) complete") ));
        return ETrue;
        }
    TBool moreNeeded = ETrue;
    TBool copyNeeded = EFalse;
    TInt copyLength = KErrNotFound;
    TInt lineLength = iLineBuffer.Length();
    TInt lineMaxLength = iLineBuffer.MaxLength();
    TInt freeLineSpace = lineMaxLength - lineLength;
    TInt inputLength = iInput->Length();
    TInt startIndex = ( iEndIndex>=0 ) ? iEndIndex : 0;
    if ( startIndex >= inputLength )
        {
        iEndIndex = KErrNotFound;
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() (input end) complete") ));
        return ETrue;
        }
    // Cases here:
    // Case1: If no data in iLineBuffer and end-of-line character in iInputBuffer[start]
    // Case2: If no data in iLineBuffer and non-end-of-line character in iInputBuffer[start]
    // Case3: If data in iLineBuffer and end-of-line character in iInputBuffer[start]
    // Case4: If data in iLineBuffer and non-end-of-line character in iInputBuffer[start]
    // Summary: Cases 1, 2 and 4 can be combined. Case 3 needs a separate check.
    TChar character = (*iInput)[startIndex];
    TBool endOfLine = IsEndOfLine(character);
    if ( lineLength>0 && endOfLine )
        {
        moreNeeded = HandleSpecialBufferManagement( startIndex,
                                                    copyLength,
                                                    copyNeeded );
        }
    else
        {
        moreNeeded = HandleGenericBufferManagement( startIndex,
                                                    copyLength,
                                                    copyNeeded );
        }
    if ( copyNeeded && copyLength>0 )
        {
        // Check the case copyLength does not fit to iLineBuffer
        // This case should be handled by returning "more data needed"
        // Also reset the iLineBuffer to ensure the handling doesn't stuck
        // for rest of the commands (usability case)
        if ( copyLength > freeLineSpace )
            {
            iLineBuffer.Zero();
            iEndIndex = KErrNotFound;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() after (%d bytes):"), iLineBuffer.Length() ));
            FTRACE(FPrintRaw(iLineBuffer) );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() (overflow) complete") ));
            return ETrue;
            }
        iLineBuffer.Append( &(*iInput)[startIndex], copyLength );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() after (%d bytes):"), iLineBuffer.Length() ));
        FTRACE(FPrintRaw(iLineBuffer) );
        }
    if ( moreNeeded )
        {
        iEndIndex = KErrNotFound;
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() (more needed) complete") ));
        return ETrue;
        }
    // As a last step adjust the possible multiple IsDelimiterCharacter()
    // characters and set length of iLineBuffer. Leave iEndIndex untouched.
    lineLength = iLineBuffer.Length();
    for ( TInt i=lineLength-1; i>=0; i-- )
        {
        TChar character = iLineBuffer[i];
        if ( !IsDelimiterCharacter(character) )
            {
            iLineBuffer.SetLength( i + 1 );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() after (%d bytes):"), iLineBuffer.Length() ));
            FTRACE(FPrintRaw(iLineBuffer) );
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractLineFromInputBuffer() (line found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles generic buffer management
// (explanation in ExtractLineFromInputBuffer())
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleGenericBufferManagement( TInt& aStartIndex,
                                                       TInt& aCopyLength,
                                                       TBool& aCopyNeeded )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleGenericBufferManagement()") ));
    TInt inputLength = iInput->Length();
    TInt currentIndex = SkipEndOfLineCharacters( aStartIndex );
    if ( currentIndex >= inputLength )
        {
        // No data in iLineBuffer and only end-of-lines in new buffer
        // return with "need more data"
        iEndIndex = inputLength;
        aCopyLength = 0;
        aCopyNeeded = EFalse;
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleGenericBufferManagement() (new end for old no data) complete") ));
        return ETrue;
        }
    // No data in iLineBuffer and non-end-of-line character found
    // Try to find the first start-of-line condition
    TInt lineLength = iLineBuffer.Length();
    if ( lineLength == 0 )
        {
        currentIndex = SkipSubCommandDelimiterCharacters( aStartIndex );
        if ( currentIndex >= inputLength )
            {
            // No data in iLineBuffer and only end-of-lines+delimiter in new buffer
            // return with "need more data"
            iEndIndex = inputLength;
            aCopyLength = 0;
            aCopyNeeded = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::HandleGenericBufferManagement() (new end+delim for old no data) complete") ));
            return ETrue;
            }
        }
    aStartIndex = currentIndex;
    // No data in iLineBuffer and other than end-of-line or delimiter character found
    // Variable currentIndex is now the start of new command
    // Next try to find the end of the command
    TInt endIndex = FindEndOfLine( aStartIndex );
    if ( endIndex >= inputLength )
        {
        // No data in iLineBuffer and start of command found without end
        // return with "need more data"
        iEndIndex = inputLength;
        aCopyLength = inputLength - aStartIndex;
        aCopyNeeded = ETrue;
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleGenericBufferManagement() (start but no end for old no data) complete") ));
        return ETrue;
        }
    // No data in iLineBuffer and end-of-line character found
    // Try to skip possible multiple end-of-line characters
    currentIndex = SkipEndOfLineCharacters( endIndex );
    // Variable currentIndex is now either start of next command or end of iInput
    // Note that this requires that Case 2 must skip the possible IsDelimiterCharacter()s
    iEndIndex = currentIndex;
    aCopyLength = endIndex - aStartIndex;
    aCopyNeeded = ETrue;
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleGenericBufferManagement() (line found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles special buffer management
// (explanation in ExtractLineFromInputBuffer())
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleSpecialBufferManagement( TInt aStartIndex,
                                                       TInt& aCopyLength,
                                                       TBool& aCopyNeeded )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialBufferManagement()") ));
    TInt currentIndex = SkipEndOfLineCharacters( aStartIndex );
    // Variable currentIndex is now either start of next command or end of iInput
    iEndIndex = currentIndex;
    aCopyLength = 0;
    aCopyNeeded = EFalse;
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialBufferManagement() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Skips end-of-line characters
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::SkipEndOfLineCharacters( TInt aStartIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SkipEndOfLineCharacters()") ));
    TInt foundIndex = iInput->Length();
    TInt inputLength = foundIndex;
    for ( TInt i=aStartIndex; i<inputLength; i++ )
        {
        TChar character = (*iInput)[i];
        if ( !IsEndOfLine(character) )
            {
            foundIndex = i;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::SkipEndOfLineCharacters() complete") ));
    return foundIndex;
    }

// ---------------------------------------------------------------------------
// Skips subcommand delimiter characters
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::SkipSubCommandDelimiterCharacters( TInt aStartIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SkipSubCommandDelimiterCharacters()") ));
    TInt inputLength = iInput->Length();
    TInt foundIndex = inputLength;
    for ( TInt i=aStartIndex; i<inputLength; i++ )
        {
        TChar character = (*iInput)[i];
        if ( !IsDelimiterCharacter(character) )
            {
            foundIndex = i;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::SkipSubCommandDelimiterCharacters() complete") ));
    return foundIndex;
    }

// ---------------------------------------------------------------------------
// Finds the end of the line
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::FindEndOfLine( TInt aStartIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindEndOfLine()") ));
    TInt inputLength = iInput->Length();
    TInt foundIndex = inputLength;
    for ( TInt i=aStartIndex; i<inputLength; i++ )
        {
        TChar character = (*iInput)[i];
        // Checking for IsDelimiterCharacter() here needs more logic (a parser).
        // Just check with "IsEndOfLine()"
        if ( IsEndOfLine(character) )
            {
            foundIndex = i;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindEndOfLine() complete") ));
    return foundIndex;
    }

// ---------------------------------------------------------------------------
// Handles next subcommand from line buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleNextSubCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextSubCommand()") ));
    if ( iHandleState != EDunStateAtCmdHandling )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextSubCommand() (not ready) complete") ));
        return EFalse;
        }
    TBool extracted = ExtractNextSubCommand();
    if ( !extracted )
        {
        ManageEndOfCmdHandling( ETrue, ETrue );
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextSubCommand() (last) complete") ));
        return EFalse;
        }
    // Next convert the decoded AT command to uppercase
    // Don't check for case status -> let mixed cases pass
    TInt oldLength = iParseInfo.iSendBuffer.Length();
    iParseInfo.iSendBuffer.SetLength( iParseInfo.iLimit );
    iParseInfo.iSendBuffer.UpperCase();
    iParseInfo.iSendBuffer.SetLength( oldLength );
    // Next always send the command to ATEXT
    iCmdPusher->IssueRequest( iParseInfo.iSendBuffer );
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextSubCommand() complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Manages end of AT command handling
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ManageEndOfCmdHandling( TBool aNotifyLocal,
                                               TBool aNotifyExternal )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling()") ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling() (loc=%d, ext=%d)"), aNotifyLocal, aNotifyExternal ));
    // If iEndIndex is (>=0 && <iInput.Length()) it means more data waits in
    // iInput that didn't fit in iInputBuffer.
    TInt cmdLength = iInput->Length();
    TBool subBlock = ( iEndIndex>=0&&iEndIndex<cmdLength ) ? ETrue : EFalse;
    if ( iLineBuffer.Length()>0 && !subBlock )
        {
        // Line buffer set and no partial subblock, copy to lastbuffer
        iLastBuffer.Copy( iLineBuffer );
        }
    iLineBuffer.Zero();
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = 0;
    iDecodeInfo.iPrevExists = EFalse;
    iParseInfo.iLimit = KErrNotFound;
    iParseInfo.iSendBuffer.Zero();
    iEditorModeInfo.iContentFound = EFalse;
    iHandleState = EDunStateIdle;
    if ( aNotifyLocal )
        {
        iCmdPusher->SetEndOfCmdLine();
        }
    // iEndIndex must not be reset to KErrNotFound only when
    // ExtractLineFromInputBuffer() found the next line
    // (when moreNeeded is EFalse)
    TBool resetIndex = ETrue;
    if ( aNotifyExternal )
        {
        TBool moreNeeded = ExtractLineFromInputBuffer();
        if ( moreNeeded )
            {
            iUpstream->NotifyParserNeedsMoreData();
            }
        else
            {
            // AppendBlockToInputBuffer() was able to fill with known end, handle next
            iHandleState = EDunStateAtCmdHandling;
            HandleNextSubCommand();
            resetIndex = EFalse;
            }
        }
    if ( resetIndex )
        {
        iEndIndex = KErrNotFound;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling() complete") ));
    }

// ---------------------------------------------------------------------------
// Extracts next subcommand from line buffer to send buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ExtractNextSubCommand( TBool aPeek )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextSubCommand()") ));
    TDunDecodeInfo oldInfo = iDecodeInfo;
    iParseInfo.iLimit = KErrNotFound;
    iParseInfo.iSendBuffer.Zero();
    // Find start of subcommand from line buffer
    TInt startIndex = FindStartOfSubCommand();
    if ( startIndex < 0 )
        {
        RestoreOldDecodeInfo( aPeek, oldInfo );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextDecodedCommand() (no start) complete") ));
        return EFalse;
        }
    iDecodeInfo.iDecodeIndex = startIndex;
    TBool specialCmd = EFalse;
    TInt endIndex = KErrNotFound;
    specialCmd = CheckSpecialCommand( endIndex );
    if ( !specialCmd )
        {
        FindSubCommand( endIndex );
        }
    TInt lineLength = iLineBuffer.Length();
    TBool inStartLimits = ( startIndex >= 0 && startIndex < lineLength ) ? ETrue : EFalse;
    TBool inEndLimits   = ( endIndex   >= 0 && endIndex   < lineLength ) ? ETrue : EFalse;
    if ( !inStartLimits || !inEndLimits )
        {
        RestoreOldDecodeInfo( aPeek, oldInfo );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextSubCommand() (no end) complete") ));
        return EFalse;
        }
    TInt cmdLength = endIndex - startIndex + 1;
    // If the limit was not already set then do it now
    if ( iParseInfo.iLimit < 0 )
        {
        iParseInfo.iLimit = cmdLength;
        }
    // Next create a new command
    if ( !iDecodeInfo.iFirstDecode )
        {
        _LIT( KAtPrefix, "AT" );
        iParseInfo.iSendBuffer.Append( KAtPrefix );
        if ( !specialCmd )  // Already added with CheckSpecialCommand()
            {
            iParseInfo.iLimit += 2;  // Length of "AT"
            }
        // Note: The length of iDecodeBuffer is not exceeded here because "AT"
        // is added only for the second commands after that.
        }
    iParseInfo.iSendBuffer.Append( &iLineBuffer[startIndex], cmdLength );
    // Change settings for the next decode round
    iDecodeInfo.iFirstDecode = EFalse;
    iDecodeInfo.iDecodeIndex = endIndex + 1;
    RestoreOldDecodeInfo( aPeek, oldInfo );
    if ( !aPeek )
        {
        iDecodeInfo.iCmdsHandled++;
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextSubCommand() (handled=%d)"), iDecodeInfo.iCmdsHandled ));
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextSubCommand() complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Finds the start of subcommand from line buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::FindStartOfSubCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindStartOfSubCommand()") ));
    TInt i;
    TInt foundIndex = KErrNotFound;
    TInt lineLength = iLineBuffer.Length();
    for ( i=iDecodeInfo.iDecodeIndex; i<lineLength; i++ )
        {
        TChar character = iLineBuffer[i];
        if ( !IsDelimiterCharacter(character) )
            {
            foundIndex = i;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindStartOfSubCommand() complete") ));
    return foundIndex;
    }

// ---------------------------------------------------------------------------
// Restores old decode info. For ExtractNextDecodedCommand() when aPeeks is
// ETrue.
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::RestoreOldDecodeInfo( TBool aPeek,
                                             TDunDecodeInfo& aOldInfo )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RestoreOldDecodeInfo()") ));
    if ( aPeek )
        {
        iEditorModeInfo.iPeekInfo = iDecodeInfo;
        iDecodeInfo = aOldInfo;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RestoreOldDecodeInfo() complete") ));
    }

// ---------------------------------------------------------------------------
// Tests for end of AT command line
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsEndOfLine( TChar& aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand()") ));
    if ( aCharacter==iCarriageReturn || aCharacter==iLineFeed )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand() (found) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks if character is delimiter character
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsDelimiterCharacter( TChar aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter()") ));
    if ( aCharacter.IsSpace() || aCharacter==';' || aCharacter==0x00 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter() (not delimiter) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks if character is of extended group
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsExtendedCharacter( TChar aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter()") ));
    if ( aCharacter=='+'  || aCharacter=='&' || aCharacter=='%' ||
         aCharacter=='\\' || aCharacter=='*' || aCharacter=='#' ||
         aCharacter=='$'  || aCharacter=='^' )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter() (not extended) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks special command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::CheckSpecialCommand( TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand()") ));
    TInt atPrefixLen = 0;
    TInt startIndex = iDecodeInfo.iDecodeIndex;
    TInt newLength = iLineBuffer.Length() - startIndex;
    TBuf8<KDunLineBufLength> upperBuf;
    if ( !iDecodeInfo.iFirstDecode )
        {
        // For cases such as "ATM1L3DT*99#" "DT" must have "AT"
        _LIT8( KAtPrefix, "AT" );
        upperBuf.Copy( KAtPrefix );
        atPrefixLen = 2;  // "AT"
        newLength += atPrefixLen;
        }
    upperBuf.Append( &iLineBuffer[startIndex], newLength );
    upperBuf.UpperCase();
    TInt i;
    TInt count = iSpecials.Count();
    for ( i=0; i<count; i++ )
        {
        HBufC8* specialCmd = iSpecials[i];
        TInt specialLength = specialCmd->Length();
        if ( newLength < specialLength )
            {
            continue;
            }
        TInt origLength = newLength;
        if ( newLength > specialLength )
            {
            upperBuf.SetLength( specialLength );
            }
        TInt cmpResult = upperBuf.Compare( *specialCmd );
        upperBuf.SetLength( origLength );
        if ( cmpResult == 0 )
            {
            iParseInfo.iLimit = specialLength;
            aEndIndex = (origLength-1) + startIndex - atPrefixLen;
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand() complete") ));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Saves character decode state for a found character
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::SaveFoundCharDecodeState( TChar aCharacter,
                                                 TBool aAddSpecial )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SaveFoundCharDecodeState()") ));
    iDecodeInfo.iPrevExists = ETrue;
    iDecodeInfo.iPrevChar = aCharacter;
    if ( aAddSpecial )
        {
        iDecodeInfo.iSpecialFound =
                iAtSpecialCmdHandler->IsCompleteSubCommand( aCharacter );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::SaveFoundCharDecodeState() complete") ));
    }

// ---------------------------------------------------------------------------
// Saves character decode state for a not found character
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::SaveNotFoundCharDecodeState()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SaveNotFoundCharDecodeState()") ));
    iDecodeInfo.iPrevExists = EFalse;
    // Note: don't set iAssignFound or iInQuotes here
    iDecodeInfo.iSpecialFound = EFalse;
    FTRACE(FPrint( _L("CDunAtCmdHandler::SaveNotFoundCharDecodeState() complete") ));
    }

// ---------------------------------------------------------------------------
// Find quotes within subcommands
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::FindSubCommandQuotes( TChar aCharacter,
                                              TInt aStartIndex,
                                              TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandQuotes()") ));
    if ( aCharacter == '"' )
        {
        if ( iParseInfo.iLimit < 0 )  // Only first the first '"'
            {
            iParseInfo.iLimit = aEndIndex - aStartIndex;
            }
        iDecodeInfo.iInQuotes ^= ETrue;  // EFalse to ETrue or ETrue to EFalse
        SaveFoundCharDecodeState( aCharacter, EFalse );
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandQuotes() (quote) complete") ));
        return ETrue;
        }
    // The next ones are those that are not in quotes.
    // We still need to save the iParseInfo.iLimit and skip non-delimiter characters.
    if ( aCharacter == '=' )
        {
        if ( iParseInfo.iLimit < 0 )  // Only first the first '"'
            {
            iParseInfo.iLimit = aEndIndex - aStartIndex;
            }
        iDecodeInfo.iAssignFound = ETrue;
        SaveFoundCharDecodeState( aCharacter, EFalse );
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandQuotes() (equals) complete") ));
        return ETrue;
        }
    if ( iDecodeInfo.iInQuotes )
        {
        SaveNotFoundCharDecodeState();
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandQuotes() (in quotes) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandQuotes() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Check if in next subcommand's extended border
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsExtendedBorder( TChar aCharacter,
                                          TInt aStartIndex,
                                          TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedBorder()") ));
    TInt expectedIndex = 0;  // "+CMD" when iDecodeInfo.iFirstDecode is EFalse
    TInt extendedIndex = aEndIndex - aStartIndex;  // absolute index to the extended character
    if ( iDecodeInfo.iFirstDecode )
        {
        expectedIndex = 2;  // "AT+CMD"
        }
    if ( extendedIndex == expectedIndex )
        {
        iDecodeInfo.iExtendedIndex = aEndIndex;
        SaveFoundCharDecodeState( aCharacter );
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedBorder() (no border normal) complete") ));
        return EFalse;
        }
    // Now suspect border found so peek the next character after the suspected
    // extended character. If it is not alphabetical character, return with EFalse.
    // This case is to detect the cases such as "AT+VTS={*,3000}", where '*' would
    // be the start of the next command in normal cases.
    TInt peekIndex = aEndIndex + 1;
    TInt lineLength = iLineBuffer.Length();
    if ( peekIndex < lineLength )
        {
        TChar nextCharacter = iLineBuffer[peekIndex];
        if ( !nextCharacter.IsAlpha() )
            {
            SaveFoundCharDecodeState( aCharacter );
            FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedBorder() (no border special) complete") ));
            return EFalse;
            }
        }
    aEndIndex--;
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedBorder() (border) complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Finds subcommand with alphanumeric borders
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::FindSubCommandAlphaBorder( TChar aCharacter,
                                                   TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder()") ));
    if ( iDecodeInfo.iAssignFound && !iDecodeInfo.iInQuotes )
        {
        // Check the special case when assigning a number with "basic" command
        // and there is no delimiter after it. In this case <Numeric>|<Alpha>
        // border must be detected but only for a "basic" command, not for
        // extended.
        if ( iDecodeInfo.iExtendedIndex<0    && iDecodeInfo.iPrevExists &&
             iDecodeInfo.iPrevChar.IsDigit() && aCharacter.IsAlpha() )
            {
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (N|A) complete") ));
            return ETrue;
            }
        // The code below is for the following type of cases:
        // (do not check alphanumeric borders if "=" set without quotes):
        // AT+CMD=a
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (skip) complete") ));
        return EFalse;
        }
    if ( !iDecodeInfo.iPrevExists || !aCharacter.IsAlpha() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (not found) complete") ));
        return EFalse;
        }
    if ( iDecodeInfo.iPrevChar.IsAlpha() )
        {
        // The check below detects the following type of cases
        // (note that special handling is needed to separate the Alpha|Alpha boundary):
        // AT&FE0
        if ( iDecodeInfo.iSpecialFound )
            {
            // Special command was found before and this is Alpha|Alpha boundary -> end
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (special) complete") ));
            return ETrue;
            }
        // The code below is for the following type of cases
        // (note there is no border between C|M, for example -> continue):
        // ATCMD
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (continue) complete") ));
        return EFalse;
        }
    // The code below is for skipping the following type of cases:
    // AT+CMD [the '+' must be skipped]
    if ( aEndIndex-1 == iDecodeInfo.iExtendedIndex )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (extended) complete") ));
        return EFalse;
        }
    // The code below is for the following type of cases:
    // ATCMD?ATCMD
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommandAlphaBorder() (boundary) complete") ));
    aEndIndex--;
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Finds subcommand
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::FindSubCommand( TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand()") ));
    TInt startIndex = iDecodeInfo.iDecodeIndex;
    aEndIndex = startIndex;
    TBool found = EFalse;
    TInt lineLength = iLineBuffer.Length();
    iDecodeInfo.iAssignFound = EFalse;
    iDecodeInfo.iInQuotes = EFalse;
    iDecodeInfo.iExtendedIndex = KErrNotFound;
    SaveNotFoundCharDecodeState();
    iAtSpecialCmdHandler->ResetComparisonBuffer();  // just to be sure
    for ( ; aEndIndex<lineLength; aEndIndex++ )
        {
        TChar character = iLineBuffer[aEndIndex];
        found = FindSubCommandQuotes( character, startIndex, aEndIndex );
        if ( found )
            {
            continue;
            }
        if ( character == '?' )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand() (?) complete") ));
            return KErrNone;
            }
        // The check below detects the following type of cases:
        // ATCMD<delimiter>
        if ( IsDelimiterCharacter(character) )
            {
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand() (delimiter) complete") ));
            return KErrNone;
            }
        // The check below detects the following type of cases:
        // ATCMD+CMD [first + as delimiter]
        // AT+CMD+CMD [second + as delimiter]
        if ( IsExtendedCharacter(character) )
            {
            found = IsExtendedBorder( character, startIndex, aEndIndex );
            if ( !found )
                {
                continue;
                }
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand() (extended) complete") ));
            return KErrNone;
            }
        found = FindSubCommandAlphaBorder( character, aEndIndex );
        if ( found )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand() (alpha sub) complete") ));
            return KErrNone;
            }
        SaveFoundCharDecodeState( character );
        }
    aEndIndex--;
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindSubCommand() (not found) complete") ));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Check if "A/" command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsASlashCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand()") ));
    if ( iLineBuffer.Length() == 2 )
        {
        if ( iLineBuffer[1] == '/' &&
            (iLineBuffer[0] == 'A' || iLineBuffer[0] == 'a') )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand() (found) complete") ));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles "A/" command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleASlashCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand()") ));
    // If not "A/" command, return
    if ( !IsASlashCommand() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() (no push) complete") ));
        return EFalse;
        }
    iEndIndex = iInput->Length();  // Causes skipping of last '/' in ManageEndOfCmdHandling()
    // If "A/" command and last buffer exist, set the last buffer as the current buffer
    if ( iLastBuffer.Length() > 0 )
        {
        iLineBuffer.Copy( iLastBuffer );
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() (copy) complete") ));
        return EFalse;
        }
    // Last buffer not set so return "ERROR"
    iDownstream->NotifyDataPushRequest( &iErrorBuffer, NULL );
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Manages command mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageCommandModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange()" ) ));
    if ( aMode & KCommandModeChanged )
        {
        if ( aMode & KModeCommand )  // command mode ON
            {
            ReportCommandModeChange( ETrue );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() command mode changed ON" ) ));
            }
        else  // command mode OFF
            {
            ReportCommandModeChange( EFalse );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() command mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange()" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Reports command mode start/end change
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ReportCommandModeChange( TBool aStart )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ReportCommandModeChange()" ) ));
    TInt i;
    TInt count = iCmdCallbacks.Count();
    if ( aStart )
        {
        if ( iDataMode )
            {
            for ( i=0; i<count; i++ )
                {
                iCmdCallbacks[i]->NotifyCommandModeStart();
                }
            iDataMode = EFalse;
            }
        }
    else  // end
        {
        if ( !iDataMode )
            {
            for ( i=0; i<count; i++ )
                {
                iCmdCallbacks[i]->NotifyCommandModeEnd();
                }
            iDataMode = ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ReportCommandModeChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages echo mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageEchoModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange()" ) ));
    if ( aMode & KEchoModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() checking echo mode..." ) ));
        if ( aMode & KModeEcho )  // echo mode ON
            {
            iEchoOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() echo mode changed ON" ) ));
            }
        else  // echo mode OFF
            {
            iEchoOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() echo mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages quiet mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageQuietModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange()" ) ));
    if ( aMode & KQuietModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() checking quiet mode..." ) ));
        if ( aMode & KModeQuiet )  // quiet mode ON
            {
            iAtCmdExt.ReportQuietModeChange( ETrue );
            iQuietOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() quiet mode changed ON" ) ));
            }
        else  // quiet mode OFF
            {
            iAtCmdExt.ReportQuietModeChange( EFalse );
            iQuietOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() quiet mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages quiet mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageVerboseModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange()" ) ));
    if ( aMode & KVerboseModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() checking verbose mode..." ) ));
        if ( aMode & KModeVerbose )  // verbose mode ON
            {
            iAtCmdExt.ReportVerboseModeChange( ETrue );
            iVerboseOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyVerboseStatusChange() verbose mode changed ON" ) ));
            }
        else  // verbose mode OFF
            {
            iAtCmdExt.ReportVerboseModeChange( EFalse );
            iVerboseOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyVerboseStatusChange() verbose mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages character change
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ManageCharacterChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange()" ) ));
    if ( aMode & KCarriageChanged )
        {
        iCarriageReturn = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeCarriage, iCarriageReturn );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() carriage return changed" ) ));
        }
    else if ( aMode & KLineFeedChanged )
        {
        iLineFeed = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeLineFeed, iLineFeed );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() line feed changed" ) ));
        }
    else if ( aMode & KBackspaceChanged )
        {
        iBackspace = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeBackspace, iBackspace );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() backspace changed" ) ));
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages editor mode reply
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::ManageEditorModeReply( TBool aStart )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEditorModeReply()" ) ));
    // Two modes possible here:
    // 1) Sending data directly from DTE to DCE, i.e. no subsequent data in
    //    the input buffer -> Reissue read request from DTE.
    // 2) Sending data from input buffer to DCE -> Do not reissue read request
    //    from DTE: send the data in a loop
    // In summary: send data byte-by-byte in editor mode until end of input.
    // When end of input notify CDunUpstream to reissue the read request.
    TBool nextContentFound = FindNextContent( aStart );
    if ( !nextContentFound )
        {
        iUpstream->NotifyEditorModeReply( aStart );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEditorModeReply() complete") ));
        return KErrNone;
        }
    // In block mode end the block mode by sending <ESC> and hope it works.
    iEscapeBuffer.Zero();
    iEscapeBuffer.Append( KDunEscape );
    iCmdPusher->IssueRequest( iEscapeBuffer, EFalse );
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEditorModeReply() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Finds the next content from the input data
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::FindNextContent( TBool aStart )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindNextContent()" ) ));
    if ( !aStart )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindNextContent() (skip) complete" ) ));
        return iEditorModeInfo.iContentFound;
        }
    // If iEndIndex is (>=0 && <iInput.Length()) it means more data waits in
    // iInput that didn't fit in iInputBuffer. Only check FindStartOfCommand()
    // if iEndIndex < 0, meaning more data is needed from CDunUpstream.
    TBool contentFound = EFalse;
    TInt cmdLength = iInput->Length();
    TBool subBlock = ( iEndIndex>=0&&iEndIndex<cmdLength ) ? ETrue : EFalse;
    if ( subBlock )
        {
        contentFound = ETrue;
        }
    if ( !contentFound )
        {
        contentFound = ExtractNextSubCommand( ETrue );  // peek
        }
    iEditorModeInfo.iContentFound = contentFound;
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindNextContent() complete" ) ));
    return contentFound;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about end of AT command processing. This is after all reply data
// for an AT command is multiplexed to the downstream.
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyEndOfProcessing( TInt /*aError*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfProcessing()" ) ));
    TBool editorMode = iCmdPusher->EditorMode();
    if ( editorMode )
        {
        ManageEditorModeReply( ETrue );
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfProcessing() (editor) complete" ) ));
        return KErrNone;
        }
    HandleNextSubCommand();
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfProcessing() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about request to stop AT command handling for the rest of the
// command line data
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::NotifyEndOfCmdLineProcessing()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfCmdLineProcessing()" ) ));
    ManageEndOfCmdHandling( ETrue, ETrue );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfCmdLineProcessing() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about request to peek for the next command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::NotifyNextCommandPeekRequest()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyNextCommandPeekRequest()") ));
    TBool extracted = ExtractNextSubCommand( ETrue );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyNextCommandPeekRequest() complete") ));
    return extracted;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about editor mode reply
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyEditorModeReply()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEditorModeReply()") ));
    TInt retVal = ManageEditorModeReply( EFalse );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEditorModeReply() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunAtEcomListen.
// Notifies about new plugin installation
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyPluginInstallation( TUid& /*aPluginUid*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation()" ) ));
    CDunAtUrcHandler* urcHandler = NULL;
    TRAPD( retTrap, urcHandler=AddOneUrcHandlerL() );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (trapped!) complete" ) ));
        return retTrap;
        }
    TInt retTemp = urcHandler->IssueRequest();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (issuerequest) complete" ) ));
        return retTemp;
        }
    TUid ownerUid = urcHandler->OwnerUid();
    iAtCmdExt.ReportListenerUpdateReady( ownerUid, EEcomTypeInstall );
    // As a last step recreate the special command data
    retTemp = RecreateSpecialCommands();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (recreate) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtEcomListen.
// Notifies about existing plugin uninstallation
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyPluginUninstallation( TUid& aPluginUid )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation()" ) ));
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=count-1; i>=0; i-- )
        {
        TUid ownerUid = iUrcHandlers[i]->OwnerUid();
        if ( ownerUid == aPluginUid )
            {
            delete iUrcHandlers[i];
            iUrcHandlers.Remove( i );
            iAtCmdExt.ReportListenerUpdateReady( ownerUid,
                                                 EEcomTypeUninstall );
            }
        }
    // As a last step recreate the special command data
    TInt retTemp = RecreateSpecialCommands();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation() (recreate) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtModeListen.
// Gets called on mode status change
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyModeStatusChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange()") ));
    TBool commandModeSet = ManageCommandModeChange( aMode );
    TBool echoModeSet = ManageEchoModeChange( aMode );
    TBool quietModeSet = ManageQuietModeChange( aMode );
    TBool verboseModeSet = ManageVerboseModeChange( aMode );
    if ( quietModeSet || verboseModeSet )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() (regenerate) mode set" ) ));
        RegenerateReplyStrings();
        return KErrNone;
        }
    // Keep the following after "quietModeSet || verboseModeSet" in order to
    // regenerate the reply also if two modes change at the same time
    if ( commandModeSet || echoModeSet )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() mode set" ) ));
        return KErrNone;
        }
    ManageCharacterChange( aMode );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: CR=%u, LF=%u, BS=%u"), iCarriageReturn, iLineFeed, iBackspace ));
    RegenerateReplyStrings();
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() complete") ));
    return KErrNone;
    }
