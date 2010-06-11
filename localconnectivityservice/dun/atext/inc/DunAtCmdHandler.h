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

#ifndef C_CDUNATCMDHANDLER_H
#define C_CDUNATCMDHANDLER_H

#include <atext.h>
#include <e32base.h>
#include <atextcommon.h>
#include "DunDataPusher.h"
#include "DunAtCmdPusher.h"
#include "DunAtCmdEchoer.h"
#include "DunAtEcomListen.h"
#include "DunAtModeListen.h"
#include "DunAtNvramListen.h"
#include "DunAtSpecialCmdHandler.h"

const TInt KDunChSetMaxCharLen = 1;          // Only ASCII supported for now
const TInt KDunOkBufLength     = 1+1+2+1+1;  // <CR>+<LF>+"OK"+<CR>+<LF>
const TInt KDunErrorBufLength  = 1+1+5+1+1;  // <CR>+<LF>+"ERROR"+<CR>+<LF>
const TInt KDunInputBufLength  = (512 + 1);  // 512 chars for command + <CR>
const TInt KDunEscBufLength    = 1;          // Escape (0x1B) character

class CDunAtUrcHandler;
class MDunConnMon;
class MDunStreamManipulator;

/**
 *  Class used for storing information related to string conversion and parsing
 *
 *  @lib dunatext.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunParseInfo )
    {

public:

    /**
     * Buffer for sending
     */
    TBuf8<KDunInputBufLength> iSendBuffer;

    /**
     * Conversion limit for upper case conversion.
     * This is needed to convert the base part of special commands to upper case
     * where the part after the base part should not be converted to upper case.
     */
    TInt iLimit;

    };

/**
 *  Class used for AT command decoding related functionality
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( TDunDecodeInfo )
    {

public:

    /**
     * Flag to indicate if first decode
     */
    TBool iFirstDecode;

    /**
     * Index in iInputBuffer for decoding to iDecodeBuffer
     */
    TInt iDecodeIndex;

    /**
     * Index in iInputBuffer for extended character position 
     */
    TInt iExtendedIndex;

    /**
     * Previous character in parsing
     */
    TChar iPrevChar;

    /**
     * Flag to indicate if previous character exists
     */
    TBool iPrevExists;

    /**
     * Flag to indicate if assignment mark found
     */
    TBool iAssignFound;

    /**
     * Flag to indicate if processing inside quotes 
     */
    TBool iInQuotes;

    /**
     * Flag to indicate if special subcommand found
     */
    TBool iSpecialFound;
    
    /**
     * Buffer for parsing
     */
    TBuf8<KDunInputBufLength> iDecodeBuffer;

    };

/**
 *  Class used for AT command editor mode related functionality
 *
 *  @lib dunatext.lib
 *  @since TB9.2
 */
NONSHARABLE_CLASS( TDunEditorModeInfo )
    {

public:

    /**
     * Flag to indicate if content found (not used if iContentFindStarted is EFalse)
     */
    TBool iContentFound;

    /**
     * AT command decoding related information for peeked data
     * (not to be used if HandleNextDecodedCommand() returns EFalse)
     */
    TDunDecodeInfo iPeekInfo;

    };

/**
 *  Notification interface class for command mode start/end
 *
 *  @lib dunutils.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunCmdModeMonitor )
    {

public:

    /**
     * Notifies about command mode start
     *
     * @since S60 5.0
     * @return None
     */
    virtual void NotifyCommandModeStart() = 0;

    /**
     * Notifies about command mode end
     *
     * @since S60 5.0
     * @return None
     */
    virtual void NotifyCommandModeEnd() = 0;

    };

/**
 *  Notification interface class for status changes in AT command handling
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MDunAtCmdStatusReporter )
    {

public:

    /**
     * Notifies about AT command handling start
     *
     * @since S60 5.0
     * @return None
     */
    virtual void NotifyAtCmdHandlingStart() = 0;

    /**
     * Notifies about AT command handling end
     *
     * @since S60 5.0
     * @param aEndIndex Index to the start of next command
     * @return None
     */
    virtual void NotifyAtCmdHandlingEnd( TInt aStartIndex ) = 0;

    /**
     * Notifies about editor mode reply
     *
     * @since TB9.2
     * @param aStart ETrue if start of editor mode, EFalse otherwise
     * @return None
     */
    virtual void NotifyEditorModeReply( TBool aStart ) = 0;

    };

