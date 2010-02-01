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
#include <obex/internal/obexinternalheader.h>
#include <obextransportinfo.h>
#include <obexirtransportinfo.h>
#include <obex/transport/obextransportcontrollerbase.h>
#include <obex/internal/obexpacket.h>
#include "logger.h"
#include "obexsetpathdata.h"
#include "OBEXUTIL.H"
#include "authentication.h"
#include <ecom/ecom.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEX");
#endif

/**
Constructor - set initial values and copy in protocol policy information
@internalComponent
*/
CObex::CObex()
	{
#ifdef _DEBUG
	CObexLog::Connect();
#endif
	FLOG(_L("Creating CObex"));
	SetConnectState(EConnIdle);
	}

void CObex::ConstructL(TObexTransportInfo& aObexTransportInfo)
	{
	iTransportController = CObexTransportControllerBase::NewL(aObexTransportInfo);
	iAuthEngine = CObexAuthenticator::NewL();
	iSuppressedObexAuthElements = EObexNoSuppressedAuthElements;
	}

/** 
Destructor. 
*/
CObex::~CObex()
	{
	
	delete iTransportController;
	delete iAuthEngine;
	delete iChallPassword;
	delete iRespPassword;
	delete iRemoteUID;
	delete iRemoteRealm;
	delete iRxChallenge;
	
	// This must be done AFTER destroying the transport controller.
	REComSession::FinalClose();

	// iNotifyHandler is deleted in the derived classes where it is instantiated

#ifdef _DEBUG
	CObexLog::Close();
#endif
	}

/** 
Get the socket address of the remote device.

This is the address of the device OBEX is connected to over an IrDA or Bluetooth 
socket. 

@param aAddr Socket address. 

@publishedAll
@released
*/
EXPORT_C void CObex::RemoteAddr(TSockAddr& aAddr)
	{
	LOG_LINE
	LOG_FUNC
	
	// iTransportController must be valid for all the CObex life cycle 
	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
		
	iTransportController->RemoteAddr(aAddr);
	}

/** 
Sets the local Who field.

This is used to identify the local end of the OBEX session when the OBEX connection 
is made. If it is required, set it before establishing the connection.

@param aInfo Who field
@return KErrNone or KErrArgument if aInfo is empty  

@publishedAll
@released
*/
EXPORT_C TInt CObex::SetLocalWho(const TDesC8& aInfo)
	{ 
	LOG_LINE
	LOG_FUNC

	if(aInfo.Length())
		{ iLocalInfo.iWho.Copy(aInfo); return KErrNone;} 
	else
		return KErrArgument;
	}

/** 
Indicates if the Server / Client is currently authenticating the OBEX
connection.

@return ETrue if the Server / Client is currently authenticating the OBEX connection otherwise EFalse.

@publishedAll
@released
*/
EXPORT_C TBool CObex::IsAuthenticating() const
	{
	return (ConnectState() >= EConnChallRxed && ConnectState() <= EWaitForUserInput);
	}
	

/**
@param aPassword Password to use in challenge response
@internalComponent
*/
void CObex::PrepareChallResponseL(const TDesC& aPassword)
	{

	FLOG(_L("CObex::PrepareChallResponse\r\n"));
	iOutgoingChallResp.Zero(); 
	TRequestDigest response;
	delete iRespPassword;
	iRespPassword = NULL;
	iRespPassword = HBufC8::NewL(aPassword.Length());
	TPtr8 ptr = iRespPassword->Des();
	CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, aPassword);
	
	iAuthEngine->GenerateResponseL(ptr, iIncomingNonce, response);
	FLOG(_L("PrepareChallResponse: Digest response generated"));

	FLOG(_L("PrepareChallResponse: Adding response hash"));
	iOutgoingChallResp.Append(KObexRespTag); //0x00
	iOutgoingChallResp.Append(KObexRespSize); //16
	iOutgoingChallResp.Append(response);

	//only reply with UserID if requested
	if (iUserIDRequested )
		{
		FLOG(_L("PrepareChallResponse: User ID required, adding zero length field"));
		iOutgoingChallResp.Append(KObexRespUserIDTag); //0x01
		//Fixme, add in the size of the user ID
		iOutgoingChallResp.Append(0); //assume no UserID size of 0
		//any user ID would have to be inserted here
		//iOutgoingChallResp.Append(userID); //Fixme what is the user ID?
		}
	
	FLOG(_L("PrepareChallResponse: Adding nonce of challenge we're replying to"));
	iOutgoingChallResp.Append(KObexRespNonceTag);
	iOutgoingChallResp.Append(KObexNonceSize);
	iOutgoingChallResp.Append(iIncomingNonce);

	FLOG(_L(" PrepareChallResponse - complete response generated"));
	}

