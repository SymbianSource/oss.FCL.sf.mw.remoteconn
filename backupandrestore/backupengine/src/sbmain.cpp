// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Main entry point in the process
// 
//

/**
 @file
*/

#include "sbmain.h"
#include "sbeserver.h"
#include "abserver.h"
#include <connect/tserverstart.h>
#include "sbedataownermanager.h"
#include "ecom/ecom.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbmainTraces.h"
#endif

namespace conn
	{	
	void CSBActiveScheduler::Error(TInt aError) const
	/**
	Executed by the active scheduler when an error occurs. Ideally errors 
	should be handled in the RunError() method of active objects and 
	not in here. 
	
	It logs an error and stops the active scheduler, which will cause
	execution to terminate.
	
	@param aError The error code
	*/
		{
		OstTraceFunctionEntry0( CSBACTIVESCHEDULER_ERROR_ENTRY );
		RDebug::Print(_L("Secure Backup unhandled error %d"), aError);
		Stop();
		OstTraceFunctionExit0( CSBACTIVESCHEDULER_ERROR_EXIT );
		}
	}


using namespace conn;

//
// EKA1 server startup code - platform unspecific.


static void RunServerL()
	/**
	Installs active scheduler, creates the CSBEServer and CABServer objects,
	signals the client, and begins to handle client requests..

	@param aStart A TServerStart object passed in by the client
	@leave RThread::Rename() If an error is returned
	*/
	{
	OstTraceFunctionEntry0( _CONN_RUNSERVERL_ENTRY );
	//
	// Create and install an active scheduler.
    CSBActiveScheduler* pScheduler = new (ELeave) CSBActiveScheduler();
	CleanupStack::PushL(pScheduler);
	CActiveScheduler::Install(pScheduler);
	
	CDataOwnerManager* pDOM = CDataOwnerManager::NewLC();

	// create the two servers to run in this process
	CSBEServer* pSBEServer = CSBEServer::NewLC(pDOM);
    CABServer* pABServer = CABServer::NewLC(pDOM);
    
    // register the AB Server with the DOM
    pDOM->SetActiveBackupServer(pABServer);

	// Rename the thread (for debugging)
	User::RenameThread(KSBImageName);

    RProcess::Rendezvous(KErrNone);

	//
	// Start the AS.  Will return after 2 seconds
	// of inactivity on the server.
	CSBActiveScheduler::Start();
	
	TDriveList driveList;
	TBURPartType burPartType = EBURUnset;
	TBackupIncType buIncType = ENoBackup;
	driveList.FillZ();
	
	// Ensure that the device is put back into normal mode, in case a disconnection has ocurred
	pDOM->SetBURModeL(driveList, burPartType, buIncType);

	CleanupStack::PopAndDestroy(pABServer);
	CleanupStack::PopAndDestroy(pSBEServer);
	CleanupStack::PopAndDestroy(pDOM);
	CleanupStack::PopAndDestroy(pScheduler);
	OstTraceFunctionExit0( _CONN_RUNSERVERL_EXIT );
	}


static TInt RunServer()
/**
Creates cleanup framework and call leaving variant of RunServer

@param aStart A TServerStart object 
@return Any error trapped by RunServerL
*/
	{
	OstTraceFunctionEntry0( _CONN_RUNSERVER_ENTRY );
	__UHEAP_MARK;
	CTrapCleanup* pCleanup = CTrapCleanup::New();
	TInt nRet = KErrNoMemory;

	if(pCleanup != NULL)
		{
#ifdef __OOM_TESTING__
	    TInt i = 0;
	    while (nRet == KErrNoMemory || nRet == KErrNone)
		    {
		    __UHEAP_SETFAIL(RHeap::EDeterministic,i++);
		    __UHEAP_MARK;

		    TRAP(nRet, RunServerL());
		    // required because we have an indirect
		    // dependency on ecom, we apparently need
		    // need to clean up after our dependencies
		    REComSession::FinalClose();

		    __UHEAP_MARKEND;
		    __UHEAP_RESET;
		    }
#else
		TRAP(nRet, RunServerL());
#endif			
	    delete pCleanup;
	    }

    // required because we have an indirect
    // dependency on ecom, we apparently need
    // need to clean up after our dependencies
    REComSession::FinalClose();

    __UHEAP_MARKEND;
    OstTraceFunctionExit0( _CONN_RUNSERVER_EXIT );
    return nRet;
    }




  
TInt E32Main()
/**
Standard Symbian OS entry point

@return Symbian OS internal value.
*/
	{
	OstTraceFunctionEntry0( _E32MAIN_ENTRY );
	TInt nRet = KErrNone;
	
    nRet = RunServer();
    
	OstTraceFunctionExit0( _E32MAIN_EXIT );
	return nRet;
	}


