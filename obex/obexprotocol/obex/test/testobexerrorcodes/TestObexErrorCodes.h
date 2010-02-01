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

#ifndef __TESTOBEXERRORCODES_H__
#define __TESTOBEXERRORCODES_H__

#include <e32cons.h>
#include <e32def.h>
#include <e32std.h>
#include <e32base.h>
#include <obex.h>

class CControllerConsoleTest;
class CBaseErrorCodeTest;
class CObexClientHandler;

enum TTestProgress { EIdle, EConnecting, EDisconnecting, EConnected, EPutting };

class CControllerTestMasterClass : CBase
	{
public:
	void ConstructL();
	static CControllerTestMasterClass* NewL();
	static CControllerTestMasterClass* NewLC();
	~CControllerTestMasterClass();

public:
	CControllerConsoleTest* iConsoleTest;
	};



class CControllerConsoleTest : CActive
	{

public:	
	void ConstructL(CControllerTestMasterClass*);
	static CControllerConsoleTest* NewL(CControllerTestMasterClass*);
	static CControllerConsoleTest* NewLC(CControllerTestMasterClass*);
	~CControllerConsoleTest();
public:
	void IssueRequest(); 
	void DoCancel();
	void RunL();
	void TestAll();
	TInt RunError(TInt);
	void DoTest();

private:
	CControllerConsoleTest();
	void StartTest();
	void StopTest();
	void Menu();
private:
	CControllerTestMasterClass* iOwner;

public:
	CConsoleBase* iConsole;
	CBaseErrorCodeTest* iTest;
	TBool iStarted;
	};



class CBaseErrorCodeTest : public CBase 
	{
public:
	static CBaseErrorCodeTest* NewL(CControllerConsoleTest* aOwner);
	virtual ~CBaseErrorCodeTest();

public:

	virtual	void Start();
	virtual void Stop();


protected:

	CBaseErrorCodeTest();
	void ConstructL(CControllerConsoleTest* aOwner);
	void LoadObexL();

public:
	CControllerConsoleTest* iOwner;



protected:
	CObexBufObject*  iObject;
	CBufFlat *iBuf;
	RLibrary         iLibrary;					// Use RLibrary object to interface to the DLL
	TInt iErrorCode;
	};

/**
*
* The public API for Server test
*            
*/

class CTestObexErrorCodesServer : public CBaseErrorCodeTest, MObexServerNotify
	{
public:
	static CTestObexErrorCodesServer* NewL(CControllerConsoleTest* aOwner);
	virtual ~CTestObexErrorCodesServer();
	void StartIrdaServerL();

public:

	virtual	void Start();
	virtual	void Stop();


private:
	CTestObexErrorCodesServer();


	// Virtual functions from OBEX
	virtual void ErrorIndication(TInt aError);
	virtual void TransportUpIndication();
	virtual void TransportDownIndication();
	virtual TInt ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
	virtual void ObexDisconnectIndication(const TDesC8& aInfo);
	virtual CObexBufObject* PutRequestIndication();
	virtual TInt PutPacketIndication();
	virtual TInt PutCompleteIndication();
	virtual CObexBufObject* GetRequestIndication(CObexBaseObject *aRequiredObject);
	virtual TInt GetPacketIndication();
	virtual TInt GetCompleteIndication();
	virtual TInt SetPathIndication(const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
	virtual void AbortIndication();

private:
	CObexServer* iServer;
	};

/**
*
* The public API for Server test
*            
*/

class CTestObexErrorCodesClient : public CBaseErrorCodeTest
	{
public:
	static CTestObexErrorCodesClient* NewL(CControllerConsoleTest* aOwner);
	virtual ~CTestObexErrorCodesClient();
	void StartIrdaClientL();
	void ActionComplete(TObexResponse aResponse);

public:
	void CallBack(TRequestStatus aStatus, TTestProgress aActivity);
	virtual	void Start();
	virtual	void Stop();


private:
	CTestObexErrorCodesClient();

private:
	CObexClientHandler* iClient;
	};



class CObexClientHandler : public CActive
	{
public:
	static CObexClientHandler* NewL(CTestObexErrorCodesClient* aOwner, 
									TObexProtocolInfo& aObexProtocolInfoPtr);
	~CObexClientHandler();

public:
	void Connect();
	void Disconnect();
	void Put();

private:

	CObexClientHandler();
	void ConstructL(CTestObexErrorCodesClient* aOwner,
					TObexProtocolInfo& aObexProtocolInfoPtr);

	void DoCancel();
	void RunL();



private:
	CObexClient* iClient;
	CTestObexErrorCodesClient* iOwner;	
	TTestProgress iActivity;
	CObexFileObject* iFileObject;

	};

#endif // __TESTOBEXERRORCODES_H__