/**
@internalComponent
*/
TInt CObex::GenerateChallenge(CObexPacket& aPacket)
	{
	TInt retValue = KErrNone;

	FLOG(_L("CObex::GenerateChallenge\r\n"));
	iOutgoingNonce.Zero();
	iAuthEngine->GenerateNonce(iOutgoingNonce);
	//now pack all the data together for the overall challenge
	TBuf8<KObexChallHeaderSize> outGoingHeader; //this size assumes no Realm data
	outGoingHeader.Zero();
	outGoingHeader.Append(KObexChallNonceTag);
	outGoingHeader.Append(KObexNonceSize);
	outGoingHeader.Append(iOutgoingNonce);
	
	// Add authentication header only if the client has not disallowed it
	if (!(iSuppressedObexAuthElements & EObexSuppressChallengeOptionsAuthElement))
		{		
		outGoingHeader.Append(KObexChallOptionsTag);
		outGoingHeader.Append(1); //size is always 1
		outGoingHeader.Append(0); //all options off
		}
		
	// Add realm header only if the client has not disallowed it	
	if (!(iSuppressedObexAuthElements & EObexSuppressRealmAuthElement))
		{
		outGoingHeader.Append(KObexChallRealmTag);
		outGoingHeader.Append(0); //no realm so size = 0
		}

	TObexInternalHeader hdr;
	hdr.Set(TObexInternalHeader::EAuthChallenge, (const_cast<TUint8*> (outGoingHeader.Ptr())), outGoingHeader.Size());
	if(!aPacket.InsertData(hdr))
		retValue = KErrOverflow;
		
	return (retValue);
	}

