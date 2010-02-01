// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <charconv.h>
#include <utf.h>
#include <obex.h>
#include <obextransportinfo.h>
#include <obex/transport/obextransportcontrollerbase.h>
#include <obex/internal/obextransportconstants.h>
#include <obex/internal/obexinternalheader.h>
#include <obex/internal/obexpacket.h>
#include "logger.h"
#include "obexsetpathdata.h"
#include "OBEXUTIL.H"
#include "obexheaderutil.h"
#include "authentication.h"
#include "obexpacketsignaller.h"
#include "obexpackettimer.h"
#include "obexnotifyhandlerclient.h"
#include "obexerrorengine.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

// Constant used to identify if the last received response code field
// has been populated (i.e., if the first response has been received
// from the OBEX server).  The response code field is only 7 bits (+ the
// 'final bit') so the use of this 16 bit number will ensure the value
// for KUnpopulatedResponseCode never overlaps with an actual code.
const static TUint16 KUnpopulatedResponseCode = 0xffff;

// For debug builds check that when an underlying error is set
// it has not already been set since the start of the client operation
// and check that a last error has been set when the operation completes.
#define SET_LAST_ERROR(_error) __ASSERT_DEBUG(!iIsLastErrorSet, IrOBEXUtil::Fault(ELastErrorSetTwice)); iErrorEngine->SetLastError(CObexErrorEngine::_error); iIsLastErrorSet = ETrue
#define CHECK_LAST_ERROR_IS_SET __ASSERT_DEBUG(iIsLastErrorSet, IrOBEXUtil::Fault(ELastErrorNotSet)); iIsLastErrorSet = EFalse


// Constructor - set initial values
CObexClient::CObexClient() : CObex()
	{
	iCurrentOperation = EOpIdle;
	iConnectionID = KConnIDInvalid;
	iLastReceivedResponseOpcode = static_cast<TObexResponse>(KUnpopulatedResponseCode);
	}

void CObexClient::ConstructL(TObexTransportInfo& aObexTransportInfo)
	{
	CObex::ConstructL(aObexTransportInfo);
	iNotifyHandler = new(ELeave)CObexNotifyHandlerClient(*this);
	iTransportController->SetOwner(*iNotifyHandler);

	//	iHeaderSet is used to store headers received in Put Response
	iHeaderSet = CObexHeaderSet::NewL();
	iHeader = CObexHeader::NewL();
	iPacketProcessSignaller = CObexPacketSignaller::NewL();
	iErrorEngine = CObexErrorEngine::NewL();
	iPacketTimer = CObexPacketTimer::NewL(*this);
	}

/** Destructor. 
	
@publishedAll
@released
*/
EXPORT_C CObexClient::~CObexClient()
	{
	LOG_LINE
	LOG_FUNC

	Error(KErrCompletion);
	delete iHeader;
	delete iHeaderSet;
	delete iPacketProcessSignaller;
	delete iNotifyHandler;
	delete iErrorEngine;
	delete iPacketTimer;
	}

/** A call back from the the service with the password required for use with generating 
the challenge response. 

@param aPassword Password

@leave KErrNotReady if this function is not called from a MObexAuthChallengeHandler::GetUserPasswordL callback.

@publishedAll
@released
*/
EXPORT_C void CObexClient::UserPasswordL(const TDesC& aPassword)
	{
	LOG_LINE
	LOG_FUNC

	//now have a password, get a nonce, and get it hashed then reply
	FLOG(_L("CObexClient::UserPasswordL\n\r"));
	if (GetConnectState() == EWaitForUserInput)
		{
		PrepareChallResponseL( aPassword); 

		TObexInternalHeader hdr;
		hdr.Set(TObexInternalHeader::EAuthResponse, CONST_CAST(TUint8*, iOutgoingChallResp.Ptr()), iOutgoingChallResp.Size());
		FLOG(_L("CObexClient::UserPasswordL EAuth heaqder added\n\r")); 
		if(iTransportController->SendPacket().InsertData(hdr))
			{
			SetConnectState(EWaitForFinalResponse);
			iTransportController->SendPacket().SetFinal();
			SendRequestPacket();
			FLOG(_L("CObexClient::UserPasswordL packet sent\n\r")); 

			}
		else
			{
			LEAVEIFERRORL(KErrGeneral);
			}
		}
	else
		{
		LEAVEIFERRORL(KErrNotReady);
		}
	}

/** Allocates and constructs a new OBEX client object.

The received protocol information object, aObexProtocolInfoPtr, specifies the transport protocol to use:
For the standard transports the following are used, TObexIrProtocolInfo for IrDA, TObexBluetoothProtocolInfo 
for Bluetooth, TObexUsbProtocolInfo for USB.

@param aObexProtocolInfoPtr Protocol information object describing the transport to use
@return New OBEX client object 
	
@publishedAll
@released
*/
EXPORT_C CObexClient* CObexClient::NewL(TObexProtocolInfo& aObexProtocolInfoPtr)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	TObexProtocolPolicy defaultProtocolPolicy;	// no packet sizing policy specified, so use default	
	TObexTransportInfo*  transportInfo = IrOBEXUtil::CreateTransportInfoL(aObexProtocolInfoPtr, defaultProtocolPolicy);
	CleanupStack::PushL(transportInfo);
	CObexClient* client = CObexClient::NewL(*transportInfo);
	CleanupStack::PopAndDestroy(transportInfo);
	return client;
	}

