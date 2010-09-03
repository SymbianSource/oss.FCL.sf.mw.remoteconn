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

// System includes
#include "e32base.h"
#include "ptpippanic.h"
#include "e32property.h" 

// MTP includes
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperesponse.h>

// Plugin includes
#include "mtpdebug.h"
#include "cptpipcommandhandler.h"
#include "cptpipeventhandler.h"
#include "cptpipconnection.h"
#include "ptpipsocketpublish.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipconnectionTraces.h"
#endif
 

// File type constants.
const TInt KMTPNullChunkSize(0x00020000); // 100KB
const TUint32 KPTPIPDataHeaderSize = 12;  // Size of len, type and tran id. 
#define UNUSED_VAR(a) (a) = (a)

/**
 PTPIP device class connection factory method.
 @param aConnectionMgr The MTP connection manager interface.
 @return A pointer to an PTPIP device class connection. Ownership IS transfered.
 @leave One of the system wide error codes, if a processing failure occurs.
 */
CPTPIPConnection* CPTPIPConnection::NewL(MMTPConnectionMgr& aConnectionMgr )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_NEWL_ENTRY );
	CPTPIPConnection* self = new(ELeave) CPTPIPConnection(aConnectionMgr);
	CleanupStack::PushL (self );
	self->ConstructL ( );
	CleanupStack::Pop (self );
	OstTraceFunctionExit0( CPTPIPCONNECTION_NEWL_EXIT );
	return self;
	}

/**
 Destructor
 */
CPTPIPConnection::~CPTPIPConnection( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_CPTPIPCONNECTION_ENTRY );
	StopConnection ( );

	// Delete all the handlers which will close the sockets.
	delete iCommandHandler;
	delete iEventHandler;

	delete iPTPIPCommandContainer;
	delete iPTPIPDataContainer;
	delete iPTPIPEventContainer;

        if (iProtocolLayer != NULL)
           {
           iProtocolLayer->Unbind(*this);
           }
	
	iNullBuffer.Close();
	OstTraceFunctionExit0( CPTPIPCONNECTION_CPTPIPCONNECTION_EXIT );
	}

/**
 Second phase constructor
 @leave One of the system wide error codes, if a processing failure occurs.
 */
void CPTPIPConnection::ConstructL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_CONSTRUCTL_ENTRY );

	// Construct the Command and event handlers
	iCommandHandler = CPTPIPCommandHandler::NewL (*this );
	iEventHandler = CPTPIPEventHandler::NewL (*this );

	iPTPIPCommandContainer = CPTPIPGenericContainer::NewL ( );
	iPTPIPDataContainer = CPTPIPDataContainer::NewL ( );
	iPTPIPEventContainer = CPTPIPGenericContainer::NewL ( );

	// Transfer the sockets
	TransferSocketsL();

	SetConnectionState (EInitialising );
	CompleteSelf (KErrNone );

	OstTraceFunctionExit0( CPTPIPCONNECTION_CONSTRUCTL_EXIT );
	}

/**
 Constructor
 */
CPTPIPConnection::CPTPIPConnection(MMTPConnectionMgr& aConnectionMgr ) :
									CActive(EPriorityStandard), 
									iConnectionMgr(&aConnectionMgr)
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPCONNECTION_CPTPIPCONNECTION_ENTRY );
	CActiveScheduler::Add (this );
	OstTraceFunctionExit0( DUP1_CPTPIPCONNECTION_CPTPIPCONNECTION_EXIT );
	}

//
// MMTPTransportConnection functions
//

/**
 Binds the protocol layer. 
 @param aProtocol - The connectionProtocol provides the SPI or the observer
 for the communication to happen from the transport to the framework. 
 */
void CPTPIPConnection::BindL(MMTPConnectionProtocol& aProtocol )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_BINDL_ENTRY );
	iProtocolLayer = &aProtocol;
	OstTraceFunctionExit0( CPTPIPCONNECTION_BINDL_EXIT );
	}

/**
 Returns the protocol layer.
 @return The SPI or the observer protocol layer
 */
MMTPConnectionProtocol& CPTPIPConnection::BoundProtocolLayer( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_BOUNDPROTOCOLLAYER_ENTRY );
	__ASSERT_ALWAYS(iProtocolLayer, Panic(EPTPIPBadState));
	OstTraceFunctionExit0( CPTPIPCONNECTION_BOUNDPROTOCOLLAYER_EXIT );
	return *iProtocolLayer;
	}

/**
 Close the connection, stop all data transfer activity and and wait for the host 
 to issue a Device Reset Request.
 */
void CPTPIPConnection::CloseConnection( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_CLOSECONNECTION_ENTRY );
	StopConnection ( );
	OstTraceFunctionExit0( CPTPIPCONNECTION_CLOSECONNECTION_EXIT );
	}

/**
 A transaction is one set of request, data transfer and response phases. 
 The fw calls this to mark the end of an MTP transaction. 
 */
void CPTPIPConnection::TransactionCompleteL(const TMTPTypeRequest& /*aRequest*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_TRANSACTIONCOMPLETEL_ENTRY );
	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_TRANSACTIONCOMPLETEL, "******** Transaction Complete **************" );

	SetTransactionPhase (EIdlePhase );

	// Clear the cancel flag.
	iCancelOnCommandState = ECancelNotReceived;
	iCancelOnEventState = ECancelNotReceived;

	// Again start listening for the command request. 
	InitiateCommandRequestPhaseL( );

	OstTraceFunctionExit0( CPTPIPCONNECTION_TRANSACTIONCOMPLETEL_EXIT );
	}

/**
 Called by the fw to indicate that protocol layer is not using this transport anymore 
 and so we will not have any callback/SPI funtions to the fw any more
 */
void CPTPIPConnection::Unbind(MMTPConnectionProtocol& /*aProtocol*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_UNBIND_ENTRY );
	__ASSERT_DEBUG(iProtocolLayer, Panic(EPTPIPBadState));
	// Protocol will no longer be bound to the transport
	iProtocolLayer = NULL;
	OstTraceFunctionExit0( CPTPIPCONNECTION_UNBIND_EXIT );
	}

/**
 Not used
 */
TAny* CPTPIPConnection::GetExtendedInterface(TUid /*aInterfaceUid*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_GETEXTENDEDINTERFACE_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_GETEXTENDEDINTERFACE_EXIT );
	return NULL;
	}

/*
 * return PTPIP transport implementation UID
 */
TUint CPTPIPConnection::GetImplementationUid()
    {
    OstTraceFunctionEntry0( CPTPIPCONNECTION_GETIMPLEMENTATIONUID_ENTRY );
    OstTraceFunctionExit0( CPTPIPCONNECTION_GETIMPLEMENTATIONUID_EXIT );
    return KMTPPTPIPTransportImplementationUid;
    }

//
// Active Object functions
//

/**
 Used to cancel the outstanding requests, and reset internal states as appropriate. 
 */
void CPTPIPConnection::DoCancel( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_DOCANCEL_ENTRY );

	iCommandHandler->Cancel( );
	iEventHandler->Cancel( );

	SetConnectionState(ECancelled );

	OstTraceFunctionExit0( CPTPIPCONNECTION_DOCANCEL_EXIT );
	}

/**
The connection behaves as an active object during the PTPIP connection establishment phase.
Subsequently, it behaves as a normal object whose functions are invoked by the MTP framework
or by the SocketHandler on sending/ receiving data. 

Thus the runl is not hit after the initial connection establishment. 
 */
void CPTPIPConnection::RunL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RUNL_ENTRY );
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_RUNL, "Current State is %d, and status is %d", iState, iStatus.Int() );
	

	if(iStatus != KErrNone )
		{
		CloseConnection( );
		}
	else 
		{

		switch(iState )
		{
		// ConstructL is complete, 
		// the PTPIP connection establishment will be complete after the init ack is sent.
		case EInitialising:
			SendInitAckL( );
			break;

			// Now the PTPIP connection has been established. 
		case EInitialisationComplete:
			SetConnectionState(EStartListening );
			CompleteSelf(KErrNone );
			break;

			// Start listeing for requests on the 2 channels
		case EStartListening:
			InitiateEventRequestPhaseL( );
			InitiateCommandRequestPhaseL( );
			break;

		default:
			OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_RUNL, "PTPIP ERROR: Invalid  connection state" );
			
			Panic(EPTPIPBadState );
			break;
		}
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_RUNL_EXIT );
	}

