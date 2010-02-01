// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include <c32comm.h>
#include "TestObexErrorCodes.h"

#if defined (__WINS__)
#define PDD_NAME _L("ECDRV")
#define LDD_NAME _L("ECOMM")
#else  // __GCC32__
#define PDD_NAME _L("EUART2")
#define LDD_NAME _L("ECOMM")
#define FIR_PDD_NAME _L("DIFIR")	// Brutus FIR PDD
#define FIR_LDD_NAME _L("EFIR")		// Brutus FIR LDD
#endif



void Startup()
	{
	CActiveScheduler *pA=new CActiveScheduler;
	CActiveScheduler::Install(pA);

	CControllerTestMasterClass* bigBoss = CControllerTestMasterClass::NewL();
	
	CActiveScheduler::Start();
	delete bigBoss;
	bigBoss = NULL;
	delete pA;
	}


void CControllerTestMasterClass::ConstructL()
	{
	//make the initial active object everything
	iConsoleTest = CControllerConsoleTest::NewL(this);

	iConsoleTest->IssueRequest();

	}

CControllerTestMasterClass* CControllerTestMasterClass::NewL()
	{
	CControllerTestMasterClass* self = NewLC();
	CleanupStack::Pop();
	return self;
	}

CControllerTestMasterClass* CControllerTestMasterClass::NewLC()
	{
	CControllerTestMasterClass* self=new (ELeave) CControllerTestMasterClass;
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}



CControllerTestMasterClass::~CControllerTestMasterClass()
	{
	delete iConsoleTest;
	iConsoleTest = NULL;
	}
	


GLDEF_C TInt E32Main()
	{

	__UHEAP_MARK;
	CTrapCleanup* cleanup=CTrapCleanup::New(); // get clean-up stack

	
	// start scheduler 
	TRAPD(error, Startup() );
	__ASSERT_ALWAYS(!error,User::Panic(_L("Testing Registry"),error));


	delete cleanup; // destroy clean-up stack
	__UHEAP_MARKEND;
	return KErrNone;
	}



//
// CRegistryConsoleTest
//
CControllerConsoleTest::~CControllerConsoleTest()
	{
	Cancel();


	delete iTest;
	delete iConsole;
	iConsole = NULL;
	iOwner = NULL;  

	}


void CControllerConsoleTest::DoCancel()
	{
	iConsole->ReadCancel();
	}


void CControllerConsoleTest::Menu()
	{
	iConsole->Printf(_L("\nOBEX error code tests ..."));
	iConsole->Printf(_L("\nPress '1' to start Client Side Tests"));
	iConsole->Printf(_L("\nPress '2' to start Server Side Tests"));
	iConsole->Printf(_L("\nPress '3' to start"));
	iConsole->Printf(_L("\nPress '4' to stop"));
	iConsole->Printf(_L("\nAny other key to exit"));
	}

void CControllerConsoleTest::IssueRequest()
	{
	Menu();
	iConsole->Read(iStatus);
	SetActive();
	}
void CControllerConsoleTest::ConstructL(CControllerTestMasterClass* aOwner)
    {
 
	iOwner = aOwner;
	iConsole = Console::NewL(_L("Controller Test"),
					TSize(-1,-1));
	DoTest();	//set everything up, load everything


	CActiveScheduler::Add(this); 
    }

TInt CControllerConsoleTest::RunError(TInt)
	{
	IssueRequest();
	return KErrNone;
	}


void CControllerConsoleTest::StartTest()
	{
	iTest->Start();

	iStarted = ETrue;
	
	}

void CControllerConsoleTest::StopTest()
	{

	if ( iStarted )
		{
		iTest->Stop();

		iStarted = EFalse;
		}
	else
		{
		iConsole->Printf(_L("DOOH, can't stop a test which hasn't started\n"));
		}
	}

