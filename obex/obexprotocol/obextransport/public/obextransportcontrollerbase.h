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

#ifndef __OBEXTRANSPORTCONTROLLERBASE_H__
#define __OBEXTRANSPORTCONTROLLERBASE_H__

#include <e32base.h>
#include <obex/transport/mobextransportnotify.h>
#include <obex/transport/obexconnector.h>
#include <obex/internal/obextransportconstants.h>
#include <obex/transport/obextransport.hrh>
#include <obextypes.h>

class CObexPacket;
class MObexNotifyExtend;
class TSockAddr;
class CObexActiveRW;
class TObexTransportInfo;

/**
@file
@publishedPartner
@released
*/

/**
Interface UID for transport controllers.
Transport controllers are instantiations of this interface.
*/
const TInt KObexTransportControllerInterfaceUid = KObexTransportInterfaceUid;

/**
Abstraction for transport controller implementations. Defines an interface to 
be used by Obex protocol layer as the facade to the transport layer.

In a concrete transport controller's NewL, the following things must be done: 
(a) CObexTransportControllerBase::iConnector must be assigned to a 
CObexConnector-derived type. (This may be reliant on the TObexTransportInfo 
object passed in at this time.) iConnector is owned by 
CObexTransportControllerBase.
(b) CObexTransportControllerBase::iTransportInfo must be assigned to a 
persistent (i.e. heap-based) TObexTransportInfo or derived type. The derived 
transport controller may cast the TObexTransportInfo given at construction 
time to a known derived type and copy the data from it to the new instance on 
the heap. Note that ownership of the given TObexTransportInfo *remains with 
the client*. iTransportInfo is then owned by CObexTransportControllerBase.

In a concrete transport controller's NewTransportL, two instances of types 
derived from CObexActiveRW must be created, and stored in 
CObexTransportControllerBase::iActiveReader and 
CObexTransportControllerBase::iActiveWriter. CObexTransportControllerBase owns 
these.
*/
class CObexTransportControllerBase : public CBase, public MObexTransportNotify
	{
public:
	IMPORT_C static CObexTransportControllerBase* NewL(TObexTransportInfo& aTransportInfo);
	IMPORT_C ~CObexTransportControllerBase();
	
public:
	IMPORT_C void SetOwner(MObexNotifyExtend& aOwner);

	//delegated to activerw class
	IMPORT_C CObexPacket& SendPacket () ;	//< Access packet buffers
	IMPORT_C CObexPacket& ReceivePacket ();
	IMPORT_C void Send ();						//< Actually carry out transfer
	IMPORT_C void Send (TObexOpcode aOpcode);	//< Prepare and send a 3 byte packet
	IMPORT_C void Receive ();
	
	IMPORT_C TBool IsWriteActive () const;				//< is the ActiveWriter busy
	IMPORT_C TInt InsertLocalConnectInfo (CObexPacket &aPacket, TUint8 aVersion, TUint8 aFlags);
	IMPORT_C TBool ExtractRemoteConnectInfo (CObexPacket &aPacket, TUint8& aVersion, TUint8& aFlags);
	IMPORT_C void CancelTransfers();
	
	//delegated to connector class
	IMPORT_C void ConnectL ();
	IMPORT_C void CancelConnect();
	IMPORT_C void AcceptConnectionL();
	IMPORT_C void CancelAccept();
	IMPORT_C TBool  BringTransportDown();
	IMPORT_C void SignalTransportError();
	
	//public non-virtual methods to aid preventing bc/sc breaks when the client interface changes
	IMPORT_C void RemoteAddr(TSockAddr& aAddr);
	IMPORT_C TBool IsTransportRestartable() const;
	IMPORT_C const TObexTransportInfo* TransportInfo() const;
	
protected:
	IMPORT_C CObexTransportControllerBase();

private:
	void BaseConstructL();

private: // implemented  by derived controller 
	virtual void DoRemoteAddr(TSockAddr& aAddr)=0;
	virtual TBool DoIsTransportRestartable() const = 0;
	virtual void NewTransportL(TObexConnectionInfo& aInfo) = 0;
	virtual TUint16 GetReceivePacketDataLimit() = 0;
	
protected: // from MObexTransportNotify
	IMPORT_C virtual void DoProcess(CObexPacket &aPacket);
	IMPORT_C virtual void DoError(TInt aError);
	IMPORT_C virtual void DoTransportUp(TObexConnectionInfo& aInfo);
	IMPORT_C virtual void DoSignalPacketProcessEvent(TObexPacketProcessEvent aEvent);
	
protected:	//Extension pattern
	IMPORT_C virtual TAny* GetInterface(TUid aUid);
	
private: // utility
	void DeleteTransport();
	void ConstructPacketsL();
	void InitialiseTransportL(TObexConnectionInfo& aInfo);
	void TransportUpError(TInt aError);

protected: // owned
	CObexConnector* iConnector;
	CObexActiveRW* iActiveReader;
	CObexActiveRW* iActiveWriter;
	TObexConnectionInfo iConnectionInfo;
	TObexTransportInfo* iTransportInfo;

private: // unowned
	MObexNotifyExtend* iOwner;

private: // owned 
	CObexPacket* iReceivePacket;
	CObexPacket* iSendPacket;
	TAny* iFuture1;
	// Instance identifier key
	TUid iPrivateEComUID; 
	};

#endif // __OBEXTRANSPORTCONTROLLERBASE_H__
