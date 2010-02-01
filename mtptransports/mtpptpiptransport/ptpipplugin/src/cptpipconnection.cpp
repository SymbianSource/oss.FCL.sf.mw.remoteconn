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
#include "cptpipcommandhandler.h"
#include "cptpipeventhandler.h"
#include "cptpipconnection.h"
#include "ptpipsocketpublish.h" 

// File type constants.
const TInt KMTPNullChunkSize(0x00020000); // 100KB
const TUint32 KPTPIPDataHeaderSize = 12;  // Size of len, type and tran id. 
__FLOG_STMT(_LIT8(KComponent,"PTPIPConnection");)
#define UNUSED_VAR(a) (a) = (a)

/**
 PTPIP device class connection factory method.
 @param aConnectionMgr The MTP connection manager interface.
 @return A pointer to an PTPIP device class connection. Ownership IS transfered.
 @leave One of the system wide error codes, if a processing failure occurs.
 */
CPTPIPConnection* CPTPIPConnection::NewL(MMTPConnectionMgr& aConnectionMgr )
	{
	CPTPIPConnection* self = new(ELeave) CPTPIPConnection(aConnectionMgr);
	CleanupStack::PushL (self );
	self->ConstructL ( );
	CleanupStack::Pop (self );
	return self;
	}

/**
 Destructor
 */
CPTPIPConnection::~CPTPIPConnection( )
	{
	__FLOG(_L8("Destructor - Entry"));
	StopConnection ( );

	// Delete all the handlers which will close the sockets.
	delete iCommandHandler;
	delete iEventHandler;

	delete iPTPIPCommandContainer;
	delete iPTPIPDataContainer;
	delete iPTPIPEventContainer;
	
	iNullBuffer.Close();

	__FLOG(_L8("Destructor - Exit"));
	__FLOG_CLOSE;
	}

/**
 Second phase constructor
 @leave One of the system wide error codes, if a processing failure occurs.
 */
void CPTPIPConnection::ConstructL( )
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));

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

	__FLOG(_L8("ConstructL - Exit"));
	}

/**
 Constructor
 */
CPTPIPConnection::CPTPIPConnection(MMTPConnectionMgr& aConnectionMgr ) :
									CActive(EPriorityStandard), 
									iConnectionMgr(&aConnectionMgr)
	{
	CActiveScheduler::Add (this );
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
	__FLOG(_L8("BindL - Entry"));
	iProtocolLayer = &aProtocol;
	__FLOG(_L8("BindL - Exit"));
	}

/**
 Returns the protocol layer.
 @return The SPI or the observer protocol layer
 */
MMTPConnectionProtocol& CPTPIPConnection::BoundProtocolLayer( )
	{
	__FLOG(_L8("BoundProtocolLayer - Entry"));
	__ASSERT_ALWAYS(iProtocolLayer, Panic(EPTPIPBadState));
	__FLOG(_L8("BoundProtocolLayer - Exit"));
	return *iProtocolLayer;
	}

/**
 Close the connection, stop all data transfer activity and and wait for the host 
 to issue a Device Reset Request.
 */
void CPTPIPConnection::CloseConnection( )
	{
	__FLOG(_L8("CloseConnection - Entry"));
	StopConnection ( );
	__FLOG(_L8("CloseConnection - Exit"));
	}

/**
 A transaction is one set of request, data transfer and response phases. 
 The fw calls this to mark the end of an MTP transaction. 
 */
void CPTPIPConnection::TransactionCompleteL(const TMTPTypeRequest& /*aRequest*/)
	{
	__FLOG(_L8("TransactionCompleteL - Entry"));
	__FLOG(_L8("******** Transaction Complete **************"));
	SetTransactionPhase (EIdlePhase );

	// Clear the cancel flag.
	iCancelOnCommandState = ECancelNotReceived;
	iCancelOnEventState = ECancelNotReceived;

	// Again start listening for the command request. 
	InitiateCommandRequestPhaseL( );

	__FLOG(_L8("TransactionCompleteL - Exit"));
	}

/**
 Called by the fw to indicate that protocol layer is not using this transport anymore 
 and so we will not have any callback/SPI funtions to the fw any more
 */
void CPTPIPConnection::Unbind(MMTPConnectionProtocol& /*aProtocol*/)
	{
	__FLOG(_L8("Unbind - Entry"));
	__ASSERT_DEBUG(iProtocolLayer, Panic(EPTPIPBadState));
	// Protocol will no longer be bound to the transport
	iProtocolLayer = NULL;
	__FLOG(_L8("Unbind - Exit"));
	}

/**
 Not used
 */
TAny* CPTPIPConnection::GetExtendedInterface(TUid /*aInterfaceUid*/)
	{
	return NULL;
	}

/*
 * return PTPIP transport implementation UID
 */
