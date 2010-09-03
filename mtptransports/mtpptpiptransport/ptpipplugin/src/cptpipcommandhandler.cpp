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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipcommandhandlerTraces.h"
#endif


const TUint32 KPTPIPHeaderSize = 8;
const TUint32 KPTPIPDataHeaderSize = 12;

/**
Creates the channel for commands. The base class constructl is called. 
*/
CPTPIPCommandHandler* CPTPIPCommandHandler::NewL(CPTPIPConnection& aConnection)
	{
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_NEWL_ENTRY );
	
	CPTPIPCommandHandler* self = new(ELeave) CPTPIPCommandHandler(aConnection);
	CleanupStack::PushL(self);
    self->ConstructL();
	CleanupStack::Pop();
	OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_NEWL_EXIT );
	return self;
	}

/**
Desctructor
*/
CPTPIPCommandHandler::~CPTPIPCommandHandler()
	{
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_CPTPIPCOMMANDHANDLER_ENTRY );
	OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_CPTPIPCOMMANDHANDLER_EXIT );
	}

/**
Constructor
*/
CPTPIPCommandHandler::CPTPIPCommandHandler(CPTPIPConnection& aConnection):
								CPTPIPSocketHandlerBase(aConnection, CActive::EPriorityStandard)
								
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPCOMMANDHANDLER_CPTPIPCOMMANDHANDLER_ENTRY );
	OstTraceFunctionExit0( DUP1_CPTPIPCOMMANDHANDLER_CPTPIPCOMMANDHANDLER_EXIT );
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
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_SENDCOMMANDDATAL_ENTRY );
    SendDataL(aRtoIData, aTransactionId);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_SENDCOMMANDDATAL_EXIT );
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
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_SENDCOMMANDL_ENTRY );
    SendDataL(aResponse, 0);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_SENDCOMMANDL_EXIT );
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
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_SENDDATACOMPLETEL_ENTRY );
    Connection().SendCommandChannelCompleteL(aError, aSource);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_SENDDATACOMPLETEL_EXIT );
    }


/**
Initiates the receiving of the info on the command channel, used for both commands
 and data. 
@param aData The buffer given by the framework in which the command request will be received from the initiator
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::ReceiveCommandRequestL(MMTPType& aRequest)
    {
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_RECEIVECOMMANDREQUESTL_ENTRY );
    ReceiveDataL(aRequest);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_RECEIVECOMMANDREQUESTL_EXIT );
    }

/**
Initiates the receiving of the info on the command channel, used for both commands
 and data. 
@param aItoRData The buffer given by the framework in which the command data will be received from the initiator
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CPTPIPCommandHandler::ReceiveCommandDataL(MMTPType& aItoRData)
    {
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_RECEIVECOMMANDDATAL_ENTRY );
	ReceiveDataL(aItoRData);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_RECEIVECOMMANDDATAL_EXIT );
    }


/**
Marks the completion of receiving data over the command socket, which was started 
by ReceiveCommandDataL. 
@param aError - The error if any returned by the sockethandler. (KErrNone if no errors)
@param aSink - The buffer in which data received from the initiator is returned to the framework
*/
void CPTPIPCommandHandler::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_RECEIVEDATACOMPLETEL_ENTRY );
	Connection().ReceiveCommandChannelCompleteL(aError, aSink);
    OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_RECEIVEDATACOMPLETEL_EXIT );
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
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_PARSEPTPIPHEADERL_ENTRY );
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
	OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_PARSEPTPIPHEADERL_EXIT );
	return type;

	}

/**
 * This function is implemented in the event handler class for 
 * handling the sending of the init acknowledgement. 
 */
TBool CPTPIPCommandHandler::HandleInitAck()
	{
	OstTraceFunctionEntry0( CPTPIPCOMMANDHANDLER_HANDLEINITACK_ENTRY );
	OstTraceFunctionExit0( CPTPIPCOMMANDHANDLER_HANDLEINITACK_EXIT );
	return EFalse;
	}
