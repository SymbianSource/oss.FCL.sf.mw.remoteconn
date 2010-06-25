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
* Description:  Special AT command handler
*
*/

#include "DunAtSpecialCmdHandler.h"
#include "DunDebug.h"

const TInt KDefaultGranularity = 1;

// AT command(s) below is part of the AT&FE0Q0V1&C1&D2+IFC=3,1. command which
// is sent by MAC. There is no delimiter between "AT&F" and "E0".
// Only list those commands where alphabetical boundary detection is needed
// (i.e. "AT&F0" is not needed as "AT&F0E0" has non-alphabetical boundary)
_LIT8( KSpecialATCmd1, "AT&F" );

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler* CDunAtSpecialCmdHandler::NewL()
    {
    CDunAtSpecialCmdHandler* self = new (ELeave) CDunAtSpecialCmdHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CDunAtSpecialCmdHandler::CDunAtSpecialCmdHandler
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler::CDunAtSpecialCmdHandler()
    {
    }

// ---------------------------------------------------------------------------
// CDunAtSpecialCmdHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtSpecialCmdHandler::ConstructL()
    {
    iSpecialCmds = new (ELeave) CDesC8ArrayFlat( KDefaultGranularity );
    // Add here all special commands which need to be handled
    iSpecialCmds->AppendL( KSpecialATCmd1 );
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler()
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler()") ));
    delete iSpecialCmds;
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Checks if the command has to be treated special way.
// For example in case of MAC, it sends command AT&FE0Q0V1&C1&D2+IFC=3,1.
// meaning there is no delimiters in the command.
// In case of MAC we try to search AT&F (sub command) string from the beginning
// of the command.
// Search is done character by character basis.
// ---------------------------------------------------------------------------
//
TBool CDunAtSpecialCmdHandler::IsCompleteSubCommand( TChar aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand()") ));
    iBuffer.Append( aCharacter );
    TBool completeSubCmd = EFalse;

    if( !IsDataReadyForComparison(iBuffer.Length()) )
        {
        // No need to do comparison because we don't have correct amount of data
        FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), no need to compare") ));
        return completeSubCmd;
        }

    TInt count = iSpecialCmds->Count();
    for ( TInt i=0; i<count; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Compare(iBuffer) == 0 )
            {
            FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), match found, cmd index %d"), i ));
            // Reset internal buffer for next comparison.
            ResetComparisonBuffer();
            completeSubCmd = ETrue;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand() complete") ));
    return completeSubCmd;
    }

// ---------------------------------------------------------------------------
// Resets the buffer used for comparisons
// ---------------------------------------------------------------------------
//
void CDunAtSpecialCmdHandler::ResetComparisonBuffer()
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::ResetComparisonBuffer()") ));
    iBuffer.FillZ();
    iBuffer.Zero();
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::ResetComparisonBuffer() complete") ));
    }

// ---------------------------------------------------------------------------
// Defines when comparison is excecuted, checks if the data lengths are equal.
// ---------------------------------------------------------------------------
//
TBool CDunAtSpecialCmdHandler::IsDataReadyForComparison( TInt aLength )
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsDataReadyForComparison()") ));
    TInt count = iSpecialCmds->Count();
    for ( TInt i=0; i<count; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Length() == aLength )
            {
            FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsDataReadyForComparison() (ready) complete") ));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsDataReadyForComparison() (not ready) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Defines minimum length of the special commands.
// ---------------------------------------------------------------------------
//
TInt CDunAtSpecialCmdHandler::MinimumLength()
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::MinimumLength()") ));
    TInt length = iSpecialCmds->MdcaPoint(0).Length();
    TInt count = iSpecialCmds->Count();
    for ( TInt i=1; i<count; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Length() < length )
            {
            length = iSpecialCmds->MdcaPoint(i).Length();
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::MinimumLength() complete") ));
    return length;
    }