/**
 Called when an error occurs in the RunL 
 */
TInt CPTPIPConnection::RunError(TInt aError )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RUNERROR_ENTRY );
	
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_RUNERROR, "PTPIP ERROR: Error received is %d", aError );
	

	// Cancel all the outstanding requests.
	Cancel( );

	// Stop the connection, if necessary.
	StopConnection( );

	OstTraceFunctionExit0( CPTPIPCONNECTION_RUNERROR_EXIT );
	return KErrNone;
	}

//
// Receive data functions
//

/**
 Called internally to recevive the commands over the command channel. 
 This will invoke the command handler 
 to listen on the socket and get the data into the buffers passed. 
 */
void CPTPIPConnection::InitiateCommandRequestPhaseL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_INITIATECOMMANDREQUESTPHASEL_ENTRY );
	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_INITIATECOMMANDREQUESTPHASEL, "******** Phase 1 - Request **************" );
	
	// Set current state to request phase
	SetTransactionPhase(ERequestPhase );

	// The PTPIP data buffer is a member of connection.  
	// Since we are expecting a request now, set the payload to request type
	iPTPIPRequestPayload.Reset( );
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPRequestPayload );

	// Call the CommandHandler to get the request in the container
	iCommandHandler->ReceiveCommandRequestL(*iPTPIPCommandContainer );

	OstTraceFunctionExit0( CPTPIPCONNECTION_INITIATECOMMANDREQUESTPHASEL_EXIT );
	}

/** 
 Called to indicate completion of receive of data started by InitiateCommandRequestPhaseL.
 There is no point in reporting the error up to the f/w since no transaction has started.
 USB does nothing at this stage. In PTP , the connection is closed.
 @param aError - The error if any, received from socket.
 */
void CPTPIPConnection::ReceiveCommandCompleteL(TInt aError )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL_ENTRY );

	if(KErrNone != aError )
		{
		OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL, "PTPIP Error: Received error=%d in request phase, closing  connection", aError );
		
		CloseConnection( );
		}
	else if(ValidateTransactionPhase(ERequestPhase ) )
		{
		// Request block received.
		TPTPIPTypeRequestPayload* pRequest = static_cast<TPTPIPTypeRequestPayload*>(iPTPIPCommandContainer->Payload());

		TUint16 op(pRequest->Uint16(TPTPIPTypeRequestPayload::EOpCode ));
		TUint32	tran(pRequest->Uint32(TPTPIPTypeRequestPayload::ETransactionId ));
		TUint32 sessionId = KMTPSessionNone;
		OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL, "Command block received with op = 0x%04X ,transId = %d", static_cast<TUint32>(op), tran );
		
		// Reset the iMTPRequest.
		iMTPRequest.Reset( );

		// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
		iMTPRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, op );
		iMTPRequest.SetUint32(TMTPTypeRequest::ERequestTransactionID, tran );

		// Set SessionID.
		if(op == EMTPOpCodeOpenSession )
			{
			OstTrace0( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL, "Processing OpenSession request" );
			
			}
		else if(op == EMTPOpCodeCloseSession || op == EMTPOpCodeResetDevice )
			{
			OstTrace0( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL, "Processing CloseSession or the ResetDevice request" );
			
			// Force CloseSession requests to be processed outside an active session. 
			// ResetDevice currently behaves the same way as CloseSession. 
			iMTPRequest.SetUint32(TMTPTypeRequest::ERequestParameter1,
					iMTPSessionId );
			}
		else
			{
			sessionId = iMTPSessionId;
			OstTrace1( TRACE_NORMAL, DUP4_CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL, "Processing general request on session %d", sessionId );
			
			}
		
		iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID,sessionId );

		// Set Parameter 1 .. Parameter 5.
		pRequest->CopyOut(iMTPRequest, TMTPTypeRequest::ERequestParameter1,	TMTPTypeRequest::ERequestParameter5 );
		pRequest->Reset( );

		// Notify the protocol layer.
		
		BoundProtocolLayer().ReceivedRequestL(iMTPRequest );

		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVECOMMANDCOMPLETEL_EXIT );
	}

/**
 Called to get data over the command channel,( in turn its called by f/ws ReceiveData)
 @param aData - The buffer provided by the F/w in which to receive data.
 */
void CPTPIPConnection::ReceiveCommandDataL(MMTPType& aData )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVECOMMANDDATAL_ENTRY );

	iRecvData = 0;
	iTotalRecvData = 0;
	
	// First we will receive a PTPIP start data packet which is stored in the commandContainer
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPStartDataPayload );

	// The mtp buffer is passed from the framework and will be passed on as the payload.
	// in the next ptpip data packet.
	iPTPIPDataContainer->SetPayloadL(&aData );
	iCommandHandler->ReceiveCommandDataL(*iPTPIPCommandContainer );

	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVECOMMANDDATAL_EXIT );
	}

/**
 Called to indicate completion of receive of data started by ReceiveCommandDataL.
 Any errors received are sent back up to the framework. 
 */
void CPTPIPConnection::ReceiveCommandDataCompleteL(TInt aError )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVECOMMANDDATACOMPLETEL_ENTRY );
	if(ValidateTransactionPhase(EDataIToRPhase ) )
		{
		// Data block received, notify the protocol layer.
		iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
		BoundProtocolLayer().ReceiveDataCompleteL(aError, *iPTPIPDataContainer->Payload(), iMTPRequest );
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVECOMMANDDATACOMPLETEL_EXIT );
	}

/**
 Called by the command handler to indicate completion of receive on the command channel.
 Now check whether it was commands or data completion and call the appropriate function. 
 */

