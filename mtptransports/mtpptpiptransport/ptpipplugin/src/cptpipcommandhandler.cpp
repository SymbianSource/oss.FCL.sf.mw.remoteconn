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

#include "cptpipcommandhandler.h"

const TUint32 KPTPIPHeaderSize = 8;
const TUint32 KPTPIPDataHeaderSize = 12;

__FLOG_STMT(_LIT8(KComponent,"CCommandHandler");) 

/**
Creates the channel for commands. The base class constructl is called. 
*/
CPTPIPCommandHandler* CPTPIPCommandHandler::NewL(CPTPIPConnection& aConnection)
	{
	
	CPTPIPCommandHandler* self = new(ELeave) CPTPIPCommandHandler(aConnection);
	CleanupStack::PushL(self);
#ifdef __FLOG_ACTIVE    
    self->ConstructL(KComponent);
#else
    self->ConstructL();
#endif	
	CleanupStack::Pop();
	return self;
	}

/**
Desctructor
*/
CPTPIPCommandHandler::~CPTPIPCommandHandler()
	{
	__FLOG(_L8("Destructor - Entry"));
	__FLOG(_L8("Destructor - Exit"));
	}

/**
Constructor
*/
CPTPIPCommandHandler::CPTPIPCommandHandler(CPTPIPConnection& aConnection):
								CPTPIPSocketHandlerBase(aConnection, CActive::EPriorityStandard)
								
	{
	}

/**
Initiates the sending of the command data. The base class implements the handling
of the socket to send the actual data. 
@param aRtoIData The buffer containing data populated by the framework to be sent to the initiator
@param aTransactionId The id of the current ongoing transaction.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::SendCommandDataL(const MMTPType& aRtoIData, TUint32 aTransactionId)
    {
	__FLOG(_L8("SendCommandDataL - Entry"));
    SendDataL(aRtoIData, aTransactionId);
	__FLOG(_L8("SendCommandDataL - Exit"));
    }
    
/**
Initiates the sending of the command data. The base class implements the handling
of the socket to send the actual data. 
@param aData The buffer containing the response populated by the framework to be sent to the initiator 
@param aTransactionId The id of the current ongoing transaction.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::SendCommandL(const MMTPType& aResponse)
    {
	__FLOG(_L8("SendCommandDataL - Entry"));
    SendDataL(aResponse, 0);
	__FLOG(_L8("SendCommandDataL - Exit"));
    }

/**
Signals the completion of sending data over the socket, which was started by 
SendCommandDataL. Its called by the base sockethandler and in turn informs the 
connection.
@param aError - The error if any returned by the sockethandler. (KErrNone if no errors)
@param aSource - The buffer of data which had been given by the framework to send to initiator.
*/
void CPTPIPCommandHandler::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
	__FLOG(_L8("SendDataCompleteL - Entry"));
    Connection().SendCommandChannelCompleteL(aError, aSource);
	__FLOG(_L8("SendDataCompleteL - Exit"));
    }


/**
Initiates the receiving of the info on the command channel, used for both commands
 and data. 
@param aData The buffer given by the framework in which the command request will be received from the initiator
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::ReceiveCommandRequestL(MMTPType& aRequest)
    {
	__FLOG(_L8("ReceiveCommandDataL - Entry"));
    ReceiveDataL(aRequest);
	__FLOG(_L8("ReceiveCommandDataL - Exit"));
    }

/**
Initiates the receiving of the info on the command channel, used for both commands
 and data. 
@param aItoRData The buffer given by the framework in which the command data will be received from the initiator
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::ReceiveCommandDataL(MMTPType& aItoRData)
    {
	__FLOG(_L8("ReceiveCommandDataL - Entry"));
	ReceiveDataL(aItoRData);
	__FLOG(_L8("ReceiveCommandDataL - Exit"));
    }


/**
Marks the completion of receiving data over the command socket, which was started 
by ReceiveCommandDataL. 
@param aError - The error if any returned by the sockethandler. (KErrNone if no errors)
@param aSink - The buffer in which data received from the initiator is returned to the framework
*/
void CPTPIPCommandHandler::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
	__FLOG(_L8("ReceiveDataCompleteL - Entry"));
	
	Connection().ReceiveCommandChannelCompleteL(aError, aSink);
	__FLOG(_L8("ReceiveDataCompleteL - Exit"));
    }
    
/**
Validates if the current payload in the container is the correct type to hold the
data that is coming in. 

For command container, the payloads can be request, response and start data. 
For the data container, the payload is already set from the MTP framework. 

@return type The ptpip packet type, like request, cancel etc 
*/   
TInt CPTPIPCommandHandler::ParsePTPIPHeaderL()
	{
	__FLOG(_L8("ValidateAndSetPayload - Entry"));
	TUint32 type = 0;
	
	// If this is a request or event, then the first chunk will have 8 bytes
    if (KPTPIPHeaderSize == iReceiveChunkData.MaxLength())
    	{
    	iPTPPacketLength = Connection().CommandContainer()->Uint32L(CPTPIPGenericContainer::EPacketLength);
    	type = Connection().ValidateAndSetCommandPayloadL();
    	}
    // if this is a data header, then the first chunk will have 12 bytes. 
    else if (KPTPIPDataHeaderSize == iReceiveChunkData.MaxLength()) 
    	{
       	iPTPPacketLength = Connection().DataContainer()->Uint32L(CPTPIPDataContainer::EPacketLength);
    	type = Connection().ValidateDataPacketL();
    	}
	__FLOG(_L8("ValidateAndSetPayload - Exit"));	
	return type;

	}

/**
 * This function is implemented in the event handler class for 
 * handling the sending of the init acknowledgement. 
 */
TBool CPTPIPCommandHandler::HandleInitAck()
	{
	return EFalse;
	}