TUint CPTPIPConnection::GetImplementationUid()
    {
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
	__FLOG(_L8("DoCancel - Entry"));

	iCommandHandler->Cancel( );
	iEventHandler->Cancel( );

	SetConnectionState(ECancelled );

	__FLOG(_L8("DoCancel - Exit"));
	}

/**
The connection behaves as an active object during the PTPIP connection establishment phase.
Subsequently, it behaves as a normal object whose functions are invoked by the MTP framework
or by the SocketHandler on sending/ receiving data. 

Thus the runl is not hit after the initial connection establishment. 
 */
void CPTPIPConnection::RunL( )
	{
	__FLOG(_L8("RunL - Entry"));
	__FLOG_VA((_L8("Current State is %d, and status is %d"), iState, iStatus.Int()));

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
			__FLOG(_L8("PTPIP ERROR: Invalid  connection state"));
			Panic(EPTPIPBadState );
			break;
		}
		}

	__FLOG(_L8("RunL - Exit"));
	}

/**
 Called when an error occurs in the RunL 
 */
#ifdef __FLOG_ACTIVE
TInt CPTPIPConnection::RunError(TInt aError )
#else
TInt CPTPIPConnection::RunError(TInt /*aError*/ )
#endif
	{
	__FLOG(_L8("RunError - Entry"));
	__FLOG_VA((_L8("PTPIP ERROR: Error received is %d"), aError));

	// Cancel all the outstanding requests.
	Cancel( );

	// Stop the connection, if necessary.
	StopConnection( );

	__FLOG(_L8("RunError - Exit"));
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
	__FLOG(_L8("InitiateCommandRequestPhaseL - Entry"));
	__FLOG(_L8("******** Phase 1 - Request **************"));
	// Set current state to request phase
	SetTransactionPhase(ERequestPhase );

	// The PTPIP data buffer is a member of connection.  
	// Since we are expecting a request now, set the payload to request type
	iPTPIPRequestPayload.Reset( );
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPRequestPayload );

	// Call the CommandHandler to get the request in the container
	iCommandHandler->ReceiveCommandRequestL(*iPTPIPCommandContainer );

	__FLOG(_L8("InitiateCommandRequestPhaseL - Exit"));
	}

/** 
 Called to indicate completion of receive of data started by InitiateCommandRequestPhaseL.
 There is no point in reporting the error up to the f/w since no transaction has started.
 USB does nothing at this stage. In PTP , the connection is closed.
 @param aError - The error if any, received from socket.
 */
void CPTPIPConnection::ReceiveCommandCompleteL(TInt aError )
	{
	__FLOG(_L8("ReceiveCommandCompleteL - Entry"));

	if(KErrNone != aError )
		{
		__FLOG_VA((_L8("PTPIP Error: Received error=%d in request phase, closing  connection"), aError));
		CloseConnection( );
		}
	else if(ValidateTransactionPhase(ERequestPhase ) )
		{
		// Request block received.
		TPTPIPTypeRequestPayload* pRequest = static_cast<TPTPIPTypeRequestPayload*>(iPTPIPCommandContainer->Payload());

		TUint16 op(pRequest->Uint16(TPTPIPTypeRequestPayload::EOpCode ));
		TUint32	tran(pRequest->Uint32(TPTPIPTypeRequestPayload::ETransactionId ));
		TUint32 sessionId = KMTPSessionNone;
		__FLOG_VA((_L8("Command block received with op = 0x%04X ,transId = %d"), op, tran));

		// Reset the iMTPRequest.
		iMTPRequest.Reset( );

		// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
		iMTPRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, op );
		iMTPRequest.SetUint32(TMTPTypeRequest::ERequestTransactionID, tran );

		// Set SessionID.
		if(op == EMTPOpCodeOpenSession )
			{
			__FLOG(_L8("Processing OpenSession request"));
			}
		else if(op == EMTPOpCodeCloseSession || op == EMTPOpCodeResetDevice )
			{
			__FLOG(_L8("Processing CloseSession or the ResetDevice request"));
			// Force CloseSession requests to be processed outside an active session. 
			// ResetDevice currently behaves the same way as CloseSession. 
			iMTPRequest.SetUint32(TMTPTypeRequest::ERequestParameter1,
					iMTPSessionId );
			}
		else
			{
			sessionId = iMTPSessionId;
			__FLOG_VA((_L8("Processing general request on session %d"), sessionId));
			}
		
		iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID,sessionId );

		// Set Parameter 1 .. Parameter 5.
		pRequest->CopyOut(iMTPRequest, TMTPTypeRequest::ERequestParameter1,	TMTPTypeRequest::ERequestParameter5 );
		pRequest->Reset( );

		// Notify the protocol layer.
		
		BoundProtocolLayer().ReceivedRequestL(iMTPRequest );

		}
	__FLOG(_L8("ReceiveCommandCompleteL - Exit"));
	}

/**
 Called to get data over the command channel,( in turn its called by f/ws ReceiveData)
 @param aData - The buffer provided by the F/w in which to receive data.
 */
