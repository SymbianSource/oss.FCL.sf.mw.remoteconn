// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <ecom/ecom.h>
#include <obex/transport/obextransportcontrollerbase.h>
#include <obex/internal/mobexnotifyextend.h>
#include <obexirtransportinfo.h>
#include <obex/internal/obexactiverw.h>
#include <obextransportinfo.h>
#include <obex/internal/obexpacket.h>
#include <obexpanics.h>
#include "ObexTransportUtil.h" 
#include "obexconnectdata.h" 
#include "logger.h"
#include "obextransportfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "OBEXCT");
#endif

//Category used for internal panics
_LIT(KPanicCat, "TransCTBase");

// The minimum allowed number of 'transport controller' implementations. If 
// NewL doesn't find at least this many, then in debug NewL panics, and in 
// release it leaves.
const TUint KMinimumNumImplementations = 1;

/**
Constructs a CObexTransportControllerBase PlugIn object.
 
@internalTechnology
@return	A new CObexTransportControllerBase PlugIn object	
@panic TransCTBase EInvalidNumberOfTransportImplementations if there is more than one implementation for a given interface uid and transport name (equal to the default data in the plugin resource file
*/
EXPORT_C CObexTransportControllerBase* CObexTransportControllerBase::NewL(TObexTransportInfo& aTransportInfo)
	{
	LOG_LINE
	LOG_STATIC_FUNC_ENTRY

	//Making a copy as Collapse modifies the original iTransportName
	TBuf<60> transportName = aTransportInfo.iTransportName;
	TPtrC8 params = transportName.Collapse();	
	TEComResolverParams resolverParams; 
	resolverParams.SetDataType(*reinterpret_cast<TDesC8*>(&params));
	
	RImplInfoPtrArray implInfoArray;
	REComSession::ListImplementationsL(TUid::Uid(KObexTransportControllerInterfaceUid), resolverParams, KRomOnlyResolverUid,  implInfoArray);
	
	CleanupResetAndDestroyPushL(implInfoArray);
	__ASSERT_DEBUG(implInfoArray.Count() <= KMinimumNumImplementations, PANIC(KObexTransportPanicCat, EInvalidNumberOfTransportImplementations) ); 
	if (implInfoArray.Count() <  KMinimumNumImplementations)
		{
		User::Leave(KErrNotFound);
		}

	const TUid uid = implInfoArray[KMinimumNumImplementations - 1]->ImplementationUid();
	CleanupStack::PopAndDestroy(&implInfoArray);
	
	CObexTransportControllerBase* ptr = reinterpret_cast<CObexTransportControllerBase*> 
		(REComSession::CreateImplementationL(uid, _FOFF(CObexTransportControllerBase, iPrivateEComUID), 
			reinterpret_cast<TAny*>(&aTransportInfo)));
			
	CleanupStack::PushL(ptr);
	// Do any base construction here. This may in future include allocation of 
	// iFuture1.
	ptr->BaseConstructL();
	CleanupStack::Pop(ptr);
	return ptr;
	}
	
/**
Constructor.
 
@publishedPartner
@released
*/
EXPORT_C CObexTransportControllerBase::CObexTransportControllerBase()
	{
	LOG_LINE
	LOG_FUNC
	}

/**
This function is a place holder for future use. If iFuture1 variable is used 
it will need this function for any allocation required.
This function is called from CObexTransportControllerBase::NewL. 
*/
void CObexTransportControllerBase::BaseConstructL()
	{
	LOG_LINE
	LOG_FUNC
	}

/**
Sets the owner information from the received aOwner

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::SetOwner(MObexNotifyExtend& aOwner)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_ALWAYS(iOwner==NULL, PANIC(KPanicCat, EOwnerAlreadySet));
	iOwner = &aOwner;
	}

/**
Destructor.
 
@publishedPartner
@released
*/
EXPORT_C CObexTransportControllerBase::~CObexTransportControllerBase()
	{
	LOG_LINE
	LOG_FUNC

	delete iConnector;
	DeleteTransport();
	delete iTransportInfo;

	REComSession::DestroyedImplementation(iPrivateEComUID);
	}
