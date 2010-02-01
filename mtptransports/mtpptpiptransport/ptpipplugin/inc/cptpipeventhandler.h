// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#ifndef CEVENTHANDLER_H_
#define CEVENTHANDLER_H_

#include "e32def.h"
#include "cptpipsockhandlerbase.h"

/**
The PTP/IP protocol specifies two TCP Connections - 

1) Command or Data Connection
2) Event Connection.

This class is responsible for handling the event connection of the PTPIP protocol.
This class will control the socket used to send and receive the PTPIP events over 
the TCP/IP Connection .
*/

class CPTPIPEventHandler : public CPTPIPSocketHandlerBase
	{
public:
	static CPTPIPEventHandler* NewL(CPTPIPConnection& aConnectionHandler);
	~CPTPIPEventHandler();
	void SendInitAck(CPTPIPGenericContainer* aEvtAck);

	void SendEventL(const MMTPType& aData);
	void ReceiveEventL(MMTPType& aEvent);
	
private:
	CPTPIPEventHandler(CPTPIPConnection& aConnectionHandler);
	
	void SendDataCompleteL(TInt aError, const MMTPType& aSource);
	void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);
	
	TInt ParsePTPIPHeaderL();
	TBool HandleInitAck();
	
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER;    
	
	};

#endif /*CEVENTHANDLER_H_*/