/** Allocates and constructs a new OBEX client object with packet sizing information.

The received protocol information object, aObexProtocolInfoPtr, specifies the transport protocol to use:
For the standard transports the following are used, TObexIrProtocolInfo for IrDA, TObexBluetoothProtocolInfo 
for Bluetooth, TObexUsbProtocolInfo for USB.

The aObexProtocolPolicy parameter specifies the packet sizing policy for this OBEX object.

@param aObexProtocolInfoPtr Protocol information object describing the transport to use
@param aObexProtocolPolicy Protocol policy object specifying the packet sizes to use
@return New OBEX client object 
	
@publishedAll
@released
*/
EXPORT_C  CObexClient* CObexClient::NewL(TObexProtocolInfo& aObexProtocolInfoPtr, TObexProtocolPolicy& aObexProtocolPolicy)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	TObexTransportInfo*  transportInfo = IrOBEXUtil::CreateTransportInfoL(aObexProtocolInfoPtr, aObexProtocolPolicy);
	CleanupStack::PushL(transportInfo);
	CObexClient* client = CObexClient::NewL(*transportInfo);
	CleanupStack::PopAndDestroy(transportInfo);
	return client;
	}

/** Allocates and constructs a new OBEX client object with packet sizing information.

The received transport information object, aObexTransportInfo, specifies the transport protocol packet sizes to use:
For the standard transports the following are used, TObexIrProtocolInfo for IrDA, TObexBluetoothProtocolInfo 
for Bluetooth, TObexUsbProtocolInfo for USB.

@param aObexTransportInfo  Transport information object describing the transport and packet sizes to use
@return New OBEX client object 

@capability WriteDeviceData If the TObexIrV3TransportInfo is passed as the argument
                            and the associated name is valid.
	
@publishedAll
@released
*/
EXPORT_C  CObexClient* CObexClient::NewL(TObexTransportInfo& aObexTransportInfo)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexClient* self = new(ELeave) CObexClient();
	CleanupStack::PushL(self);
	self->ConstructL(aObexTransportInfo);
	CleanupStack::Pop(self);
	return(self);
	}

/** OBEX CONNECT operation to any available remote machine, specifying an object 
to pass.

@param aObject OBEX object to pass to the remote machine
@param aStatus Asynchronous status word. On completion, KErrNone if it was 
successful, or a system-wide error code 	

@publishedAll
@released
*/
EXPORT_C void CObexClient::Connect(CObexBaseObject& aObject, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::Connect no password but some header data\r\n"));
		EmptyHeaderSet();
		iChallenge = EFalse;
		OBEX_TRAP(Error, ClientCommandL(EOpConnect, static_cast<TAny*>(&aObject), aStatus));
		}
	}

/** OBEX CONNECT operation to any available remote machine, specifying an object 
to pass and a password.

@param aObject OBEX object to pass to the remote machine
@param aPassword Password to access remote machine
@param aStatus Asynchronous status word. On completion, KErrNone if it was 
successful, or a system-wide error code 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::ConnectL(CObexBaseObject& aObject, const TDesC& aPassword, 
								   TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::Connect with password and some header info\r\n"));
		EmptyHeaderSet();
		delete iChallPassword;
		iChallPassword = NULL;
		iChallPassword = HBufC8::NewL(aPassword.Size());
		TPtr8 ptr = iChallPassword->Des();
		CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aPassword);
		iChallenge = ETrue;
		OBEX_TRAP(Error, ClientCommandL(EOpConnect, static_cast<TAny*>(&aObject), aStatus));
		}
	}


/** OBEX CONNECT operation to any available remote machine, specifying a password.

@param aPassword Password to access remote machine
@param aStatus On completion, KErrNone if it was successful, or a system-wide 
error code 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::ConnectL(const TDesC& aPassword, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::Connect with password but no header info\r\n"));
		EmptyHeaderSet();
		delete iChallPassword;
		iChallPassword = NULL;
		iChallPassword = HBufC8::NewL(aPassword.Length());
		TPtr8 ptr = iChallPassword->Des();
		CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aPassword);
		iChallenge = ETrue;
		OBEX_TRAP(Error, ClientCommandL(EOpConnect, NULL, aStatus));
		}
	}

/** OBEX CONNECT operation to any available remote machine.

@param aStatus Asynchronous status word. On completion, KErrNone if it was 
successful, or a system-wide error code 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::Connect(TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("\tno password no header"));
		EmptyHeaderSet();
		iChallenge = EFalse;
		OBEX_TRAP(Error, ClientCommandL(EOpConnect, NULL, aStatus));
		}
	}

/** OBEX DISCONNECT operation.

This terminates the OBEX connection, and closes the transport on receiving 
any response from the server. 

@param aStatus Asynchronous status word. On completion, KErrNone on normal 
disconnection, or KErrDisconnected if the server dropped the transport before 
sending an OBEX response (which is valid behaviour). 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::Disconnect(TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::Disconnect\r\n"));
		EmptyHeaderSet();
		OBEX_TRAP (Error, ClientCommandL(EOpDisconnect, NULL, aStatus));
		}
	}

/** OBEX PUT operation.

Any valid header that is also present in aObject’s header mask will be sent 
to the server, along with the object body specified by the implementation 
of aObject. 

@param aObject OBEX object to put
@param aStatus Asynchronous status word. On completion, KErrNone if the server 
accepted the object and received it fully, or the appropriate file error if 
the data file could not be opened 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::Put(CObexBaseObject& aObject, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("\tnot already active"));
		EmptyHeaderSet();
		OBEX_TRAP(Error, ClientCommandL(EOpPut, static_cast<TAny*>(&aObject), aStatus));
		}
	}

/** OBEX GET operation.

The caller specifies in aObject the headers to send to the server to specify 
the object to get: normally just a name is expected. If the server can serve 
the request, the object it returns will be loaded into aObject on completion. 
All headers returned by the server that are also allowed by the object’s 
header mask will be loaded into the relevant attributes of aObject. Any object 
body is stored according to the implementation type of the CObexBaseObject 
passed.

@param aObject OBEX object to get; on completion, the retrieved object
@param aStatus Asynchronous status word. On completion, KErrNone if the server 
passed back an OBEX object, or the appropriate file error if the data file 
could not be opened 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::Get(CObexBaseObject& aObject, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::Get\r\n"));
		EmptyHeaderSet();
		OBEX_TRAP(Error, ClientCommandL(EOpGet, static_cast<TAny*>(&aObject), aStatus));
		}
	}

/** OBEX SETPATH operation.

This changes the remote device's current path. 

@param aPathInfo Information to send in the SETPATH command. If you do not 
which to send a name, make sure CObex::TSetPathInfo::iNamePresent is set to 
false.
@param aStatus Asynchronous status word. On completion, KErrNone if successful, 
or a system-wide error code 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::SetPath(TSetPathInfo& aPathInfo, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC

	if(!AlreadyActive(aStatus))
		{
		FLOG(_L("CObexClient::SetPath\r\n"));
		EmptyHeaderSet();
		OBEX_TRAP(Error, ClientCommandL(EOpSetPath, static_cast<TAny*>(&aPathInfo), aStatus));
		}
	}

/** OBEX ABORT operation.

The function sends the OBEX abort command to the remote machine if a multi-packet 
operation (i.e. PUT or GET) is in progress. An operation in progress will 
complete with KErrAbort. 
	
@publishedAll
@released
*/
EXPORT_C void CObexClient::Abort()
	{
	LOG_LINE
	LOG_FUNC

	if(iPendingRequest &&(iCurrentOperation == EOpPut 
							|| iCurrentOperation == EOpGet || iCurrentOperation == EOpGetResponse))
		{
		// We have two phase abort, async. abort request, followed by async. response
		// To achive this without extra members we use extra operations in TOperation
		// EOpAbortNoFBit for the asynchronous request
		// EOpAbort for waiting for the response
		iCurrentOperation = EOpAbortNoFBit;
		}
	}