void CPTPIPConnection::ReceiveCommandDataL(MMTPType& aData )
	{
	__FLOG(_L8("ReceiveCommandDataL - Entry"));

	iRecvData = 0;
	iTotalRecvData = 0;
	
	// First we will receive a PTPIP start data packet which is stored in the commandContainer
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPStartDataPayload );

	// The mtp buffer is passed from the framework and will be passed on as the payload.
	// in the next ptpip data packet.
	iPTPIPDataContainer->SetPayloadL(&aData );
	iCommandHandler->ReceiveCommandDataL(*iPTPIPCommandContainer );

	__FLOG(_L8("ReceiveCommandDataL - Exit"));
	}

/**
 Called to indicate completion of receive of data started by ReceiveCommandDataL.
 Any errors received are sent back up to the framework. 
 */
void CPTPIPConnection::ReceiveCommandDataCompleteL(TInt aError )
	{
	__FLOG(_L8("ReceiveCommandDataCompleteL - Entry"));
	if(ValidateTransactionPhase(EDataIToRPhase ) )
		{
		// Data block received, notify the protocol layer.
		iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
		BoundProtocolLayer().ReceiveDataCompleteL(aError, *iPTPIPDataContainer->Payload(), iMTPRequest );
		}
	__FLOG(_L8("ReceiveCommandDataCompleteL - Exit"));
	}

/**
 Called by the command handler to indicate completion of receive on the command channel.
 Now check whether it was commands or data completion and call the appropriate function. 
 */

void CPTPIPConnection::ReceiveCommandChannelCompleteL(TInt aError, MMTPType& /*aSource*/)
	{
	__FLOG(_L8("ReceiveCommandChannelCompleteL - Entry"));
	__FLOG(_L8("******** Receiving 1 ptpip packet on command/data channel complete **************"));
	HandleTCPError(aError );
	TUint32	typeCommand = iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	TUint32	typeData = iPTPIPDataContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	__FLOG_VA((_L8("type on the command buffer is %d and type on the data buffer is %d"), typeCommand, typeData ) );


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
			__FLOG_VA((_L8("Total data to receive in data phase is %ld"), iTotalRecvData));						
			
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
			
			
			__FLOG_VA((_L8("Data received so far in data phase is %ld"), iRecvData));
			if(iRecvData <= iTotalRecvData )
				{
				iCommandHandler->ReceiveCommandDataL(*iPTPIPDataContainer );
				}
			else
				{
				__FLOG_VA((_L8("PTPIP ERROR: The data received so far= %ld is more than expected data = %ld "), iRecvData, iTotalRecvData));
				CloseConnection( );
				}			
			break;
		
		case EPTPIPPacketTypeEndData:
			iRecvData += iPTPIPDataContainer->Uint32L(CPTPIPDataContainer::EPacketLength );
			iRecvData -= KPTPIPDataHeaderSize; // The header size is not included in the total size sent. 
			
			iCommandHandler->iUseOffset = EFalse;
			
			__FLOG_VA((_L8("Data received so far in data phase is %ld"), iRecvData));
			if(iTotalRecvData == iRecvData )
				{
				ReceiveCommandDataCompleteL(aError );
				}
			else
				{
				__FLOG_VA((_L8("PTPIP ERROR: The data received so far= %ld is not equal to expected data = %ld "), iRecvData, iTotalRecvData));
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
			__FLOG_VA((_L8("PTPIP ERROR: Unexpected type received,  data container = %d, command container =%d "), typeData, typeCommand));
			CloseConnection( );
			break;
		} // switch data
	} // switch command

	__FLOG(_L8("ReceiveCommandChannelCompleteL - Exit"));
	}

/**
 Called by MTP fw to get data from the transport.
 */
void CPTPIPConnection::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/)
	{
	__FLOG(_L8("ReceiveDataL - Entry"));
	__FLOG(_L8("******** Phase 2 - Data I to R **************"));
	SetTransactionPhase(EDataIToRPhase );
	ReceiveCommandDataL(aData );
	__FLOG(_L8("ReceiveDataL - Exit"));
	}

/**
 Called by MTP fw to cancel the receiving data.
 */
void CPTPIPConnection::ReceiveDataCancelL(const TMTPTypeRequest& /*aRequest*/)
	{
	__FLOG(_L8("ReceiveDataCancelL - Entry"));

	iCommandHandler->CancelReceiveL(KErrCancel );
	__FLOG(_L8("ReceiveDataCancelL - Exit"));
	}

/**
 This will invoke the event handler to listen on the socket and get the events 
 into the buffers passed. 
 */
