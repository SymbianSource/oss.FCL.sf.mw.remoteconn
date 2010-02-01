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

#ifndef _CLIENTHANDLER_H
#define _CLIENTHANDLER_H

#include <es_sock.h>
#include <e32test.h>
#include <es_prot.h>
#include <e32cons.h>
#include <obex.h>
#include <btmanclient.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexinternalconstants.h>
#include <btsdp.h>
#include <obexfinalpacketobserver.h>

#ifdef ERROR_RESOLUTION_EXTENSION
#include <obex/extensionapis/obexclienterrorresolver.h>
#endif // ERROR_RESOLUTION_EXTENSION

#ifdef EMPTY_HEADERS_TEST
#include "emptyheaderstest.h"
#endif //EMPTY_HEADERS_TEST

#include "btutils.h"
#include "constants.h"


class CActiveConsole;

class CObexClientHandler : public CActive, public MObexAuthChallengeHandler,
							private MRFCOMMServiceSeeker,
							public MObexFinalPacketObserver
//-----------------------------------------------------------
	{
public:
	// Construction
	CObexClientHandler(CActiveConsole* aParent);
	static CObexClientHandler* NewL(CActiveConsole* aParent, TTransport aTransport);
	void ConstructL(TTransport aTransport);

	// Destruction
	~CObexClientHandler();

	void SetMode(TUint mode);

	void Abort();
	void Connect();
	void Disconnect();
	void GetByNameL();
	void GetByTypeL();
	void Put(TDes& filename);
	void SetName(TDes& aName);
	void SetObexName();
	void SetPath();
	void SetType();
	void GetReferenceL(TInt aReferenceId);
	void PutReferenceL(TInt aReferenceId);
	void ChangeAuthenticationChallengeHeaders(TChar aChar);

	void ConnectWithAuthenticationL();
	void ConnectWithTarget();
	void ConnectWithAuthenticationAndTargetL();
	void EnablePassword();
	void ChangeChallengePassword();
	void SetCommandTimeout();
	void ChangeResponsePassword();
	void RemoteAddress();
	void GetUserPasswordL(const TDesC& aUserID);
	void SearchResult(TInt aError, TUint8 aPort, TInt aProfileVersion);
	void HTTPTest1L();
	void HTTPTest2L();
	void HTTPTest3L();
	void HTTPTest4L();
	void AppParamsTestL();
	void SetCurrentTestNumber();
	void ResetCurrentTestNumber();
	
	void MofpoFinalPacketStarted();
	void MofpoFinalPacketFinished();
#ifdef ERROR_RESOLUTION_EXTENSION
	void LastError();
#endif // ERROR_RESOLUTION_EXTENSION

#ifdef EMPTY_HEADERS_TEST
	void EmptyHeaderTestL(TPtrC aName, TPtrC8 aType, TPtrC aDesc, TPtrC8 aTarget, TPtrC8 aAppParam);
#endif //EMPTY_HEADERS_TEST

private:
	void RunL ();
	void DoCancel ();
	TInt SetUpObject (TDes& filename);
	void DisplayObjectL();
	void SaveObject();

public:
	TBuf<64> iObexName;
	TBuf<64> iFilename1;
	TBuf<64> iFilename2;
	TBuf<64> iFilename3;
	TBuf<16> iChallengePassword;
	TBuf<16> iResponsePassword;
	TBuf<32> iGetType;
	TPath iInboxRoot;
	
private:
	enum TSendState
		{
		EIdle,
		EConnecting,
		EConnected,
		EPutting,
		EGetting,
		EDisconnecting,
		ESettingPath,
		EGettingReference,
		EPuttingReference
		};

	CActiveConsole* iParent;
	CObexClient* iClient;
#ifdef ERROR_RESOLUTION_EXTENSION
	CObexClientErrorResolver* iClientErrorResolver;
#endif // ERROR_RESOLUTION_EXTENSION

#ifdef EMPTY_HEADERS_TEST
	CObexEmptyHeaderTest* iObexEmptyHeaderTest;
#endif //EMPTY_HEADERS_TEST
	
	CObexFileObject* iFileObject;
	CObexBufObject* iObject;
	CBufFlat* iObjectBuffer;
	TSendState iState;
	CObexNullObject* iTargetHeaderObject;
	CRFCOMMServiceFinder* iSdpServiceFinder;
	TBTDevAddr iDevAddr;
	TInt iCurrentTestNumber;

	TTime iStartTime;
	};


#endif // _CLIENTHANDLER_H