/**
This function can be called following the successful completion of a Put,
and will return a reference to a CObexHeaderSet containing all the headers
that were contained in the final Put response packet returned from the 
peer Obex server.

The headers in the header set will be deleted by any subsequent call to 
CObexClient functions that trigger Obex commands (ie. Connect, Disconnect, 
Put, Get, SetPath).

The reference should not be retained beyond the end of the lifetime of the
CObexClient object.

@publishedAll
@released

@return const reference to a Headerset containing headers returned in final
Put response packet.
*/
EXPORT_C const CObexHeaderSet& CObexClient::GetPutFinalResponseHeaders()
	{
	LOG_LINE
	LOG_FUNC

	return *iHeaderSet;
	}

/** Sets a final packet observer.

This replaces any previous observer.  The observer will receive a callback
when a final packet for a put or get request begins to be sent and 
another when the send completes.  Although the start and finish callbacks
are guaranteed to be in order, no guarantees are made about the ordering
with respect to the completion of the put or get request.

This does not transfer ownership.

@publishedAll
@released
@param aObserver The observer to receive packet process events.  This may
				 be NULL.
*/
EXPORT_C void CObexClient::SetFinalPacketObserver(MObexFinalPacketObserver* aObserver)
	{
	iPacketProcessSignaller->SetFinalPacketObserver(aObserver);
	}

/** Get last server response code
This method returns the last received OBEX response code.
The method must not be called prior to a response notification being received by the
client application.  If the method is called a panic will be raised. 

@return The last received OBEX response code.
@panic ENoResponseCodeToReturn   Panics if the method is called prior to a response being received from the OBEX server.  
@publishedAll
@released
*/
EXPORT_C TObexResponse CObexClient::LastServerResponseCode() const
	{	
	LOG_LINE
	LOG_FUNC
	// If the last received response code has not been set (i.e., no response has been received) then panic
	// the client.
	__ASSERT_ALWAYS(iLastReceivedResponseOpcode != KUnpopulatedResponseCode, IrOBEXUtil::Panic(ENoResponseCodeToReturn));
	return iLastReceivedResponseOpcode;
	}

/** Sets the Command Idle Time-out.

This function sets the timer period to be observed during the progress of
all OBEX commands. If the OBEX server does not respond within the set period
the current OBEX command will be completed with an error of KErrIrObexRespTimedOut
and the transport will be disconnected. 

Setting a time-out value of 0, all OBEX commands will be allowed to continue
indefinitely.

@param aTimeOut The time-out period in Microseconds.
@publishedAll
@released
*/
EXPORT_C void CObexClient::SetCommandTimeOut(TTimeIntervalMicroSeconds32 aTimeOut)
	{
	LOG_LINE
	LOG_FUNC	
	iCmdTimeOutDuration = aTimeOut;	
	}

TBool CObexClient::AlreadyActive(TRequestStatus& aStatus)
//can't do more than one request at atime
	{
	if(iPendingRequest)
		{
		TRequestStatus* tempStatus = &aStatus;
		User::RequestComplete(tempStatus,KErrAccessDenied);
		return ETrue;
		}
	return EFalse;
	}

void CObexClient::ResetConnectionID()
	{
	iConnectionID = KConnIDInvalid;
	iConnectionIdSet = EFalse;
	}
			
void CObexClient::SetConnectionID(TUint32 aConnectionID)
	{
	iConnectionID = aConnectionID;
	iConnectionIdSet = ETrue;
	}
	
