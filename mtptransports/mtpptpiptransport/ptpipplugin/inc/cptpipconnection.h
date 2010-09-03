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


#ifndef CPTPIPCONNECTION_H_
#define CPTPIPCONNECTION_H_

#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptypenull.h>


#include "mmtpconnectionmgr.h"
#include "mmtpconnectionprotocol.h"


#include "mmtptransportconnection.h"
#include "ptpipdatatypes.h"
#include "cptpipgenericcontainer.h"
#include "cptpipdatacontainer.h"
#include "tptpiprequestpayload.h"
#include "tptpipresponsepayload.h"
#include "tptpipstartdatapayload.h"
#include "tptpipinitevtack.h"

#include "ptpipprotocolconstants.h"



class CPTPIPCommandHandler;
class CPTPIPEventHandler;
class MMTPConnectionProtocol;
class MMTPConnectionMgr;

/**
Implements the MTP USB device class connection protocol and transport layer 
interface.

This is the class implementing the main API functions exposed to the framework layer
for exchange of data with the transport layer.

It also has the communication protocol which exposes the SPI / observer functions 
exposed by the framework layer , which is used by the transport to communicate with 
the framework. 

It implements a state machine which keeps track of whether we are sending data , 
expecting response etc. 

The error handling strategy is as follows:
If unexpected data is received on the event channel , we close the connection.
If unexpected data is received on the command channel the we close the connection.
If the internal state is invalid, then a panic is raised. 
Any errors received, are passed up to the MTP fw, except for TCP disconnection
which is converted to KErrAbort. 

@internalComponent
*/
class CPTPIPConnection : public CActive,
						 public MMTPTransportConnection
{
public :

	enum TConnectionState
		{
		EIdle                   = 0x00000000,
		EInitialising           = 0x00000001,
		EInitialisationComplete = 0x00000002,
		EStartListening         = 0x00000003,
		EDataSendInProgress     = 0x00000004,
		EDataSendFinished       = 0x00000007,
		ECancelled				= 0x00000008
		};

	
	enum TCancelState
		{
		ECancelNotReceived        = 0x00000000,
		
		ECancelCmdReceived        = 0x10000001,
		ECancelCmdHandleInProgress= 0x10000002,
		ECancelCmdHandled         = 0x10000003,
		
		ECancelEvtReceived        = 0x20000001,
		ECancelEvtHandled         = 0x20000003,
		
		ECancelChannel            = 0xF0000000
		};


	static CPTPIPConnection* NewL(MMTPConnectionMgr& aConnectionMgr);
	~CPTPIPConnection();
	

public : // from MMTPTransportConnection
	virtual void BindL(MMTPConnectionProtocol& aProtocol) ;
	virtual MMTPConnectionProtocol& BoundProtocolLayer(); 
	virtual void CloseConnection() ;
	virtual void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest) ;
	virtual void ReceiveDataCancelL(const TMTPTypeRequest& aRequest) ;
	virtual void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest) ; 
	virtual void SendDataCancelL(const TMTPTypeRequest& aRequest) ;
	virtual void SendEventL(const TMTPTypeEvent& aEvent) ;
	virtual void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest) ;
	virtual void TransactionCompleteL(const TMTPTypeRequest& aRequest) ;
	virtual void Unbind(MMTPConnectionProtocol& aProtocol) ;
	virtual TAny* GetExtendedInterface(TUid aInterfaceUid) ; 
	virtual TUint GetImplementationUid();
	
public : //from CActive
	void DoCancel();
	void RunL();
	TInt RunError(TInt aError);
	
public : // Other functions
	void GetCancelPayload();
	void CompleteSelf(TInt aCompletionCode);
	void SetConnectionState(TConnectionState);
	
	// Receiving functions:
	void ReceiveCommandChannelCompleteL(TInt aError, MMTPType& aSource);
	
	void ReceiveEventCompleteL(TInt aError, MMTPType& aSource);
	void ReceiveCommandCompleteL(TInt aError);
	
	void ReceiveCommandDataL(MMTPType& aData) ;
	void ReceiveCommandDataCompleteL(TInt aError);
	
	

	// Sending funtions
	void SendCommandChannelCompleteL(TInt aError, const MMTPType& aSource);
	
	void SendResponseCompleteL(TInt aError, MMTPType& aSource);
	void SendEventCompleteL(TInt aError, const MMTPType& aSource);

	void SendStartDataPacketL();
	void SendDataPacketL();
	void SendCommandCompleteL(TInt aError);
	void SendCommandDataCompleteL(TInt aError);

	
	CPTPIPGenericContainer* CommandContainer();
	CPTPIPGenericContainer* EventContainer();
	CPTPIPDataContainer* DataContainer();
	TUint32 ValidateAndSetCommandPayloadL();
	TUint32 ValidateAndSetEventPayloadL();
	TUint32 ValidateDataPacketL();
	TMTPTransactionPhase TransactionPhase() const;
	void HandleError(TInt aError);
	void SetDataTypeInDataContainerL(TPTPIPPacketTypeCode aType);
	TBool ConnectionOpen() const;