/**
 *  Class for AT command handler and notifier
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunAtCmdHandler ) : public CBase,
                                        public MDunAtCmdPusher,
                                        public MDunAtEcomListen,
                                        public MDunAtModeListen
    {

public:

    /**
     * Two-phased constructor.
     * @param aUpstream Callback to upstream
     * @param aDownstream Callback to downstream
     * @param aConnectionName Connection identifier name
     * @return Instance of self
     */
	IMPORT_C static CDunAtCmdHandler* NewL(
	    MDunAtCmdStatusReporter* aUpstream,
	    MDunStreamManipulator* aDownstream,
	    const TDesC8* aConnectionName );

    /**
    * Destructor.
    */
    virtual ~CDunAtCmdHandler();

    /**
     * Resets data to initial values
     *
     * @since S60 5.0
     * @return None
     */
    IMPORT_C void ResetData();

    /**
     * Adds callback for command mode notification
     * The callback will be called when command mode starts or ends
     *
     * @since S60 5.0
     * @param aCallback Callback to call when command mode starts or ends
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt AddCmdModeCallback( MDunCmdModeMonitor* aCallback );

    /**
     * Parses an AT command
     *
     * @since S60 5.0
     * @param aCommand Command to parse
     * @param aPartialInput ETrue if partial input, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt ParseCommand( TDesC8& aCommand, TBool& aPartialInput );

    /**
     * Manages request to abort command handling
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt ManageAbortRequest();

    /**
     * Sets end of command line marker on for the possible series of AT
     * commands.
     *
     * @since S60 5.0
     * @param aClearInput ETrue to clear input buffer, EFalse otherwise
     * @return None
     */
    IMPORT_C void SetEndOfCmdLine( TBool aClearInput );

    /**
     * Sends a character to be echoed
     *
     * @since TB9.2
     * @param aInput Input to echo
     * @param aCallback Callback to echo request completions
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt SendEchoCharacter( const TDesC8* aInput,
                                     MDunAtCmdEchoer* aCallback );

    /**
     * Stops sending of AT command from decode buffer
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt Stop();

    /**
     * Starts URC message handling
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt StartUrc();

    /**
     * Stops URC message handling
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C TInt StopUrc();

private:

    CDunAtCmdHandler( MDunAtCmdStatusReporter* aUpstream,
                      MDunStreamManipulator* aDownstream,
                      const TDesC8* aConnectionName );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Creates plugin handlers for this class
     *
     * @since S60 5.0
     * @return None
     */
    void CreatePluginHandlersL();

    /**
     * Creates the array of special commands
     *
     * @since S60 5.0
     * @return None
     */
    void CreateSpecialCommandsL();

    /**
     * Recreates special command data.
     * This is done when a plugin is installed or uninstalled.
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt RecreateSpecialCommands();

    /**
     * Gets default settings from RATExtCommon and sets them to RATExt
     *
     * @since S60 5.0
     * @return None
     */
    void GetAndSetDefaultSettingsL();

    /**
     * Regenerates the reply strings based on settings
     *
     * @since S60 5.0
     * @return ETrue if quiet mode, EFalse otherwise
     */
    TBool RegenerateReplyStrings();

    /**
     * Regenerates the ok reply based on settings
     *
     * @since S60 5.0
     * @return ETrue if quiet mode, EFalse otherwise
     */
    TBool RegenerateOkReply();

    /**
     * Regenerates the error reply based on settings
     *
     * @since S60 5.0
     * @return ETrue if quiet mode, EFalse otherwise
     */
    TBool RegenerateErrorReply();

    /**
     * Gets current mode
     *
     * @since S60 5.0
     * @param aMask Mask for current mode (only one supported)
     * @return New current mode
     */
    TUint GetCurrentModeL( TUint aMask );

    /**
     * Instantiates one URC message handling class instance and adds it to
     * the URC message handler array
     *
     * @since S60 3.2
     * @return None
     */
    CDunAtUrcHandler* AddOneUrcHandlerL();

    /**
     * Deletes all instantiated URC message handlers
     *
     * @since S60 5.0
     * @return None
     */
    void DeletePluginHandlers();

    /**
     * Manages partial AT command
     *
     * @since S60 5.0
     * @param aCommand Command to process
     * @param aNeedsCarriage ETrue if full and non-consumed AT command needs
     *                       carriage return (AT command "A/")
     * @return ETrue if no other processing needed, EFalse otherwise
     */
    TBool ManagePartialCommand( TDesC8& aCommand,
                                TBool& aNeedsCarriage );

    /**
     * Echoes a command if echo is on
     *
     * @since S60 5.0
     * @param aDes String descriptor
     * @return ETrue if echo push started, EFalse otherwise
     */
    TBool EchoCommand( TDesC8& aDes );

    /**
     * Handles backspace and cancel characters
     *
     * @since S60 5.0
     * @param aCommand Command to process
     * @return ETrue if special character found, EFalse otherwise
     */
    TBool HandleSpecialCharacters( TDesC8& aCommand );

    /**
     * Appends command to input buffer
     *
     * @since S60 5.0
     * @param aCommand Command to append to input buffer
     * @param aEndFound ETrue if end (carriage return) was found
     * @return ETrue if overflow was found, EFalse otherwise
     */
    TBool AppendCommandToInputBuffer( TDesC8& aCommand, TBool& aEndFound );

    /**
     * Handles next decoded command from input buffer
     *
     * @since S60 5.0
     * @return ETrue if last command decoded, EFalse otherwise
     */
    TBool HandleNextDecodedCommand();

    /**
     * Finds the start of the next command
     *
     * @since TB9.2
     * @return Index to the next command or Symbian error code on error
     */
    TInt FindStartOfNextCommand();

    /**
     * Manages end of AT command handling
     *
     * @since S60 5.0
     * @param aNotifyExternal Notify external parties
     * @param aNotifyLocal Notify local parties
     * @param aClearInput ETrue to clear input buffer, EFalse otherwise
     * @return None
     */
    void ManageEndOfCmdHandling( TBool aNotifyExternal,
                                 TBool aNotifyLocal,
                                 TBool aClearInput );

    /**
     * Extracts next decoded command from input buffer to decode buffer
     *
     * @since S60 5.0
     * @param aPeek Peek for the next command if ETrue, EFalse otherwise
     * @return ETrue if command extracted, EFalse otherwise
     */
    TBool ExtractNextDecodedCommand( TBool aPeek=EFalse );

    /**
     * Restores old decode info. For ExtractNextDecodedCommand() when aPeeks is
     * ETrue.
     *
     * @since S60 5.0
     * @param aPeek Peek for the next command if ETrue, EFalse otherwise
     * @param aOldInfo Old information to restore when aPeek is ETrue
     * @return None
     */
    void RestoreOldDecodeInfo( TBool aPeek, TDunDecodeInfo& aOldInfo );

    /**
     * Finds end of an AT command
     *
     * @since S60 5.0
     * @param aDes String descriptor
     * @param aStartIndex Start index for search
     * @return Index if found, KErrNotFound otherwise
     */
    TInt FindEndOfCommand( TDesC8& aDes, TInt aStartIndex=0 );

    /**
     * Tests for end of AT command character
     *
     * @since S60 5.0
     * @param aCharacter Character to test
     * @return ETrue if end of command, EFalse otherwise
     */
    TBool IsEndOfCommand( TChar& aCharacter );

    /**
     * Finds start of a decoded AT command
     *
     * @since S60 5.0
     * @param aDes String descriptor
     * @param aStartIndex Start index for search
     * @return Index if found, KErrNotFound otherwise
     */
    TInt FindStartOfDecodedCommand( TDesC8& aDes,
                                    TInt aStartIndex );

    /**
     * Checks if character is delimiter character
     *
     * @since S60 5.0
     * @param aCharacter Character to test
     * @return ETrue if delimiter character, EFalse otherwise
     */
    TBool IsDelimiterCharacter( TChar aCharacter );

    /**
     * Checks if character is of extended group
     *
     * @since S60 5.0
     * @param aCharacter Character to test
     * @return ETrue if extended character, EFalse otherwise
     */
    TBool IsExtendedCharacter( TChar aCharacter );

    /**
     * Checks special command
     *
     * @since S60 5.0
     * @param aStartIndex Start index (doesn't change)
     * @param aEndIndex End index (changes)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TBool CheckSpecialCommand( TInt aStartIndex,
                               TInt& aEndIndex );

    /**
     * Saves character decode state for a found character
     *
     * @since TB9.2
     * @param aCharacter Character to save a state for
     * @param aAddSpecial ETrue to add character for special command,
     *                    EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    void SaveFoundCharDecodeState( TChar aCharacter,
                                   TBool aAddSpecial=ETrue );
    
    /**
     * Saves character decode state for a not found character
     *
     * @since TB9.2
     * @param aStartIndex Start index (doesn't change)
     * @param aEndIndex End index (changes)
     * @return Symbian error code on error, KErrNone otherwise
     */
    void SaveNotFoundCharDecodeState();

    /**
     * Find quotes within subcommands
     *
     * @since TB9.2
     * @param aCharacter Character to check
     * @param aStartIndex Start index (doesn't change)
     * @param aEndIndex End index (changes)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TBool FindSubCommandQuotes( TChar aCharacter, TInt aStartIndex, TInt& aEndIndex );

    /**
     * Check if in next subcommand's extended border
     *
     * @since TB9.2
     * @param aCharacter Extended character to check
     * @param aStartIndex Start index (doesn't change)
     * @param aEndIndex End index (changes)
     * @return ETrue if in next command's extended border, EFalse otherwise
     */
    TBool IsExtendedBorder( TChar aCharacter, TInt aStartIndex, TInt& aEndIndex );
    
    /**
     * Finds subcommand with alphanumeric borders
     *
     * @since TB9.2
     * @param aCharacter Character to check
     * @param aEndIndex End index (changes)
     * @return ETrue if alpha border found, EFalse otherwise
     */
    TBool FindSubCommandAlphaBorder( TChar aCharacter, TInt& aEndIndex );

    /**
     * Finds subcommand
     *
     * @since TB9.2
     * @param aStartIndex Start index (doesn't change)
     * @param aEndIndex End index (changes)
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt FindSubCommand( TInt aStartIndex, TInt& aEndIndex );

    /**
     * Check if "A/" command
     *
     * @since S60 5.0
     * @return ETrue if "A/" command, EFalse otherwise
     */
    TBool IsASlashCommand();

    /**
     * Handles "A/" command
     *
     * @since S60 5.0
     * @return ETrue if error reply push started, EFalse otherwise
     */
    TBool HandleASlashCommand();

    /**
     * Resets parse buffers
     *
     * @since S60 5.0
     * @param aClearInput ETrue to clear input buffer, EFalse otherwise
     * @return None
     */
    void ResetParseBuffers( TBool aClearInput=ETrue );

    /**
     * Manages command mode change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return ETrue if command mode change detected, EFalse otherwise
     */
    TBool ManageCommandModeChange( TUint aMode );

    /**
     * Reports command mode start/end change
     *
     * @since S60 3.2
     * @param aStart Command mode start if ETrue, end otherwise
     * @return None
     */
    void ReportCommandModeChange( TBool aStart );

    /**
     * Manages echo mode change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return ETrue if echo mode change detected, EFalse otherwise
     */
    TBool ManageEchoModeChange( TUint aMode );

    /**
     * Manages quiet mode change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return ETrue if quiet mode change detected, EFalse otherwise
     */
    TBool ManageQuietModeChange( TUint aMode );

    /**
     * Manages verbose mode change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return ETrue if verbose mode change detected, EFalse otherwise
     */
    TBool ManageVerboseModeChange( TUint aMode );

    /**
     * Manages character change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return None
     */
    void ManageCharacterChange( TUint aMode );

    /**
     * Manages editor mode reply
     *
     * @since TB9.2
     * @param aStart ETrue if start of editor mode, EFalse otherwise
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ManageEditorModeReply( TBool aStart );

    /**
     * Finds the next content from the input buffer
     *
     * @since TB9.2
     * @param aStart ETrue if start of editor mode, EFalse otherwise
     * @return ETrue if next content found, EFalse otherwise
     */
    TBool FindNextContent( TBool aStart );

