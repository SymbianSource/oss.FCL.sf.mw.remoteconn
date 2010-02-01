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

/**
 @file
 @internalComponent 
*/

#include <c32comm.h>
#include <obexusbtransportinfo.h>
#include "ObexUsbTransportController.h"
#include "usbconn.h"
#include "logger.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "USB");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ObexUsbTC");
enum
	{
	KUsbTCError = 0,
	};
#endif


/**
Constructs a CObexUsbTransportController object

@param	aTransportInfo	 Reference to a TObexTransportInfo object. Will be cast to TObexUsbTransportInfo.

@return	A new CObexUsbTransportController  object
*/
CObexUsbTransportController* CObexUsbTransportController::NewL(TObexTransportInfo& aTransportInfo)
	{
	CObexUsbTransportController* self = new(ELeave) CObexUsbTransportController;
	CleanupStack::PushL(self);
	self->ConstructL(aTransportInfo);
	CleanupStack::Pop(self);
	return(self);
	}

/**
CObexUsbTransportController constructor.
*/
CObexUsbTransportController::CObexUsbTransportController()
	:CObexTransportControllerBase()
	{
	}
	
	
/**
2nd Phase Construction.

@param	aTransportInfo	Reference to a TObexTransportInfo object. Will be cast to TObexUsbTransportInfo.
*/
void CObexUsbTransportController::ConstructL(TObexTransportInfo& aTransportInfo)
	{
	//Note: The CObexUsbConnector will determine if UsbTransportInfo has type TObexUsbTransportInfo or TObexUsbV2TransportInfo.
	//CObexUsbTransportController does not need to know	
	TObexUsbTransportInfo& usbTransportInfo = static_cast<TObexUsbTransportInfo&>(aTransportInfo);
	
	iConnector  = CObexUsbConnector::NewL(*this,  usbTransportInfo); 
	iTransportInfo = new(ELeave) TObexUsbTransportInfo;
	Mem::Copy(iTransportInfo, &usbTransportInfo, sizeof(TObexUsbTransportInfo));
	}

/**
CObexUsbTransportController destructor
*/
CObexUsbTransportController::~CObexUsbTransportController()
	{
	}


/**
Gets the socket associated with the connector and asks the socket for its  remote name. 
 
@param	aAddr	Reference to a TSockAddr. Since this is the USB transport controller, there is no socket
					address to return, so the reference is zeroed.
*/
void CObexUsbTransportController::DoRemoteAddr(TSockAddr& aAddr)
	{
	Mem::FillZ(&aAddr, sizeof(TSockAddr));
	}


/**
Used indirectly by CObex::ControlledTransportDown() to determine whether the transport
can be forced down, or if the connection should just be cancelled and cleaned up.

@return	The USB transport cannot be restarted, so should not be forced down. Hence return EFalse	
*/
TBool CObexUsbTransportController::DoIsTransportRestartable() const
	{
	return EFalse;
	}


/**
Returns the receive packet data limit

@return	The receive packet data limit	
*/
TUint16 CObexUsbTransportController::GetReceivePacketDataLimit()
	{
	LOG_FUNC
	
	// Set initial "software throttle" for packets (how big OBEX says they are)
	__ASSERT_DEBUG(iTransportInfo, PANIC(KPanicCat, KUsbTCError));
	TUint16 receivePacketDataLimit = iTransportInfo->iReceiveMtu;
	return receivePacketDataLimit;
	}
	
	
/*
Creates the reader and writer and constructs the packets
Called via CObexTransportControllerBase::DoTransportUp

@param	aInfo Connection info to pass to active reader/writer classes
*/	
void CObexUsbTransportController::NewTransportL(TObexConnectionInfo& aInfo)
	{
	__ASSERT_DEBUG((aInfo.iSocketType == TObexConnectionInfo::EUsbStream), PANIC(KPanicCat, KUsbTCError));
	
	//Get the RDevUsbcClient to pass to the active reader/writer
	RDevUsbcClient* usbc = reinterpret_cast<CObexUsbConnector*>(iConnector)->TransportObject();
	
	// Get the packet size of the link
	TInt pktSize = reinterpret_cast<CObexUsbConnector*>(iConnector)->GetTransportPacketSize();
	
	//CObexUsbActiveReader(MObexTransportNotify* aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo);
	iActiveReader = CObexUsbActiveReader::NewL(*this, *usbc, aInfo);	
	//CObexUsbActiveWriter(MObexTransportNotify* aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo);
	iActiveWriter = CObexUsbActiveWriter::NewL(*this, *usbc, aInfo, pktSize);
	
	iConnectionInfo = aInfo;
	}

	
/**
Implements MObexTransportNotify
Process the received packet.

@param	aPacket	A reference to a CObexPacket
*/
void CObexUsbTransportController::DoProcess(CObexPacket &aPacket)
	{
	CObexTransportControllerBase::DoProcess(aPacket);
	}

	
/*
Implements MObexTransportNotify
Calls base implementation
Future proofing against BC breaks

@param	aError	The error value
*/
void CObexUsbTransportController::DoError(TInt aError) 
	{
	CObexTransportControllerBase::DoError(aError);
	}

	
/*
Implements MObexTransportNotify
Calls base implementation
Future proofing against BC breaks

@param	aInfo Connection info to pass to active reader/writer classes
*/
void CObexUsbTransportController::DoTransportUp(TObexConnectionInfo& aInfo) 
	{
	CObexTransportControllerBase::DoTransportUp(aInfo);
	}

