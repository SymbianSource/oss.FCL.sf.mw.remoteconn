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
#include <obex/internal/obexinternalheader.h>
#include <obextransportinfo.h>
#include <obex.h>
#include <obex/internal/obexpacket.h>
#include <obex/transport/obextransportcontrollerbase.h>
#include <obex/internal/obextransportconstants.h>
#include <obex/extensionapis/mobexserverrequestpacketnotify.h>
#include <obex/internal/mobexserverrequestpacketnotifyregister.h>
#include "logger.h"
#include "obexsetpathdata.h"
#include "OBEXUTIL.H"
#include "authentication.h"
#include "obexnotifyhandlerserver.h"
#include "obexserverstatemachine.h"
#include "obexservernotifysyncwrapper.h"
#include "obexheaderutil.h"
#include "obexserverrequestpacketengine.h"
#include "obexpacketsignaller.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/**
Constructor - set initial values 
@internalTechnology
*/
CObexServer::CObexServer() : CObex()
	{
	iCurrentOperation = EOpIdle;
	iEnabled = EFalse;

	//the connectionID is fixed at the moment
	ResetConnectionID();
	SetConnectionID(0xc30fa596);

	iTargetChecking = EIfPresent;
	}

void CObexServer::ConstructL(TObexTransportInfo& aObexTransportInfo)
	{
	CObex::ConstructL(aObexTransportInfo);
	iNotifyHandler = new(ELeave)CObexNotifyHandlerServer(*this);
	iTransportController->SetOwner(*iNotifyHandler);

	iHeader = CObexHeader::NewL();
	iStateMachine = CObexServerStateMachine::NewL(*this, *iTransportController);
	iSyncWrapper = CObexServerNotifySyncWrapper::NewL(*this, *iStateMachine);
	iPacketProcessSignaller = CObexPacketSignaller::NewL();
	}

/**
Destructor. 
*/
CObexServer::~CObexServer()
	{
	FLOG(_L("CObexServer Destructor\r\n"));
	Stop();
	
	delete iPacketProcessSignaller;
	delete iSyncWrapper;
	delete iStateMachine;
	delete iHeader;
	delete iNotifyHandler;
	delete iServerRequestPacketEngine;
	}

void CObexServer::ResetConnectionID()
	{
	iConnectionID = KConnIDInvalid;
	iConnectionIdSet = EFalse;
	}

void CObexServer::SetConnectionID(TUint32 aConnectionID)
	{
	iConnectionID = aConnectionID;
	iConnectionIdSet = ETrue;
	}

TUint32 CObexServer::ConnectionID()
	{
	return (iConnectionID);
	}

TInt CObexServer::PrepareFinalChallResponse(CObexPacket& aPacket, TConnectState& aNextState)
	{

	FLOG(_L("CObexServer::PrepareFinalChallResponse\r\n"));

	aPacket.SetOpcode(ERespSuccess); 

	TInt retValue = AddConnectionIDHeader(aPacket);	
	if (retValue == KErrNone)
		{
		FLOG(_L("PrepareFinalChallResponse ConnectionID header Added\r\n"));
		if (iCallBack)
			{
			FLOG(_L("PrepareFinalChallResponse Requesting User Password\r\n"));

			//the actual asking of the password happens later in the method OnPacketReceive
			//wait for the reply
			aNextState = EWaitForUserInput;
			retValue = KErrGeneral; //mustn't send yet wait for reply from user
			}
		else //else can't Auth challenge so drop link
			{
			FLOG(_L("PrepareFinalChallResponse Can't request User Password for Chall dropping link\r\n"));

			retValue = KErrIrObexConnectChallRejected;
			aNextState = EConnTransport;
			aPacket.SetOpcode(ERespNotImplemented);
			}
		}
	else
		{
		aNextState = EDropLink;
		}
	return (retValue);
	}


/** A call back from the the service with the password required for use with generating 
the challenge response. 

@param aPassword Password

@leave KErrNotReady if this function is not called from a MObexAuthChallengeHandler::GetUserPasswordL callback.

@publishedAll
@released
*/
EXPORT_C void CObexServer::UserPasswordL(const TDesC& aPassword)
	{
	LOG_LINE
	LOG_FUNC

	//now have a password, get a nonce, and get it hashed then reply
	if (GetConnectState() == EWaitForUserInput)
		{
		FLOG(_L("CObexServer::UserPasswordL\r\n"));
		PrepareChallResponseL(aPassword);
		FLOG(_L("UserPasswordL - PrepareChallResponse Success\r\n"));

		TObexInternalHeader hdr;
		hdr.Set(TObexInternalHeader::EAuthResponse, (const_cast<TUint8*> (iOutgoingChallResp.Ptr())), iOutgoingChallResp.Size());
		if(iTransportController->SendPacket().InsertData(hdr))
			{
			FLOG(_L("UserPasswordL Inserting EAuthResponse Header\r\n"));

			SetConnectState(EConnObex); //all finished
			iTransportController->SendPacket().SetFinal();
			iTransportController->Send();
			//inform the client that the connection was succesfull
			iOwner->ObexConnectIndication(iRemoteInfo, TPtr8(NULL, 0));
			iStateMachine->ConnectionComplete();
			}
		else
			{
			User::Leave(KErrGeneral);
			}
		}
	else
		{
		User::Leave(KErrNotReady);
		}
	}


TInt CObexServer::AddConnectionIDHeader(CObexPacket& aPacket)
	{
	TInt retValue = KErrNone;
	//if the Target header was used for the connection
	//then reply with ConnectionID and Who headers
	if(iTargetReceived)
		{
		//this solution can only handle one connection at a time therefore
		//can safely use the same connection ID repeatedly
		//when used the ConnectionID must be first
		TObexInternalHeader hdr;
		FLOG(_L("CObexServer::AddConnectionIDHeader Inserting EConnectionID Header\r\n"));
 
 		hdr.Set(TObexInternalHeader::EConnectionID, iConnectionID);
		
		if(aPacket.InsertData(hdr))
			{ 
			// Connection ID header inserted correctly
			// Now set a WHO header.
			// This logic is a bit backwards due to problems with the 'no target header checking'
			// state.  Instead of inserting our local Who header, we copy the Target header back.
			// This works in the checking states because we drop any connection where the local
			// Who is not identical to the Target header received.
			// When not checking targets, this may mean that the client gets connected to a server
			// which knows nothing about the service, yet thinks it is talking to a strict peer.
			// However the server wouldn't understand what was going on anyway, so we're not really
			// in a worse state than we would be if we did something more fancy.  Ultimately the
			// application must drop the connection---probably by deleting the Obex server or
			// returning errors to all attempted operations.
			
			FLOG(_L("CObexServer::AddConnectionIDHeader Inserting EWho Header\r\n"));

			hdr.Set(TObexInternalHeader::EWho, (const_cast<TUint8*> (iRemoteInfo.iTargetHeader.Ptr())), 
																iRemoteInfo.iTargetHeader.Size());
			if(!aPacket.InsertData(hdr))
				{
				retValue = KErrGeneral;
				}
			}
		else
			{
			retValue = KErrGeneral;
			}
		}
	return (retValue);
	}

