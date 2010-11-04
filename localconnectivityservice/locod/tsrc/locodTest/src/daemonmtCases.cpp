/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ?Description
*
*/



// INCLUDE FILES
#include <e32math.h>
#include <StartupDomainPSKeys.h>
#include "daemonmt.h"



// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// Cdaemonmt::Case
// Returns a test case by number.
//
// This function contains an array of all available test cases 
// i.e pair of case name and test function. If case specified by parameter
// aCaseNumber is found from array, then that item is returned.
// 
// The reason for this rather complicated function is to specify all the
// test cases only in one place. It is not necessary to understand how
// function pointers to class member functions works when adding new test
// cases. See function body for instructions how to add new test case.
// -----------------------------------------------------------------------------
//
const TCaseInfo Cdaemonmt::Case ( 
    const TInt aCaseNumber ) const 
     {

    /**
    * To add new test cases, implement new test case function and add new 
    * line to KCases array specify the name of the case and the function 
    * doing the test case
    * In practice, do following
    * 1) Make copy of existing test case function and change its name
    *    and functionality. Note that the function must be added to 
    *    daemonmt.cpp file and to daemonmt.h 
    *    header file.
    *
    * 2) Add entry to following KCases array either by using:
    *
    * 2.1: FUNCENTRY or ENTRY macro
    * ENTRY macro takes two parameters: test case name and test case 
    * function name.
    *
    * FUNCENTRY macro takes only test case function name as a parameter and
    * uses that as a test case name and test case function name.
    *
    * Or
    *
    * 2.2: OOM_FUNCENTRY or OOM_ENTRY macro. Note that these macros are used
    * only with OOM (Out-Of-Memory) testing!
    *
    * OOM_ENTRY macro takes five parameters: test case name, test case 
    * function name, TBool which specifies is method supposed to be run using
    * OOM conditions, TInt value for first heap memory allocation failure and 
    * TInt value for last heap memory allocation failure.
    * 
    * OOM_FUNCENTRY macro takes test case function name as a parameter and uses
    * that as a test case name, TBool which specifies is method supposed to be
    * run using OOM conditions, TInt value for first heap memory allocation 
    * failure and TInt value for last heap memory allocation failure. 
    */ 

    static TCaseInfoInternal const KCases[] =
        {
        // To add new test cases, add new items to this array
        ENTRY( "StartDaemon test", Cdaemonmt::StartDaemon )
        };

    // Verify that case number is valid
    if( (TUint) aCaseNumber >= sizeof( KCases ) / 
                               sizeof( TCaseInfoInternal ) )
        {
        // Invalid case, construct empty object
        TCaseInfo null( (const TText*) L"" );
        null.iMethod = NULL;
        null.iIsOOMTest = EFalse;
        null.iFirstMemoryAllocation = 0;
        null.iLastMemoryAllocation = 0;
        return null;
        } 

    // Construct TCaseInfo object and return it
    TCaseInfo tmp ( KCases[ aCaseNumber ].iCaseName );
    tmp.iMethod = KCases[ aCaseNumber ].iMethod;
    tmp.iIsOOMTest = KCases[ aCaseNumber ].iIsOOMTest;
    tmp.iFirstMemoryAllocation = KCases[ aCaseNumber ].iFirstMemoryAllocation;
    tmp.iLastMemoryAllocation = KCases[ aCaseNumber ].iLastMemoryAllocation;
    return tmp;
    }