void CPTPIPConnection::ReceiveCommandChannelCompleteL(TInt aError, MMTPType& /*aSource*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL_ENTRY );
	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "******** Receiving 1 ptpip packet on command/data channel complete **************" );
	
	HandleTCPError(aError );
	TUint32	typeCommand = iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	TUint32	typeData = iPTPIPDataContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "type on the command buffer is %d and type on the data buffer is %d", typeCommand, typeData );
	


	switch (typeCommand)
	{
	case EPTPIPPacketTypeOperationRequest:
		ReceiveCommandCompleteL(aError );
		break;
		
	case EPTPIPPacketTypeStartData:
		if(aError != KErrNone )
			{
			ReceiveCommandCompleteL(aError );
			}
		else
			{
			// Save the total data expected. 
			iTotalRecvData =(static_cast<TPTPIPTypeStartDataPayload*>(iPTPIPCommandContainer->Payload()))->Uint64(TPTPIPTypeStartDataPayload::ETotalSize );
			OstTrace1( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "Total data to receive in data phase is %ld", iTotalRecvData );
			
			
			//reset the command container 
			iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, 0 );
			iCommandHandler->ReceiveCommandDataL(*iPTPIPDataContainer );
			}
		break;
		
	case EPTPIPPacketTypeCancel:
		{
		TMTPTypeInt32* pTransId;
		pTransId = static_cast<TMTPTypeInt32*>(iPTPIPCommandContainer->Payload());
		iCancelOnCommandState = ECancelCmdReceived;
		HandleCommandCancelL(pTransId->Value());
		}
		break;
		
		
	default:
		// No known types came on the command container, now check the data container.
		switch (typeData)
		{
		case EPTPIPPacketTypeData:
			// One PTPIP packet has been received. We will now continue getting the next PTPIP packets. 
			iRecvData += iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::EPacketLength );
			iRecvData -= KPTPIPDataHeaderSize; // The header size is not included in the total size sent. 
			
			//If more data is expected,then set the flag to use the current offset
			//in the chunk
			if (iRecvData < iTotalRecvData)
			{
			iCommandHandler->iUseOffset = ETrue;	
				
			}
			
			OstTrace1( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "Data received so far in data phase is %ld", iRecvData );
			
			if(iRecvData <= iTotalRecvData )
				{
				iCommandHandler->ReceiveCommandDataL(*iPTPIPDataContainer );
				}
			else
				{
                TBuf<8> recvData;
                recvData.Num( iRecvData, EDecimal );
                TBuf<8> totalRecvData;
                totalRecvData.Num( iTotalRecvData, EDecimal );
				OstTraceExt2( TRACE_NORMAL, DUP5_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "PTPIP ERROR: The data received so far= %S is more than expected data = %S",
				        recvData, totalRecvData );
				
				CloseConnection( );
				}			
			break;
		
		case EPTPIPPacketTypeEndData:
			iRecvData += iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::EPacketLength );
			iRecvData -= KPTPIPDataHeaderSize; // The header size is not included in the total size sent. 
			
			iCommandHandler->iUseOffset = EFalse;
			
			OstTrace1( TRACE_NORMAL, DUP6_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "Data received so far in data phase is %ld", iRecvData );
			
			if(iTotalRecvData == iRecvData )
				{
				ReceiveCommandDataCompleteL(aError );
				}
			else
				{
                TBuf<8> recvData;
                recvData.Num( iRecvData, EDecimal );
                TBuf<8> totalRecvData;
                totalRecvData.Num( iTotalRecvData, EDecimal );
				OstTraceExt2( TRACE_NORMAL, DUP4_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "PTPIP ERROR: The data received so far= %S is not equal to expected data = %S", recvData, totalRecvData );
				
				CloseConnection( );
				}
			break;
		
		case EPTPIPPacketTypeCancel:
			{
			TUint32 transId = iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::ETransactionId);
			iCancelOnCommandState = ECancelCmdReceived;
			iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
			HandleCommandCancelL(transId);
			}
			break;
		
		default:
			OstTraceExt2( TRACE_NORMAL, DUP7_CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL, "PTPIP ERROR: Unexpected type received,  data container = %d, command container =%d ", typeData, typeCommand );
			CloseConnection( );
			break;
		} // switch data
	} // switch command

	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVECOMMANDCHANNELCOMPLETEL_EXIT );
	}

/**
 Called by MTP fw to get data from the transport.
 */
void CPTPIPConnection::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVEDATAL_ENTRY );
	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_RECEIVEDATAL, "******** Phase 2 - Data I to R **************" );
	
	SetTransactionPhase(EDataIToRPhase );
	ReceiveCommandDataL(aData );
	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVEDATAL_EXIT );
	}

/**
 Called by MTP fw to cancel the receiving data.
 */
void CPTPIPConnection::ReceiveDataCancelL(const TMTPTypeRequest& /*aRequest*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVEDATACANCELL_ENTRY );

	iCommandHandler->CancelReceiveL(KErrCancel );
	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVEDATACANCELL_EXIT );
	}

/**
 This will invoke the event handler to listen on the socket and get the events 
 into the buffers passed. 
 */
void CPTPIPConnection::InitiateEventRequestPhaseL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_INITIATEEVENTREQUESTPHASEL_ENTRY );

	// Initialise the PTP buffers to get the data. 
	iPTPIPEventPayload.Reset( );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, NULL );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength,	NULL );
	iPTPIPEventContainer->SetPayloadL(&iPTPIPEventPayload );

	// Call the EventHandler
	iEventHandler->ReceiveEventL(*iPTPIPEventContainer );
	OstTraceFunctionExit0( CPTPIPCONNECTION_INITIATEEVENTREQUESTPHASEL_EXIT );
	}

/**
 Signals the completion of receving an event. 
 */
