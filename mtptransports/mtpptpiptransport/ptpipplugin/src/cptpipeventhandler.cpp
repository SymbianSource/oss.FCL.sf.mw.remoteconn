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
// ceventhandler.cpp
// 
//

/**
 @internalComponent
*/

#include "cptpipeventhandler.h"
#include "tptpipinitevtack.h"
#include "ptpippanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipeventhandlerTraces.h"
#endif


/**
Creates the channel for commands. The base class constructl is called. 
*/
CPTPIPEventHandler* CPTPIPEventHandler::NewL(CPTPIPConnection& aConnection)
	{
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_NEWL_ENTRY );
	
	CPTPIPEventHandler* self = new(ELeave) CPTPIPEventHandler(aConnection);
	CleanupStack::PushL(self);
    self->ConstructL();
	CleanupStack::Pop();
	OstTraceFunctionExit0( CPTPIPEVENTHANDLER_NEWL_EXIT );
	return self;
	}

/**
Destructor
*/
CPTPIPEventHandler::~CPTPIPEventHandler()
	{
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_CPTPIPEVENTHANDLER_ENTRY );
	OstTraceFunctionExit0( CPTPIPEVENTHANDLER_CPTPIPEVENTHANDLER_EXIT );
	}

/**
Constructor
*/
CPTPIPEventHandler::CPTPIPEventHandler(CPTPIPConnection& aConnection): 
	CPTPIPSocketHandlerBase(aConnection, CActive::EPriorityUserInput )
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPEVENTHANDLER_CPTPIPEVENTHANDLER_ENTRY );
	OstTraceFunctionExit0( DUP1_CPTPIPEVENTHANDLER_CPTPIPEVENTHANDLER_EXIT );
	}

/**
Sends the init ack packet, which is created in the connection class. 
@param aEvtAck The packet containing the event acknowledgement to be sent to initiator.
*/
void CPTPIPEventHandler::SendInitAck(CPTPIPGenericContainer* aEvtAck)
	{
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_SENDINITACK_ENTRY );
	iChunkStatus = aEvtAck->FirstReadChunk(iSendChunkData);
	iSendData.Set(iSendChunkData);
	iSocket.Send(iSendData,0,iStatus);
	SetState(EInitSendInProgress);
	SetActive();
	OstTraceFunctionExit0( CPTPIPEVENTHANDLER_SENDINITACK_EXIT );
	}

/**
Initiates the sending of the event data over the event channel. The base class
 implements the handling of the socket to send the actual data. 
@param aData The buffer containing the event which is created by the framework to send to the initiator
*/
void CPTPIPEventHandler::SendEventL(const MMTPType& aEvent)
    {
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_SENDEVENTL_ENTRY );
	
	// We need to stop listening, and send the event. 
	Cancel();
	
    SendDataL(aEvent, 0);
    OstTraceFunctionExit0( CPTPIPEVENTHANDLER_SENDEVENTL_EXIT );
    }

/**
Signals the completion of sending data over the socket, which was started by 
SendEventL. It is called by the base sockethandler and in turn informs the 
connection.
@param aError - The error if any returned by the sockethandler. (KErrNone if no errors)
@param aSource - The event buffer which had been given by the framework to send to initiator.
*/
void CPTPIPEventHandler::SendDataCompleteL(TInt aError, const MMTPType& aSource)
    {
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_SENDDATACOMPLETEL_ENTRY );
    Connection().SendEventCompleteL(aError, aSource);
    OstTraceFunctionExit0( CPTPIPEVENTHANDLER_SENDDATACOMPLETEL_EXIT );
    }

/**
Initiates the receiving of the event data on the event channel. 
@param aEvent The buffer containing the event received from the initiator
*/
void CPTPIPEventHandler::ReceiveEventL(MMTPType& aEvent)
    {
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_RECEIVEEVENTL_ENTRY );
    ReceiveDataL(aEvent);
    OstTraceFunctionExit0( CPTPIPEVENTHANDLER_RECEIVEEVENTL_EXIT );
    }

/**
Marks the completion of receiving data over the event channel, which was started 
by ReceiveEventL. It is called by the base sockethandler and in turn informs the
connection.
@param aError - The error if any returned by the sockethandler. (KErrNone if no errors)
@param aSink - The event buffer in which an event has been received from the initiator
*/
void CPTPIPEventHandler::ReceiveDataCompleteL(TInt aError, MMTPType& aSink)
    {
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_RECEIVEDATACOMPLETEL_ENTRY );
    Connection().ReceiveEventCompleteL(aError, aSink); 
    OstTraceFunctionExit0( CPTPIPEVENTHANDLER_RECEIVEDATACOMPLETEL_EXIT );
    }

/**
Parses the PTPIP header, gets and validates that the packet type is correct 
and sets the value of the packet length.
@return Type The PTPIP packet type received ie event or cancel. In case of invalid type, it returns 0
*/   
TInt CPTPIPEventHandler::ParsePTPIPHeaderL()
	{
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_PARSEPTPIPHEADERL_ENTRY );
	
	TUint32 type = Connection().ValidateAndSetEventPayloadL();
	iPTPPacketLength = Connection().EventContainer()->Uint32L(CPTPIPGenericContainer::EPacketLength);
	
	OstTraceFunctionExit0( CPTPIPEVENTHANDLER_PARSEPTPIPHEADERL_EXIT );
	return type;
	}

/**
Called during the PTP connection establishment phase to mark the completion 
of sending the init ack to the initiator
@return Flag stating that the ini has been sent successfully.
*/
TBool CPTPIPEventHandler::HandleInitAck()
	{
	OstTraceFunctionEntry0( CPTPIPEVENTHANDLER_HANDLEINITACK_ENTRY );
	TBool isHandled(EFalse);
	
	if (iState == EInitSendInProgress)
		{
		//Now signal the connection, set its state and set it to active.
		Connection().SetConnectionState(CPTPIPConnection::EInitialisationComplete);
		Connection().CompleteSelf(iStatus.Int());
		iState = EIdle;
		isHandled = ETrue;
		}
	OstTraceFunctionExit0( CPTPIPEVENTHANDLER_HANDLEINITACK_EXIT );
	return isHandled;
	}



	
	