void CObexClient::ClientCommandL(TOperation aOp, TAny* aParam, TRequestStatus& aStatus)
	{
	LOG_LINE
	LOG_FUNC
	
	// Note: Function Calls to this method must be enclosed with an if statement utilising
	// 		 CObexClient::AlreadyActive

	SetRequest(aStatus, aOp);
	if(aOp != EOpConnect &&(GetConnectState() != EConnObex || iTransportController == NULL))
		{
		SET_LAST_ERROR(EDisconnected);
		LEAVEIFERRORL(KErrDisconnected);
		}

	switch(aOp)
		{
	case EOpConnect:
		{
		switch(GetConnectState())
			{
		case EConnIdle:
			{
			// Don t start the Packet timer, It will be done once the Transport is up(see :onTransportUp)
			ResetConnectionID(); //set connection ID to invalid and flag to EFalse
			iCurrentObject = static_cast<CObexBaseObject*>(aParam);
			TRAPD(err, iTransportController->ConnectL());
			if (err != KErrNone)
				{
				SET_LAST_ERROR(ETransportUpFailed);
				LEAVEIFERRORL(err);
				}
			}
 			break;
		case EConnTransport:
			iCurrentObject = static_cast<CObexBaseObject*>(aParam);
			//Transport is already connected just do the connection
			//Previously TransportUp was called.  However the transport is already connected
			//thus the only required actions are the following.  
			OnTransportUp();
			iTransportController->Receive();
			break;
		default:
			SET_LAST_ERROR(EAlreadyConnected);
			CompleteRequest(KErrAlreadyExists);
			}
		}
		break;
	case EOpDisconnect:
		{
		iTransportController->SendPacket().Init(TUint8(EOpDisconnect));
		iTransportController->SendPacket().SetFinal();

		// Insert the ConnectionID if necessary
		if (iConnectionIdSet)
			{
						 
			TObexInternalHeader connectionID; 
			connectionID.Set(TObexInternalHeader::EConnectionID, iConnectionID);
			 
			if(!iTransportController->SendPacket().InsertData(connectionID))
				{
				LEAVEIFERRORL(KErrOverflow);
				}
			}
		SendRequestPacket();
		}
		break;
	case EOpPut:
	case EOpGet:
	// The Put and Get "request" state machines are identical
	// and live inside the object streamer.
		{
		//
		// For 'Get' the params are what to 'Get' and where to put it
		// For 'Put' the params are what to put
		// Either way we need to send a request based on the params
		iCurrentObject = static_cast<CObexBaseObject*>(aParam);

		// Initialise the object ready for sending
		TInt err = iCurrentObject->InitSend(TObexOpcode(aOp));
		if (err != KErrNone)
			{
			SET_LAST_ERROR(ECannotInitialiseObject);
			LEAVEIFERRORL(err);
			}

		//if a ConnectionID was added then it's important to add the Connection ID
		//header here, allow it to be processed by PrepareNextSendPacket
		if ( iConnectionIdSet )
			{
			FLOG(_L("\tiConnectionIdSet is true"));
			TRAPD(err, iCurrentObject->SetConnectionIdL(iConnectionID));
			if (err != KErrNone)
				{
				SET_LAST_ERROR(ECannotSetConnectionId);
				LEAVEIFERRORL(err);
				}
			}

		// If we can...
		if(iCurrentObject->PrepareNextSendPacket(iTransportController->SendPacket()) != CObexBaseObject::EError)
			{	
			iTransportController->SendPacket().AddPacketProcessEvents(EObexFinalPacketStarted | EObexFinalPacketFinished);
					
			// ...send the first request packet
			FLOG(_L("\tsending first request packet..."));
			SendRequestPacket();
			}
		else
			{
			FLOG(_L("\tleaving..."));
			SET_LAST_ERROR(ECannotPreparePacket);
			LEAVEIFERRORL(KErrGeneral);
			}
		}
		break;

	case EOpSetPath:
		{// Build up a setinfo packet and send it.
		TSetPathInfo* info = static_cast<TSetPathInfo*>(aParam);

		iTransportController->SendPacket().Init(TUint8(aOp)); 
		iTransportController->SendPacket().SetFinal();

		TObexSetPathData data;
		data.iFlags = info->iFlags;
		data.iConstants = info->iConstants;
		if(!iTransportController->SendPacket().InsertData(data))
			{
			LEAVEIFERRORL(KErrOverflow);
			}

		//insert the ConnectionID if necessary
		if ( iConnectionIdSet )
			{ 
			
			TObexInternalHeader connectionID;
			connectionID.Set(TObexInternalHeader::EConnectionID, iConnectionID); 
			
			if(!iTransportController->SendPacket().InsertData(connectionID))
				{
				LEAVEIFERRORL(KErrOverflow);
				}
			}
		if(info->iNamePresent)
			{
			TObexInternalHeader name;
			name.Set(TObexInternalHeader::EName, info->iName);
			if(!iTransportController->SendPacket().InsertData(name))
				{
				LEAVEIFERRORL(KErrOverflow);
				}
			}
		SendRequestPacket();
		break;
		}
	default:
		IrOBEXUtil::Fault(EClientCommandOpUnrecognised);
		}

	}