/**
Allocates and constructs a new OBEX server object.

The received protocol information object, aObexProtocolInfoPtr, specifies the 
transport protocol to use:
For the standard transports the following are used, TObexIrProtocolInfo for 
IrDA, TObexBluetoothProtocolInfo for Bluetooth, TObexUsbProtocolInfo for USB.

@param aObexProtocolInfoPtr Protocol information object describing the 
transport to use
@return New OBEX server object 

@publishedAll
@released
*/
EXPORT_C CObexServer* CObexServer::NewL(TObexProtocolInfo& aObexProtocolInfoPtr)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	TObexProtocolPolicy defaultProtocolPolicy;	// no packet sizing policy specified, so use default		
	TObexTransportInfo* transportInfo = IrOBEXUtil::CreateTransportInfoL(aObexProtocolInfoPtr, defaultProtocolPolicy);
	CleanupStack::PushL(transportInfo);
	CObexServer* server = CObexServer::NewL(*transportInfo);
	CleanupStack::PopAndDestroy(transportInfo);
	return server;
	}

/**
Allocates and constructs a new OBEX server object with packet sizing 
information.

The received protocol information object, aObexProtocolInfoPtr, specifies the 
transport protocol to use:
For the standard transports the following are used, TObexIrProtocolInfo for 
IrDA, TObexBluetoothProtocolInfo for Bluetooth, TObexUsbProtocolInfo for USB.

The aObexProtocolPolicy parameter specifies the packet sizing policy for this 
OBEX object.

@param aObexProtocolInfoPtr Protocol information object describing the 
transport to use
@param aObexProtocolPolicy Protocol policy object specifying the packet sizes 
to use
@return New OBEX server object 
	
@publishedAll
@released
*/
EXPORT_C CObexServer* CObexServer::NewL(TObexProtocolInfo& aObexProtocolInfoPtr, 
										TObexProtocolPolicy& aObexProtocolPolicy)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	TObexTransportInfo* transportInfo = IrOBEXUtil::CreateTransportInfoL(aObexProtocolInfoPtr, aObexProtocolPolicy);	
	CleanupStack::PushL(transportInfo);
	CObexServer* server =  CObexServer::NewL(*transportInfo);
	CleanupStack::PopAndDestroy(transportInfo);
	return server;
	}

/** 
Allocates and constructs a new OBEX server object with packet sizing 
information.

The received transport information object, aObexTransportInfo, specifies the 
transport protocol and packet sizes to use:
For the standard transports the following are used, TObexIrProtocolInfo for 
IrDA, TObexBluetoothProtocolInfo for Bluetooth, TObexUsbProtocolInfo for USB.

@param aObexTransportInfo  Transport information object describing the 
transport and packet sizes  to use
@return New OBEX server object 

@capability WriteDeviceData If the TObexIrV3TransportInfo is passed as the argument
                            and the associated name is valid.

@publishedAll
@released
*/
EXPORT_C CObexServer* CObexServer::NewL(TObexTransportInfo& aObexTransportInfo)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	CObexServer* self = new(ELeave) CObexServer();
	CleanupStack::PushL(self);
	self->ConstructL(aObexTransportInfo);
	CleanupStack::Pop(self);
	return(self);
	}

/** Starts the server, specifying a synchronous notification interface.

If the server is already started, no state changes occur (i.e. any connections/operations 
in progress are not interrupted), but the notifications will be sent to aOwner. 
This allows "child" servers to take over ownership of existing connections. 

Details of this function behaviour depend on the transport specified when 
constructed: in general a listener socket is created, its port number registered 
as appropriate, and an accept queued.

@param aOwner Server notification interface
@return KErrArgument if parameter is NULL, KErrAlreadyExists if server has already
been started (but notification object will still be updated), otherwise a system wide
error code 
@panic OBEX EChangeInterfaceDuringWait when attempting to change the interface at an inappropriate time.

@publishedAll
@released
*/
EXPORT_C TInt CObexServer::Start(MObexServerNotify* aOwner)
	{
	LOG_LINE
	LOG_FUNC

	if(aOwner == NULL)
		{
		return(KErrArgument);
		}
	
	// Pass this synchronous interface to the synchronous wrapper
	// and pass the synchronous wrapper on to the asynchronous Start()
	iSyncWrapper->SetNotifier(aOwner);
	return Start(iSyncWrapper);
	}


/** Starts the server, specifying an asynchronous notification interface.

If the server is already started, no state changes occur (i.e. any connections/operations 
in progress are not interrupted), but the notifications will be sent to aOwner. 
This allows "child" servers to take over ownership of existing connections. 

Details of this function behaviour depend on the transport specified when 
constructed: in general a listener socket is created, its port number registered 
as appropriate, and an accept queued.

@param aOwner Server notification interface
@return KErrArgument if parameter is NULL, KErrAlreadyExists if server has already
been started (but notification object will still be updated), otherwise a system wide
error code 
@panic OBEX EChangeInterfaceDuringWait when attempting to change the interface at an inappropriate time.

@publishedAll
@released
*/
EXPORT_C TInt CObexServer::Start(MObexServerNotifyAsync* aOwner)
	{
	if(aOwner == NULL)
		{
		return(KErrArgument);
		}

	iOwner = aOwner;											

	iStateMachine->Start(*iOwner);
	if(iEnabled)
		{
		return(KErrAlreadyExists);
		}
	iEnabled = ETrue;
	return(AcceptConnection());
	}
	