/**
Cancels outstanding reads or writes

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::CancelTransfers()
	{
	LOG_LINE
	LOG_FUNC

	if (iActiveReader)
		{
		iActiveReader->Cancel();
		}
	if (iActiveWriter)
		{
		iActiveWriter->Cancel();
		}
	}
/**
Calls on the constructor to take down the transport

@internalTechnology
@return TBool ETrue if the transport was taken down
*/
EXPORT_C TBool CObexTransportControllerBase::BringTransportDown()
	{
	LOG_LINE
	LOG_FUNC

	DeleteTransport();
	return iConnector->BringTransportDown();
	}

/**
Calls ConnectL on the connector

@internalTechnology
*/	
EXPORT_C void CObexTransportControllerBase::ConnectL ()
	{
	LOG_LINE
	LOG_FUNC

	iConnector->ConnectL();
	}

/**
Ask Connector to cancel connect

@see CObexTransportControllerBase::ConnectL
@internalTechnology
*/	
EXPORT_C void CObexTransportControllerBase::CancelConnect()
	{
	LOG_LINE
	LOG_FUNC

	iConnector->CancelConnect();
	}

/**
Calls accept connection on the connector
@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::AcceptConnectionL()
	{
	LOG_LINE
	LOG_FUNC

	iConnector->AcceptL();
	}

/**
Calls Cancel Accept on he connector

@see CObexTransportControllerBase::AcceptConnectionL
@internalTechnology
*/	
EXPORT_C void CObexTransportControllerBase::CancelAccept()
	{
	LOG_LINE
	LOG_FUNC

	if ( iConnector)
		{
		iConnector->CancelAccept();
		}
	}

/**
Calls signal transport error on the connector

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::SignalTransportError()
	{
	LOG_LINE
	LOG_FUNC

	// If this is called during construction, iConnector could be NULL
	if(iConnector)
		{
		iConnector->SignalTransportError();
		}
	}

/**
returns the send packet

@return CObexPacket the packet that will be sent
@internalTechnology
*/	
EXPORT_C CObexPacket& CObexTransportControllerBase::SendPacket ()
	{
	LOG_LINE
	LOG_FUNC

	return *iSendPacket;
	}

/**
Returns the packet received

@return CObexPacket the packet that will be sent
@internalTechnology
*/
EXPORT_C CObexPacket& CObexTransportControllerBase::ReceivePacket ()
	{
	LOG_LINE
	LOG_FUNC

	return *iReceivePacket;
	}

/**
Ask active writer to do a send

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::Send ()
	{
	LOG_LINE
	LOG_FUNC

	LOG3(_L8("Packet Sent, Opcode 0x%2x (0x%2x with final bit cleared), Length %d"), (iSendPacket->IsFinal() ? (iSendPacket->Opcode() | KObexPacketFinalBit) : iSendPacket->Opcode()), iSendPacket->Opcode(), iSendPacket->PacketSize());
	FTRACE(iSendPacket->Dump ());

	iActiveWriter->Transfer (*iSendPacket);
	}

/**
Init the send packet wtih the received, aOpcode, set final to true,  and send the packet

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::Send (TObexOpcode aOpcode)
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\taOpcode = %d"), aOpcode);

	iSendPacket->Init (aOpcode);
	iSendPacket->SetFinal (ETrue);
	Send ();
	}

/**
Ask activer reader to do a read

@internalTechnology
*/
EXPORT_C void CObexTransportControllerBase::Receive ()
	{
	LOG_LINE
	LOG_FUNC

	iActiveReader->Transfer (*iReceivePacket);
	}

/**
Is there a write outstanding

@return TBool  return ETrue is there is a write activer otherwise EFalse
@internalTechnology
*/		
EXPORT_C TBool CObexTransportControllerBase::IsWriteActive ()  const
	{
	LOG_LINE
	LOG_FUNC

	if (iActiveWriter)
		{
		return (iActiveWriter->IsActive());
		}
	else // this has been added incase of null iActiveWriter
		{
		return EFalse;
		}
	}

