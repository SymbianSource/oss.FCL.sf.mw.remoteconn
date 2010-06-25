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
* Description:
*
*/

#include "dunutilstest.h"	// Cdunutilstest

//  Member Functions


Cdunutilstest* Cdunutilstest::NewL(CTestModuleIf& aTestModuleIf)
    {
    Cdunutilstest* self = new (ELeave) Cdunutilstest( aTestModuleIf );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;

    }

Cdunutilstest::Cdunutilstest(CTestModuleIf& aTestModuleIf ):
        CScriptBase( aTestModuleIf )
    {
    }

void Cdunutilstest::ConstructL()
    {
    // second phase constructor, anything that may leave must be constructed here
    mDunandler = CDunNoteHandler::NewL();
    
    }

Cdunutilstest::~Cdunutilstest()
    {
    delete mDunandler;
    }

TInt Cdunutilstest::RunMethodL( CStifItemParser& aItem )
    {
    
    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "LaunchDialog", Cdunutilstest::LaunchDialogL )
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

TInt Cdunutilstest::LaunchDialogL( CStifItemParser& /*aItem*/ )
    {
  
    TTimeIntervalMicroSeconds32 time(7000000); //7 Seconds
    
    mDunandler->IssueRequest();
    User::After(time);
    return KErrNone;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// LibEntryL is a polymorphic Dll entry point.
// Returns: CScriptBase: New CScriptBase derived object
// -----------------------------------------------------------------------------
//
EXPORT_C CScriptBase* LibEntryL( 
    CTestModuleIf& aTestModuleIf ) // Backpointer to STIF Test Framework
    {

    return ( CScriptBase* ) Cdunutilstest::NewL( aTestModuleIf );

    }