/** Disconnects any transfer in progress and disables further connections. 

@publishedAll
@released
*/
EXPORT_C void CObexServer::Stop()
	{// Cancel and Disable accepts, and bring and transport down.
	LOG_LINE
	LOG_FUNC

	if(!iEnabled)
		{
		return;
		}
	iEnabled = EFalse;
	ControlledTransportDown();
	
	// just check that iTransportController is still valid here (that is what we
	// aspect to be)
	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
	
	iTransportController->CancelAccept();
	iStateMachine->Stop();
	iOwner = NULL;
	iSyncWrapper->SetNotifier(NULL);
	}

TInt CObexServer::AcceptConnection()
	{
	if(iEnabled && iOwner)
		{
		iCurrentOperation = EOpIdle;
		TRAPD(err, iTransportController->AcceptConnectionL());
		if(err != KErrNone)
			{
			iEnabled = EFalse;
			}
		return(err);
		}
	else
		{
		return(KErrNone);
		}
	}



/** Sets a password required to access the server.

When a password is set, a client must specify it to access the server.

@param aPassword Password 

@publishedAll
@released
*/
EXPORT_C void CObexServer::SetChallengeL(const TDesC& aPassword)
	{
	LOG_LINE
	LOG_FUNC

	delete iChallPassword;
	iChallPassword = NULL;
	iChallPassword = HBufC8::NewL(aPassword.Length());
	TPtr8 ptr = iChallPassword->Des();
	CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aPassword);
	iChallenge = ETrue;
	}

/** Resets the password.

After this call, a client does not need to provide a password to access the 
server. 

@publishedAll
@released
*/
EXPORT_C void CObexServer::ResetChallenge()
	{
	LOG_LINE
	LOG_FUNC

	delete iChallPassword;
	iChallPassword = NULL;
	iChallenge = EFalse;
	}

/**	
Specifies target header checking behaviour.
	
Supports three behaviours---never check, always check, and check only if a target
header has been sent.  The default behaviour is to only check when a target header
has been sent.
	
No checking allows a form of multiplexing to be used, where one server object may
respond to multiple target headers.  The behaviour desired by the client can be
determined by examining the target header specified in the Connect.
	
@param aChecking The desired level of target header checking.
@publishedAll
@released
*/
EXPORT_C void CObexServer::SetTargetChecking(TTargetChecking aChecking)
	{
	LOG_LINE
	LOG_FUNC

	iTargetChecking = aChecking;
	}


/**
Prepare next packet for the connection attempt
ConnectionID and Who headers are Mandatory if the Target header was used in the connection from
@param aPacket Packet to fill
@internalComponent
*/
TInt CObexServer::PrepareConnectPacket(CObexPacket& aPacket) 
	{
	FLOG(_L("CObexServer::PrepareConnectPacket\r\n"));
	TInt retValue = KErrNone;
	TConnectState nextState = GetConnectState();

	if(!iTransportController->InsertLocalConnectInfo(aPacket, iLocalInfo.iVersion, iLocalInfo.iFlags))
		{
		FLOG(_L("PrepareConnectPacket Local data insertion FAILED\r\n"));
		return(KErrGeneral);
		}
	FLOG(_L("PrepareConnectPacket Local data inserted\r\n"));

	if(GetConnectState() == ESimpleConnRequest)	//no Auth requested by the Client
		{
		FLOG(_L("PrepareConnectPacket GetConnectState() == ESimpleConnRequest\r\n"));
		//if the Server must challenge
		if(iChallenge) 
			{
			FLOG(_L("PrepareConnectPacket Challenge Required\r\n"));

			aPacket.SetOpcode(ERespUnauthorized);  
			retValue = GenerateChallenge(aPacket);
			if ( retValue == KErrNone ) 
				{
				FLOG(_L("PrepareConnectPacket Challenge generated\r\n"));
				nextState = ESimpleConnChallIssued;
				}
			else
				{
				FLOG(_L("PrepareConnectPacket Challenge generation FAILED\r\n"));
				nextState = EConnTransport;
				aPacket.SetOpcode(ERespInternalError);
				}
			}
		else //don't require Authentication
			{
			FLOG(_L("PrepareConnectPacket No Challenge Required\r\n"));

			aPacket.SetOpcode(ERespSuccess); 
			//if the Target header was used for the connection
			//if so then reply with ConnectionID and Who headers
			if ((retValue = AddConnectionIDHeader(aPacket)) == KErrNone)
				{
				FLOG(_L("PrepareConnectPacket ConnectionID Inserted\r\n"));
				nextState = EConnObex;
				}
			else
				{
				nextState = EDropLink;
				FLOG(_L("PrepareConnectPacket ConnectionID Insertion FAILED\r\n"));
				}
			}
		} //end if(GetConnectState() == ESimpleConnRequest) 
	else if (GetConnectState() == EChallConnRequested)
		{
		FLOG(_L("PrepareConnectPacket GetConnectState() == EChallConnRequested\r\n"));

		//if the Server must challenge
		if(iChallenge) 
			{
			FLOG(_L("PrepareConnectPacket Challenge required\r\n"));
			aPacket.SetOpcode(ERespUnauthorized); 
			retValue = GenerateChallenge(aPacket);
			if ( retValue == KErrNone ) 
				{
				FLOG(_L("PrepareConnectPacket Challenge Generated\r\n"));
				nextState = EChallConnChallIssued; //chall answered with another chall
				}
			else
				{
				FLOG(_L("PrepareConnectPacket Challenge Generation FAILED\r\n"));
				nextState = EConnTransport;
				aPacket.SetOpcode(ERespInternalError);
				}
			}
		else //don't require Authentication
			{ //the response would already have been verified in ParseConnectPacket() 
			//get password from user, prepare a response to the challenge
			FLOG(_L("PrepareConnectPacket Challenge Not Required\r\n"));
			retValue = PrepareFinalChallResponse(aPacket, nextState);
			}
		} //end else if GetConnectState() == EChallConnRequested
	else if (GetConnectState() == EFinalChallRxed)
		{	//
		retValue = PrepareFinalChallResponse(aPacket, nextState);
		}
	else if ( GetConnectState() == EFinalResponseReceived )
		{ //the response had to be OK otherwise would never have gotten this far
		aPacket.SetOpcode(ERespSuccess); 
		//if the Target header was used for the connection
		//if so then reply with ConnectionID and Who headers
		if ((retValue = AddConnectionIDHeader(aPacket)) == KErrNone)
			{
			nextState = EConnObex;
			FLOG(_L("PrepareConnectPacket ConnectionID header Added\r\n"));
			}
		else
			{
			nextState = EDropLink;
			FLOG(_L("PrepareConnectPacket ConnectionID header Addition FAILED\r\n"));
			}
		}
	else //it's all gone wrong
		{
		FLOG(_L("PrepareConnectPacket complete failure, bad state\r\n"));

		//break connection, inform user
		nextState = EConnTransport;
		aPacket.SetOpcode(ERespInternalError);
		retValue = KErrGeneral;
		}
	//if the Server is now connected inform the client
	if ( nextState == EConnObex)
		iOwner->ObexConnectIndication(iRemoteInfo, TPtr8(NULL, 0));

	SetConnectState(nextState);
	return(retValue);
	}