void CPTPIPConnection::ReceiveEventCompleteL(TInt aError, MMTPType& /*aSource*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL_ENTRY );

	TUint32	type = iPTPIPEventContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL, "Error value is %d and type is %d", static_cast<TInt32>(aError), static_cast<TInt32>(type));
	

	if(KErrNone != aError )
		{
		OstTrace1( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL, "PTPIP Error: Received error=%d in request phase, closing  connection", aError );
		
		CloseConnection( );
		}
	else
		{
		// For a probe request, we just send a probe response and don't notify the MTP f/w. 
		if( type == EPTPIPPacketTypeProbeRequest )
			{
			OstTrace0( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL, "Received a probe request, sending back a probe response" );
			
			// Send the response, 
			iPTPIPEventContainer->SetPayloadL(NULL );
			iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength, iPTPIPEventContainer->Size( ) );
			iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, EPTPIPPacketTypeProbeResponse );

			// Call the EventHandler
			iEventHandler->SendEventL(*iPTPIPEventContainer );
			}
		else if(type == EPTPIPPacketTypeCancel )
			{			
			iCancelOnEventState = ECancelCmdReceived;
			HandleEventCancelL(); 
			}
		else if(type == EPTPIPPacketTypeEvent )
			{
			// Request block received.
			TPTPIPTypeResponsePayload* pEvent = static_cast<TPTPIPTypeResponsePayload*>(iPTPIPEventContainer->Payload());
			TUint16	op(pEvent->Uint16(TPTPIPTypeResponsePayload::EResponseCode ));
			OstTrace1( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL, "Event block 0x%04X received", op );
			
			

			// Reset the iMTPRequest.
			iMTPEvent.Reset( );

			// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
			iMTPEvent.SetUint16(TMTPTypeEvent::EEventCode, op );
			iMTPEvent.SetUint32(TMTPTypeEvent::EEventSessionID,	iMTPSessionId );
			iMTPEvent.SetUint32(TMTPTypeEvent::EEventTransactionID,	pEvent->Uint32(TPTPIPTypeResponsePayload::ETransactionId ) );

			// Set Parameter 1 .. Parameter 3.
			pEvent->CopyOut(iMTPRequest,TMTPTypeResponse::EResponseParameter1, TMTPTypeResponse::EResponseParameter3 );
			pEvent->Reset( );

			BoundProtocolLayer().ReceivedEventL(iMTPEvent );
			InitiateEventRequestPhaseL( );
			}
		// If unexpected data is received , its ignored in the release mode. 
		else
			{
			OstTrace0( TRACE_NORMAL, DUP4_CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL, "PTPIP ERROR : Unknown event type received, ignoring it." );
			
			__ASSERT_DEBUG(type, Panic(EPTPIPBadState));
			}
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_RECEIVEEVENTCOMPLETEL_EXIT );
	}

//
// Send data functions
//
/**
 Called by the MTP f/w to send the response to the initiator.
 Also called from this connection itself to send the cancel response. 
 */
void CPTPIPConnection::SendResponseL(const TMTPTypeResponse& aResponse,	const TMTPTypeRequest& aRequest )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDRESPONSEL_ENTRY );

	// Update the transaction state.
	SetTransactionPhase(EResponsePhase );
	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_SENDRESPONSEL, "******** Phase 3 - Response **************" );
	
	
	if(iCancelOnCommandState  )
		{
		OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_SENDRESPONSEL, "Cancel has been received from initiator, so send own response" );
		
		SendCancelResponseL(aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID ));
		}
	else
		{
		TUint16	opCode(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode ));
		TUint16 rspCode(aResponse.Uint16(TMTPTypeResponse::EResponseCode ));
		OstTraceExt2( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_SENDRESPONSEL, "ResponseCode = 0x%04X, Operation Code = 0x%04X", rspCode, opCode );
		

		if((opCode == EMTPOpCodeOpenSession) &&(rspCode == EMTPRespCodeOK) )
			{
			// An session has been opened. Record the active SessionID.
			iMTPSessionId = aRequest.Uint32(TMTPTypeRequest::ERequestParameter1 );
			OstTrace1( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_SENDRESPONSEL, "Processing OpenSession response, SessionID = %d", iMTPSessionId );
			
			}
		else if(((opCode == EMTPOpCodeCloseSession) ||
				(opCode == EMTPOpCodeResetDevice))&&(rspCode == EMTPRespCodeOK) )
			{
			// An session has been closed. Clear the active SessionID.        
			OstTrace1( TRACE_NORMAL, DUP4_CPTPIPCONNECTION_SENDRESPONSEL, "Processing CloseSession or ResetDevice response, SessionID = %d", iMTPSessionId );
			
			iMTPSessionId = KMTPSessionNone;
			}

		//Setup the parameter block payload dataset. Note that since this is a 
		//variable length dataset, it must first be reset.

		iPTPIPResponsePayload.Reset( );
		TUint numberOfNullParam = TMTPTypeResponse::EResponseParameter5 - TMTPTypeResponse::EResponseParameter1 + 1;

		iPTPIPResponsePayload.CopyIn(aResponse,	
		                             TMTPTypeResponse::EResponseParameter1, TMTPTypeResponse::EResponseParameter5, 
		                             ETrue,
		                             numberOfNullParam);
		iPTPIPResponsePayload.SetUint16(TPTPIPTypeResponsePayload::EResponseCode, rspCode );
		iPTPIPResponsePayload.SetUint32(TPTPIPTypeResponsePayload::ETransactionId, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID ) );

		// Setup the command container.
		iPTPIPCommandContainer->SetPayloadL(const_cast<TPTPIPTypeResponsePayload*>(&iPTPIPResponsePayload) );
		iPTPIPCommandContainer->SetUint32L(	CPTPIPGenericContainer::EPacketLength, static_cast<TUint32>(iPTPIPCommandContainer->Size()) );
		iPTPIPCommandContainer->SetUint32L(	CPTPIPGenericContainer::EPacketType, EPTPIPPacketTypeOperationResponse );

		// Initiate the command send sequence.
		OstTraceExt2( TRACE_NORMAL, DUP5_CPTPIPCONNECTION_SENDRESPONSEL, "Sending response 0x%04X(%d bytes)",
		        static_cast<TUint32>(iPTPIPResponsePayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode)),
		        iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketLength));
		
		iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDRESPONSEL_EXIT );
	}

/**
 Send response complete
 */
void CPTPIPConnection::SendCommandCompleteL(TInt aError )
	{
    OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDCOMMANDCOMPLETEL_ENTRY );


	if(ValidateTransactionPhase(EResponsePhase ) )
		{
		BoundProtocolLayer().SendResponseCompleteL( aError,
		                                            *static_cast<TMTPTypeResponse*>(iPTPIPCommandContainer->Payload()), 
		                                            iMTPRequest );
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDCOMMANDCOMPLETEL_EXIT );
	}

/**
 Send data complete
 */
void CPTPIPConnection::SendCommandDataCompleteL(TInt aError )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDCOMMANDDATACOMPLETEL_ENTRY );

	if(ValidateTransactionPhase(EDataRToIPhase ) )
		{
		BoundProtocolLayer().SendDataCompleteL(aError, *iPTPIPDataContainer->Payload(), iMTPRequest );
		}
	SetConnectionState(EDataSendFinished);
	iPTPIPDataContainer->SetPayloadL(NULL );

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDCOMMANDDATACOMPLETEL_EXIT );
	}

/**
 Called by the command handler to indicate completion of send on the command channel.
 Now check whether it was commands or data completion and call the appropriate function. 
 */
void CPTPIPConnection::SendCommandChannelCompleteL(TInt aError, const MMTPType& /*aSource*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDCOMMANDCHANNELCOMPLETEL_ENTRY );

	// Now see whether we have completed getting data or commands, and call the appropriate function.
	TUint typeCommand = iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	TUint typeData = iPTPIPDataContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_SENDCOMMANDCHANNELCOMPLETEL, "type on the command buffer is %d and type on the data buffer is %d", typeCommand, typeData );
	
	
	
	// if we have received a cancel on the event channel then terminate the current sending
	// and handle the receiving and sending of cancel on the channel.
	if (iCancelOnEventState && !iCancelOnCommandState)
		{
		HandleCancelDuringSendL(); 	
		}
	
	// A command has been sent
	else if((EPTPIPPacketTypeOperationResponse == typeCommand) &&(0 == typeData) )
		{
		// If this response was a cancel, then we don't inform the framework, as it was internally generated
		if(iCancelOnCommandState )
			{
			iCancelOnCommandState = ECancelCmdHandled;
			SendCommandCompleteL(aError );
			HandleCommandCancelCompleteL( );
			}
		// Tell the framework that a command has been received. 
		else
			{
			iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, 0 );
			SendCommandCompleteL(aError );
			}
		}

	// Tell the connection that data has been sent.
	else if((EPTPIPPacketTypeEndData == typeData) &&(0 == typeCommand ) )
			{
			iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
			SendCommandDataCompleteL(aError );
			}

	// We sent the start data packet, and we now have to send the actual data.  
	else if((EPTPIPPacketTypeStartData == typeCommand) &&(EDataSendInProgress == iState ) )
			{
			iPTPIPCommandContainer->SetUint32L(	CPTPIPGenericContainer::EPacketType, 0 );
			SendDataPacketL( );
			}

	// Any other type indicates a programming error, and a panic is raised. 
	else
		{
		OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_SENDCOMMANDCHANNELCOMPLETEL, "PTPIP ERROR: Unexpected type in sent data, type = = %d, command =%d", typeData, typeCommand );
		
		Panic(EPTPIPBadState );
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDCOMMANDCHANNELCOMPLETEL_EXIT );
	}

/**
 Called by the MTP fw to send the data by the transport via the sockets
 */
void CPTPIPConnection::SendDataL(const MMTPType& aData,	const TMTPTypeRequest& aRequest )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDDATAL_ENTRY );

	OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_SENDDATAL, "******** Phase 2 - Data R to I **************" );
	
	SetTransactionPhase(EDataRToIPhase );
	SetConnectionState(EDataSendInProgress );

	// Save the actual data in the dataContainer
	iPTPIPDataContainer->SetPayloadL(const_cast<MMTPType*>(&aData) );
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketLength,	iPTPIPDataContainer->Size( ) );

	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType,EPTPIPPacketTypeEndData );
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::ETransactionId,aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID ) );

	// Create the start data ptpip packet. 
	iPTPIPStartDataPayload.Reset( );
	iPTPIPStartDataPayload.SetUint32(TPTPIPTypeStartDataPayload::ETransactionId, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID ) );
	iPTPIPStartDataPayload.SetUint64(TPTPIPTypeStartDataPayload::ETotalSize, aData.Size( ) );

	iPTPIPCommandContainer->SetPayloadL(&iPTPIPStartDataPayload );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength, iPTPIPCommandContainer->Size( ) );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketType,	EPTPIPPacketTypeStartData );

	// First send the start data packet, once this is complete, it will invoke the 
	// SendCommandChannelCompleteL, where we will check the state and send the 
	// actual data in the next packet, which has been saved in the dataContainer. 
	SendStartDataPacketL( );

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDDATAL_EXIT );
	}