TInt Cdaemonmt::StartDaemon( TTestResult& aResult )
	{
	iBtSettings = CBtSettings::NewL();
	iLog->Log( _L( "Starting test and setting BT Power ON" ) );
	TInt error = iBtSettings->SetPowerState( EBTPowerOn );
	if( error )
		{
		iLog->Log( _L( "Error %d when setting BT power state" ), error );
		aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
		return KErrNone;
		}
    
    TInt systemState = KErrNone;
    error = iProperty.Get( KPSUidStartup, KPSGlobalSystemState, systemState );
    if( error )
    	{
    	iLog->Log( _L( "Failed to get GlobalSystemState" ) );
    	}
    
    // Switching GlobalSystemState to ESwStateShuttingDown causes locod
    // not to load bearer plugins immediately after its starts.
    iLog->Log( _L( "Changing GlobalSystemState to ESwStateShuttingDown" ) );
    error = iProperty.Set( KPSUidStartup, KPSGlobalSystemState, ESwStateShuttingDown );
    if( error )
    	{
    	iLog->Log( _L( "Failed to switch GlobalSystemState to ESwStateShuttingDown with error: %d" ), error );
    	aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
    	return KErrNone;
    	}

    _LIT(KDaemonExe, "locod.exe");
    // Starts Daemon in a new process.
    const TUidType exeUid(KNullUid, KNullUid, TUid::Uid(0x2000276D));
    RProcess daemon;
    TInt r = daemon.Create(KDaemonExe, KNullDesC, exeUid);
    TBuf<256> err;
    if (r != KErrNone)
    	{
    	_LIT( KDescription, "StartDaemon failed: daemon.Create() %d" );
    	aResult.SetResult( r, KDescription );    
    	err.Format( KDescription, r);
    	iLog->Log( err );
    	return r;
    	}
    daemon.Resume();

    // Switching GlobalSystemState to ESwStateNormalRfOn causes locod
    // to load bearer plugins.
    iLog->Log( _L( "Changing GlobalSystemState to ESwStateNormalRfOn" ) );
    error = iProperty.Set( KPSUidStartup, KPSGlobalSystemState, ESwStateNormalRfOn );
    if( error )
    	{
    	iLog->Log( _L( "Failed to switch GlobalSystemState to ESwStateNormalRfOn with error: %d" ), error );
    	aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
    	return KErrNone;
    	}

    User::After( 2000000 );

    // Switching GlobalSystemState to ESwStateShuttingDown causes locod
    // to unload bearer plugins.
    iLog->Log( _L( "Changing GlobalSystemState to ESwStateShuttingDown" ) );
    error = iProperty.Set( KPSUidStartup, KPSGlobalSystemState, ESwStateShuttingDown );
    if( error )
    	{
    	iLog->Log( _L( "Failed to switch GlobalSystemState to ESwStateShuttingDown with error: %d" ), error );
    	aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
    	return KErrNone;
    	}

    User::After( 2000000 );
    daemon.Close();

    // Switching GlobalSystemState to the state forom the start of the test to leave the LC stack
    // working after the test.
    if( systemState != KErrNone )
    	{
    	iLog->Log( _L( "Changing GlobalSystemState to %d" ), systemState );
    	error = iProperty.Set( KPSUidStartup, KPSGlobalSystemState, systemState );
    	}
    else
    	{
    	iLog->Log( _L( "Changing GlobalSystemState to ESwStateNormalRfOn" ) );
    	error = iProperty.Set( KPSUidStartup, KPSGlobalSystemState, ESwStateNormalRfOn );
    	}
    
    if( error )
    	{
    	iLog->Log( _L( "Failed to switch GlobalSystemState to ESwStateNormalRfOn with error: %d" ), error );
    	aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
    	return KErrNone;
    	}
    
    User::After( 2000000 );
    
    iLog->Log( _L( "Ending test and setting BT Power ON" ) );
    error = iBtSettings->SetPowerState( EBTPowerOn );
    if( error )
    	{
    	iLog->Log( _L( "Error %d when setting BT power state" ), error );
    	aResult.SetResult( KErrGeneral, _L( "StartDaemon failed" ) );
    	return KErrNone;
    	}

    User::After( 2000000 );
    
    delete iBtSettings;
    iBtSettings = NULL;
    
    aResult.SetResult( KErrNone, _L( "StartDaemon passed" ) );
    return KErrNone;
	}

//  End of File
