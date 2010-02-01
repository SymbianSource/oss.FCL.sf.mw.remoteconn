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


#ifndef __OBEXUSBTRANSPORTCONTROLLER_H__
#define __OBEXUSBTRANSPORTCONTROLLER_H__

#include <obex/transport/obextransportcontrollerbase.h>

/**
CObexUsbTransportController
Transport Controller for the usb transport
*/
NONSHARABLE_CLASS(CObexUsbTransportController) : public CObexTransportControllerBase
	{
public://NewL functions
	static CObexUsbTransportController* NewL(TObexTransportInfo& aTransportInfo);

public://Functions derived from CBase
	virtual  ~CObexUsbTransportController();
	
public://Functions derived from  CObexTransportControllerBase
	virtual void DoRemoteAddr(TSockAddr& aAddr);
	virtual void NewTransportL(TObexConnectionInfo& aInfo);
	virtual TBool DoIsTransportRestartable() const;
	TUint16 GetReceivePacketDataLimit();
	
public://Functions implemented from MObexTransportNotify
	virtual void DoProcess(CObexPacket &aPacket) ;
	virtual void DoError(TInt aError) ;
	virtual void DoTransportUp(TObexConnectionInfo& aInfo);	
	
private:	
	void ConstructL(TObexTransportInfo& aTransportInfo);
	CObexUsbTransportController();
	};

#endif //__OBEXUSBTRANSPORTCONTROLLER_H__