/**
 The data will be sent in 2 ptpip operations. 
 first the start data ptpip packet will be sent. This has the totalk size and transaction. 
 next the actual data packet will be sent, with the end data ptp ip header.
 */
void CPTPIPConnection::SendStartDataPacketL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDSTARTDATAPACKETL_ENTRY );

	SetConnectionState(EDataSendInProgress );
	iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDSTARTDATAPACKETL_EXIT );
	}

/**
 Send the actual data, which has come from the MTP framework 
 */
void CPTPIPConnection::SendDataPacketL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDDATAPACKETL_ENTRY );
	
	MMTPType* payLoad = iPTPIPDataContainer->Payload();
	
	TPtr8 headerChunk(NULL, 0);
	TBool hasTransportHeader = payLoad->ReserveTransportHeader(KPTPIPDataHeaderSize, headerChunk);
	if (hasTransportHeader)
	    {
        const TInt KLengthOffset = 0;
        const TInt KTypeOffset = 4;
        const TInt KXIDOffset = 8;
        TUint32 pkgLength = iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::EPacketLength);
        TUint32 pkgType = iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::EPacketType);
        TUint32 transId = iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::ETransactionId);
        
        memcpy(&(headerChunk[KLengthOffset]), &pkgLength, sizeof(TUint32));
        memcpy(&(headerChunk[KTypeOffset]), &pkgType, sizeof(TUint32));
        memcpy(&(headerChunk[KXIDOffset]), &transId, sizeof(TUint32));

        SetConnectionState(EDataSendInProgress );
        iCommandHandler->SendCommandDataL(*payLoad, transId);
	    }
	else
	    {
	    
	    SetConnectionState(EDataSendInProgress );
	    iCommandHandler->SendCommandDataL(*iPTPIPDataContainer,	iPTPIPDataContainer->Uint32L(TMTPTypeRequest::ERequestTransactionID ) );
	    }

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDDATAPACKETL_EXIT );
	}

/**
 Called by the fw to cancel the sending of data
 */
void CPTPIPConnection::SendDataCancelL(const TMTPTypeRequest& /*aRequest*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDDATACANCELL_ENTRY );
	iCommandHandler->CancelSendL(KErrCancel );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDDATACANCELL_EXIT );
	}

/**
 Called by the fw to send an event. 
 */
void CPTPIPConnection::SendEventL(const TMTPTypeEvent& aEvent )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDEVENTL_ENTRY );

    // Reset the event.
    iMTPEvent.Reset(); 
    MMTPType::CopyL(aEvent, iMTPEvent);
    
	TUint16 opCode(aEvent.Uint16(TMTPTypeEvent::EEventCode ));
	TUint32 tran(aEvent.Uint32(TMTPTypeEvent::EEventTransactionID ));
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_SENDEVENTL, "Sending event with Operation Code = 0x%04X and tran id = %d", static_cast<TUint32>(opCode), tran );
	

	TBool isNullParamValid = EFalse;
	TUint numberOfNullParam = 0;

	iPTPIPEventPayload.CopyIn(aEvent, 
	                          TMTPTypeResponse::EResponseParameter1,TMTPTypeResponse::EResponseParameter3, 
	                          isNullParamValid,
	                          numberOfNullParam );
	
	iPTPIPEventPayload.SetUint16(TPTPIPTypeResponsePayload::EResponseCode, opCode );
	iPTPIPEventPayload.SetUint32(TPTPIPTypeResponsePayload::ETransactionId,	tran );

	// Setup the bulk container.
	iPTPIPEventContainer->SetPayloadL(const_cast<TPTPIPTypeResponsePayload*>(&iPTPIPEventPayload) );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength,	static_cast<TUint32>(iPTPIPEventContainer->Size()) );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, EPTPIPPacketTypeEvent );

	// Initiate the event send sequence.
	OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_SENDEVENTL, "Sending response 0x%04X(%d bytes)",
	        static_cast<TUint32>(iPTPIPEventPayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode)),
	        iPTPIPEventContainer->Uint32L(CPTPIPGenericContainer::EPacketLength));
	

	iEventHandler->SendEventL(*iPTPIPEventContainer );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDEVENTL_EXIT );
	}

/**
 Marks the completion of the asynchronous send event. 
 */
void CPTPIPConnection::SendEventCompleteL(TInt aError, const MMTPType& /*aSource*/)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDEVENTCOMPLETEL_ENTRY );
	TUint type = iPTPIPEventContainer->Uint32L(CPTPIPGenericContainer::EPacketType );

	// Notify the fw that event was sent. 
	if(type == EPTPIPPacketTypeEvent )
		{
		// Notify the fw
		BoundProtocolLayer().SendEventCompleteL(aError, iMTPEvent );
		}

#ifdef _DEBUG
	//In case we sent a probe response, we dont' need to notify the fw.
	else
		if(type == EPTPIPPacketTypeProbeResponse )
			{
			OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_SENDEVENTCOMPLETEL, "Probe response was sent successfully" );
			}
		else
			{
			// If unexpected data was sent , it is ignored in the release mode. 
			OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_SENDEVENTCOMPLETEL, "PTPIP ERROR: An invalid send event completion signalled" );
			
			__ASSERT_DEBUG(type, Panic(EPTPIPBadState));
			}
#endif
	
	// Restart listening for events
	InitiateEventRequestPhaseL( );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDEVENTCOMPLETEL_EXIT );
	}

//
// Cancel handling functions
//

/**
 Handle the cancel on the event channel. This can come before 
 or after the cancel on the command channle, and it can also come 
 in any of the MTP transaction states ( request, response, data)
 */
