// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef _TOBEXCON_H
#define _TOBEXCON_H

#include <e32test.h>
#include <e32cons.h>
#include <irdaextensions.h>
#include <obex.h>
#include <btmanclient.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexinternalconstants.h>
#include <btsdp.h>
#include <obexfinalpacketobserver.h>
#include "constants.h"


class CObexClientHandler;
class CObexServerHandler;
class CObexServerHandlerFile;
class CReferenceHandler;

class CActiveConsole : public CActive
//-----------------------------------
	{
public:
	  // Construction
	CActiveConsole(CConsoleBase* aConsole);
	static CActiveConsole* NewLC(CConsoleBase* aConsole);
	void ConstructL();

	  // Destruction
	~CActiveConsole();

	  // Issue request
	void RequestCharacter();
	
	  // Cancel request.
	  // Defined as pure virtual by CActive;
	  // implementation provided by this class.
	void DoCancel();

	  // Service completed request.
	  // Defined as pure virtual by CActive;
	  // implementation provided by this class,
	void RunL();

	void ProcessKeyPressL(TChar aChar);

	CConsoleBase* Console();
	void SetPassword(TDes& aPassword);
	void SetInboxRoot();
	TBool DisplayHeaders();
	void GetString(TDes& aPassword);
	TBool SetNumber(TUint& aNumber);
	void ClientErrored();
	
	TBool UseIrdaNickname() const;
	const TDesC& GetIrdaNickname() const;
	 
public:
	  // Data members defined by this class
	CConsoleBase*			iConsole;					// A console for reading from
	CObexClientHandler*		iObexClientHandler;	// Client wrapper for CObexClient
	CObexServerHandler*		iObexServerHandler;	// Client wrapper for CObexServer
	CObexServerHandlerFile*	iObexServerHandlerFile; //Client wrapper for CObexServer
	TUint					iTestMode;
	TTransport				iTransport;
	CReferenceHandler*		iRefHandler;
	TBool					iDisplayHeaders;
	TPath					iInboxRoot;
	TUint 					iDiscoverySlots;
	TUint 					iDiscoveryAttempts;
	TUint16					iRecvMTU;
	TUint16					iTransmitMTU;
	RIrdaDiscoveryResponseSuppressor iSupp;
	TBuf<20> iIrDANicknameBuffer;
	TBool iPerformanceLoggingEnabled;
	};




class CReferenceHandler : public CBase
	{
public:

	enum TObexBufObjectComparison
		{
		EAllSame = 0,
		EDifferentName = 1,
		EDifferentType = 2,
		EDifferentLength = 4,
		EDifferentTime = 8,
		EDifferentDescription = 16,
		EDifferentTarget = 32,
		EDifferentBuf = 64,
		EDifferentHttp = 128,
		EDifferentAppParam = 256,
		EDifferentUserDefined = 512,
		EDifferentCount = 1024,
		EDifferentCreatorID = 2048,
		EDifferentWanUUID = 4096, 
		EDifferentObjectClass = 8192};

	static CReferenceHandler* NewL();
	CReferenceHandler();
	~CReferenceHandler();

	void CreateReferenceL(CObexBufObject& aObject, CBufFlat& aObjectBuf, TInt aReferenceId, TTransport aTransport);

	TInt CompareObjectToReferenceL(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);

private:
	void CreateReferenceObject1L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject2L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject3L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject4L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject5L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject6L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject7L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject8L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject9L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);
	void CreateReferenceObject10L(CObexBufObject& aObject, CBufFlat& aObjectBuf, TTransport aTransport);

	};

#endif // _TOBEXCON_H