// from base class MDunAtCmdPusher

    /**
     * From MDunAtCmdPusher.
     * Notifies about end of AT command processing.
     * This is after all reply data for an AT command is multiplexed to the
     * downstream.
     *
     * @since S60 5.0
     * @param aError Error code of command processing completion
     * @return None
     */
    TInt NotifyEndOfProcessing( TInt aError );

    /**
     * Notifies about request to stop AT command handling for the rest of the
     * command line data
     *
     * @since S60 5.0
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyEndOfCmdLineProcessing();

    /**
     * Notifies about request to peek for the next command
     *
     * @since S60 5.0
     * @return ETrue if next command exists, EFalse otherwise
     */
    TBool NotifyNextCommandPeekRequest();

    /**
     * Notifies about editor mode reply
     *
     * @since TB9.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyEditorModeReply();

// from base class MDunAtCmdEchoer

    /**
     * Notifies about completed echo in text mode
     *
     * @since TB9.2
     * @return None
     */
    void NotifyEchoComplete();

// from base class MDunAtEcomListen

    /**
     * From MDunAtEcomListen.
     * Notifies about new plugin installation
     *
     * @since S60 5.0
     * @return None
     */
    TInt NotifyPluginInstallation( TUid& aPluginUid );

    /**
     * From MDunAtEcomListen.
     * Notifies about existing plugin uninstallation
     *
     * @since S60 5.0
     * @return None
     */
    TInt NotifyPluginUninstallation( TUid& aPluginUid );