void CObexClient::OnPacketReceive(CObexPacket& aPacket)
	{
	LOG_LINE
	LOG_FUNC
	
	//Cancel the timer
	if (iPacketTimer->IsActive())
		{
		iPacketTimer->Cancel();
		}
	
	// Store the response code including the 'final bit'.
	iLastReceivedResponseOpcode = static_cast<TObexResponse>(aPacket.Opcode() | (aPacket.IsFinal() ? KObexPacketFinalBit : 0));
	
	if(iTransportController->IsWriteActive())
		{
		FLOG(_L("OnPacketReceive received request whilst writing... dropping connection\r\n"));
		SET_LAST_ERROR(EResponseWhileWriting);
		CompleteRequest(KErrNone);
		ControlledTransportDown();
		return;
		}

	// Initialise the send packet to ensure that we do not
	// accidentally send the same packet as last time!
	iTransportController->SendPacket().Init(0);

	switch(iCurrentOperation)
		{
	case EOpConnect:
		{
		FLOG(_L("CObexClient::OnPacketReceive Connect OpCode\r\n"));
		if(ParseConnectPacket(aPacket) == KErrNone)
			{
			FLOG(_L("OnPacketReceive Connect Packet parsed \r\n"));

			//the request is only completed if now connected
			if(GetConnectState() == EConnObex)
				{
				SET_LAST_ERROR(ENoError);
				CompleteRequest(KErrNone);
				}
			else //otherwise still some outstanding issues
				{
				iTransportController->SendPacket().Init(EOpConnect); 
				TInt err = PrepareConnectPacket(iTransportController->SendPacket());
				if( err == KErrNone )
					{
					FLOG(_L("OnPacketReceive PrepareConnectPacket SUCCESS\r\n"));

					iTransportController->SendPacket().SetFinal();
					SendRequestPacket();
					}
				else if ( GetConnectState() != EWaitForUserInput )
					{
					FLOG(_L("OnPacketReceive PrepareConnectPacket FAILED\r\n"));
					// Last underlying error already set in PrepareConnectPacket().
					Error(err);
					}
				}
			}
		else
			{
			FLOG(_L("OnPacketReceive Connect Packet Parse FAILED\r\n"));
			// Last underlying error already set in ParseConnectPacket().
			Error(KErrAbort);
			}
		}
		break;
	case EOpDisconnect:
		{
		FLOG(_L("CObexClient::OnPacketReceive DisConnect OpCode\r\n"));
		switch (aPacket.Opcode())
			{
			case ERespSuccess:
				{
				SET_LAST_ERROR(ENoError);
				CompleteRequest(KErrNone);
				}
				break;
			case ERespServiceUnavailable:
				{
				SET_LAST_ERROR(EBadConnectionId);
				CompleteRequest(KErrDisconnected);
				}
				break;
			default:
				{
				SET_LAST_ERROR(EErrorResponseFromServer);
				CompleteRequest(KErrDisconnected);
				}
			}
		
		ControlledTransportDown(); // Bring down the transport anyway, otherwise we could end up in a situation where we can't
		}
		break;
	case EOpPut:
		{
		FLOG(_L("CObexClient::OnPacketReceive Put OpCode\r\n"));
		if(!aPacket.IsFinal())
			{// Multipacket responses not allowed.
			SET_LAST_ERROR(EMultipacketResponse);
			Error(KErrCommsOverrun);	
			break;
			}
		if(aPacket.Opcode() != ERespContinue 
			&& aPacket.Opcode() != ERespSuccess
			&& aPacket.Opcode() != ERespPartialContent)
			{// Server has returned an OBEX error response. Deal with it...
			TInt err = IrOBEXUtil::EpocError(aPacket.Opcode());
			LOG1(_L8("Put error: %d"), err);
			SET_LAST_ERROR(EErrorResponseFromServer);
			CompleteRequest(err);
			break;
			}

		{
		CObexBaseObject::TProgress progress = 
			iCurrentObject->PrepareNextSendPacket(iTransportController->SendPacket());
		iTransportController->SendPacket().AddPacketProcessEvents(EObexFinalPacketStarted | EObexFinalPacketFinished);

		// Work around for a problem both with some DoCoMo phones and Windows 2000:
		// Their server sends "Success" when it should actually send "Continue",
		// so we accept either here.
		if (((progress == CObexBaseObject::EContinue || progress == CObexBaseObject::ELastPacket)
			&& (aPacket.Opcode () == ERespContinue || aPacket.Opcode () == ERespSuccess))||
			(progress == CObexBaseObject::EComplete 
			&& (aPacket.Opcode() == ERespContinue )))
			{// More stuff to send.
			SendRequestPacket();
			}
		else if(progress == CObexBaseObject::EComplete 
			     && (aPacket.Opcode() == ERespSuccess || aPacket.Opcode() == ERespPartialContent ))
			{// We've completed okay.
			//	There may be headers to extract from this final put response
			TObexInternalHeader header;

			while(aPacket.ExtractData(header))
				{
				FLOG(_L("OnPacketReceive Extracting header from final Put Response"));
				TInt err=IrOBEXHeaderUtil::ParseHeader(header, *iHeaderSet);
				if(err != KErrNone)
					{
					SET_LAST_ERROR(ECannotExtractFinalPutHeader);
					Error(err);
					}
				}
			TInt err = IrOBEXUtil::EpocError(aPacket.Opcode());
			if (err == KErrNone)
				{
				SET_LAST_ERROR(ENoError);
				}
			else
				{
				SET_LAST_ERROR(EErrorResponseFromServer);
				}
			CompleteRequest(err);
			}
		else
			{// We're out of sync with server. Give up.
			SET_LAST_ERROR(EPutOutOfSync);
			Error(KErrGeneral);
			}
		}
		break;
		}
	case EOpGet:
		{
		FLOG(_L("CObexClient::OnPacketReceive Get OpCode\r\n"));
		if(!aPacket.IsFinal())
			{
			SET_LAST_ERROR(EMultipacketResponse);
			Error(KErrCommsOverrun); //??? WTF? Comms overrun?
			break;
			}

		// There's only two valid responses to a 'Get' Request...
		TUint8 respCode = aPacket.Opcode();
		if( respCode != ERespContinue &&
			respCode != ERespSuccess )
			// ... and if we didn't get one of them...
			{
			// ...there's not much else we can do
			SET_LAST_ERROR(EErrorResponseFromServer);
			CompleteRequest(IrOBEXUtil::EpocError(respCode));
			break;
			}

		// Now we know the response was probably valid, see if we need 
		// to send another request packet
		CObexBaseObject::TProgress progress;
		progress = iCurrentObject->PrepareNextSendPacket(iTransportController->SendPacket());
		
		if( progress != CObexBaseObject::EComplete &&
		    progress != CObexBaseObject::ELastPacket)
			// We're not done sending the request yet. 
			{
			// So we'd better not have got a 'Success' from the remote end
			if( progress == CObexBaseObject::EContinue &&
				respCode == ERespContinue)
				{
				iTransportController->SendPacket().AddPacketProcessEvents(EObexFinalPacketStarted | EObexFinalPacketFinished);
				
				// Not finished sending the request yet, so send the next packet
				SendRequestPacket();
				}
			else
				{
				// Something went wrong - can't continue.
				SET_LAST_ERROR(EGetPrematureSuccess);
				CompleteRequest(KErrGeneral);
				}

			// We're done with this packet
			break;
			}

		// Still here? We MUST have got an EComplete from the sending state 
		// machine. That means that the response we're handling RIGHT NOW is 
		// the first packet of the thing we are 'Get'ting.

		// Must initialise the object to receive the data from the server
		iCurrentObject->InitReceive();
		// Change state so we handle the next response properly
		iCurrentOperation = EOpGetResponse;
		}
		// ** NB ** NO BREAK - DROP THROUGH TO HANDLE THE RESPONSE
	case EOpGetResponse:
	// Expecting a possibly multi-packet 'Get' response
		{
		FLOG(_L("CObexClient::OnPacketReceive GetResponse OpCode"));

		if(iCurrentObject->ParseNextReceivePacket(aPacket) == CObexBaseObject::EError)
			{
			SET_LAST_ERROR(EGetResponseParseError);
			Error(KErrGeneral);
			break;
			}
		if(aPacket.Opcode() == ERespContinue)
			{
			// Send a blank 'Get' request to solicit the next bit of the response
			SendRequestPacket(EOpGet);
			FTRACE(iTransportController->SendPacket().Dump());
			}
		else
			{
			// Got all of the response, give it to the user
			if (aPacket.Opcode() == ERespSuccess)
				{
				SET_LAST_ERROR(ENoError);
				}
			else
				{
				SET_LAST_ERROR(EErrorResponseFromServer);
				}
			CompleteRequest(IrOBEXUtil::EpocError(aPacket.Opcode()));
			}
		}
		break;
	case EOpSetPath:
		FLOG(_L("CObexClient::OnPacketReceive Set Path OpCode\r\n"));
		if (aPacket.Opcode() == ERespSuccess)
			{
			SET_LAST_ERROR(ENoError);
			}
		else
			{
			SET_LAST_ERROR(EErrorResponseFromServer);
			}
		CompleteRequest(IrOBEXUtil::EpocError(aPacket.Opcode()));
		break;
	case EOpAbortNoFBit:
		FLOG(_L("CObexClient::OnPacketReceive Abort NoFBit OpCode\r\n"));
		SendRequestPacket(EOpAbort);
		iCurrentOperation = EOpAbort;
		break;
	case EOpAbort:
		{
		FLOG(_L("CObexClient::OnPacketReceive Abort OpCode\r\n"));

		SET_LAST_ERROR(EAborted);
		if(aPacket.IsFinal() && aPacket.Opcode() == ERespSuccess)
			{// Just complete the put/get with code aborted
			CompleteRequest(KErrAbort);
			}
		else
			{// Report an error while aborting -> causes a disconnect
			Error(KErrAbort);
			}
		}
		break;
	default:
//		FPrint(_L("CObexClient::OnPacketReceive unknown opcode 0x%X!"),
//			iCurrentOperation);
		break;
		}

	// Client has finished with the read packet so queue the next read (if necessary)
	if(iConnectState >= EConnTransport)
		{
		iTransportController->Receive();
		}

	}