void CPTPIPConnection::HandleEventCancelL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLEEVENTCANCELL_ENTRY );
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_HANDLEEVENTCANCELL, "iCancelOnCommandState = 0x%04X, and  iCancelOnEventState = 0x%04X", iCancelOnCommandState, iCancelOnEventState );
	

	// Check whether the cancel has already been received on the command channel. 
	// If so then we can simply ignore this on the event channel. 
	switch(iCancelOnCommandState )
		{
		case ECancelCmdHandled:
			// Cancel has already been received and handled on the command channel
			// ignore the cancel on event channel and reset the state to none, 
			// and start listening for the next transaction.
			iCancelOnCommandState = ECancelNotReceived;
			iCancelOnEventState = ECancelNotReceived;
			InitiateEventRequestPhaseL();
			break;

		case ECancelCmdReceived:
		case ECancelCmdHandleInProgress:
			// cancel has already been received on the command channel and is being 
			// handled. Ignore the cancel on event channel. 
			iCancelOnEventState = ECancelEvtHandled;
			break;

		case ECancelNotReceived:
			// cancel on command has not yet been received. depending on the current 
			// mtp transaction state, handle the cancel. 

			switch(iTransactionState )
				{
				case EDataIToRPhase:
					SetNULLPacketL();
					iCancelOnEventState = ECancelEvtHandled;
					break;

				case EDataRToIPhase:
					// Set the commandHandler's cancel flag on. 
					// It will complete sending the current PTPIP packet and then handle.
					// Once a PTPIP packet has been sent , the sendCommandChannel complete will 
					// be invoked, and the cancel will be checked and handled. 
					iCancelOnEventState = ECancelEvtHandled;
					iCommandHandler->SetCancel();
					break;

				case EResponsePhase:
				case ERequestPhase:
				default:
					OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_HANDLEEVENTCANCELL, "Cancel received on event channel during a non data phase, ignoring, as this will be handled when its received on command channel." );
					
					iCancelOnEventState = ECancelEvtHandled;
					break;
				}// end of switch for transaction phase.

			break;
		default:
			break;
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLEEVENTCANCELL_EXIT );
	}

/** 
 Handle the cancel on the command channel. This can come before 
 or after the cancel on the event channel, and it can also come 
 in any of the MTP transaction states ( request, response, data)
 */
void CPTPIPConnection::HandleCommandCancelL(TUint32 aTransId )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLECOMMANDCANCELL_ENTRY );

	switch(iTransactionState )
		{
		case ERequestPhase:
			OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_HANDLECOMMANDCANCELL, "Cancel received during the request phase before the request packet, ignoring." );
			
			iCancelOnCommandState = ECancelCmdHandled;
			if (iCancelOnEventState == ECancelNotReceived)
				{
				// Wait for it to be received on event
				OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_HANDLECOMMANDCANCELL, "Awaiting cancel on the event channel." );
				}
			else
				{
				HandleCommandCancelCompleteL();
				}			
			InitiateCommandRequestPhaseL();
			break;
			
		case EDataRToIPhase:
			iCancelOnCommandState = ECancelCmdHandleInProgress;
			SendCancelToFrameworkL(aTransId );
			SendCommandDataCompleteL(KErrCancel);
			break;
			
		case EDataIToRPhase:
			iCancelOnCommandState = ECancelCmdHandleInProgress;
			SendCancelToFrameworkL(aTransId );
			ReceiveCommandDataCompleteL(KErrCancel);
			break;
			
		case EResponsePhase:
			iCancelOnCommandState = ECancelCmdHandled;
			if (iCancelOnEventState == ECancelNotReceived)
				{
				// Wait for it to be received on event
				OstTrace0( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_HANDLECOMMANDCANCELL, "Awaiting cancel on the event channel." );
				}
			else
				{
				HandleCommandCancelCompleteL();
				}
			
			break;
		}// switch

	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLECOMMANDCANCELL_EXIT );
	}
	
	
void CPTPIPConnection::HandleCancelDuringSendL()
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLECANCELDURINGSENDL_ENTRY );
	iCommandHandler->Cancel( );
	// Now start listening for the cancel on command channel.
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
	iPTPIPCommandContainer->SetUint32L(	CPTPIPGenericContainer::EPacketType, 0 );
	iPTPIPCommandCancelPayload.Set(0 );
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPCommandCancelPayload );
	iCommandHandler->ReceiveCommandRequestL(*iPTPIPCommandContainer );	
	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLECANCELDURINGSENDL_EXIT );
	}

/**
 Called when the reponse to the cancel has been sent to the initiator
 */
void CPTPIPConnection::HandleCommandCancelCompleteL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLECOMMANDCANCELCOMPLETEL_ENTRY );	
	//now cancel handling is complete.

	if((ECancelCmdHandled == iCancelOnCommandState) &&(ECancelEvtHandled == iCancelOnEventState) )
		{
		OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_HANDLECOMMANDCANCELCOMPLETEL, "Completed handling cancel on both channels." );
		
		// Cancel has already been received and handled on the command channel
		// ignore the cancel on event channel and reset the state to none, 
		// and start listening for the next transaction.
		iCancelOnCommandState = ECancelNotReceived;
		iCancelOnEventState = ECancelNotReceived;
		// The transaction has been cancelled, now start listening again for next transaction.
		InitiateEventRequestPhaseL();
		}
	// if the cancel has not been received yet on event, we wait for it. 
	else if(ECancelEvtHandled != iCancelOnEventState )
		{
		OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_HANDLECOMMANDCANCELCOMPLETEL, "Waiting for the cancel on the event channel. " );
		
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLECOMMANDCANCELCOMPLETEL_EXIT );
	}

/**
 Inform the MTP framework that a cancel has been received. 
 */
void CPTPIPConnection::SendCancelToFrameworkL(TUint32 aTransId )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDCANCELTOFRAMEWORKL_ENTRY );

	// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
	iMTPEvent.Reset( );
	iMTPEvent.SetUint16(TMTPTypeEvent::EEventCode,	EMTPEventCodeCancelTransaction );
	iMTPEvent.SetUint32(TMTPTypeEvent::EEventSessionID, iMTPSessionId );
	iMTPEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, aTransId );

	BoundProtocolLayer().ReceivedEventL(iMTPEvent );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDCANCELTOFRAMEWORKL_EXIT );
	}

/**
 Send the response to the cancel event. 
 */
void CPTPIPConnection::SendCancelResponseL(TUint32 aTransId )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDCANCELRESPONSEL_ENTRY );
	iPTPIPResponsePayload.Reset( );
	iPTPIPResponsePayload.SetUint16(TPTPIPTypeResponsePayload::EResponseCode, EMTPRespCodeTransactionCancelled );
	iPTPIPResponsePayload.SetUint32(TPTPIPTypeResponsePayload::ETransactionId, aTransId );

	// Setup the command container.
	iPTPIPCommandContainer->SetPayloadL(const_cast<TPTPIPTypeResponsePayload*>(&iPTPIPResponsePayload) );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength, static_cast<TUint32>(iPTPIPCommandContainer->Size()) );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketType,	EPTPIPPacketTypeOperationResponse );

	// Initiate the command send sequence.
	OstTraceExt2( TRACE_NORMAL, CPTPIPCONNECTION_SENDCANCELRESPONSEL, "Sending response 0x%04X(%d bytes)", 
                static_cast<TUint32>(iPTPIPResponsePayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode)),
                iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketLength));
	
	iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDCANCELRESPONSEL_EXIT );
	}

/**
 * If the cancel packet is received on the event channel first
 * and we are in the DataItoR phase, we have to continue reading and ignoreing the 
 * data packets sent by the initiator, into dummy buffers until the cancel packet is received. 
 */
void CPTPIPConnection::SetNULLPacketL()
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SETNULLPACKETL_ENTRY );
    // Setup the bulk container and initiate the bulk data receive sequence.
    iNullBuffer.Close();
    iNullBuffer.CreateL(KMTPNullChunkSize);
    iNullBuffer.SetLength(KMTPNullChunkSize);
    iNull.SetBuffer(iNullBuffer);
	iPTPIPDataContainer->SetPayloadL(&iNull);
	OstTraceFunctionExit0( CPTPIPCONNECTION_SETNULLPACKETL_EXIT );
	}

//
// Getters , Setters and other helper functions
//

/**
 This function will transfer the two command and event sockets from the Controller
 and indicate the successful transfer to the PTP controller
 @leave - In case the Publish and Subscribe mechanism gives any errors while getting the 
 property names, then a leave occurs. 
 Also in case opening or transferring the socket fails, a leave is generated. 
 */