void CControllerConsoleTest::DoTest()
	{
	iStarted = EFalse;
	iConsole->Printf(_L("\n Loading tests"));



	TInt ret;
#ifdef __EPOC32__
	iConsole->Printf(_L("Loading C32...\n"));
	ret=StartC32();
	if (ret!=KErrNone && ret!=KErrAlreadyExists)
		iConsole->Printf(_L("	Failed %d\n"), ret);
	else
		iConsole->Printf(_L("	Success\n"));
#endif

// load up the drivers
	iConsole->Printf(_L("Loading SIR PDD...\n"));
	ret=User::LoadPhysicalDevice(PDD_NAME);
	if (ret!=KErrNone && ret!=KErrAlreadyExists)
		iConsole->Printf(_L("	Failed %d\n"), ret);
	else
		iConsole->Printf(_L("	Success\n"));

	iConsole->Printf(_L("Loading SIR LDD...\n"));
    ret=User::LoadLogicalDevice(LDD_NAME);
	if (ret!=KErrNone && ret!=KErrAlreadyExists)
		iConsole->Printf(_L("	Failed %d\n"), ret);
	else
		iConsole->Printf(_L("	Success\n"));
#ifdef __EPOC32__
	iConsole->Printf(_L("Loading Fast IR PDD\n"));

	ret=User::LoadPhysicalDevice(FIR_PDD_NAME);
	if (ret!=KErrNone && ret!=KErrAlreadyExists)
		iConsole->Printf(_L("	Failed %d\n"), ret);
	else
		iConsole->Printf(_L("	Success\n"));

	iConsole->Printf(_L("Loading Fast IR LDD\n"));
	ret=User::LoadLogicalDevice(FIR_LDD_NAME);
	if (ret!=KErrNone && ret!=KErrAlreadyExists)
		iConsole->Printf(_L("	Failed %d\n"), ret);
	else
		iConsole->Printf(_L("	Success\n"));
#endif	


	}


void CControllerConsoleTest::RunL()
	{

	TInt key = iConsole->KeyCode();
	switch (key)
		{
		case '1':
			{
			iTest = CTestObexErrorCodesClient::NewL(this);
			IssueRequest(); //back to main menu
			}
			break;
		case '2':
			{
			iTest = CTestObexErrorCodesServer::NewL(this);
			IssueRequest(); //back to main menu
			}
			break;
		case '3':
			{
			if ( !iStarted )
				StartTest();
			else 
				iConsole->Printf(_L("Already Started\n"));

			IssueRequest(); //back to main menu

			}
			break;
		case '4':
			{
			if ( iStarted )
				StopTest();
			else 
				iConsole->Printf(_L("Not started yet\n"));

			IssueRequest(); //back to main menu
			}
			break;
		default:
			CActiveScheduler::Stop();
			break;
		}

	}


CControllerConsoleTest* CControllerConsoleTest::NewL(CControllerTestMasterClass* aOwner)
	{
	CControllerConsoleTest* self = NewLC(aOwner);
	CleanupStack::Pop();
	return self;
	}

CControllerConsoleTest* CControllerConsoleTest::NewLC(CControllerTestMasterClass* aOwner)
	{
	CControllerConsoleTest* self=new (ELeave) CControllerConsoleTest;
	CleanupStack::PushL(self);
	self->ConstructL(aOwner);
	return self;

	}

CControllerConsoleTest::CControllerConsoleTest()
			: CActive(CActive::EPriorityStandard)
	{
	}





CBaseErrorCodeTest* CBaseErrorCodeTest::NewL(CControllerConsoleTest* aOwner)
	{
	CBaseErrorCodeTest* self = new(ELeave) CBaseErrorCodeTest;
	CleanupStack::PushL(self);
	self->ConstructL(aOwner);
	CleanupStack::Pop( self );
	return self;
	}

CBaseErrorCodeTest::~CBaseErrorCodeTest()
	{
	delete iObject;
    delete iBuf;
    iLibrary.Close();
	}


CBaseErrorCodeTest::CBaseErrorCodeTest()
	{
	}

void CBaseErrorCodeTest::ConstructL(CControllerConsoleTest* aOwner)
	{
	iOwner = aOwner;

	iOwner->iConsole->Printf(_L("About to load the IRObex DLL...\n"));

    
    TRAPD(ret,LoadObexL());	// Load Protocol
    if (ret!=KErrNone)
		iOwner->iConsole->Printf(_L("Error %d on loading IrObex DLL\r\n"),ret);
    else
		iOwner->iConsole->Printf(_L("Successfully loaded IrObex DLL\r\n"));
	}

void CBaseErrorCodeTest::LoadObexL()
	{
	User::LeaveIfError(iLibrary.Load(_L("IROBEX.DLL")));
	}

void CBaseErrorCodeTest::Start()
	{
	}

void CBaseErrorCodeTest::Stop()
	{
	}