void CPTPIPConnection::InitiateEventRequestPhaseL( )
	{
	__FLOG(_L8("InitiateEventRequestPhaseL - Entry"));

	// Initialise the PTP buffers to get the data. 
	iPTPIPEventPayload.Reset( );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketType, NULL );
	iPTPIPEventContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength,	NULL );
	iPTPIPEventContainer->SetPayloadL(&iPTPIPEventPayload );

	// Call the EventHandler
	iEventHandler->ReceiveEventL(*iPTPIPEventContainer );
	__FLOG(_L8("InitiateEventRequestPhaseL - Exit"));
	}

/**
 Signals the completion of receving an event. 
 */
void CPTPIPConnection::ReceiveEventCompleteL(TInt aError, MMTPType& /*aSource*/)
	{
	__FLOG(_L8("ReceiveEventCompleteL - Entry"));

	TUint32	type = iPTPIPEventContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	__FLOG_VA((_L8("Error value is %d and type is %d"), aError, type));

	if(KErrNone != aError )
		{
		__FLOG_VA((_L8("PTPIP Error: Received error=%d in request phase, closing  connection"), aError));
		CloseConnection( );
		}
	else
		{
		// For a probe request, we just send a probe response and don't notify the MTP f/w. 
		if( type == EPTPIPPacketTypeProbeRequest )
			{
			__FLOG(_L8("Received a probe request, sending back a probe response"));
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
			__FLOG_VA((_L8("Event block 0x%04X received"), op));

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
			__FLOG(_L8("PTPIP ERROR : Unknown event type received, ignoring it."));
			__ASSERT_DEBUG(type, Panic(EPTPIPBadState));
			}
		}
	__FLOG(_L8("ReceiveEventCompleteL - Exit"));
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
	__FLOG(_L8("SendResponseL - Entry"));

	// Update the transaction state.
	SetTransactionPhase(EResponsePhase );
	__FLOG(_L8("******** Phase 3 - Response **************"));
	
	if(iCancelOnCommandState  )
		{
		__FLOG(_L8("Cancel has been received from initiator, so send own response"));
		
		SendCancelResponseL(aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID ));
		}
	else
		{
		TUint16	opCode(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode ));
		TUint16 rspCode(aResponse.Uint16(TMTPTypeResponse::EResponseCode ));
		__FLOG_VA((_L8("ResponseCode = 0x%04X, Operation Code = 0x%04X"), rspCode, opCode));

		if((opCode == EMTPOpCodeOpenSession) &&(rspCode == EMTPRespCodeOK) )
			{
			// An session has been opened. Record the active SessionID.
			iMTPSessionId = aRequest.Uint32(TMTPTypeRequest::ERequestParameter1 );
			__FLOG_VA((_L8("Processing OpenSession response, SessionID = %d"), iMTPSessionId));
			}
		else if(((opCode == EMTPOpCodeCloseSession) ||
				(opCode == EMTPOpCodeResetDevice))&&(rspCode == EMTPRespCodeOK) )
			{
			// An session has been closed. Clear the active SessionID.        
			__FLOG_VA((_L8("Processing CloseSession or ResetDevice response, SessionID = %d"), iMTPSessionId));
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
		__FLOG_VA((_L8("Sending response 0x%04X(%d bytes)"),
						iPTPIPResponsePayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode),
						iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketLength)));
		iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
		}

	__FLOG(_L8("SendResponseL - Exit"));
	}

/**
 Send response complete
 */
void CPTPIPConnection::SendCommandCompleteL(TInt aError )
	{

	__FLOG(_L8("SendCommandCompleteL - Entry"));

	if(ValidateTransactionPhase(EResponsePhase ) )
		{
		BoundProtocolLayer().SendResponseCompleteL( aError,
		                                            *static_cast<TMTPTypeResponse*>(iPTPIPCommandContainer->Payload()), 
		                                            iMTPRequest );
		}
	__FLOG(_L8("SendCommandCompleteL - Exit"));
	}

/**
 Send data complete
 */
void CPTPIPConnection::SendCommandDataCompleteL(TInt aError )
	{
	__FLOG(_L8("SendCommandDataCompleteL - Entry"));

	if(ValidateTransactionPhase(EDataRToIPhase ) )
		{
		BoundProtocolLayer().SendDataCompleteL(aError, *iPTPIPDataContainer->Payload(), iMTPRequest );
		}
	SetConnectionState(EDataSendFinished);
	iPTPIPDataContainer->SetPayloadL(NULL );

	__FLOG(_L8("SendCommandDataCompleteL - Exit"));
	}

/**
 Called by the command handler to indicate completion of send on the command channel.
 Now check whether it was commands or data completion and call the appropriate function. 
 */
void CPTPIPConnection::SendCommandChannelCompleteL(TInt aError, const MMTPType& /*aSource*/)
	{
	__FLOG(_L8("SendCommandChannelCompleteL - Entry"));

	// Now see whether we have completed getting data or commands, and call the appropriate function.
	TUint typeCommand = iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	TUint typeData = iPTPIPDataContainer->Uint32L(CPTPIPGenericContainer::EPacketType );
	__FLOG_VA((_L8("type on the command buffer is %d and type on the data buffer is %d"), typeCommand, typeData ) );
	
	
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
		__FLOG_VA((_L8("PTPIP ERROR: Unexpected type in sent data, type = = %d, command =%d "), typeData, typeCommand));
		Panic(EPTPIPBadState );
		}

	__FLOG(_L8("SendCommandChannelCompleteL - Exit"));
	}