void CPTPIPConnection::TransferSocketsL( )
	{
OstTraceFunctionEntry0( CPTPIPCONNECTION_TRANSFERSOCKETSL_ENTRY );


	TName evtsockname, cmdsockname;
	TUid propertyUid=iConnectionMgr->ClientSId();
	TInt err = RProperty::Get(propertyUid, ECommandSocketName, cmdsockname );
	LEAVEIFERROR(err, 
	                        OstTrace1( TRACE_ERROR, DUP1_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));

	err = RProperty::Get(propertyUid, EEventSocketName, evtsockname );
	LEAVEIFERROR(err, 
	                        OstTrace1( TRACE_ERROR, DUP2_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));

	RSocketServ serversocket;
	err=serversocket.Connect( );
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_TRANSFERSOCKETSL, "Connected to socketServer with %d code", err );
	
	
	if (KErrNone == err)
		{			
		err = iCommandHandler->Socket().Open(serversocket );
		LEAVEIFERROR(err, 
		        OstTrace1( TRACE_ERROR, DUP3_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));
		err = iEventHandler->Socket().Open(serversocket );
	    LEAVEIFERROR(err, 
	            OstTrace1( TRACE_ERROR, DUP4_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));
	    err=iCommandHandler->Socket().Transfer(serversocket, cmdsockname );
	    LEAVEIFERROR(err, 
	            OstTrace1( TRACE_ERROR, DUP5_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));
	    err=iEventHandler->Socket().Transfer(serversocket, evtsockname );
	    LEAVEIFERROR(err, 
	            OstTrace1( TRACE_ERROR, DUP6_CPTPIPCONNECTION_TRANSFERSOCKETSL, "error code is %d", err ));
		}
	
	iCommandHandler->SetSocketOptions();
	iEventHandler->SetSocketOptions();

	OstTraceFunctionExit0( CPTPIPCONNECTION_TRANSFERSOCKETSL_EXIT );
	}


/**
 Connection establishment has 4 steps, the first 3 are completed by the controller process:

 1. Initiator connects to command socket, sends the init command request
 2. Responder replies with the init command ack
 3. Initiator connects to the command socket, sends the init event request

 4. Responder replies with the init event ack or init event fail. 

 The last step of sending the init ack is done by the transport plugin from 
 the mtp process. This is done after the the framework has loaded, the sockets have
 been transferred to this process and the transport is up. 
 */
void CPTPIPConnection::SendInitAckL( )
	{
OstTraceFunctionEntry0( CPTPIPCONNECTION_SENDINITACKL_ENTRY );

	iPTPIPEventContainer->SetPayloadL(NULL );
	iPTPIPEventContainer->SetUint32L(TPTPIPInitEvtAck::ELength,	iPTPIPEventContainer->Size( ) );
	iPTPIPEventContainer->SetUint32L(TPTPIPInitEvtAck::EType, EPTPIPPacketTypeEventAck );

	// Send the packet
	iEventHandler->SendInitAck(iPTPIPEventContainer );

	OstTraceFunctionExit0( CPTPIPCONNECTION_SENDINITACKL_EXIT );
	}

/**
 Stop the connection. 

 First cancel the command and socket handlers which are controlled by it
 and complete any data send of receive commands with error code of abort. 

 Also inform the fw that connection is closed. 

 */
void CPTPIPConnection::StopConnection( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_STOPCONNECTION_ENTRY );

	if(ConnectionOpen( ) )
		{
		OstTrace0( TRACE_NORMAL, CPTPIPCONNECTION_STOPCONNECTION, "Stopping socket handlers" );
		
		iEventHandler->Cancel( );
		iCommandHandler->Cancel( );
		if(iTransactionState == EDataIToRPhase )
			{
			OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_STOPCONNECTION, "Aborting active I to R data phase" );
			
			TRAPD(err, BoundProtocolLayer().ReceiveDataCompleteL(KErrAbort, *iPTPIPDataContainer->Payload(), iMTPRequest));
			UNUSED_VAR(err);
			}
		else
			if(iTransactionState == EDataRToIPhase )
				{
				OstTrace0( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_STOPCONNECTION, "Aborting active R to I data phase" );
				
				TRAPD(err, BoundProtocolLayer().SendDataCompleteL(KErrAbort, *iPTPIPDataContainer->Payload(), iMTPRequest))	;
				UNUSED_VAR(err);
				}
		OstTrace0( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_STOPCONNECTION, "Notifying protocol layer connection closed" );
		
		iConnectionMgr->ConnectionClosed(*this );
		SetTransactionPhase(EUndefined );
		SetConnectionState(EIdle );
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_STOPCONNECTION_EXIT );
	}

/**
 * Invoked by the SocketHandler when there is an error.
 */
void CPTPIPConnection::HandleError(TInt aError)
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLEERROR_ENTRY );
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_HANDLEERROR, "SocketHandler received an error=%d, stopping connection.", aError );
	
	StopConnection();
	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLEERROR_EXIT );
	}

/**
 Used to trigger the RunL, by first setting itself to active and 
 then simulating a fake asynchronous service provider which will complete us 
 with a completion code.
 */
void CPTPIPConnection::CompleteSelf(TInt aCompletionCode )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_COMPLETESELF_ENTRY );
	// Setting ourselves active to wait to be done by ASP.
	SetActive( );

	// Simulating a fake ASP which completes us.
	TRequestStatus* stat = &iStatus;
	User::RequestComplete(stat, aCompletionCode );
	OstTraceFunctionExit0( CPTPIPCONNECTION_COMPLETESELF_EXIT );
	}

/**
 Setter for transaction phase(request, dataItoR, dataRtoI, or response)
 */
void CPTPIPConnection::SetTransactionPhase(TMTPTransactionPhase aPhase )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SETTRANSACTIONPHASE_ENTRY );
	iTransactionState = aPhase;
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_SETTRANSACTIONPHASE, "Transaction Phase set to 0x%08X", iTransactionState );
	
	OstTraceFunctionExit0( CPTPIPCONNECTION_SETTRANSACTIONPHASE_EXIT );
	}

/**
 Setter for connection state,( initialising, send data, etc)
 */
void CPTPIPConnection::SetConnectionState(TConnectionState aState )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SETCONNECTIONSTATE_ENTRY );
	iState = aState;
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_SETCONNECTIONSTATE, "Connection state set to 0x%08X", iState );
	
	OstTraceFunctionExit0( CPTPIPCONNECTION_SETCONNECTIONSTATE_EXIT );
	}

/**
 Getter 
 */
TBool CPTPIPConnection::ConnectionOpen( ) const
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_CONNECTIONOPEN_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_CONNECTIONOPEN_EXIT );
	return((iState >= EInitialising) && (iState <= EDataSendFinished));
	}

/**
 Getter for the command container, 
 */
CPTPIPGenericContainer* CPTPIPConnection::CommandContainer( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_COMMANDCONTAINER_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_COMMANDCONTAINER_EXIT );
	return iPTPIPCommandContainer;
	}

/**
 Getter for the event container
 */
CPTPIPGenericContainer* CPTPIPConnection::EventContainer( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_EVENTCONTAINER_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_EVENTCONTAINER_EXIT );
	return iPTPIPEventContainer;
	}

/**
 Getter for the data container
 */
CPTPIPDataContainer* CPTPIPConnection::DataContainer( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_DATACONTAINER_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_DATACONTAINER_EXIT );
	return iPTPIPDataContainer;
	}

