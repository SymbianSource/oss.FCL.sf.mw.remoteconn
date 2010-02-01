// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include "TOBEXCON.H"

#ifdef __EPOC32__
#define FIR_PDD_NAME _L("DIFIR")	// Brutus FIR PDD
#define FIR_LDD_NAME _L("EFIR")		// Brutus FIR LDD
#endif

#ifdef __WINS__
_LIT(KWinsPddName,"ecdrv");
_LIT(KWinsLddName,"ecomm");
#endif //__WINS__

void RunAppL(void)
	{
	// Construct and install the active scheduler
	CActiveScheduler *exampleScheduler = new (ELeave) CActiveScheduler();
	// Push onto the cleanup stack
	CleanupStack::PushL(exampleScheduler);
	// Install as the active scheduler
	CActiveScheduler::Install(exampleScheduler); 
	//Create Obex objects and console handler
	CConsoleBase* aConsole = 
	  //Console::NewL(_L("OBEX DLL Test Code"),TSize(20,10));	
	  Console::NewL(_L("OBEX DLL Test Code"),TSize(KConsFullScreen,KConsFullScreen));
		// Load Device drivers

#ifdef __EPOC32__
	TInt ret1;
	aConsole->Printf(_L("Loading C32...\n"));
	ret1=StartC32();
	if (ret1!=KErrNone && ret1!=KErrAlreadyExists)
		aConsole->Printf(_L("	Failed %d\n"), ret1);
	else
		aConsole->Printf(_L("	Sucess\n"));
#endif
#ifdef __WINS__
	User::LoadLogicalDevice(KWinsLddName);
	User::LoadPhysicalDevice(KWinsPddName);
#endif //__WINS__

#ifdef __EPOC32__
	TInt ret2;

	aConsole->Printf(_L("Loading Fast IR PDD\n"));

	ret2=User::LoadPhysicalDevice(FIR_PDD_NAME);
	if (ret2!=KErrNone && ret2!=KErrAlreadyExists)
		aConsole->Printf(_L("	Failed %d\n"), ret2);
	else
		aConsole->Printf(_L("	Sucess\n"));

	aConsole->Printf(_L("Loading Fast IR LDD\n"));
	ret2=User::LoadLogicalDevice(FIR_LDD_NAME);
	if (ret2!=KErrNone && ret2!=KErrAlreadyExists)
		aConsole->Printf(_L("	Failed %d\n"), ret2);
	else
		aConsole->Printf(_L("	Success\n"));
#endif	

	CleanupStack::PushL(aConsole);
	CActiveConsole* my_console = CActiveConsole::NewLC(aConsole);
	my_console->RequestCharacter();
	CActiveScheduler::Start();
	CleanupStack::PopAndDestroy(3); 
	}

TInt E32Main()
	{
	__UHEAP_MARK;
	CTrapCleanup* cleanup=CTrapCleanup::New(); // get clean-up stack
	TRAPD(error,RunAppL()); // more initialization, then do example
	__ASSERT_ALWAYS(!error,User::Panic(_L("TOBEX"),error));
	delete cleanup; // destroy clean-up stack
	__UHEAP_MARKEND;

	return 0; // and return
	}
