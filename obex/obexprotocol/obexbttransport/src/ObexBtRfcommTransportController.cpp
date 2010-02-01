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

#include <obexbttransportinfo.h>
#include <obex/transport/obexactivereader.h>
#include <obex/transport/obexactivewriter.h>
#include "ObexBtRfcommTransportController.h"
#include "ObexRfcommConnector.h"
#include "logger.h"
#include "obexbtfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "RFCOMM");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "BtRfcommTc");
#endif

/**
Constructs a CObexBtRfcommTransportController object.
@return Ownership of a new CObexIrdaTtpTransportController object.
*/
CObexBtRfcommTransportController* CObexBtRfcommTransportController::NewL(TObexTransportInfo& aTransportInfo)
	{
	LOG_STATIC_FUNC_ENTRY

	CObexBtRfcommTransportController* self = new(ELeave) CObexBtRfcommTransportController;
	CleanupStack::PushL(self);
	self->ConstructL(aTransportInfo);
	CleanupStack::Pop(self);
	return self;
	}
	
/** Constructor.
*/
CObexBtRfcommTransportController::CObexBtRfcommTransportController()
	:CObexTransportControllerBase()
	{
	LOG_FUNC
	}

/** 2nd phase construction.
*/
void CObexBtRfcommTransportController::ConstructL(TObexTransportInfo& aTransportInfo)
	{
	LOG_FUNC

	// It's important in the current implementation to do these these in 
	// roughly this order. We pass the address part of the base class's 
	// transport info by reference to the connector and thence to the 
	// listener. If the user has specified RFCOMM 'passive auto bind' then 
	// just updating the address in the listener will update the transport 
	// controller base's transport info for later query by the user. Using 
	// references saves having to add functions to achieve a manual update of 
	// the data.
	iTransportInfo = new(ELeave) TObexBtTransportInfo;
	Mem::Copy(iTransportInfo, &aTransportInfo, sizeof(TObexBtTransportInfo));
	TObexBtTransportInfo* btTransportInfo = static_cast<TObexBtTransportInfo*>(iTransportInfo);
	iConnector = CObexRfcommConnector::NewL(*this, btTransportInfo->iAddr);
	}

/** Destructor.
*/
CObexBtRfcommTransportController::~CObexBtRfcommTransportController()
	{
	LOG_FUNC
	}
	
/**
Gets the socket associated with the connector and asks the socket for its  remote name. 
This can be cast to the appropriate TSockAddr-derived class (TBTSockAddr for BT). 
*/
void CObexBtRfcommTransportController::DoRemoteAddr(TSockAddr& aAddr)
	{
	LOG_FUNC
	LOG(_L8("\taAddr:"));
	LOGHEXDESC(aAddr);

	__ASSERT_DEBUG(iConnector, PANIC(KPanicCat, ERfcommTcInternalError));
	RSocket& socket = static_cast<CObexRfcommConnector*>(iConnector)->GetSocket();
	socket.RemoteName(aAddr);
	}
	
TBool CObexBtRfcommTransportController::DoIsTransportRestartable() const	
	{
	LOG_FUNC

	return ETrue;
	}

TUint16 CObexBtRfcommTransportController::GetReceivePacketDataLimit() 
	{
	LOG_FUNC
	
	// Set initial "software throttle" for packets (how big OBEX says they are)
	__ASSERT_DEBUG(iTransportInfo, PANIC(KPanicCat, ERfcommTcInternalError));
	TUint16 receivePacketDataLimit = iTransportInfo->iReceiveMtu;
	return receivePacketDataLimit;
	}
	
/**
Creates the reader and writer and constructs the packets
@param aInfo Connection info to pass to active reader/writer classes
*/	
void CObexBtRfcommTransportController::NewTransportL(TObexConnectionInfo& aInfo)
	{
	LOG_FUNC

	__ASSERT_DEBUG(iConnector, PANIC(KPanicCat, ERfcommTcInternalError));
	RSocket& socket = static_cast<CObexRfcommConnector*>(iConnector)->GetSocket();
	iActiveReader = CObexActiveReader::NewL(*this, socket, aInfo);
	iActiveWriter = CObexActiveWriter::NewL(*this, socket, aInfo);
	}
	
//From MObexTransportNotify
/**
Process the received packet
@param aPacket Packet to process
*/
void CObexBtRfcommTransportController::DoProcess(CObexPacket &aPacket)
	{
	LOG_FUNC

	CObexTransportControllerBase::DoProcess(aPacket);
	}
	
/**
Calls base implementation
Future proofing against BC breaks
@param aError Error code
*/
void CObexBtRfcommTransportController::DoError(TInt aError) 
	{
	LOG_FUNC
	LOG1(_L8("\taError = %d"), aError);

	CObexTransportControllerBase::DoError(aError);
	}
	
/**
Calls base implementation
Future proofing against BC breaks
@param aInfo Connection info to pass to active reader/writer classes
*/
void CObexBtRfcommTransportController::DoTransportUp(TObexConnectionInfo& aInfo) 
	{
	LOG_FUNC

	CObexTransportControllerBase::DoTransportUp(aInfo);
	}