/**
 Called by the MTP fw to send the data by the transport via the sockets
 */
void CPTPIPConnection::SendDataL(const MMTPType& aData,	const TMTPTypeRequest& aRequest )
	{
	__FLOG(_L8("SendDataL - Entry"));

	__FLOG(_L8("******** Phase 2 - Data R to I **************"));
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

	__FLOG(_L8("SendDataL - Exit"));
	}

/**
 The data will be sent in 2 ptpip operations. 
 first the start data ptpip packet will be sent. This has the totalk size and transaction. 
 next the actual data packet will be sent, with the end data ptp ip header.
 */
void CPTPIPConnection::SendStartDataPacketL( )
	{
	__FLOG(_L8("SendStartDataPacketL - Entry"));

	SetConnectionState(EDataSendInProgress );
	iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
	__FLOG(_L8("SendStartDataPacketL - Exit"));
	}

/**
 Send the actual data, which has come from the MTP framework 
 */
void CPTPIPConnection::SendDataPacketL( )
	{
	__FLOG(_L8("SendDataPacketL - Entry"));
	
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

	__FLOG(_L8("SendDataPacketL - Exit"));
	}

/**
 Called by the fw to cancel the sending of data
 */
void CPTPIPConnection::SendDataCancelL(const TMTPTypeRequest& /*aRequest*/)
	{
	__FLOG(_L8("SendDataCancelL - Entry"));
	iCommandHandler->CancelSendL(KErrCancel );
	__FLOG(_L8("SendDataCancelL - Exit"));
	}

/**
 Called by the fw to send an event. 
 */
void CPTPIPConnection::SendEventL(const TMTPTypeEvent& aEvent )
	{
	__FLOG(_L8("SendEventL - Entry"));

    // Reset the event.
    iMTPEvent.Reset(); 
    MMTPType::CopyL(aEvent, iMTPEvent);
    
	TUint16 opCode(aEvent.Uint16(TMTPTypeEvent::EEventCode ));
	TUint32 tran(aEvent.Uint32(TMTPTypeEvent::EEventTransactionID ));
	__FLOG_VA((_L8(" Sending event with Operation Code = 0x%04X and tran id = %d"), opCode, tran ));

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
	__FLOG_VA((_L8("Sending response 0x%04X(%d bytes)"),
					iPTPIPEventPayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode),
					iPTPIPEventContainer->Uint32L(CPTPIPGenericContainer::EPacketLength)));

	iEventHandler->SendEventL(*iPTPIPEventContainer );
	__FLOG(_L8("SendEventL - Exit"));
	}

/**
 Marks the completion of the asynchronous send event. 
 */
void CPTPIPConnection::SendEventCompleteL(TInt aError, const MMTPType& /*aSource*/)
	{
	__FLOG(_L8("SendEventCompleteL - Entry"));
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
			__FLOG(_L8("Probe response was sent successfully"));
			}
		else
			{
			// If unexpected data was sent , it is ignored in the release mode. 
			__FLOG(_L8("PTPIP ERROR: An invalid send event completion signalled"));
			__ASSERT_DEBUG(type, Panic(EPTPIPBadState));
			}
#endif
	
	// Restart listening for events
	InitiateEventRequestPhaseL( );
	__FLOG(_L8("SendEventCompleteL - Exit"));
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
	__FLOG(_L8("HandleEventCancelL - Entry"));
	__FLOG_VA((_L8("iCancelOnCommandState = 0x%04X, and  iCancelOnEventState = 0x%04X"), iCancelOnCommandState, iCancelOnEventState));

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
					__FLOG(_L8(" Cancel received on event channel during a non data phase, ignoring, as this will be handled when its received on command channel."));
					iCancelOnEventState = ECancelEvtHandled;
					break;
				}// end of switch for transaction phase.

			break;
		default:
			break;
		}
	__FLOG(_L8("HandleEventCancelL - Exit"));
	}

/** 
 Handle the cancel on the command channel. This can come before 
 or after the cancel on the event channel, and it can also come 
 in any of the MTP transaction states ( request, response, data)
 */
void CPTPIPConnection::HandleCommandCancelL(TUint32 aTransId )
	{
	__FLOG(_L8("HandleCommandCancelL - Entry"));

	switch(iTransactionState )
		{
		case ERequestPhase:
			__FLOG(_L8(" Cancel received during the request phase before the request packet, ignoring."));
			iCancelOnCommandState = ECancelCmdHandled;
			if (iCancelOnEventState == ECancelNotReceived)
				{
				// Wait for it to be received on event
				__FLOG(_L8("Awaiting cancel on the event channel."));
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
				__FLOG(_L8("Awaiting cancel on the event channel."));
				}
			else
				{
				HandleCommandCancelCompleteL();
				}
			
			break;
		}// switch

	__FLOG(_L8("HandleCommandCancelL - Exit"));
	}
	
	