// from base class MDunAtModeListen

    /**
     * From MDunAtModeListen.
     * Gets called on mode status change
     *
     * @since S60 5.0
     * @param aMode Mode to manage
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt NotifyModeStatusChange( TUint aMode );

private:  // data

    /**
     * Callback to call when AT command handling status changes
     * Not own.
     */
    MDunAtCmdStatusReporter* iUpstream;

    /**
     * Callback to call when data to push
     * Not own.
     */
    MDunStreamManipulator* iDownstream;

    /**
     * Callback(s) to call when command mode starts or ends
     * Usually two needed: one for upstream and second for downstream
     */
    RPointerArray<MDunCmdModeMonitor> iCmdCallbacks;

    /**
     * Connection identifier name
     */
    const TDesC8* iConnectionName;

    /**
     * Current state of AT command handling: active or inactive
     */
    TDunState iHandleState;

    /**
     * Character for carriage return
     */
    TInt8 iCarriageReturn;

    /**
     * Character for line feed
     */
    TInt8 iLineFeed;

    /**
     * Character for backspace
     */
    TInt8 iBackspace;

    /**
     * Current command to ParseCommand()
     * Not own.
     */
    TDesC8* iCommand;

    /**
     * Special commands for parsing
     */
    RPointerArray<HBufC8> iSpecials;

    /**
     * Buffer for character echoing
     */
    TBuf8<KDunChSetMaxCharLen> iEchoBuffer;

    /**
     * Buffer for ok reply
     */
    TBuf8<KDunOkBufLength> iOkBuffer;

    /**
     * Buffer for error reply
     */
    TBuf8<KDunErrorBufLength> iErrorBuffer;

    /**
     * Buffer for AT command input
     */
    TBuf8<KDunInputBufLength> iInputBuffer;

    /**
     * Buffer for last AT command input (for "A/")
     */
    TBuf8<KDunInputBufLength> iLastBuffer;

    /**
     * Buffer for <ESC> command
     */
    TBuf8<KDunEscBufLength> iEscapeBuffer;

    /**
     * AT command decoding related information
     */
    TDunDecodeInfo iDecodeInfo;

    /**
     * Information for parsing
     */
    TDunParseInfo iParseInfo;

    /**
     * Information for editor mode
     */
    TDunEditorModeInfo iEditorModeInfo;

    /**
     * AT command reply pusher
     * Own.
     */
    CDunAtCmdPusher* iCmdPusher;

    /**
     * AT command reply echoer
     * Own.
     */
    CDunAtCmdEchoer* iCmdEchoer;

    /**
     * URC message handlers
     * Own.
     */
    RPointerArray<CDunAtUrcHandler> iUrcHandlers;

    /**
     * ECOM plugin interface status change listener
     * Own.
     */
    CDunAtEcomListen* iEcomListen;

    /**
     * Modem mode status change listener
     * Own.
     */
    CDunAtModeListen* iModeListen;

    /**
     * NVRAM status change listener
     * Own.
     */
    CDunAtNvramListen* iNvramListen;

    /**
     * Flag to mark command mode start/end
     */
    TBool iDataMode;

    /**
     * Flag to be set on if echo on
     */
    TBool iEchoOn;

    /**
     * Flag to be set on if quiet mode on
     */
    TBool iQuietOn;

    /**
     * Flag to be set on if verbose mode on
     */
    TBool iVerboseOn;

    /**
     * End index for command delimiter
     */
    TInt iEndIndex;

    /**
     * AT command extension
     */
    RATExt iAtCmdExt;

    /**
     * AT command extension to common functionality
     */
    RATExtCommon iAtCmdExtCommon;

    /**
     * Special AT command handler for handling commands like AT&FE0Q0V1&C1&D2+IFC=3,1.
     */
    CDunAtSpecialCmdHandler* iAtSpecialCmdHandler;
    };

#endif  // C_CDUNATCMDHANDLER_H
