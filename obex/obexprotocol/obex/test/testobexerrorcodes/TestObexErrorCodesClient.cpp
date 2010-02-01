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

#include <f32file.h>
#include "TestObexErrorCodes.h"

/**
 * Static NewL
 *
 * @return Base class*
 */
CTestObexErrorCodesClient* CTestObexErrorCodesClient::NewL(CControllerConsoleTest* aOwner)
	{
	CTestObexErrorCodesClient* self = new(ELeave) CTestObexErrorCodesClient;
	CleanupStack::PushL(self);
	self->ConstructL(aOwner);
	self->StartIrdaClientL();
	CleanupStack::Pop( self );

	return self;
	}




/**
*
* CTestObexErrorCodesClient
*
*/

CTestObexErrorCodesClient::CTestObexErrorCodesClient()
	{
	}

/**
*
* ~CTestObexErrorCodesClient
*
*/

CTestObexErrorCodesClient::~CTestObexErrorCodesClient()
	{
	delete iClient;
   	}

void CTestObexErrorCodesClient::Stop()
	{
	iOwner->iConsole->Printf(_L("About to Stop the iClient ..\n"));


	}


void CTestObexErrorCodesClient::Start()
	{
//Start the test

	iClient->Connect();	//should fail with appropriate error code


	}




void CTestObexErrorCodesClient::StartIrdaClientL()
	{

	iOwner->iConsole->Printf(_L("About to load the CObexClient...\n"));

	TObexIrProtocolInfo aInfo;
	aInfo.iTransport     = _L("IrTinyTP");
	aInfo.iClassName     = _L8("OBEX");
	aInfo.iAttributeName = _L8("IrDA:TinyTP:LsapSel");


	iClient  = CObexClientHandler::NewL (this, aInfo);
		

	iBuf = CBufFlat::NewL(5000);//5000 - what should I put??
	iObject  = CObexBufObject::NewL (iBuf);
	iOwner->iConsole->Printf(_L("Irda Client loaded Success \n"));

	
	
	}

void CTestObexErrorCodesClient::CallBack(TRequestStatus aStatus, TTestProgress aActivity)
	{
	
	//first of all establish what has just happened.
	if ( aActivity == EConnecting ) //then it's time for the first PUT
		{
		iClient->Put();
		iErrorCode = ERespContinue;
		}
	else if (aActivity == EPutting )
		{
		iOwner->iConsole->Printf(_L("Put Complete with Code %d \n"), aStatus.Int());
		if ( iErrorCode < ERespDatabaseLocked )
			{
			iErrorCode++;
			iClient->Put();
			}
		}

	}




CObexClientHandler* CObexClientHandler::NewL(CTestObexErrorCodesClient* aOwner,
											 TObexProtocolInfo& aObexProtocolInfoPtr)
	{
	CObexClientHandler* self = new(ELeave) CObexClientHandler;
	CleanupStack::PushL(self);
	self->ConstructL(aOwner, aObexProtocolInfoPtr);
	CleanupStack::Pop( self );

	return self;
	
	}

CObexClientHandler::~CObexClientHandler()
	{
	delete iClient;
	}

void CObexClientHandler::Connect()
	{
	if (!IsActive() )
		{
		iActivity = EConnecting;

		iClient->Connect(iStatus);
		SetActive();
		}
	else
		iOwner->iOwner->iConsole->Printf(_L("\r\nError: Client handler already active\r\n"));
	}

void CObexClientHandler::Disconnect()
	{
	if (!IsActive() )
		{
		iActivity = EDisconnecting;
		iClient->Disconnect(iStatus);
		SetActive();
		}
	else
		iOwner->iOwner->iConsole->Printf(_L("\r\nError: Client handler already active\r\n"));

	}

CObexClientHandler::CObexClientHandler()
    : CActive(EPriorityNormal)
	{
	}

void CObexClientHandler::ConstructL(CTestObexErrorCodesClient* aOwner,
									TObexProtocolInfo& aObexProtocolInfoPtr)
	{
	iOwner = aOwner;

    iFileObject = CObexFileObject::NewL();


	iClient = CObexClient::NewL(aObexProtocolInfoPtr);
    CActiveScheduler::Add(this);			// Add to active scheduler
	iActivity = EIdle;
	}

void CObexClientHandler::DoCancel()
	{
	iClient->Abort();
	}

void CObexClientHandler::RunL()
	{
	
	iOwner->CallBack(iStatus, iActivity);
		
	}

void CObexClientHandler::Put()
	{
    if(IsActive())
		{

		iOwner->iOwner->iConsole->Printf(_L("\r\nError: Client handler already active\r\n"));
		return;
		}

	TInt err;

	TRAP(err, iFileObject->InitFromFileL (_L("Contacts.vcf")));

	if (err != KErrNone)
		{
		RFs fs;
		RFile f;
		if ((fs.Connect () != KErrNone) || 
			(f.Create (fs, _L("Contacts.vcf"), EFileShareExclusive | EFileWrite) != KErrNone))
			iOwner->iOwner->iConsole->Printf(_L("\r\nError reading 'Contacts.vcf'.\r\nI tried to create this file for you, but failed to do that too. Sorry.\r\n\r\n"));
		else
			{
			f.Write (_L8("Test file for sending from EPOC\r\n\r\nLooks like obex is sending OK!!\r\n"));
			f.Close ();
			iOwner->iOwner->iConsole->Printf(_L("\r\nFile 'Contacts.vcf' did not exist, so I've created one.\r\nPlease try again.\r\n\r\n"));
			}
		fs.Close ();
		}


	if( err != KErrNone)
		{
		iOwner->iOwner->iConsole->Printf(_L("\r\n Couldnt set up object : error %d \r\n"),err);
		return;
		}

	iClient->Put(*iFileObject,iStatus);
	SetActive();
	iActivity = EPutting;
	}