void CPTPIPConnection::HandleCancelDuringSendL()
	{
	__FLOG(_L8("HandleCancelDuringSendL - Entry"));	
	iCommandHandler->Cancel( );
	// Now start listening for the cancel on command channel.
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, 0 );
	iPTPIPCommandContainer->SetUint32L(	CPTPIPGenericContainer::EPacketType, 0 );
	iPTPIPCommandCancelPayload.Set(0 );
	iPTPIPCommandContainer->SetPayloadL(&iPTPIPCommandCancelPayload );
	iCommandHandler->ReceiveCommandRequestL(*iPTPIPCommandContainer );	
	__FLOG(_L8("HandleCancelDuringSendL - Exit"));
	}

/**
 Called when the reponse to the cancel has been sent to the initiator
 */
void CPTPIPConnection::HandleCommandCancelCompleteL( )
	{
	__FLOG(_L8("HandleCommandCancelCompleteL - Entry"));	
	//now cancel handling is complete.

	if((ECancelCmdHandled == iCancelOnCommandState) &&(ECancelEvtHandled == iCancelOnEventState) )
		{
		__FLOG(_L8("Completed handling cancel on both channels. "));
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
		__FLOG(_L8("Waiting for the cancel on the event channel. "));
		}
	__FLOG(_L8("HandleCommandCancelCompleteL - Exit"));
	}

/**
 Inform the MTP framework that a cancel has been received. 
 */
void CPTPIPConnection::SendCancelToFrameworkL(TUint32 aTransId )
	{
	__FLOG(_L8("SendCancelToFramework - Entry"));

	// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
	iMTPEvent.Reset( );
	iMTPEvent.SetUint16(TMTPTypeEvent::EEventCode,	EMTPEventCodeCancelTransaction );
	iMTPEvent.SetUint32(TMTPTypeEvent::EEventSessionID, iMTPSessionId );
	iMTPEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, aTransId );

	BoundProtocolLayer().ReceivedEventL(iMTPEvent );
	__FLOG(_L8("SendCancelToFramework - Exit"));
	}

/**
 Send the response to the cancel event. 
 */
void CPTPIPConnection::SendCancelResponseL(TUint32 aTransId )
	{
	__FLOG(_L8("SendCancelResponse - Entry"));
	iPTPIPResponsePayload.Reset( );
	iPTPIPResponsePayload.SetUint16(TPTPIPTypeResponsePayload::EResponseCode, EMTPRespCodeTransactionCancelled );
	iPTPIPResponsePayload.SetUint32(TPTPIPTypeResponsePayload::ETransactionId, aTransId );

	// Setup the command container.
	iPTPIPCommandContainer->SetPayloadL(const_cast<TPTPIPTypeResponsePayload*>(&iPTPIPResponsePayload) );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketLength, static_cast<TUint32>(iPTPIPCommandContainer->Size()) );
	iPTPIPCommandContainer->SetUint32L(CPTPIPGenericContainer::EPacketType,	EPTPIPPacketTypeOperationResponse );

	// Initiate the command send sequence.
	__FLOG_VA((_L8("Sending response 0x%04X(%d bytes)"),
					iPTPIPResponsePayload.Uint16(TPTPIPTypeResponsePayload::EResponseCode),
					iPTPIPCommandContainer->Uint32L(CPTPIPGenericContainer::EPacketLength)));
	iCommandHandler->SendCommandL(*iPTPIPCommandContainer );
	__FLOG(_L8("SendCancelResponse - Exit"));
	}

/**
 * If the cancel packet is received on the event channel first
 * and we are in the DataItoR phase, we have to continue reading and ignoreing the 
 * data packets sent by the initiator, into dummy buffers until the cancel packet is received. 
 */