/**
@internalComponent
*/
void CObex::ProcessChallResponseL(const TObexInternalHeader& hdr)
	{
	FLOG(_L("CObex::ProcessChallResponse"));
	TBool retValue = ETrue;
	TInt responseSize = hdr.HVSize();
	TInt elementsLeft = responseSize;	//keep track of how many elements still to be removed

	if ( responseSize > KChallResponseSize )
		{
		retValue = EFalse;
		FLOG(_L("ProcessChallResponse - Response header too big FAILED"));
		}
	else if (responseSize < KMinChallResponseSize)
		{ 
		retValue = EFalse;
		FLOG(_L("ProcessChallResponse - Response header too small FAILED"));
		}

	//get the response 
	iIncomingChallResp.Zero();
	iIncomingChallResp.Copy(hdr.HVByteSeq(), responseSize);
	
	//there is no reason to assume that the data is going to arrive
	//in any particular order
	TInt extractionPosn = 0;

	//ensure space enough for the tag and size  
	while ((extractionPosn <  (responseSize-1) ) && retValue)
		{
		switch (iIncomingChallResp[extractionPosn++])
			{
			case KObexRespTag:
				{
				elementsLeft--; //moved passed the tag
				if ((iIncomingChallResp[extractionPosn++] == KObexRespSize)&&
					(elementsLeft > KObexRespSize)) //must be greater to allow for size field
					{
					elementsLeft--; //moved passed the size
					FLOG(_L("ProcessChallResponse - iIncoming Request Response extracted"));
					iIncomingRequestDigest.Zero();
					iIncomingRequestDigest.Append((TUint8*)iIncomingChallResp.Ptr() + extractionPosn, KObexRespSize);
					extractionPosn += KObexRespSize;
					elementsLeft -= KObexRespSize;
					}
				else
					{
					FLOG(_L("ProcessChallResponse - iIncoming Request Response extraction FAILED"));
					retValue = EFalse;
					}
				}
				break;
			case KObexRespUserIDTag:
				{
				elementsLeft--; //moved passed the tag
				TInt userIDSize = iIncomingChallResp[extractionPosn++];
				if ( userIDSize > 0)
					{
					elementsLeft--; //moved passed the User ID size
					if (( userIDSize > 0 )&&(userIDSize <= elementsLeft ))
						{
						elementsLeft -= userIDSize;
						FLOG(_L("ProcessChallResponse - iIncoming ASCII UserID Extracted"));
						delete iRemoteUID;
						iRemoteUID = NULL;
						iRemoteUID = HBufC::NewL(userIDSize); 
						TPtrC8 ptr((TUint8*)iIncomingChallResp.Ptr() + extractionPosn, userIDSize);
						TPtr ptrUID((TUint16*)iRemoteUID->Ptr(), userIDSize);
						CnvUtfConverter::ConvertToUnicodeFromUtf8(ptrUID, ptr);
						extractionPosn += userIDSize;
						}
					}
				else
					{
					retValue = EFalse;
					FLOG(_L("ProcessChallResponse - iIncoming UserID Extraction FAILED"));
					}
				}
			break;
			//don't bother extracting the Nonce as we do not support multiple Nonce replies
			//the assumption is the reply received is for the Nonce we sent out
			case KObexRespNonceTag:
				{
				FLOG(_L("ProcessChallResponse: extracting incoming nonce"));
				elementsLeft--; //moved passed the tag
				TInt nonceSize = iIncomingChallResp[extractionPosn++];
				if ( nonceSize > 0 )
					{
					elementsLeft--; //moved passed size
					if ( nonceSize <= elementsLeft )
						{
						FTRACE(
						TPtrC8 incomingNonce(iIncomingChallResp.Ptr() + extractionPosn, nonceSize);
						if (incomingNonce != iOutgoingNonce)
							{
							FLOG(_L("ProcessChallResponse: incoming nonce does not match our challenge. Continuing anyway."));
							}
						);
						
						elementsLeft -= nonceSize;
						extractionPosn += nonceSize;
						}
					else
						{
						FLOG(_L("ProcessChallResponse - iIncoming Request Response extraction, bad nonce size FAILED"));
						retValue = EFalse;
						}
					}
				}
				break;
			default: 
				{
				FLOG(_L("ProcessChallResponse - iIncoming Request Response extraction,unknown tag type FAILED"));
				retValue = EFalse;
				}
			break;
			
			}
		TRAPD(err, iAuthEngine->ChallengeResponseL(*iChallPassword, iOutgoingNonce, iIncomingRequestDigest));
		if ( err != KErrNone)
			{
			FLOG(_L("ProcessChallResponse - Responsed Denied"));
			User::Leave(err);
			}
		}
	if (!retValue )
		User::Leave(KErrGeneral);
	}

