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

#ifndef CCOMMANDHANDLER_H_
#define CCOMMANDHANDLER_H_

#include "cptpipsockhandlerbase.h"
#include "cptpipconnection.h"

/**
The PTP/IP protocol specifies two TCP Connections - 

1) Command or Data Connection ( this class encapsulates this)
2) Event Connection.

This class is responsible for handling the command connection of the PTPIP protocol.
This class will control the socket used to send and receive the PTPIP command data
over the TCP/IP Connection .

*/
class CPTPIPCommandHandler : public CPTPIPSocketHandlerBase
	{
public :
	static CPTPIPCommandHandler* NewL(CPTPIPConnection& aConnection);
	~CPTPIPCommandHandler();
	
	void ReceiveCommandDataL(MMTPType& aItoRData);
	void ReceiveCommandRequestL(MMTPType& aRequest);
	void SendCommandDataL(const MMTPType& aRtoIData, TUint32 aTransactionId);
	void SendCommandL(const MMTPType& aResponse);
	
	TInt ParsePTPIPHeaderL();
	TBool HandleInitAck();
private :
	CPTPIPCommandHandler(CPTPIPConnection& aConnection);

	void SendDataCompleteL(TInt aError, const MMTPType& aSource);
	void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER;    
	
	};

#endif /*CCOMMANDHANDLER_H_*/