/**
 Getter for the transaction phase: request, dataItoR, dataRtoI or response
 */
TMTPTransactionPhase CPTPIPConnection::TransactionPhase( ) const
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_TRANSACTIONPHASE_ENTRY );
	OstTraceFunctionExit0( CPTPIPCONNECTION_TRANSACTIONPHASE_EXIT );
	return iTransactionState;
	}

/**
 Takes the 4 bytes from the chunk(iReceiveChunkData) and return 
 whether the type is a valid request, cancel or probe packet. 

 @return: The container type, in case of an unknown type, 
 the value 0 ( undefined) is returned

 */
TUint32 CPTPIPConnection::ValidateAndSetCommandPayloadL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL_ENTRY );

	TUint32 containerType = CommandContainer()->Uint32L(CPTPIPGenericContainer::EPacketType );
	
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL, "PTP packet type  = %d, adjust payload accordingly", containerType );
	

	switch(containerType )
		{
		case EPTPIPPacketTypeOperationRequest:
			if (!ValidateTransactionPhase(ERequestPhase ))
				{
				OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL, "PTPIP ERROR: Request data unexpected in this phase, setting type to undefined" );
				
				containerType = EPTPIPPacketTypeUndefined;
				}
			// Nothing to do , the payload is already set.  In case this is unexpected, 
			//then the validate function will close the connection. 
			break;

		case EPTPIPPacketTypeStartData:
			if (!ValidateTransactionPhase(EDataIToRPhase ))
				{
				OstTrace0( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL, "PTPIP ERROR: Start data unexpected in this phase, setting type to undefined" );
				
				containerType = EPTPIPPacketTypeUndefined;
				}
			// Nothing to do , the payload is already set.  In case this is unexpected, 
			//then the validate function will close the connection. 
			break;

		case EPTPIPPacketTypeCancel:
			// This can come on the command channel either during the data phase or 
			// during the command phase. 
			// In data phase, no payload is needed on the data container. 
			if (EDataIToRPhase == iTransactionState)
				{
				DataContainer()->SetPayloadL(NULL);
				}
			else 
				{
				CommandContainer()->SetPayloadL(&iPTPIPCommandCancelPayload );
				}
			break;
			
		case EPTPIPPacketTypeOperationResponse:
			OstTrace0( TRACE_NORMAL, DUP3_CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL, "PTPIP ERROR: Response not expected from the initiator, setting type to undefined" );
			
			containerType = EPTPIPPacketTypeUndefined;			
			// As per the protocol, the initiator cannot send a response, 
			// only the responder( here device)  will create a response, 
			// if this is recieved it is an erro
			break;
			

		default:
			OstTrace1( TRACE_NORMAL, DUP4_CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL, "PTPIP ERROR: Invalid packet type received %d", containerType );
			
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_VALIDATEANDSETCOMMANDPAYLOADL_EXIT );
	return containerType;
	}

/**
 Takes the 4 bytes from the chunk(iReceiveChunkData) and return 
 whether the type is a valid event packet. 

 @return: The container type, in case of an unknown type, 
 the value 0 ( undefined) is returned

 */
TUint32 CPTPIPConnection::ValidateDataPacketL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_VALIDATEDATAPACKETL_ENTRY );

	TUint32 containerType = DataContainer()->Uint32L(CPTPIPDataContainer::EPacketType );
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_VALIDATEDATAPACKETL, "PTP data packet type  = %d", containerType );
	

	switch(containerType )
		{
		case EPTPIPPacketTypeData:
		case EPTPIPPacketTypeEndData:
			if (!ValidateTransactionPhase(EDataIToRPhase ))
				{
				OstTrace0( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_VALIDATEDATAPACKETL, "PTPIP ERROR: Receiving data unexpected in this phase, setting type to undefined" );
				
				containerType = EPTPIPPacketTypeUndefined;
				}
			break;

		default:
			OstTrace1( TRACE_NORMAL, DUP2_CPTPIPCONNECTION_VALIDATEDATAPACKETL, "PTPIP ERROR: Unexpected or Invalid packet type received while expecting data packet%d ", containerType );
			
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_VALIDATEDATAPACKETL_EXIT );
	return containerType;
	}

/**
 Takes the 4 bytes from the chunk(iReceiveChunkData) and return 
 whether the type is a valid event packet. 

 @return: The container type, in case of an unknown type, 
 the value 0 ( undefined) is returned

 */
TUint32 CPTPIPConnection::ValidateAndSetEventPayloadL( )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_VALIDATEANDSETEVENTPAYLOADL_ENTRY );

	TUint32 containerType = EventContainer()->Uint32L(CPTPIPGenericContainer::EPacketType );
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_VALIDATEANDSETEVENTPAYLOADL, "PTP event packet type  = %d, adjust payload accordingly", containerType );
	

	switch(containerType )
		{
		case EPTPIPPacketTypeProbeRequest:
			EventContainer()->SetPayloadL(NULL );
			break;

		case EPTPIPPacketTypeCancel:
			EventContainer()->SetPayloadL(&iPTPIPEventCancelPayload );
			break;

		case EPTPIPPacketTypeEvent:
			EventContainer()->SetPayloadL(&iPTPIPEventPayload );
			break;

		default:
			OstTrace1( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_VALIDATEANDSETEVENTPAYLOADL, "PTPIP ERROR: Invalid packet type received %d", containerType );
			
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	OstTraceFunctionExit0( CPTPIPCONNECTION_VALIDATEANDSETEVENTPAYLOADL_EXIT );
	return containerType;
	}

/**
 Processes bulk transfer request transaction state checking. If the transaction 
 state is invalid, then the connection is shutdown.
 @return ETrue if the control request completion status was abnormal, otherwise
 EFalse.
 */
TBool CPTPIPConnection::ValidateTransactionPhase(
		TMTPTransactionPhase aExpectedTransactionState )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_VALIDATETRANSACTIONPHASE_ENTRY );
	OstTrace1( TRACE_NORMAL, CPTPIPCONNECTION_VALIDATETRANSACTIONPHASE, "transaction state = %d", iTransactionState );
	
	TBool valid(iTransactionState == aExpectedTransactionState);
	if(!valid )
		{
		// Invalid transaction state, close the connection.
		OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPCONNECTION_VALIDATETRANSACTIONPHASE, "PTPIP ERROR: invalid transaction state, current = %d, expected = %d", iTransactionState, aExpectedTransactionState );
		
		CloseConnection( );
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_VALIDATETRANSACTIONPHASE_EXIT );
	return valid;
	}

/**
 Convert the TCP errors, the disconnect should be reported as an abort, 
 since that is what the MTP frameword expects.
 */
TBool CPTPIPConnection::HandleTCPError(TInt& aError )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_HANDLETCPERROR_ENTRY );
	TInt ret(EFalse);
	if(aError == KErrDisconnected || aError == KErrEof)
		{
		aError = KErrAbort;
		CloseConnection( );
		ret = ETrue;
		}
	OstTraceFunctionExit0( CPTPIPCONNECTION_HANDLETCPERROR_EXIT );
	return ret;
	}

void CPTPIPConnection::SetDataTypeInDataContainerL(TPTPIPPacketTypeCode aType )
	{
	OstTraceFunctionEntry0( CPTPIPCONNECTION_SETDATATYPEINDATACONTAINERL_ENTRY );
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, aType );
	OstTraceFunctionExit0( CPTPIPCONNECTION_SETDATATYPEINDATACONTAINERL_EXIT );
	}