private:
	CPTPIPConnection(MMTPConnectionMgr& aConnectionMgr);
	void ConstructL();	

	void TransferSocketsL();

	void InitiateCommandRequestPhaseL();
	void InitiateEventRequestPhaseL();
	
	void SendInitAckL();
	
	void DoSendDataL();
	void DoReceiveDataL();
	
	// Cancel handling functions
	void HandleEventCancelL();
	void HandleCommandCancelL(TUint32 aTransId);
	void HandleCommandCancelCompleteL();
	void SendCancelToFrameworkL(TUint32 aTransId);
	void SendCancelResponseL(TUint32 aTransId);
	void HandleCancelDuringSendL();
	
	void SetTransactionPhase(TMTPTransactionPhase);
	TBool ValidateTransactionPhase(TMTPTransactionPhase aExpectedTransactionState);
	TBool HandleTCPError(TInt& aError);
	void StopConnection();
	
	void SetNULLPacketL();
	
private: // Owned

	/**
	The current state of the MTP transaction, ( request, response, data phase) 
	*/
	TMTPTransactionPhase        iTransactionState;
	
	/**
	Current state of the Connection, ( whether its initialisting, transferring data, listening)
	*/
	TInt                        iState;
	
	/**
	Flag, which is turned on when the cancel operation is received on the command channel. 
	*/
	TCancelState                iCancelOnCommandState;
	
	/**
	Flag, which is turned on when the cancel operation is received on the event channel. 
	*/
	TCancelState                iCancelOnEventState;
	
	/**
	During the sending of data this keeps track of the total data to send
	*/
	TUint64                     iTotalDataLen;

	/**
	Command handler to send the data via the command socket.
	*/
	CPTPIPCommandHandler*       iCommandHandler;
	
	/**
	EventHandler to send the data via the event socket.
	*/
	CPTPIPEventHandler*         iEventHandler;
	
	// Command Container & payloads //
	/**
	Command Container, its payload can be request, response, start data or cancel. 
	*/
	CPTPIPGenericContainer*     iPTPIPCommandContainer;
	
	/**
	Command Request Paramenter Payload
	4 - data phase
	2 - op code
	4 - tran id
	20 - 5 params
	*/
	TPTPIPTypeRequestPayload    iPTPIPRequestPayload;
	
	/**
	CommandData Response Parameter Payload
	2 - res code
	4 tran id
	20 - 5 params
	*/
	TPTPIPTypeResponsePayload   iPTPIPResponsePayload;
	
	/**
	StartData parameter payload
	4 - tran id
	8 - total length
	*/
	TPTPIPTypeStartDataPayload  iPTPIPStartDataPayload;
	
	/**
	Cancel data Parameter payload
	4 trans id
	*/
	TMTPTypeInt32               iPTPIPCommandCancelPayload;
	
	
	// Event Container & payloads //
	/**
	Event Container, its payload can be event, cancel or probe (ie no payload) 
	*/
	CPTPIPGenericContainer*     iPTPIPEventContainer;
	
	/**
	Event Payload 
	2 - res code
	4 tran id
	20 - 5 params
	*/
	TPTPIPTypeResponsePayload   iPTPIPEventPayload;

	/**
	Event Payload 
	*/
	TMTPTypeUint32              iPTPIPEventCancelPayload;

	// Data Container ( payload comes from mtp f/w) //
	/**
	4 - tran id
	? - payload, given by the framework. 
	*/
	CPTPIPDataContainer*        iPTPIPDataContainer;

	/** 
    The current active MTP SessionID, set while sending response to open session.
    */
    TUint32                     iMTPSessionId;
    
    /**
    The active MTP operation request dataset buffer.
    */
    TMTPTypeRequest             iMTPRequest;
    
    /**
    The MTP event dataset buffer.
    */
    TMTPTypeEvent               iMTPEvent;
	
	/**
    The total amount of data expected, this is filled from the ptpip start data packet. 
    */
    TUint64	                    iTotalRecvData;
	
	/**
    The data receved so far. 
    */
    TUint64	                    iRecvData;
    
	/**
	Data sink for consuming data during error recovery from
	a failed ItoR transaction.
	*/
	TMTPTypeNull                iNull;
	
	/**
	Buffer for reading discarded data into.
	*/
	RBuf8                       iNullBuffer; 
	
private: // Not Owned

	/** 
    The MTP connection manager.
    */
    MMTPConnectionMgr*			iConnectionMgr;

	/** 
    The MTP connection protocol layer binding. 
    This is the SPI/ observer - transport uses it to communicate to the MTP framework.
    */
    MMTPConnectionProtocol*		iProtocolLayer;

};


#endif /*CPTPIPCONNECTION_H_*/