/**
@internalComponent
*/
void CObex::ProcessChallengeL(const TObexInternalHeader& hdr)
	{
	FLOG(_L("CObex::ProcessChallenge\n\r"));

	//extract all the necessary data
	TInt challengeSize = hdr.HVSize();
	TInt extractionPosn = 0;
	TInt elementsLeft = challengeSize;
	TBool nonceExtracted = EFalse;

	delete iRxChallenge;
	iRxChallenge = NULL;
	iRxChallenge = HBufC8::NewL(challengeSize);
	iRxChallenge->Des().Copy(hdr.HVByteSeq(), challengeSize);
	delete iRemoteRealm;
	iRemoteRealm = NULL;



	//can't make any assumptions about the order
	//in which anything may arrive, so allow any old order
	TBool exit = EFalse;
	if ( challengeSize < (KObexNonceSize + 2))
		{
		FLOG(_L("CObex::ProcessChallenge incoming Challenge too small\n\r"));
		User::Leave(KErrGeneral);
		}

	//must be space for Tag and size
	while ((elementsLeft > 2) && (!exit))
		{
		switch (iRxChallenge->Des()[extractionPosn++])
			{
			case KObexChallNonceTag: //check the Nonce tag has arrived
				{
				elementsLeft--;  //passed the tag posn
				if (( iRxChallenge->Des()[extractionPosn++] == KObexNonceSize )
					&& (elementsLeft > KObexNonceSize))
					{
					FLOG(_L("CObex::ProcessChallenge incoming Nonce Extracted\n\r"));
					iIncomingNonce.Zero();
					//extract the Nonce data
					TPtr8 ptr((TUint8*)iRxChallenge->Ptr() + extractionPosn 
						,KObexNonceSize, KObexNonceSize);
					iIncomingNonce.Append(ptr);
					elementsLeft -= (KObexNonceSize + 1);
					nonceExtracted = ETrue;
					extractionPosn += KObexNonceSize;
					}
				else
					{
					FLOG(_L("CObex::ProcessChallenge Incorrect Nonce size\n\r"));
					exit = ETrue;
					}
				}
			break;
			case KObexChallOptionsTag:
				{
				elementsLeft--;  //passed the tag posn
				//check the options flag
				iUserIDRequested = EFalse; 
				if (( iRxChallenge->Des()[extractionPosn++] == KObexChallOptionSize )
					&& (elementsLeft > KObexChallOptionSize))
					{
					//check if the user ID is required in the reply
					if ( iRxChallenge->Des()[extractionPosn++] & KObexRequireUID )
						{
						//the userID has been requested so it MUST be sent back in the chall response
						iUserIDRequested = ETrue;
						FLOG(_L("CObex::ProcessChallenge User ID Requested\n\r"));
						}
					elementsLeft -= (KObexChallOptionSize + 1);

					}
				else
					{
					FLOG(_L("CObex::ProcessChallenge Incorrect Options size\n\r"));
					exit = ETrue;
					}
				}
				break;
			case KObexChallRealmTag:
				{
				elementsLeft--;  //passed the tag posn
				TInt size = iRxChallenge->Des()[ extractionPosn++ ];
				elementsLeft--;
				if ( (size > 0 ) && (size <= elementsLeft ) )
					{
					elementsLeft -= size;
					size--; //remove the first byte
					//check which format the data is in
 					TUint convertType=0;
 					switch(iRxChallenge->Des()[extractionPosn++])
 						{
 					case 0:
 						convertType=KCharacterSetIdentifierAscii;
 						break;
 					case 1:
 						convertType=KCharacterSetIdentifierIso88591;
 						break;
 					case 2:
 						convertType=KCharacterSetIdentifierIso88592;
 						break;
 					case 3:
 						convertType=KCharacterSetIdentifierIso88593;
 						break;
 					case 4:
 						convertType=KCharacterSetIdentifierIso88594;
 						break;
 					case 5:
 						convertType=KCharacterSetIdentifierIso88595;
 						break;
 					case 6:
 						convertType=KCharacterSetIdentifierIso88596;
 						break;
 					case 7:
 						convertType=KCharacterSetIdentifierIso88597;
 						break;
 					case 8:
 						convertType=KCharacterSetIdentifierIso88598;
 						break;
 					case 9:
 						convertType=KCharacterSetIdentifierIso88599;
 						break;
 					case 0xFF:
 						// Unicode, no conversion needed
 						break;
 					default:
 						User::Leave(KErrGeneral);
 						}
 
 					if(convertType)
 						{
 						RFs fs;
 						LEAVEIFERRORL(fs.Connect());
 						CleanupClosePushL(fs);
 						CCnvCharacterSetConverter* converter = CCnvCharacterSetConverter::NewLC();
 						if (converter->PrepareToConvertToOrFromL(convertType, fs) != CCnvCharacterSetConverter::EAvailable)
 							User::Leave(KErrGeneral);
 
 						iRemoteRealm = HBufC::NewL(size);
 						TPtr16 dest = iRemoteRealm->Des();
 						TInt state=CCnvCharacterSetConverter::KStateDefault;
 						TInt err = converter->ConvertToUnicode(dest, iRxChallenge->Mid(extractionPosn, size), state);
 						if (err < 0)
 							User::Leave(err);
 						CleanupStack::PopAndDestroy(2); // fs, converter
   						}
					else  //can only be unicode
						{ //if unicode
						FLOG(_L("CObex::ProcessChallenge incoming UNICODE Realm extracted\n\r"));
						size = size/2; //if it's UNICODE then should be an even number
						iRemoteRealm = HBufC::NewMaxL(size);
 						TPtr16 dest = iRemoteRealm->Des();
						for ( TInt x = 0; x < size; x++ )
							dest[x] = LittleEndian::Get16((TUint8*)iRxChallenge->Ptr() + extractionPosn + (2*x));
						extractionPosn += size * 2;
						}

					}
				else
					{
					FLOG(_L("CObex::ProcessChallenge Incorrect Realm size\n\r"));
					exit = ETrue;
					}
				}
				break;
			default:
				{
				FLOG(_L("CObex::ProcessChallenge Unknown Tag type\n\r"));
				exit = ETrue;
				}
				break;
			}
		}
	if ( !nonceExtracted) //the nonce is mandatory so must exist
		{
		FLOG(_L("CObex::ProcessChallenge Nonce was not extracted\n\r"));
		exit = ETrue;			
		}
	if ( exit )	
		User::Leave(KErrGeneral);
	}


