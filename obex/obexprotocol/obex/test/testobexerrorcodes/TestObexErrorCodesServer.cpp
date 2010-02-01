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

#include "TestObexErrorCodes.h"

/**
 * Static NewL
 *
 * @return Base class*
 */
CTestObexErrorCodesServer* CTestObexErrorCodesServer::NewL(CControllerConsoleTest* aOwner)
	{
	CTestObexErrorCodesServer* self = new(ELeave) CTestObexErrorCodesServer;
	CleanupStack::PushL(self);
	self->ConstructL(aOwner);
	self->StartIrdaServerL();
	CleanupStack::Pop( self );

	return self;
	}




/**
*
* CTestObexErrorCodesServer
*
*/

CTestObexErrorCodesServer::CTestObexErrorCodesServer()
	{
	}

/**
*
* ~CTestObexErrorCodesServer
*
*/

CTestObexErrorCodesServer::~CTestObexErrorCodesServer()
	{
	delete iServer;
   	}

void CTestObexErrorCodesServer::Stop()
	{
	iOwner->iConsole->Printf(_L("About to Stop the iServer ..\n"));

    if(iServer)
		iServer->Stop();
	iOwner->iConsole->Printf(_L("iServer Stopped ..\n"));

	}


void CTestObexErrorCodesServer::Start()
	{

	iOwner->iConsole->Printf(_L("About to Start the iServer ..\n"));

	TInt err = iServer->Start (this);
	iOwner->iConsole->Printf(_L("Server started with err=%d ..\n"), err);

	iErrorCode = ERespSuccess;  //first returned code not Continue as this really breaks everything
	}


/**
Virtual functions from OBEX
*/

void CTestObexErrorCodesServer::ErrorIndication(TInt /*aError*/)
	{
	}

void CTestObexErrorCodesServer::TransportUpIndication()
	{
	iOwner->iConsole->Printf(_L("Transport layer now connected\n"));
	}

void CTestObexErrorCodesServer::TransportDownIndication()
	{
	iOwner->iConsole->Printf(_L("Transport layer has gone down\n"));
	}

TInt CTestObexErrorCodesServer::ObexConnectIndication(const TObexConnectInfo& /*aRemoteInfo*/, const TDesC8& /*aInfo*/)
	{
	return KErrNone;
	}

void CTestObexErrorCodesServer::ObexDisconnectIndication(const TDesC8& /*aInfo*/)
	{
    iOwner->iConsole->Printf(_L("\r\nObex Disconnected\n"));
	}

CObexBufObject* CTestObexErrorCodesServer::PutRequestIndication()
	{
	return (iObject);
	}

TInt CTestObexErrorCodesServer::PutPacketIndication()
	{
    return (KErrNone);
	
	}

TInt CTestObexErrorCodesServer::PutCompleteIndication()
	{

	iOwner->iConsole->Printf(_L("PutComplete returning error code %d ..\n"), iErrorCode);
	//return the changing error code here
	TInt retValue = (iErrorCode * -1 ) + KErrIrObexRespBase;

	iErrorCode++;

	return (retValue);
//	return (KErrNone);

	}

CObexBufObject* CTestObexErrorCodesServer::GetRequestIndication(CObexBaseObject* /*aRequiredObject*/)
	{
	return (NULL);
	}

TInt CTestObexErrorCodesServer::GetPacketIndication()
	{
	//return the changing error code here
	TInt retValue = iErrorCode;
	if ( iErrorCode < ERespDatabaseLocked )
		iErrorCode++;
	else
		iErrorCode = ERespContinue;

	return (retValue);
	}

TInt CTestObexErrorCodesServer::GetCompleteIndication()
	{
	//return the changing error code here
	TInt retValue = iErrorCode;
	if ( iErrorCode < ERespDatabaseLocked )
		iErrorCode++;
	else
		iErrorCode = ERespContinue;

	return (retValue);
	}

TInt CTestObexErrorCodesServer::SetPathIndication(const CObex::TSetPathInfo& /*aPathInfo*/, const TDesC8& /*aInfo*/)
	{
	//return the changing error code here
	TInt retValue = iErrorCode;
	if ( iErrorCode < ERespDatabaseLocked )
		iErrorCode++;
	else
		iErrorCode = ERespContinue;

	return (retValue);
	}

void CTestObexErrorCodesServer::AbortIndication()
	{
	}


void CTestObexErrorCodesServer::StartIrdaServerL()
	{

	iOwner->iConsole->Printf(_L("About to load the CObexServer...\n"));

	TObexIrProtocolInfo aInfo;
	aInfo.iAddr.SetPort(9);//default obex server for now
	aInfo.iTransport     = _L("IrTinyTP");
	aInfo.iClassName     = _L8("OBEX");
	aInfo.iAttributeName = _L8("IrDA:TinyTP:LsapSel");


	iServer  = CObexServer::NewL (aInfo);
		

	iBuf = CBufFlat::NewL(5000);//5000 - what should I put??
	iObject  = CObexBufObject::NewL (iBuf);
	iOwner->iConsole->Printf(_L("Irda Server loaded Success \n"));


	}