/**
Prepare next packet for an  invalid connection attempt (i.e. the ParseConnectPacket failed).
A fail response (to a connect request)  includes the version, flags, and packet size information.

@param aPacket Packet to fill
@internalComponent
*/
TInt CObexServer::PrepareErroredConnectPacket(CObexPacket& aPacket)
	{
	FLOG(_L("CObexServer::PrepareErroredConnectPacket\r\n"));
	
	if ( !iTransportController->InsertLocalConnectInfo(aPacket, iLocalInfo.iVersion, iLocalInfo.iFlags))
		{
		FLOG(_L("PrepareConnectPacket Local data insertion FAILED\r\n"));
		return(KErrGeneral);
		}	
	return KErrNone;
	}

void CObexServer::SignalReadActivity()
	{
	iPacketProcessSignaller->Signal(EObexReadActivityDetected);
	}
	
void CObexServer::CheckTarget(TConnectState& aNextState, TInt& aRetVal)
	{
	FLOG(_L("Local Who:\r\n"));
	LOGHEXDESC(iLocalInfo.iWho);
	FLOG(_L("Target:\r\n"));
	LOGHEXDESC(iRemoteInfo.iTargetHeader);
	
	// Workaround for bug with PocketPC 2002---if target header is sixteen bytes of zeros, connect anyway.
	_LIT8(KZeroTarget, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

	// Allow connection iff:
	//    PocketPC attempting to connect to inbox (Sixteen bytes of zeros), when no LocalWho
	// or Target header matches LocalWho (includes Inbox connections)
	
	if (!(	// Negate as block below discards connection
		    ((iLocalInfo.iWho == KNullDesC8) && (iRemoteInfo.iTargetHeader == KZeroTarget))
		 || ( iLocalInfo.iWho == iRemoteInfo.iTargetHeader)
	   ))
		{
		FLOG(_L("ParseConnectPacket ETarget header doesn't match local iWho, dropping link\r\n"));
		aNextState = EConnTransport;
		aRetVal = ERespNotFound;
		}
	}


//if the Target header is sent then it must match the local iWho field
TInt CObexServer::ParseConnectPacket(CObexPacket& aPacket)
	{
	TConnectState nextState = GetConnectState(); //must change otherwise it's all wrong
	TBool challReceivedOK = EFalse; //authentication challenge received from client 
									//after server has issued its own authentication challenge.
	TBool respReceivedOK = EFalse;  //authentication received from client 
									//after server has issued its own authentication challenge.
	FLOG(_L("CObexServer::ParseConnectPacket\r\n"));

	if(!iTransportController->ExtractRemoteConnectInfo(aPacket, iRemoteInfo.iVersion, iRemoteInfo.iFlags))
		{
		FLOG(_L("ParseConnectPacket remote connect info extraction FAILED\r\n"));
		return KErrGeneral;
		}
	FLOG(_L("ParseConnectPacket remote connect info extracted\r\n"));

	TObexInternalHeader hdr;
  	iTargetReceived = EFalse; //if target received then must reply with ConnectionID

	//if the present state is EConnTransport then no headers are actually
	//required, a simple connect is sufficient
	if(GetConnectState() == EConnTransport)
		{
		nextState = ESimpleConnRequest;
		}

	TInt retVal = KErrNone;
	TBool authAttempted = EFalse;
	
	while (aPacket.ExtractData(hdr) && (nextState != EDropLink) && (nextState != EConnTransport))
		{
		switch(hdr.HI())
			{
			case TObexInternalHeader::ETarget:
				{
				FLOG(_L("ParseConnectPacket extracting ETarget header\r\n"));
				iTargetReceived = ETrue;
				//copy the target header into iRemoteInfo for the user
				iRemoteInfo.iTargetHeader.Copy(hdr.HVByteSeq(), hdr.HVSize() > iRemoteInfo.iTargetHeader.MaxSize() ? iRemoteInfo.iTargetHeader.MaxSize() : hdr.HVSize());

				if (iTargetChecking == EIfPresent)
					{
					FLOG(_L("EIfPresent target header checking..."));
					CheckTarget(nextState, retVal);
					}
				}
				break;
			case TObexInternalHeader::EAuthChallenge:
				{
				FLOG(_L("ParseConnectPacket EAuthChallenge Header received processing\r\n"));
				authAttempted = ETrue;
				TRAPD(err, ProcessChallengeL(hdr));
				if (!err)
					{
					FLOG(_L("ParseConnectPacket Processing Chall SUCCESS\r\n"));
					if (GetConnectState() == EConnTransport)
						{
						nextState = EChallConnRequested;
						}
					else if ((GetConnectState() == ESimpleConnChallIssued) || (GetConnectState() == EChallConnChallIssued))
						{
						challReceivedOK = ETrue; //the response must be verified first
						nextState = EFinalChallRxed;
						}
					else
						{
						nextState = EConnTransport;
						retVal = ERespInternalError;
						}
					}
				else
					{
					FLOG(_L("ParseConnectPacket Processing Chall FAILED\r\n"));

					nextState = EConnTransport;
					retVal = ERespInternalError;
					}
				}
				break;
			case TObexInternalHeader::EAuthResponse:
				{
				if (iChallenge)
					//else there is no challenge password to check against!
					{
					FLOG(_L("ParseConnectPacket EAuthResponse Header received processing\r\n"));
					authAttempted = ETrue;
					TRAPD(err, ProcessChallResponseL(hdr));
					if (err == KErrNone)
						{
						FLOG(_L("ParseConnectPacket Processing Chall Response SUCCESS\r\n"));
						if (GetConnectState() == ESimpleConnChallIssued)
							{
							respReceivedOK =ETrue;
							if (challReceivedOK) //was a new challenge issued by the Client?
								{
								nextState = EFinalChallRxed; //must respond to chall
								}
							else
								{
								nextState = EFinalResponseReceived; //everything is OK send Success
								}
							}
						else if (GetConnectState() == EChallConnChallIssued)
							{
							respReceivedOK =ETrue;
							if (challReceivedOK) //was a new challenge issued by the Client?
								{
								nextState = EFinalChallRxed; //must respond to chall
								}
							else
								{
								//If we do not later in the packet see a challenge,
								//(in which case this 'nextState' value will be overwritten),
								//the client will have come back WITHOUT re-issuing 
								//either his original challenge or a new one.
								//Treat as if client had never issued a challenge. 
								//This sequence has been observed in FOMA phones.
								nextState = EFinalResponseReceived; 
								}
							}
						else
							{
							nextState = EConnTransport;
							retVal = ERespInternalError;
							}
						}
					else if (err == KErrAccessDenied)
						{
						nextState = EConnTransport;
						retVal = ERespUnauthorized;
						FLOG(_L("ParseConnectPacket Processing Chall Response FAILED with Access Denied\r\n"));
						}
					else
						{
						nextState = EConnTransport;
						retVal = ERespInternalError;
						FLOG(_L("ParseConnectPacket Processing Chall Response FAILED\r\n"));
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
			}
		}
		
	if (((GetConnectState() == ESimpleConnChallIssued) || (GetConnectState() == EChallConnChallIssued)) && !respReceivedOK)
		// Client's connect packet should have contained an authentication response.
		// Treat as if we had rejected an authentication response.
		{
		nextState = EConnTransport;
		retVal = ERespUnauthorized;
		}
		
	if (iTargetChecking == EAlways)
		{
		FLOG(_L("EAlways target header checking..."));
		CheckTarget(nextState, retVal);
		}

	if (!authAttempted && (GetConnectState() == ESimpleConnChallIssued))
		nextState = ESimpleConnRequest;

	SetConnectState(nextState);
	
	return retVal;
	}


/** 
Check, if required, the object connection ID.

@return ETrue if it was not necessary to receive the ConnectionID or
		if it was necessary and was correctly received. Otherwise 
		EFalse.
@internalComponent
*/
TBool CObexServer::CheckObjectForConnectionId(CObexBaseObject& aObject)
	{
	TBool retValue = ETrue;

	if( iTargetReceived )
		{
		retValue = EFalse;
		if (aObject.iValidHeaders & KObexHdrConnectionID )
			{
			TUint32 connID = aObject.ConnectionID();
			if (iConnectionIdSet && (iConnectionID == connID))
				{
				retValue = ETrue;
				}
			}
		}
	return (retValue);
	}

/**
Check, if required, that the packet connection ID matches that of the Server's current connection
@return ETrue if the connection ID matches or if Target was not used in the original connection
@return EFalse if Target was not used in the original connection and the connection ID was not found
@internalComponent
*/
TBool CObexServer::CheckPacketForConnectionId(CObexPacket& aPacket)
	{
	// Connection ID check is compulsory if Target was used in the original connection
	if (!iTargetReceived)
		{
		return ETrue;
		}

	// Search for ConnectionID
	// ConnectionID should be the first header, but we check all of them just in case
	TObexInternalHeader header;
	while (aPacket.ExtractData(header))
		{
		if (header.HI() == TObexInternalHeader::EConnectionID)
			{
			TUint32 newConnectionID = (header.HVByteSeq()[0] << 24) + (header.HVByteSeq()[1] << 16)
						+ (header.HVByteSeq()[2] << 8)  + (header.HVByteSeq()[3]);

			if (ConnectionID() == newConnectionID)
				{
				return ETrue;
				}
			}
		}

	// Target was used in original connection and could not find Connection ID
	return EFalse;
	}

void CObexServer::OnPacketReceive(CObexPacket& aPacket)
	{
	FLOG(_L("CObexServer::OnPacketReceive\r\n"));
	MObexServerRequestPacketNotify* packetNotify = NULL;
	if (iServerRequestPacketEngine)
		{
		packetNotify = iServerRequestPacketEngine->RequestPacketNotify();		
		}
	// If a packet notify has been registered then we should tell it 
	// about each request packet we receive.
	if (packetNotify)
		{
		TObexResponse response;
		TBool normalOperation = ETrue;
		
		// Rebuild full OBEX opcode to pass to observer
		TObexOpcode opcode = aPacket.Opcode() | (aPacket.IsFinal() ? KObexPacketFinalBit : 0);
		
		switch (aPacket.Opcode())
			{
		case CObex::EOpConnect:
				{
				TObexConnectInfo connectInfo;
				if (!iStateMachine->Transport().ExtractRemoteConnectInfo(aPacket, connectInfo.iVersion, connectInfo.iFlags))
					{
					// If the packet cannot be parsed correctly at this stage it is very malformed
					// and so we should abort processing it for the server app, and defer handling
					// the error to the state machine.
					break;
					}
				
				TObexInternalHeader header;
				// Call ExtractData() until it returns 0 bytes read - then we know the extract
				// point will have been reset so the CObexPacket can be parsed again in the 
				// future.  For this reason do not attempt to optimise this loop.
				while (aPacket.ExtractData(header))
					{
					switch (header.HI())
						{
					case TObexInternalHeader::ETarget:
						 // Only take the first Target header found
						 if (connectInfo.iTargetHeader.Length() == 0)
							{
							TInt size = header.HVSize() > connectInfo.iTargetHeader.MaxSize() ? connectInfo.iTargetHeader.MaxSize() : header.HVSize();
							connectInfo.iTargetHeader.Copy(header.HVByteSeq(), size);
							}
						break;
					case TObexInternalHeader::EWho:
						 // Return the first Who header in packet
						 if (connectInfo.iWho.Length() == 0)
							{
							TInt size = header.HVSize() > connectInfo.iWho.MaxSize() ? connectInfo.iWho.MaxSize() : header.HVSize();
							connectInfo.iWho.Copy(header.HVByteSeq(), size);
							}
						break;
					default:
						break;
						}
					}
				
				normalOperation = packetNotify->RequestPacket(opcode, connectInfo, response);
				}
			break;
		case CObex::EOpSetPath:
				{
				TObexSetPathData data;
	
				if (!aPacket.ExtractData(data))
					{
					// If the packet cannot be parsed correctly at this stage it is very malformed
					// and so we should abort processing it for the server app, and defer handling
					// the error to the state machine.
					break;
					}
				CObex::TSetPathInfo info(data);
					
				TObexInternalHeader header;
				// Call ExtractData() until it returns 0 bytes read - then we know the extract
				// point will have been reset so the CObexPacket can be parsed again in the 
				// future.  For this reason do not attempt to optimise this loop.
				while(aPacket.ExtractData(header))
					{
					// Take the first name header we find.
					if(!info.iNamePresent && header.HI() == TObexInternalHeader::EName && header.GetHVText(info.iName) == KErrNone)
						{
						info.iNamePresent = ETrue;
						}
					}
	
				normalOperation = packetNotify->RequestPacket(opcode, info, response);
				}
			break;	
		case CObex::EOpDisconnect:
		case CObex::EOpAbortNoFBit:
		case CObex::EOpPut:
		case CObex::EOpGet:	
		default:
			normalOperation = packetNotify->RequestPacket(opcode, response);
			break;
			}

		if (!normalOperation) // Abandon processing of request
			{
			CheckServerAppResponseCode(aPacket.Opcode(), response); // a success response code => panic
			iStateMachine->OverrideRequestHandling(response);
			return;
			}
		}

	// Normal processing
	iStateMachine->OnPacketReceive(aPacket);
	}

/**
This function is to ensure that a response a server application provides the Obex Server
to respond to the Obex Client with when it has overriden the default handling of a request
packet does not represent a success.

The rationale for this is to attempt to keep the Obex peers synchronised.  As the packet has
been dropped, the client should not be lead to believe it has been received successfully.

Therefore, this function asserts that the application does not send a success response for 
the request packet received.
*/	
void CObexServer::CheckServerAppResponseCode(TObexOpcode aOpcode, TObexResponse aResponse)
	{
	TBool valid = ETrue;
	switch (aOpcode)
		{
		case CObex::EOpConnect:
			if (aResponse == ERespSuccess)
				{
				valid = EFalse;
				}
			break;
		case CObex::EOpPut:
		case CObex::EOpGet:
			if (aResponse == ERespSuccess || aResponse == ERespContinue)
				{
				valid = EFalse;
				}
			break;
		case CObex::EOpSetPath:
			if (aResponse == ERespSuccess)
				{
				valid = EFalse;
				}
			break;
		case CObex::EOpDisconnect:
		case CObex::EOpAbortNoFBit:
			// We allow any response to a abort/disconnect request,
			// as only success codes are allowed.
		default:
			break;
		}
	__ASSERT_ALWAYS(valid, IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp));
	}

void CObexServer::OnError(TInt aError)
	{
	FTRACE(FPrint(_L("OnError aError: %d iCurrentOperation: %d, iConnectState: %d"), aError, iCurrentOperation, iConnectState));

	if (aError == KErrDisconnected)
		{
		// Note: It is not clear that iCurrentOperation is ever equal
		// to EOpDisconnect but the check has been retained just in case
		if ((iCurrentOperation != EOpDisconnect) && (iConnectState > EConnTransport))
			{
			 //extended error for IrObex,("peer device aborted data transmission/obex sending") 
			iOwner->ErrorIndication(KErrIrObexServerPutPeerAborted);
			}
		}
	else
		{
		iOwner->ErrorIndication(aError);
		}
	// The state machine needs to know about the error regardless of whether ErrorIndication() is called
	iStateMachine->Error();
	}

void CObexServer::OnTransportUp()
	{
	iTargetReceived = EFalse;

	// For servers on the device using USB, there is a possibility that
	// this function can be called, even though the server is stopped,
	// as OBEX does not control the transport, the USB host does.
	// Hence the need to check if there is an active iOwner.
	if (iOwner)
		{
		iOwner->TransportUpIndication();
		}
	iStateMachine->TransportUp(); // state machine needs to know about the event regardless of Server state
	}

/**
Tell the MObexServerNotifyAsync observer the transport is down and listen
for another connection.
*/
void CObexServer::OnTransportDown()
	{// Cancel Anything waiting. Restart the accepter	

	// For servers on the device using USB, there is a possibility that
	// this function can be called, even though the server is stopped,
	// as OBEX does not control the transport, the USB host does
	// Hence the need to check if there is an active iOwner.
	if (iOwner)
		{
		iOwner->TransportDownIndication();
		}
	iStateMachine->TransportDown(); // state machine needs to know about the event regardless of Server state
	TInt err = AcceptConnection();
	if(err != KErrNone)
		Error(err);
	}

/** Signals an event has ocurred.

@released
@internalComponent
@param aEvent The event that has occurred. (TObexPacketProcessEvent)
*/
void CObexServer::SignalPacketProcessEvent(TInt aEvent)
	{
	LOG_FUNC

	// This is how we signal the completed send of an ACK to a disconnect 
	// command. Tell the state machine so it can finish the disconnection 
	// sequence.
	if(aEvent & EObexWriteCompletedFinal)
		{
		iStateMachine->WriteComplete();
		}

	// Server will have definitely finished with the read packet so queue the next read
	if(aEvent & EObexWriteCompleted)
		{
		iTransportController->Receive();
		}
		
	if(aEvent & EObexReadActivityDetected)
		{
		iStateMachine->ReadActivityDetected();
		}
	}
	
// CObexServer
/** Tests if the server is started, and is available to accept connections.
	
@return ETrue if the server is started, EFalse otherwise
	
@publishedAll
@released
*/
EXPORT_C TBool CObexServer::IsStarted() 
	{
	LOG_LINE
	LOG_FUNC

	return iEnabled;
	}

/**
Returns the operation currently being performed by the remote client, or 
EOpIdle if between operations. Note that there is no implication of whether 
the server is currently connected; EOpIdle will be returned regardless of 
connection state, if no operation is currently being performed. Use 
CObex::IsConnected () to find connection staus.

@return Operation currently being performed by the remote client

@publishedAll
@released
*/
EXPORT_C CObex::TOperation CObexServer::CurrentOperation() const 
	{
	LOG_LINE
	LOG_FUNC

	return iCurrentOperation;
	}

/**
Setter function to allow other classes in the DLL to set the Server's current operation.
Used by the Server state machine.
@see CObexServerStateMachine
@param aOperation The operation currently being performed by the remote client
@internalComponent
*/
void CObexServer::SetCurrentOperation(const CObex::TOperation aOperation)
	{
	iCurrentOperation = aOperation;
	}

/**
Specify the set of headers to return to remote Obex client in final
Put response packet. The total length of the headers when encoded 
should not exceed the maximum Obex packet payload size.

This function may be called at any point during a Put operation.
Repeated calls to this replace rather than add to the header set
for inclusion in the final Put response packet.

It may be called with a NULL pointer, which means that no headers
will be sent with the Put Final Response.

Even if this function returns with an error (even KErrNotReady) a
best-effort attempt will be made to send as many headers as will fit
in the final Put response packet.

@param aHeaderSet A set of headers to be encoded in the final Put
response packet. Ownership of the header set always passes to 
CObexServer.

@return KErrNone if the operation completes successfully.
		KErrNotReady if the current operation is not a Put.
		KErrArgument if the length of the headers when encoded
		exceeds the maximum Obex packet payload size.
		
@publishedAll
@released
*/
EXPORT_C TInt CObexServer::SetPutFinalResponseHeaders(CObexHeaderSet* aHeaderSet)
	{
	LOG_LINE
	LOG_FUNC

	//	First, let the state machine take ownership of the headerset
	iStateMachine->SetPutFinalResponseHeaderSet(aHeaderSet);

	TInt err=KErrNone;

	//	Easy check first - are we currently engaged in a Put?
	if(iCurrentOperation != EOpPut)
		{
		err=KErrNotReady;
		}

	if(!err && aHeaderSet)
		{
		//	Next, the not so easy check. Will all the headers, when encoded,
		//	fit inside a send packet? 
		//	First, how much space do we have to play with?
		TInt available = iTransportController->SendPacket().DataLimit() - KObexPacketHeaderSize;
		
		//	Next, what is the combined encoded size of all the headers?
		TInt required = 0;
		aHeaderSet->First();
		while(aHeaderSet->This(iHeader) == KErrNone)
			{
			required+=iHeader->EncodedSize();
			(void)aHeaderSet->Next();
			}

		if(required>available)
			{
			err=KErrArgument;
			}
		}
		
	return err;
	}


/**
Complete an asynchronous callback, supplying a CObexBaseObject derived object.
Passing in NULL results in an Obex level error being sent to the client -- the
semantics are that either a PUT request has been rejected or a GET request has
not found a suitable object to return.

@panic Obex ENoNotificationToComplete Raised if the server does not have a request
outstanding.
@param aObject The object passed back from application
@return result of state changes
@publishedAll
@released
*/
EXPORT_C TInt CObexServer::RequestIndicationCallback(CObexBaseObject* aObject)
	{
	LOG_LINE
	LOG_FUNC
	return iStateMachine->RequestNotificationCompleted(aObject);
	}

/**
Complete an asynchronous callback, supplying a obex response code. Applications 
should use this function when rejecting Get/Put RequestIndication in order to 
specify the response code.

@panic Obex ENoNotificationToComplete Raised if the server does not have a request
outstanding.
@panic Obex EInvalidResponseCodeFromServerApp raised if TObexResponse aResponseCode is outside range
[1,255] or it is one of the successful response (e.g. ERespSuccess, ERespContinue) 
@param TObexResponse aResponseCode Application's response to the indication as an Obex response Code. The Final bit is ignored.
@return result of state changes
@publishedAll
@released
*/
EXPORT_C TInt CObexServer::RequestIndicationCallbackWithError(TObexResponse aResponseCode)
	{
	LOG_LINE
	LOG_FUNC
	return iStateMachine->RequestNotificationCompleted(aResponseCode);
	}

/**
Complete an asynchronous callback, supplying a obex response code. Applications 
should use this function when rejecting Get/Put RequestIndication in order to 
specify the error code.

@panic Obex ENoNotificationToComplete Raised if the server does not have a request
outstanding.
@panic Obex EInvalidResponseCodeFromServerApp raised if TObexResponse aResponseCode non-negtive. Note: KErrNone is 
not acceptable because this function is only used when there is an error.
@param aErrorCode Application's response to the indication as an Obex response Code. 
@return result of state changes
@publishedAll
@released
*/	
EXPORT_C TInt CObexServer::RequestIndicationCallbackWithError(TInt aErrorCode)
	{
	LOG_LINE
	LOG_FUNC
	__ASSERT_ALWAYS(aErrorCode <= 0, IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp));
	return iStateMachine->RequestNotificationCompleted(IrOBEXUtil::ObexResponse(aErrorCode, ERespSuccess));
	}
	
	
/**
Complete an asynchronous callback, supplying a obex response code. This function is 
used for asychronously handling PutComplete, GetComplete and SetPath Indication. 

@panic Obex ENoNotificationToComplete Raised if the server does not have a request
outstanding.
@panic Obex EInvalidResponseCodeFromServerApp raised if TObexResponse aResponseCode is outside range
[1,255] or it is ERespContinue (which would confuse the client)
@param TObexResponse aResponseCode Application's response to the indication as an Obex response Code. The Final bit is ignored.
@return result of state changes
@publishedAll
@released
*/
EXPORT_C TInt CObexServer::RequestCompleteIndicationCallback(TObexResponse aResponseCode)
	{
	LOG_LINE
	LOG_FUNC
	return iStateMachine->RequestCompleteNotificationCompleted(aResponseCode);
	}

/**
Complete an asynchronous callback, supplying a obex response code. This function is 
used for asychronously handling PutComplete, GetComplete and SetPath Indication. 

@panic Obex ENoNotificationToComplete Raised if the server does not have a request
outstanding.
@panic Obex EInvalidResponseCodeFromServerApp raised if aErrorCode is positive, i.e. 
invalid Symbian error code
@param TObexResponse aResponseCode Application's response to the indication as a Symbian error code
@return result of state changes
@publishedAll
@released
*/
EXPORT_C TInt CObexServer::RequestCompleteIndicationCallback(TInt aErrorCode)
	{
	LOG_LINE
	LOG_FUNC
	__ASSERT_ALWAYS(aErrorCode <= 0, IrOBEXUtil::Panic(EInvalidResponseCodeFromServerApp));
	return iStateMachine->RequestCompleteNotificationCompleted(IrOBEXUtil::ObexResponse(aErrorCode, ERespSuccess));
	}
	
/**
Provides the pre-parsed contents of the most recently received request packet.

@param aHeaderSet A reference to a pointer that will be modified to NULL if no headers
are contained in the request packet, or to point to a new headerset containing
representations of the headers within the packet. Ownership of the headerset
(when aHeaderSet is not NULL) is passed to the caller.
@return KErrNone if successful, otherwise a system wide error code.

@publishedPartner
@released
*/
EXPORT_C TInt CObexServer::PacketHeaders(CObexHeaderSet*& aHeaderSet)
	{
	FLOG(_L8("CObexServer::PacketHeaders"));

	return DoPacketHeaders(aHeaderSet, NULL);	
	}


/**
Provides the selectively pre-parsed contents of the most recently received request packet.

@param aHeaderSet A reference to a pointer that will be modified to NULL if no interesting
header are contained in the request packet, or to point to a new headerset containing
representations of the headers within the packet that are of interest. Ownership of the 
headerset (when aHeaderSet is not NULL) is passed to the caller.
@param aHeaderCheck A reference to an MObexHeaderCheck derived class that encapsulates
whether or not a particular header or headers should be included in the returned header
set (i.e. whether the headers are "interesting").

@return KErrNone if successful, otherwise a system wide error code.

@publishedPartner
@released
*/
EXPORT_C TInt CObexServer::PacketHeaders(CObexHeaderSet*& aHeaderSet, MObexHeaderCheck& aHeaderCheck)
	{
	FLOG(_L8("CObexServer::PacketHeaders (selective)"));

	return DoPacketHeaders(aHeaderSet, &aHeaderCheck);
	}

/** Sets a read activity observer.

This replaces any previous observer.  The observer will receive a callback
when the first read arrives for a put or get request. 

This does not transfer ownership.

@publishedPartner
@released
@param aObserver The observer to receive packet process events.  This may
				 be NULL.
*/
EXPORT_C void CObexServer::SetReadActivityObserver(MObexReadActivityObserver* aObserver)
	{
	iPacketProcessSignaller->SetReadActivityObserver(aObserver);
	}
/**
Contains the functionality for the PacketHeader interface functions in a refactored way.

@param aHeaderSet A reference to a pointer that will be modified to NULL if no interesting
header are contained in the request packet, or to point to a new headerset containing
representations of the headers within the packet that are of interest. Ownership of the 
headerset (when aHeaderSet is not NULL) is passed to the caller.
@param aHeaderCheck A pointer to an MObexHeaderCheck derived class that encapsulates
whether or not a particular header or headers should be included in the returned header
set (i.e. whether the headers are "interesting").  If the pointer is NULL then that is taken
to mean that all headers should be added to the aHeaderSet.

@return KErrNone if successful, otherwise a system wide error code.
*/	
TInt CObexServer::DoPacketHeaders(CObexHeaderSet*& aHeaderSet, MObexHeaderCheck* aHeaderCheck)
	{
	FLOG(_L8("CObexServer::DoPacketHeaders"));
	
	TRAPD(err, aHeaderSet = CObexHeaderSet::NewL());
	if (err != KErrNone)
		{
		aHeaderSet = NULL;
		return err;
		}
	
	CObexPacket& packet = iTransportController->ReceivePacket();
	
	// for "non-standard" size requests ensure we correctly start
	// extracting headers where they start.
	switch (packet.Opcode())
		{
	case CObex::EOpConnect:
			{
			TObexConnectInfo connectInfo;
			if (!iStateMachine->Transport().ExtractRemoteConnectInfo(packet, connectInfo.iVersion, connectInfo.iFlags))
				{
				err = KErrUnderflow;
				}
			}
		break;
	case CObex::EOpSetPath:
			{
			TObexSetPathData data;
			if (!packet.ExtractData(data))
				{
				err = KErrUnderflow;
				}
			}
		break;
	default:	
		break;	
		}
	
	TBool interestingHeaders = EFalse;
	TObexInternalHeader header;
	// Call ExtractData() until it returns 0 bytes read - then we know the extract
	// point will have been reset so the CObexPacket can be parsed again in the 
	// future.  For this reason do not attempt to optimise this loop.
	while(packet.ExtractData(header))
		{
		// if there was an error previously we want to just keep going through the
		// loop to reset the CObexPacket extraction point.
		if (err == KErrNone && (!aHeaderCheck || aHeaderCheck->Interested(header.HI())))
			{
			err = IrOBEXHeaderUtil::ParseHeader(header, *aHeaderSet);
			if (!interestingHeaders && err == KErrNone)
				{
				interestingHeaders = ETrue;
				}
			}
		}

	if (err != KErrNone || !interestingHeaders)
		{
		delete aHeaderSet;
		aHeaderSet = NULL;
		}
	return err;	
	}

/**
Provides additional interfaces for CObexServer.

@param aUid The UID of the interface that is required.
@return A pointer to an instance implementing the interface represented by aUid.

@leave KErrNotSupported if the given UID does not represent an interface CObexServer can provide.
@leave KErrInUse if an instance of MObexServerRequestPacketNotifyRegister has already been provided
				 by an earlier call to ExtensionInterfaceL, and it has not been released.

@internalTechnology
*/
EXPORT_C TAny* CObexServer::ExtensionInterfaceL(TUid aUid)
	{
	// MObexServerRequestPacketNotifyRegister interface
	if (aUid == KObexServerRequestPacketNotifyRegisterInterface)
		{
		// We only return an instance if there are no other packet access extensions
		// hooked into the CObexServer instance (indicated by the existance of an engine).
		if (iServerRequestPacketEngine)
			{
			User::Leave(KErrInUse);
			}
		iServerRequestPacketEngine = CObexServerRequestPacketEngine::NewL(*this);
		return static_cast<MObexServerRequestPacketNotifyRegister*>(iServerRequestPacketEngine);
		}
	// if we don't know the interface UID then we don't support it.
	User::Leave(KErrNotSupported);
	return NULL; // to silence the compiler.
	}

/** 
Returns a pointer to the TObexTransportInfo being used by the OBEX transport 
layer. THE USER MUST NOT MODIFY THE OBJECT POINTED TO.
This is useful primarily when using OBEX over RFCOMM and the user has 
specified 'KRfcommPassiveAutoBind' as the port. KRfcommPassiveAutoBind makes 
RFCOMM itself find a free port. The user needs to know which port is really 
being used by RFCOMM in order to correctly populate the SDP record. 
May be called meaningfully after CObexServer::Start has returned KErrNone. 
@return Pointer to the transport layer's transport info.
@publishedAll
@released
*/
EXPORT_C const TObexTransportInfo* CObexServer::TransportInfo() const
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
	return iTransportController->TransportInfo();
	}