/** 
Sets the authentication challenge handler.

The caller must supply a MObexAuthChallengeHandler implementation to handle 
calls from the Server/Client for a request for a password.

@param aCallBack Authentication challenge handler  

@publishedAll
@released
*/
EXPORT_C void CObex::SetCallBack(MObexAuthChallengeHandler& aCallBack)
	{
	LOG_LINE
	LOG_FUNC

	iCallBack = &aCallBack;
	}

/**
@internalTechnology

This function is retained for backwards compatibility and should not be called.

Calling this function has undefined behaviour.
*/
void CObex::Process(CObexPacket& aPacket)
	{
	NotifyProcess(aPacket);
	}

/**
@internalTechnology

This function is retained for backwards compatibility and should not be called.

Calling this function has undefined behaviour.
*/
void CObex::Error(TInt aError) 
	{
	NotifyError(aError);
	}

/**
@internalTechnology

This function is retained for backwards compatibility and should not be called.

Calling this function has undefined behaviour.
*/
void CObex::TransportUp() 
	{
	NotifyTransportUp();
	}
	
/**
@internalTechnology

This function is retained for backwards compatibility and should not be called.
Use ControlledTransportDown() or ForcedTransportDown() to disconnect the transport layer.
Calling this function will result in an ETransportDownCalled panic.

@panic ObexFault ETransportDownCalled
@see ControlledTransportDown()
@see ForcedTransportDown()
*/
void CObex::TransportDown(TBool)
	{
	IrOBEXUtil::Fault(ETransportDownCalled);
	}


void CObex::NotifyProcess(CObexPacket& aPacket)
	{
	LOG2(_L8("Packet Received, opcode: 0x%2X, Length: %d"), 
					aPacket.Opcode(), aPacket.PacketSize());
	FTRACE(aPacket.Dump());
	OnPacketReceive(aPacket);
	// The queuing of the next read packet varies between Server and Client
	//     Client is done at the end of CObexClient::OnPacketReceive
	//     Server is done once the write packet completes in CObexServer::SignalPacketProcessEvent
	// This is because the Server may be mid-way through an asynchronous operation after OnPacketReceive
	// so we have to keep the current read packet
	if ((GetConnectState() == EWaitForUserInput )&&(iCallBack))
		{
		TRAPD(err, iCallBack->GetUserPasswordL(iRemoteRealm ? static_cast<TDesC&>(*iRemoteRealm) : KNullDesC()));
		if ( err )
			{
			Error(err);
			}
		}
	}

void CObex::NotifyError(TInt aError)
	{
	LOG1(_L8("Error Called: %d"), aError);

	// This call has been moved to before ForcedTransportDown(), because it
	// needs to check whether the current operation is "disconnect" or
	// not, and ForcedTransportDown() sets it to "idle".
	OnError(aError);

	if(iConnectState >= EConnTransport)
		{
		ForcedTransportDown();
		}
	}

void CObex::NotifyTransportUp()
	{
	FLOG(_L("CObex::NotifyTransportUp\n\r"));
	SetConnectState(EConnTransport);
	OnTransportUp();
	}

void CObex::NotifyTransportDown(TBool)
	{
	IrOBEXUtil::Fault(ETransportDownCalled);
	}

