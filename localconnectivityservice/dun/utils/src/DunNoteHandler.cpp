/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Manages note showing in UI
*
*/


#include <bautils.h>
#include <featmgr.h>
#include <aknSDData.h>
#include <secondarydisplay/dunsecondarydisplayapi.h>
#include "DunNoteHandler.h"
#include "DunDebug.h"

_LIT( KDunUtilsDriveSpec, "z:" );
_LIT( KDunUtilsResourceFileName, "dunutils.rsc" );

const TInt KDunCoverEnumStart     = (ECmdNone + 1);  // start after ECmdNone
const TInt KDunPtr8toPtr16Divider = 2;               // Divider for converting
const TInt KDunThreeItemsToPop    = 3;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunNoteHandler* CDunNoteHandler::NewL()
    {
    CDunNoteHandler* self = new (ELeave) CDunNoteHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDunNoteHandler::~CDunNoteHandler()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::~CDunNoteHandler()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunNoteHandler::~CDunNoteHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::ResetData()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    delete iNote;
    iNote = NULL;
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunNoteHandler::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Issues request to start showing UI note
// ---------------------------------------------------------------------------
//
TInt CDunNoteHandler::IssueRequest()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest()") ));
    if ( iNoteState != EDunStateIdle )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() (not ready) complete") ));
        return KErrNotReady;
        }
    TRAPD( retTrap, DoIssueRequestL() );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() (trapped!) complete (%d)"), retTrap));
        return retTrap;
        }
    SetActive();
    iNoteState = EDunStateUiNoting;
    FTRACE(FPrint( _L("CDunNoteHandler::IssueRequest() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops showing UI note
// ---------------------------------------------------------------------------
//
TInt CDunNoteHandler::Stop()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::Stop()") ));
    if ( iNoteState != EDunStateUiNoting )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::Stop() (not ready) complete") ));
        return KErrNotReady;
        }
    if ( !iNote )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::Stop() (iNote not initialized!) complete") ));
        return KErrGeneral;
        }
    iNote->CancelConfirmationQuery();
    Cancel();
    iNoteState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunNoteHandler::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunNoteHandler::CDunNoteHandler
// ---------------------------------------------------------------------------
//
CDunNoteHandler::CDunNoteHandler() :
    CActive( EPriorityStandard )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunNoteHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::ConstructL()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::ConstructL()") ));
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunNoteHandler::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::Initialize()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::Initialize()" ) ));
    iNote = NULL;
    iNoteState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunNoteHandler::Initialize() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Issues request to start showing UI note
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::DoIssueRequestL()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL()") ));
    if ( iNote )
        {
        FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL() (ERROR) complete") ));
        User::Leave( KErrGeneral );
        }
    HBufC16* unicodeString = NULL;
    ReadResourceTextL( R_DUN_MAXIMUM_DIALUPS, unicodeString );
    CAknGlobalConfirmationQuery* note = CAknGlobalConfirmationQuery::NewLC();
    // Publish cover UI note data
    CAknSDData* sdData = CAknSDData::NewL( KDunNoteCategory,
                                           ECmdMaxNumber - KDunCoverEnumStart,
                                           KNullDesC8 );
    note->SetSecondaryDisplayData( sdData );  // ownership transferred
    // Start to show note
    iStatus = KRequestPending;
    note->ShowConfirmationQueryL( iStatus,
                                  *unicodeString,
                                  R_AVKON_SOFTKEYS_OK_EMPTY,
                                  R_QGN_NOTE_ERROR_ANIM,
                                  KNullDesC,
                                  0,
                                  0,
                                  CAknQueryDialog::EErrorTone );
    CleanupStack::Pop( note );
    delete unicodeString;
    iNote = note;
    FTRACE(FPrint( _L("CDunNoteHandler::DoIssueRequestL() complete") ));
    }

// ---------------------------------------------------------------------------
// Reads resource string
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::ReadResourceTextL( TInt aResourceId, HBufC16*& aUnicode )
    {
    FTRACE(FPrint( _L("CDunNoteHandler::ReadNoteResourceL()") ));
    // Connect to file server (for resource file reading)
    RFs fileSession;
    CleanupClosePushL<RFs>( fileSession );
    User::LeaveIfError( fileSession.Connect() );
    // Create dunutils.rsc path and file name
    TFileName fileName;
    fileName = KDunUtilsDriveSpec;
    fileName += KDC_RESOURCE_FILES_DIR;
    fileName += KDunUtilsResourceFileName;
    // Find nearest language file for resource
    BaflUtils::NearestLanguageFile( fileSession, fileName );
    // Read note resource
    RResourceFile resourceFile;
    CleanupClosePushL<RResourceFile>( resourceFile );
    resourceFile.OpenL( fileSession, fileName );
    resourceFile.ConfirmSignatureL();
    HBufC8* readBuffer = resourceFile.AllocReadLC( aResourceId );
    // Convert read HBufC8 to HBufC16
    const TPtrC16 ptr16(reinterpret_cast<const TUint16*>
                       (readBuffer->Ptr()),
                       (readBuffer->Size() / KDunPtr8toPtr16Divider) );
    aUnicode = HBufC16::NewL( ptr16.Length() );
    *aUnicode = ptr16;
    CleanupStack::PopAndDestroy( KDunThreeItemsToPop );  // readBuffer, resourceFile, fileSession
    FTRACE(FPrint( _L("CDunNoteHandler::ReadNoteResourceL() complete") ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when UI note dismissed
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::RunL()
    {
    FTRACE(FPrint( _L("CDunNoteHandler::RunL()" ) ));
    iNoteState = EDunStateIdle;
    delete iNote;
    iNote = NULL;
    FTRACE(FPrint( _L("CDunNoteHandler::RunL() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunNoteHandler::DoCancel()
    {
    }