TInt CObexClient::ParseConnectPacket(CObexPacket& aPacket)
	{
	TInt retValue = KErrNone;
	TConnectState nextState = EDropLink;

	if(!iTransportController || !iTransportController->ExtractRemoteConnectInfo(aPacket, iRemoteInfo.iVersion, iRemoteInfo.iFlags))
		{
		FLOG(_L("CObexClient::ParseConnectPacket Extract Remote Info FAILED\r\n"));
		SET_LAST_ERROR(ECannotExtractConnectInfo);
		return KErrGeneral;
		}
	FLOG(_L("CObexClient::ParseConnectPacket Extract Remote Info Success\r\n"));

	TObexInternalHeader hdr;

	if ( aPacket.Opcode() == ERespSuccess )
		{
		FLOG(_L("ParseConnectPacket ERespSuccess Opcode\r\n"));

		//if a simple connect was originally requested
		//then a simple ERespSuccess without any headers is enough
		if ( GetConnectState() == ESimpleConnRequest )
			nextState = EConnObex;
		//if responding to a chall from the server
		if (( GetConnectState() == EWaitForFinalResponse)&&(!iChallenge)) 
			nextState = EConnObex;
		while(aPacket.ExtractData(hdr))
			{
			switch(hdr.HI()) 
				{
				case TObexInternalHeader::EWho:
					{
					FLOG(_L("ParseConnectPacket Extracting EWHO header\r\n"));
					iRemoteInfo.iWho.Copy(hdr.HVByteSeq(), hdr.HVSize() > iRemoteInfo.iWho.MaxSize() ? iRemoteInfo.iWho.MaxSize() : hdr.HVSize());
					}
				break;
				case TObexInternalHeader::EConnectionID:
					{
					FLOG(_L("ParseConnectPacket Extracting EConnectionID header\r\n"));

					TUint32 newConnectionID = ((hdr.HVByteSeq()[0] << 24) + (hdr.HVByteSeq()[1] << 16) + (hdr.HVByteSeq()[2] << 8) + (hdr.HVByteSeq()[3]));
					SetConnectionID(newConnectionID);
					
					if ( GetConnectState() == ESimpleConnRequest )
						{
						nextState = EConnObex;
						}
					}		
					break;
				case TObexInternalHeader::EAuthResponse: 
					{
					if (iChallenge)
						{
						FLOG(_L("ParseConnectPacket Extracting EAuthResponse header\r\n"));
						//extract the response into it's constituent parts
						TRAPD(err, ProcessChallResponseL(hdr));
						if ( err == KErrNone )
							{
							FLOG(_L("ParseConnectPacket ProcessChallResponse Success\r\n"));
	
							if ((GetConnectState() == EChallConnRequested) || ( GetConnectState() == EWaitForFinalResponse))
								{
								nextState = EConnObex;
								}
							else
								{
								SET_LAST_ERROR(EUnexpectedChallengeResponse);
								nextState = EDropLink;
								retValue = KErrGeneral;
								}
							}
						else
							{
							FLOG(_L("ParseConnectPacket ProcessChallResponse FAILED\r\n"));
							SET_LAST_ERROR(ECannotProcessChallenge);
							nextState = EDropLink;
							retValue = KErrGeneral;
							}
						}
					else
						{
						// if no challenge was issued, then receiving a challenge response means the peer is badly
						// behaved. For this case we simply ignore the header, anything else would be too drastic.
						FLOG(_L("ParseConnectPacket Chall Response received when no Chall issued\r\n"));
						}
					}	
					break;
				default:
					break;
				}//end switch
			}//end while		
		}
	else if (aPacket.Opcode() == ERespUnauthorized )
		{
		FLOG(_L("ParseConnectPacket ERespUnauthorized Opcode\r\n"));
		// Only valid header here is AuthChallenge, if it's absent then authentication failed.
		TBool challengeAbsent = ETrue;
		while(aPacket.ExtractData(hdr))
			{
			switch(hdr.HI()) 
				{
				case TObexInternalHeader::EAuthChallenge: 
					{
					FLOG(_L("ParseConnectPacket Extracting EAuthChallenge header\r\n"));
					challengeAbsent = EFalse;
					TRAPD(err, ProcessChallengeL(hdr));
					if ( !err )
						{
						nextState = EConnChallRxed;
						retValue = KErrNone;
						}
					else
						{
						SET_LAST_ERROR(ECannotProcessChallenge);
						retValue = KErrGeneral;
						}
#ifdef TEST_CLIENT_CHANGES_ITS_MIND_ABOUT_CHALLENGE
						//this will force the client to challenge the server
						//even if it didn't challenge initially
						//this is not standard behaviour for our client
						//but the server must be capable of handling this situation
						iChallenge = ETrue;
#endif
					}
					break;
				default:
				break;
				}//end switch
			}//end while		

		if (challengeAbsent)
			{
			SET_LAST_ERROR(EChallengeAbsent);
			retValue = KErrGeneral;
			}
		}
	else
		{
		FLOG(_L("ParseConnectPacket Unknown Opcode Opcode\r\n"));
		SET_LAST_ERROR(EBadOpcodeInConnectPacket);
		retValue = KErrGeneral;
		}


	SetConnectState(nextState);
	return(retValue);
	}