/**
This function forces the transport to be taken down 
regardless of whether or not  the underlying transport can recover without 
restarting obex applications
However if the transport controller fails to bring the transport down, then only
the obex connection is cancelled.  
This is called in error conditions

@see ControlledTransportDown()
@internalComponent
*/
void CObex::ForcedTransportDown()
	{
	// iTransportController must be valid for all the CObex life cycle 
	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
		
	if (iTransportController->BringTransportDown())
		{
		SetConnectState(EConnIdle);
		OnTransportDown();	
		}
	else 
		{
		//the transport failed to be taken down 
		CancelObexConnection();
		}
	RemoteInfoCleanup();		
	}
	
/**
This function will  tear down the transport if the transport layer supports 
transport reconnection on obex reconnection 
This is called in conditions other than error conditions

@see ForcedTransportDown()
@internalComponent
*/	
void CObex::ControlledTransportDown()
	{
	// iTransportController must be valid for all the CObex life cycle 
	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
	
	TBool canBringTransportDown = iTransportController->IsTransportRestartable();
	if  (canBringTransportDown)
		{
		ForcedTransportDown();
		}  	 	
	else
  		{
  		CancelObexConnection();
  		RemoteInfoCleanup();
   		}

	}	

/**
General cleanup of iRemoteInfo
@internalComponent
*/
void CObex::RemoteInfoCleanup()
	{
	// Some general cleanup here.
	iRemoteInfo.iTargetHeader.SetLength(0);
	iRemoteInfo.iWho.SetLength(0);
	}

/**
Put into  transport connected state but cancel any outstanding transfers and operations
@internalComponent
*/
void CObex::CancelObexConnection()
	{
	// iTransportController must be valid for all the CObex life cycle 
	__ASSERT_DEBUG(iTransportController, IrOBEXUtil::Fault(ETransportControllerNotCreated));
		
	 iTransportController->CancelTransfers();
   	SetConnectState(EConnTransport);
   	iCurrentOperation = EOpIdle;
	}

/**
Change the state of the authentication state machine
@param aNewState New state
@internalComponent
*/
void CObex::SetConnectState(TConnectState aNewState)
	{
	switch(iConnectState = aNewState)
		{
	case EConnIdle:
		FLOG(_L("### Connection State EConnIdle\r\n"));
		break;
 	case EConnTransport:
		FLOG(_L("###### Connection State EConnTransport\r\n"));
		break;
	case ESimpleConnRequest:
		FLOG(_L("######### Connection State ESimpleConnRequest\r\n"));
		break;
	case EConnObex:
		FLOG(_L("######### Connection State EConnObex\r\n"));
		break;
	case EConnChallRxed:
		FLOG(_L("######### Connection State EConnChallRxed\r\n"));
		break;
	case ESimpleConnChallIssued:
		FLOG(_L("######### Connection State ESimpleConnChallIssued\r\n"));
		break;
	case EChallConnRequested:
		FLOG(_L("######### Connection State EChallConnRequested\r\n"));
		break;
	case EChallConnChallIssued:
		FLOG(_L("######### Connection State EChallConnChallIssued\r\n"));
		break;
	case EWaitForFinalResponse:
		FLOG(_L("######### Connection State EWaitForFinalResponse\r\n"));
		break;
	case EFinalChallRxed:
		FLOG(_L("######### Connection State EConnChallReIssued\r\n"));
		break;
	case EDropLink:
		FLOG(_L("######### Connection State EDropLink\r\n"));
		break;
	default: 
		break;
		}
	}
	
// CObex::TSetPathInfo
/**
Constructor. 

This is the path information used in the SETPATH command.
The variable iFlags is set to zero by default. The variable iConstants is always set to zero as this is a reserved 
varaible. The path name is NOT present by default.

@publishedAll
@released
*/
EXPORT_C CObex::TSetPathInfo::TSetPathInfo() 
	{
	LOG_LINE
	LOG_FUNC

	iFlags = 0;
	iConstants = 0;
	iNamePresent = EFalse;
	}

/**
@internalComponent
*/
CObex::TSetPathInfo::TSetPathInfo(const TObexSetPathData& aData)
	: iFlags(aData.iFlags), iConstants(aData.iConstants), iNamePresent(EFalse)
	{
	}

