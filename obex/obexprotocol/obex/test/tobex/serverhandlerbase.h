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

#ifndef _SERVERHANDLERBASE_H
#define _SERVERHANDLERBASE_H


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
#include <obexreadactivityobserver.h>
#include "constants.h"

class CServerAsyncAO;
class CActiveConsole;
class CObexServerPacketAccessUi;


class CObexServerHandlerBase : public CBase,
#ifdef ASYNC_INDICATION
							   private MObexServerNotifyAsync, 
#else
							   private MObexServerNotify,
#endif						   							   
							   public MObexAuthChallengeHandler,
							   
							   public MObexReadActivityObserver
//-----------------------------------------------------------
	{
public:
	// Construction
	void BaseConstructL(TTransport aTransport);

	void Start();
	void Stop();
	// Destruction
	virtual ~CObexServerHandlerBase();

	void EnableAuthentication();
	void DisableAuthentication();
	void EnablePassword();
	void ChangeChallengePassword(TDesC* aPassword = NULL);
	void ChangeResponsePassword();
	void SetLocalWho();
    void SetTargetChecking(CObexServer::TTargetChecking aValue);
	void GetUserPasswordL(const TDesC& aUserID);
	void RemoteAddress();
#ifdef PACKET_ACCESS_EXTENSION
	void PacketAccessUiL(TBool aEnable);
#endif // PACKET_ACCESS_EXTENSION
	
protected:
	CObexServerHandlerBase(CActiveConsole* aParent);

private:
	// Implementation of MObexServerNotify interface
	virtual void ErrorIndication (TInt aError);
	virtual void TransportUpIndication ();
	virtual void TransportDownIndication ();
	
	virtual void ObexDisconnectIndication (const TDesC8& aInfo);
	virtual TInt PutPacketIndication ();
	virtual TInt GetPacketIndication ();
	virtual void AbortIndication ();

	
#ifdef ASYNC_INDICATION
	void GetCompleteIndication ();
	void PutCompleteIndication ();
	void SetPathIndication (const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
	void CancelIndicationCallback();
	void ObexConnectIndication  (const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
	void PutRequestIndication ();
	void GetRequestIndication (CObexBaseObject *aRequestedObject);
#else
	TInt ObexConnectIndication  (const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
	TInt GetCompleteIndication ();
	TInt PutCompleteIndication ();
	TInt SetPathIndication (const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
	CObexBufObject* PutRequestIndication ();
	CObexBufObject* GetRequestIndication (CObexBaseObject *aRequestedObject);
#endif

	// Implementation of MObexReadActivityObserver interface
	void MoraoReadActivity();
	void SetUpGetObjectL(CObexBaseObject *aRequestedObject);
	void HandleGetReferenceObjL(CObexBaseObject* aRequiredObject);
	
	void InitBluetoothL();

public:
	TBuf<16> iChallengePassword;
	TBuf<16> iResponsePassword;
	TBool iAcceptPuts;

protected:
	CObexBufObject*  iObject;
	CBufFlat* iBuf;
	CActiveConsole* iParent;

private:
	CObexServer*  iServer;
	TBool iIsAuthenticationEnabled;
	TTransport iTransportLayer;
	CSdpAttrValueDES* iProtDescList;
	RSdp iSdp;
	RSdpDatabase iSdpdb;
#ifdef ASYNC_INDICATION
	CServerAsyncAO* iServerAsyncAO;
#endif
#ifdef PACKET_ACCESS_EXTENSION	
	CObexServerPacketAccessUi* iPacketAccessUi;
#endif // PACKET_ACCESS_EXTENSION

	TTime iStartTime;
	};

#endif // _SERVERHANDLERBASE_H
