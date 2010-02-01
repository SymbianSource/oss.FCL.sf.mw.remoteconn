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
// Defines the interface to an obex connector
// 
//

#ifndef __OBEXCONNECTOR_H__
#define __OBEXCONNECTOR_H__

#include <e32base.h>

class MObexTransportNotify;

/**
@publishedPartner
@released
*/
struct TObexConnectionInfo
	{
	enum TSocketType
		{
		ESocketPacket,
		ESocketStream,
		EUsbStream,
		};
	TSocketType iSocketType;
	TInt iMaxSendSize;
	TInt iMaxRecvSize;
	};

/**
Base type for OBEX connectors
To make an OBEX protocol connector: Derive from this class and override the 
pure virtual functions with the relevent implementation  

@publishedPartner
@released
*/
class CObexConnector : public CBase
	{
public:
	/**
	This function is called as a result of an Obex client connect being 
	issued. This function may either 
	1. attempt to make a transport level connection with the transport level 
	of an Obex server. For example, in the case of socket based transports, a 
	socket is opened and a socket connect is issued. 
	2. check that a transport link exists. For example in the case of the usb 
	standard transport the connection may already be there
	In both cases, if successful, the connector will notify its owner that the 
	transport is up. Otherwise the connector may 'error' the owner or leave as 
	appropriate.
	 
	@see MObexTransportNotify::TransportUp()
	@see MObexTransportNotify::Error()
	*/
	virtual void ConnectL()=0;
	
	/**
	This function cancels actions taken as a result of the ConnectL() request
	@see ConnectL()
	*/
	virtual void CancelConnect ()=0 ;
	
	/**
	This function is called as a result of an Obex server accept being issued. 
	This function will start a transport object listening for a connect from 
	the transport layer of an Obex client, if the transport layer has not 
	already been established. 
	
	In both cases, if successful, the connector will notify its owner that the 
	transport is up. Otherwise the connector may 'error' the owner or leave as 
	appropriate.
	@see MObexTransportNotify::TransportUp()
	@see MObexTransportNotify::Error()
	*/
	virtual void AcceptL ()=0 ;
	
	/**
	This function cancels actions taken as a result of the AcceptL() request
	
	@see AcceptL()
	*/
	virtual void CancelAccept ()=0;
	
	/**
	This function takes down the transport link. If there are reasons why the 
	transport link cannot be taken down, then the function should return 
	EFalse. 
	*/
	virtual TBool BringTransportDown()=0;
	
	/**
	This function is a notification from the obex protocol layer that a 
	transport error has occured. Set any transport specific error condition 
	flags in this function.
	*/
	virtual void SignalTransportError()=0;
	
public:
	IMPORT_C CObexConnector(MObexTransportNotify& aObserver);
	IMPORT_C ~CObexConnector();

protected:
	IMPORT_C virtual TAny* GetInterface(TUid aUid);
	IMPORT_C void BaseConstructL();
	IMPORT_C MObexTransportNotify& Observer();
	
private: // owned
	TAny* iFuture1;

private: // unowned
	MObexTransportNotify& iObserver;
	};

#endif // __OBEXCONNECTOR_H__