/**
Returns true if the flags are set so that the receiver will backup a level before applying the path name

@return ETrue if the flags are set so that the receiver will backup a level before applying the path name othewise EFalse.

@publishedAll
@released
*/
EXPORT_C TBool CObex::TSetPathInfo::Parent() const 
	{
	LOG_LINE
	LOG_FUNC

	return(iFlags & KObexSetPathParent);
	}

// CObex
/**
Returns ETrue if this CObex is connected at an OBEX level, merely having
a transport connected does not satisfy this condition. I.e. the two devices
must have completed the OBEX connection request/response . All other states
return EFalse. This will be unreliable if either the server blindly 
returns the client’s who header (always reporting ETrue), or if neither
supply "who" headers (always reporting EFalse).

@return ETrue if this CObex is connected at an OBEX level.  EFalse otherwise.
@publishedAll
@released
*/
EXPORT_C TBool CObex::IsConnected() const 
	{
	LOG_LINE
	LOG_FUNC

	return(GetConnectState() == EConnObex);
	}

/**	
@return ETrue if the "who" header specified in the server’s connect response 
		matched that of the client’s connect request, and both had a length greater 
		than 0 (i.e. both specified a "who" field). Undefined if IsConnected() == EFalse. 

@publishedAll
@released
*/
EXPORT_C TBool CObex::IsStrictPeer() const 
	{
	LOG_LINE
	LOG_FUNC

	return((iLocalInfo.iWho.Length()>0) && (iLocalInfo.iWho==iRemoteInfo.iWho));
	}

/**
Use this member to gain access to (and alter, if necessary) the 
CObex::TConnectInfo structure which will be sent to the OBEX peer as part 
of the connection process. Only alter the contents of this having read and 
understood the purpose of the fields, as defined in the OBEX spec. Altering 
this structure after a connection has been made will have no effect on the
current session, but will be used for future connection attempts.

@return The connect info which will be sent to the OBEX peer.
@publishedAll
@released
*/
EXPORT_C const TObexConnectInfo& CObex::LocalInfo() const 
	{
	LOG_LINE
	LOG_FUNC

	return(iLocalInfo);
	}

/**
Use this member to read the details of the remote machine’s connection 
information, as specified by it in during OBEX connection. This data can 
not be altered, as this serves no purpose.
The content of this structure is undefined when @see IsConnected () ==
EFalse. 

@return The connect info from the remote machine.
@publishedAll
@released
*/
EXPORT_C const TObexConnectInfo& CObex::RemoteInfo() const 
	{
	LOG_LINE
	LOG_FUNC

	return(iRemoteInfo);
	}

/**
This function is in the protected scope of CObex and so is not externally usable
@publishedAll
@released
*/
EXPORT_C CObex::TConnectState CObex::ConnectState() const 
	{
	LOG_LINE
	LOG_FUNC

	// This function is exported but protected, making it not much use
	// Making it public, and exposing more information about the authentication
	// state machine is undesirable, but a public function is required for other
	// classes in the DLL. Thus another public, unexported function has been defined
	// on to which this function chains.
	return GetConnectState();
	};

/**
Get the current state of the authentication state machine
@internalComponent
*/
CObex::TConnectState CObex::GetConnectState() const 
	{
	LOG_LINE
	LOG_FUNC

	return iConnectState;
	};
	
/**
Must be called by an application that wishes to suppress the authentication or(not excusive) the realm of the authentication challenge 
@param aSuppressedObexAuthElements enum TObexSuppressedAuthElements to indicate which header elements to surpress (if any)
@panic KErrArgument if invalid value is passed in for enum
@publishedAll
@released
*/
EXPORT_C void CObex::SuppressAuthenticationHeaderElements(TObexSuppressedAuthElements aSuppressedObexAuthElements)
	{
	LOG_LINE
	LOG_FUNC
		
	// If the value passed in is not a valid bitmask then panic
	_LIT(panicText, "Invalid bitmask passed to CObex::SuppressAuthenticationHeaderElements");	
	__ASSERT_ALWAYS((!(aSuppressedObexAuthElements > EObexSuppressAllAuthElements)), User::Panic(panicText, KErrArgument));
		
	iSuppressedObexAuthElements = aSuppressedObexAuthElements;
	
	};