/**
Insert local Connection info with the received, aVersion, aFlags and from iReceivePacket into the receive packet, aPacket

@return TInt KErrNone or a symbian error
@param aPacket the packet that is updated with local connection info
@param aVersion 
@param aFlags
@internalTechnology
*/
EXPORT_C TInt CObexTransportControllerBase::InsertLocalConnectInfo (CObexPacket &aPacket, TUint8 aVersion, TUint8 aFlags)
	{
	LOG_LINE
	LOG_FUNC

	if (iReceivePacket)
		{
		TObexConnectData localinfo;
		localinfo.iVersion = aVersion;
		localinfo.iFlags = aFlags;
		localinfo.iMaxPacketLength = iReceivePacket->DataLimit();	// maximum packet length we can receive
		return (aPacket.InsertData (localinfo));	
		}
	else
		{
		return KErrGeneral;
		}
	}

/**
Extract the local information from the received packet 

@return EFalse- failed to extract the information. ETrue- extraction succeeded.
@param aPacket The packet from which  local connection info is extracted
@param aVersion
@param aFlags
@internalTechnology
*/
EXPORT_C TBool CObexTransportControllerBase::ExtractRemoteConnectInfo(CObexPacket &aPacket, TUint8& aVersion, TUint8& aFlags)
	{
	LOG_LINE
	LOG_FUNC

	if (iSendPacket)
		{
		TObexConnectData remoteinfo;
		if (!aPacket.ExtractData (remoteinfo))
			{
			return EFalse;
			}
		aVersion = remoteinfo.iVersion;
		aFlags = remoteinfo.iFlags;

		// Set the send buffer data limit (i.e. how much OBEX can use) to match the size of the
		// remote receive buffer (the setter function ensures it's a legal size)
		(void)iSendPacket->SetLegalDataLimit (remoteinfo.iMaxPacketLength);
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

/**
@internalTechnology
*/
void CObexTransportControllerBase::ConstructPacketsL() 
	{
	LOG_FUNC

	// Set actual packet sizes
	// Receive must be at least KObexPacketDefaultSize to maintain functional compatibility with previous implentations
	// (to ensure that remote ends that ignore how big we say our receive buffer is get that same grace on buffer overflow)
	__ASSERT_DEBUG(iTransportInfo, PANIC(KPanicCat, ETransportNullPointer)); 
	TUint16 receivePacketBufferSize = Max ( iTransportInfo->iReceiveMtu, KObexPacketDefaultSize );
	TUint16 sendPacketBufferSize = iTransportInfo->iTransmitMtu;

	// Set initial "software throttle" for packets (how big OBEX says they are)
	// Send packet is set to miniumum, so initial transmit cannot overflow remote end
	TUint16 receivePacketDataLimit = GetReceivePacketDataLimit();
	TUint16 sendPacketDataLimit = KObexPacketMinSize;
		
	LOG2(_L8("CObexTransportControllerBase::ConstructPacketsL send buffer %d send data limit %d"), sendPacketBufferSize, sendPacketDataLimit);
	LOG2(_L8("CObexTransportControllerBase::ConstructPacketsL receive buffer %d receive data limit %d"), receivePacketBufferSize, receivePacketDataLimit);

	// Create the packets
	iSendPacket = CObexPacket::NewL(sendPacketBufferSize, sendPacketDataLimit);
	iReceivePacket = CObexPacket::NewL(receivePacketBufferSize, receivePacketDataLimit);
	}

/**
Gets the socket associated with the connector and asks the socket for its  remote name. 
This can be cast to the appropriate TSockAddr-derived class TIrdaSockAddr for IrDA. 

@publishedPartner
@released
*/
EXPORT_C void CObexTransportControllerBase::RemoteAddr(TSockAddr& aAddr)
	{
	LOG_LINE
	LOG_FUNC

	DoRemoteAddr(aAddr);
	}

/**
Returns true if the derived transport can re-connect when an obex connection is re-connected
This is used to determine whether to take the transport down when an obex connection has been disconnected

@see DoIsTransportRestartable
@return TBool return ETrue if the derived transport can support re-connection when obex re-connects
@publishedPartner
@released
*/	
EXPORT_C TBool CObexTransportControllerBase::IsTransportRestartable() const	
	{
	LOG_LINE
	LOG_FUNC

	return DoIsTransportRestartable();
	}

//MObexTransport Notify Protected 
/**
Process a Packet.  Upcall to the owner

@publishedPartner
@released
*/
EXPORT_C void CObexTransportControllerBase::DoProcess(CObexPacket &aPacket)
	{
	LOG_LINE
	LOG_FUNC

	__ASSERT_DEBUG(iOwner, PANIC(KPanicCat, ETransportNullPointer)); 
	iOwner->Process(aPacket);
	}

/**
Reports and error to the owner

@publishedPartner
@released
*/
EXPORT_C void CObexTransportControllerBase::DoError(TInt aError) 
	{
	LOG_LINE
	LOG_FUNC
	LOG1(_L8("\taError = %d"), aError);

	__ASSERT_DEBUG(iOwner, PANIC(KPanicCat, ETransportNullPointer)); 
	iOwner->Error(aError);
	}

/**
Create the transport objects and reports that the transport is up to the owner

@publishedPartner
@released
*/	
EXPORT_C void CObexTransportControllerBase::DoTransportUp(TObexConnectionInfo& aInfo)
	{
	LOG_LINE
	LOG_FUNC

	DeleteTransport();
	TRAPD(err, InitialiseTransportL(aInfo));
	if(err != KErrNone)
		{
		TransportUpError(err);
		}
	else
		{
		__ASSERT_DEBUG(iOwner, PANIC(KPanicCat, ETransportNullPointer));
		iOwner->TransportUp();
		Receive();
		}
	}

void CObexTransportControllerBase::InitialiseTransportL(TObexConnectionInfo& aInfo)
	{
	NewTransportL(aInfo);
	ConstructPacketsL();
	}

void CObexTransportControllerBase::TransportUpError(TInt aError)
	{
	// At this stage the transport controller will have a connection set-up.
	// As a result if we get an error while setting up Obex we should bring
	// the transport down (the owner has alrady been informed of the error,
	// but we shouldn't expect them to have to bring the transport down).
	TBool broughtDown = BringTransportDown();
	// If we failed to bring down the transport there isn't much
	// more we can do.
	(void)broughtDown;

	Error(aError); // Signal error indication
	}

/**
Signals that an event related to processing the packet has occurred.
@param aEvent The event that has occurred.
*/
EXPORT_C void CObexTransportControllerBase::DoSignalPacketProcessEvent(TObexPacketProcessEvent aEvent)
	{
	LOG_FUNC
	
	__ASSERT_DEBUG(iOwner, PANIC(KPanicCat, ETransportNullPointer)); 
	iOwner->SignalPacketProcessEvent(aEvent);
	}

/**
Returns a pointer to the transport info being used by the transport. This may 
be cast to the transport info type associated with the known concrete 
transport controller.
*/
EXPORT_C const TObexTransportInfo* CObexTransportControllerBase::TransportInfo() const
	{
	LOG_FUNC

	return iTransportInfo;
	}

/**
This function is part of the extension pattern and must be implemented by all derived instantiable classes.
By default this returns null.  Any derived class that is required to extend its interface and that of this base 
class returns its new interface in the form of an M class, that it extends, if and only if  the corresponding TUid, 
aUid, is received. Otherwise the function calls the base class implementation, returning NULL.

@return The M Class representing the extension to the interface otherwise NULL
@param aUid The uid associated with the M Class that is being implemented
@publishedPartner
@released
*/
EXPORT_C TAny* CObexTransportControllerBase::GetInterface(TUid /*aUid*/)
	{
	LOG_FUNC
	
	return NULL;
	}

//private
void CObexTransportControllerBase::DeleteTransport()
	{
	LOG_LINE
	LOG_FUNC

	delete iActiveWriter;
	delete iActiveReader;
	delete iReceivePacket;
	delete iSendPacket;
	iActiveWriter = NULL;
	iActiveReader = NULL;
	iReceivePacket = NULL;
	iSendPacket = NULL;
	}