void CPTPIPConnection::SetNULLPacketL()
	{
	__FLOG(_L8("SetNULLPacketL - Entry"));
    // Setup the bulk container and initiate the bulk data receive sequence.
    iNullBuffer.Close();
    iNullBuffer.CreateL(KMTPNullChunkSize);
    iNullBuffer.SetLength(KMTPNullChunkSize);
    iNull.SetBuffer(iNullBuffer);
	iPTPIPDataContainer->SetPayloadL(&iNull);
	__FLOG(_L8("SetNULLPacketL - Exit"));
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

	__FLOG(_L8("TransferSocketsL - Entry"));

	TName evtsockname, cmdsockname;
	TUid propertyUid=iConnectionMgr->ClientSId();
	User::LeaveIfError(RProperty::Get(propertyUid, ECommandSocketName,	cmdsockname ));
	User::LeaveIfError(RProperty::Get(propertyUid, EEventSocketName, evtsockname ));

	RSocketServ serversocket;
	TInt err=serversocket.Connect( );
	__FLOG_VA((_L8("Connected to socketServer with %d code"), err) );
	
	if (KErrNone == err)
		{			
		User::LeaveIfError(iCommandHandler->Socket().Open(serversocket ));
		User::LeaveIfError(iEventHandler->Socket().Open(serversocket ));
	
		User::LeaveIfError(err=iCommandHandler->Socket().Transfer(serversocket, cmdsockname ));
		User::LeaveIfError(err=iEventHandler->Socket().Transfer(serversocket, evtsockname ));
		}
	
	iCommandHandler->SetSocketOptions();
	iEventHandler->SetSocketOptions();

	__FLOG(_L8("TransferSocketsL - Exit"));
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

	__FLOG(_L8("SendInitAckL - Entry"));

	iPTPIPEventContainer->SetPayloadL(NULL );
	iPTPIPEventContainer->SetUint32L(TPTPIPInitEvtAck::ELength,	iPTPIPEventContainer->Size( ) );
	iPTPIPEventContainer->SetUint32L(TPTPIPInitEvtAck::EType, EPTPIPPacketTypeEventAck );

	// Send the packet
	iEventHandler->SendInitAck(iPTPIPEventContainer );

	__FLOG(_L8("SendInitAckL - Exit"));
	}

/**
 Stop the connection. 

 First cancel the command and socket handlers which are controlled by it
 and complete any data send of receive commands with error code of abort. 

 Also inform the fw that connection is closed. 

 */
void CPTPIPConnection::StopConnection( )
	{
	__FLOG(_L8("StopConnection - Entry"));

	if(ConnectionOpen( ) )
		{
		__FLOG(_L8("Stopping socket handlers"));
		iEventHandler->Cancel( );
		iCommandHandler->Cancel( );
		if(iTransactionState == EDataIToRPhase )
			{
			__FLOG(_L8("Aborting active I to R data phase"));
			TRAPD(err, BoundProtocolLayer().ReceiveDataCompleteL(KErrAbort, *iPTPIPDataContainer->Payload(), iMTPRequest));
			UNUSED_VAR(err);
			}
		else
			if(iTransactionState == EDataRToIPhase )
				{
				__FLOG(_L8("Aborting active R to I data phase"));
				TRAPD(err, BoundProtocolLayer().SendDataCompleteL(KErrAbort, *iPTPIPDataContainer->Payload(), iMTPRequest))	;
				UNUSED_VAR(err);
				}

		__FLOG(_L8("Notifying protocol layer connection closed"));
		iConnectionMgr->ConnectionClosed(*this );
		SetTransactionPhase(EUndefined );
		SetConnectionState(EIdle );
		}

	__FLOG(_L8("StopConnection - Exit"));
	}

/**
 * Invoked by the SocketHandler when there is an error.
 */
#ifdef __FLOG_ACTIVE
void CPTPIPConnection::HandleError(TInt aError)
#else
void CPTPIPConnection::HandleError(TInt /*aError*/)
#endif
	{
	__FLOG_VA((_L8("SocketHandler received an error=%d, stopping connection.)"),aError));
	StopConnection();
	}

/**
 Used to trigger the RunL, by first setting itself to active and 
 then simulating a fake asynchronous service provider which will complete us 
 with a completion code.
 */
void CPTPIPConnection::CompleteSelf(TInt aCompletionCode )
	{
	// Setting ourselves active to wait to be done by ASP.
	SetActive( );

	// Simulating a fake ASP which completes us.
	TRequestStatus* stat = &iStatus;
	User::RequestComplete(stat, aCompletionCode );
	}

/**
 Setter for transaction phase(request, dataItoR, dataRtoI, or response)
 */
void CPTPIPConnection::SetTransactionPhase(TMTPTransactionPhase aPhase )
	{
	__FLOG(_L8("SetTransactionPhase - Entry"));
	iTransactionState = aPhase;
	__FLOG_VA((_L8("Transaction Phase set to 0x%08X"), iTransactionState));
	__FLOG(_L8("SetTransactionPhase - Exit"));
	}

/**
 Setter for connection state,( initialising, send data, etc)
 */
void CPTPIPConnection::SetConnectionState(TConnectionState aState )
	{
	__FLOG(_L8("SetConnectionState - Entry"));
	iState = aState;
	__FLOG_VA((_L8("Connection state set to 0x%08X"), iState));
	__FLOG(_L8("SetConnectionState - Exit"));
	}

/**
 Getter 
 */
TBool CPTPIPConnection::ConnectionOpen( ) const
	{
	return((iState >= EInitialising) && (iState <= EDataSendFinished));
	}

/**
 Getter for the command container, 
 */