void CObexClient::OnError(TInt aError)
	{
	LOG1(_L8("CObexClient::OnError(%d)"), aError);
	CompleteRequest(aError);

	// Don't reset the Obex link for a normal disconnection, as
	// technically there is no real error requiring such drastic
	// action.  In the case of USB, stalling the IN endpoint will
	// cause another round of alternate interface changes which
	// appears to confuse either Obex or the USB connector.

	// In other words the error KErrDisconnected occurs as a
	// result of the transport coming down, so there's no need to
	// signal a transport error in this case.

	// If this is called during construction, iTransportController could be NULL
	if(aError!=KErrDisconnected && iTransportController)
		{
		iTransportController->SignalTransportError();
		}
	}



TInt CObexClient::PrepareConnectPacket(CObexPacket& aPacket)
	{
	TInt retValue = KErrNone;
	TConnectState nextState = EDropLink;

	if (!iTransportController->InsertLocalConnectInfo(aPacket, iLocalInfo.iVersion, iLocalInfo.iFlags))
		{
		FLOG(_L("CObexClient::PrepareConnectPacket local data insertion FAILED\r\n"));
		SET_LAST_ERROR(ECannotInsertConnectInfo);
		return(KErrGeneral);
		}
	FLOG(_L("CObexClient::PrepareConnectPacket local data inserted Succesfully\r\n"));

	//iCurrentObject could be NULL if Connect with no headers was requested
	if ( iCurrentObject )
		iCurrentObject->PrepareConnectionHeader(iTransportController->SendPacket());


	if (( iChallenge )&&(!retValue))	//if a challenge is to be sent
		{
		FLOG(_L("PrepareConnectPacket Generating challenge\r\n"));

		retValue = GenerateChallenge(aPacket);
		if ( retValue == KErrNone) 
			{
			nextState = EChallConnRequested;
			}
		else
			{
			nextState = EDropLink;
			}
		}
	
	
	//check the state of the connect instruction
	if ( GetConnectState() == EConnTransport ) //first time round the loop
		{
		//it's going to be a simple challenge unless
		//it's already been decides it's a EChallConnRequested
		if ( nextState == EDropLink )
			nextState = ESimpleConnRequest; 
		}
	else if (GetConnectState() == EConnChallRxed)
		{							
		if (iCallBack )
			{
			FLOG(_L("PrepareConnectPacket requesting password from user\r\n"));

			//ask the user for a password
			//the callback does happens in the method OnPacketReceive()
			nextState = EWaitForUserInput;
			retValue = KErrGeneral; //mustn't send yet wait for reply from user
			}
		else
			{
			FLOG(_L("PrepareConnectPacket chall rxed but can't ask for password dropping link\r\n"));
			SET_LAST_ERROR(EChallengeRejected);
			retValue = KErrIrObexConnectChallRejected;
			nextState = EDropLink;
			}
		}					//or drop the link
	else
		{
		FLOG(_L("PrepareConnectPacket unknown connect state\r\n"));
		SET_LAST_ERROR(EPrepareConnectPacketIncorrectState);
		retValue = KErrGeneral;
		nextState = EDropLink;
		}
	SetConnectState(nextState);

	return(retValue);
	}