CPTPIPGenericContainer* CPTPIPConnection::CommandContainer( )
	{
	return iPTPIPCommandContainer;
	}

/**
 Getter for the event container
 */
CPTPIPGenericContainer* CPTPIPConnection::EventContainer( )
	{
	return iPTPIPEventContainer;
	}

/**
 Getter for the data container
 */
CPTPIPDataContainer* CPTPIPConnection::DataContainer( )
	{
	return iPTPIPDataContainer;
	}

/**
 Getter for the transaction phase: request, dataItoR, dataRtoI or response
 */
TMTPTransactionPhase CPTPIPConnection::TransactionPhase( ) const
	{
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
	__FLOG(_L8("ValidateAndSetCommandPayload - Entry"));

	TUint32 containerType = CommandContainer()->Uint32L(CPTPIPGenericContainer::EPacketType );
	
	__FLOG_VA((_L8("PTP packet type  = %d, adjust payload accordingly"), containerType));

	switch(containerType )
		{
		case EPTPIPPacketTypeOperationRequest:
			if (!ValidateTransactionPhase(ERequestPhase ))
				{
				__FLOG(_L8("PTPIP ERROR: Request data unexpected in this phase, setting type to undefined"));
				containerType = EPTPIPPacketTypeUndefined;
				}
			// Nothing to do , the payload is already set.  In case this is unexpected, 
			//then the validate function will close the connection. 
			break;

		case EPTPIPPacketTypeStartData:
			if (!ValidateTransactionPhase(EDataIToRPhase ))
				{
				__FLOG(_L8("PTPIP ERROR: Start data unexpected in this phase, setting type to undefined"));
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
			__FLOG(_L8("PTPIP ERROR: Response not expected from the initiator, setting type to undefined"));
			containerType = EPTPIPPacketTypeUndefined;			
			// As per the protocol, the initiator cannot send a response, 
			// only the responder( here device)  will create a response, 
			// if this is recieved it is an erro
			break;
			

		default:
			__FLOG_VA((_L8("PTPIP ERROR: Invalid packet type received %d )"), containerType));
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	__FLOG(_L8("ValidateAndSetCommandPayload - Exit"));
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
	__FLOG(_L8("ValidateDataPacketL - Entry"));

	TUint32 containerType = DataContainer()->Uint32L(CPTPIPDataContainer::EPacketType );
	__FLOG_VA((_L8("PTP data packet type  = %d, "), containerType));

	switch(containerType )
		{
		case EPTPIPPacketTypeData:
		case EPTPIPPacketTypeEndData:
			if (!ValidateTransactionPhase(EDataIToRPhase ))
				{
				__FLOG(_L8("PTPIP ERROR: Receiving data unexpected in this phase, setting type to undefined"));
				containerType = EPTPIPPacketTypeUndefined;
				}
			break;

		default:
			__FLOG_VA((_L8("PTPIP ERROR: Unexpected or Invalid packet type received while expecting data packet%d )"), containerType));
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	__FLOG(_L8("ValidateDataPacket - Exit"));
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
	__FLOG(_L8("ValidateAndSetEventPayload - Entry"));

	TUint32 containerType = EventContainer()->Uint32L(CPTPIPGenericContainer::EPacketType );
	__FLOG_VA((_L8("PTP event packet type  = %d, adjust payload accordingly"), containerType));

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
			__FLOG_VA((_L8("PTPIP ERROR: Invalid packet type received %d )"), containerType));
			containerType = EPTPIPPacketTypeUndefined;
			break;
		}

	__FLOG(_L8("ValidateAndSetEventPayload - Exit"));
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
	__FLOG(_L8("ValidateTransactionPhase - Entry"));
	__FLOG_VA((_L8("transaction state = %d"), iTransactionState));
	TBool valid(iTransactionState == aExpectedTransactionState);
	if(!valid )
		{
		// Invalid transaction state, close the connection.
		__FLOG_VA((_L8("PTPIP ERROR: invalid transaction state, current = %d, expected = %d"), iTransactionState, aExpectedTransactionState));
		CloseConnection( );
		}
	__FLOG(_L8("ValidateTransactionPhase - Exit"));
	return valid;
	}

/**
 Convert the TCP errors, the disconnect should be reported as an abort, 
 since that is what the MTP frameword expects.
 */
TBool CPTPIPConnection::HandleTCPError(TInt& aError )
	{
	__FLOG(_L8("TCPErrorHandled - Entry"));
	TInt ret(EFalse);
	if(aError == KErrDisconnected || aError == KErrEof)
		{
		aError = KErrAbort;
		CloseConnection( );
		ret = ETrue;
		}
	__FLOG(_L8("TCPErrorHandled - Exit"));
	return ret;
	}

void CPTPIPConnection::SetDataTypeInDataContainerL(TPTPIPPacketTypeCode aType )
	{
	iPTPIPDataContainer->SetUint32L(CPTPIPDataContainer::EPacketType, aType );
	}