void CObexClient::OnTransportUp()
	{
	FLOG(_L("CObexClient::OnTransportUp\r\n"));
	ResetConnectionID();

	iTransportController->SendPacket().Init(EOpConnect); 
	if (PrepareConnectPacket(iTransportController->SendPacket()) == KErrNone)
		{
		FLOG(_L("OnTransportUp PrepareConnectPacket Succesfull\r\n"));

		iTransportController->SendPacket().SetFinal();
		FTRACE(iTransportController->SendPacket().Dump());
		SendRequestPacket();
		}
	}

void CObexClient::OnTransportDown()
	{
	LOG_LINE
	LOG_FUNC
	
	// Cancel the timer
	if (iPacketTimer->IsActive())
		{
		iPacketTimer->Cancel();
		}
	// If there's an outstanding request, an error has occured
	// But don't do anything if an error has already been set (due to e.g. packet timer timeout)
	if(iPendingRequest && !iIsLastErrorSet)
		{
		SET_LAST_ERROR(EOpOutstandingOnTransportDown);
		Error(KErrIrObexClientPutPeerAborted); //extended error for IrObex,("Other IR device aborted the transfer")
		}
	}

/** Signals an event has ocurred.

@released
@internalComponent
@param aEvent The event that has ocurred (TObexPacketProcessEvent)
*/
void CObexClient::SignalPacketProcessEvent(TInt aEvent)
	{
	if(aEvent & KObexPacketSignallerInterestingClientEvents)
		{
		// Currently all notifications are related to writes, so only need to
		// clear events from the SendPacket.
		iTransportController->SendPacket().RemovePacketProcessEvents(aEvent);

		iPacketProcessSignaller->Signal(static_cast<TObexPacketProcessEvent>(aEvent));
		}
	}

void CObexClient::SetRequest(TRequestStatus& aStatus, TOperation aOperation)
	{
	aStatus = KRequestPending;
	iPendingRequest = &aStatus;
	iCurrentOperation = aOperation;
	}

void CObexClient::CompleteRequest(const TInt aCompletion)
	{
	LOG_LINE
	LOG_FUNC
	
	if(iPendingRequest)
		{
		// Some errors, particularly IR, only come up through OnError().
		// Thus the setting of underlying error happens here.
		switch (aCompletion)
			{
			case KErrIrObexRespTimedOut:
				{
				SET_LAST_ERROR(EResponseTimeout);
				}
				break;
			case KErrDisconnected:
				{
				// There are at least two ways of getting here.
				// The first is disruption to the transport (e.g. IrDA beam blocked)
				// and the second is attempting an operation other than connect when
				// there is no connection (see CObexClient::ClientCommandL()).
				// We don't want to set the last error twice in the second case,
				// so check that the last error is not set before setting it here.
				if (!iIsLastErrorSet)
					{
					SET_LAST_ERROR(EOpOutstandingOnTransportDown);
					}
				}
				break;
			case KErrCompletion:
				{
				// This is an error from the destructor.
				// Operation is complete, successfully or otherwise.
				// Since the operation is still outstanding and the interruption
				// was initiated locally, the operation is effectively aborted.
				SET_LAST_ERROR(EAborted);
				}
				break;
			case KErrNotReady:
				{
				// This error occurs when the BT link times out.
				SET_LAST_ERROR(EOpOutstandingOnTransportDown);
				}
				break;
			case KErrCouldNotConnect:
				{
				// BT could not connect.
				SET_LAST_ERROR(EOpOutstandingOnTransportDown);
				}
				break;
			case KErrGeneral:
				{
				// Since Obex also uses this error elsewhere, only set a last error
				// if none has been set previously.
				if (!iIsLastErrorSet)
					{
					SET_LAST_ERROR(EOpOutstandingOnTransportDown);
					}
				}
				break;
			default:
				{
				// If an error has not yet been set, then set the last error as appropriate.
				if (!iIsLastErrorSet)
					{
					// For all other errors.
					SET_LAST_ERROR(EOpOutstandingOnTransportDown);
					}
				}
			}
		CHECK_LAST_ERROR_IS_SET;
		User::RequestComplete(iPendingRequest, aCompletion);// Sets iPendingRequest = NULL
		}
	iPendingRequest=NULL;
	iCurrentOperation = EOpIdle;
	
	if (iPacketTimer)
		{
		iPacketTimer->Cancel();
		}
	}

void CObexClient::EmptyHeaderSet()
	{
	LOG_LINE
	LOG_FUNC
	
	iHeaderSet->First();
	while(iHeaderSet->This(iHeader) == KErrNone)
		{
		iHeaderSet->DeleteCurrentHeader();
		}
	}
	
void CObexClient::TimeOutCompletion()
	{
	LOG_LINE
	LOG_FUNC
	
	CompleteRequest(KErrIrObexRespTimedOut);
	// Disconnect transport to prevent the client entering an invalid state
	ForcedTransportDown();
	}

void CObexClient::SendRequestPacket()
	{
	//Send the request.
	iTransportController->Send();
	
	//Set the Timer.
	iPacketTimer->SetTimer(iCmdTimeOutDuration);
	}

void CObexClient::SendRequestPacket(TObexOpcode aObexOpcode)
	{
	//Send the request.
	iTransportController->Send(aObexOpcode);
	
	//Set the Timer.
	iPacketTimer->SetTimer(iCmdTimeOutDuration);
	}

/**
Provides additional interfaces for CObexClient.

@param aUid The UID of the interface that is required.
@return A pointer to an instance implementing the interface represented by aUid.
*/
EXPORT_C TAny* CObexClient::ExtensionInterface(TUid aUid)
	{
	TAny* ret = NULL;

	if (aUid == KObexClientErrorResolverInterface)
		{
		ret = static_cast<MObexClientErrorResolver*>(iErrorEngine);
		}

	return ret;
	}


