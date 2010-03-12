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


#include <obexusbtransportinfo.h>
#include "usbconn.h"
#include <obex/transport/obexconnector.h>
#include "logger.h"
#include "obexusbfaults.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "USB");
#endif

#ifdef _DEBUG
_LIT(KPanicCat, "ObexUsbHandler");
#endif


//

/**
CObexUsbActiveWriter factory function

@param	aTransport	Reference to a CObexUsbTransportController object. Required for a call to the CObexUsbActiveReader
@param	aOwner		Reference to a MObexTransportNotify object
@param	aUsb		Reference to a RDevUsbcClient object
@param	aInfo		Reference to a TObexConnectionInfo object
@param  aPacketSize The size of packets used on the underlying link, dependent on High Speed mode.
@return Ownership of a new CObexUsbActiveWriter.
*/
CObexUsbActiveWriter* CObexUsbActiveWriter::NewL(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb,
										   TObexConnectionInfo& aInfo, TInt aPacketSize)
	{
	CObexUsbActiveWriter* self = new(ELeave) CObexUsbActiveWriter(aOwner, aUsb, aInfo, aPacketSize);
	CleanupStack::PushL(self);
	self->BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
CObexUsbActiveWriter constructor

@param	aTransport	Reference to a CObexUsbTransportController object. Required for a call to the CObexUsbActiveReader
@param	aOwner		Reference to a MObexTransportNotify object
@param	aUsb		Reference to a RDevUsbcClient object
@param	aInfo		Reference to a TObexConnectionInfo object
@param  aPacketSize The size of packets used on the underlying link, dependent on High Speed mode.
*/
CObexUsbActiveWriter::CObexUsbActiveWriter(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb,
										   TObexConnectionInfo& aInfo, TInt aPacketSize)
: CObexWriterBase(EPriorityHigh, aOwner, aInfo),
 iUsb(aUsb), iPacketSize(aPacketSize)
	{
	}


/**
CObexUsbActiveWriter destructor.
*/
CObexUsbActiveWriter::~CObexUsbActiveWriter()
	{
	LOG_FUNC

	Cancel();
	}


/**
Start actual transfer.  May be called several times by CObexActiveRW::RunL.
Queues a write on USB endpoint.
*/
void CObexUsbActiveWriter::DoTransfer()
	{
	LOG_FUNC
	LOG1(_L("CObexUsbActiveWriter::DoTransfer [Length=%d]"),iLocation.Length());
	
	iLocation.SetMax();
	iUsb.Write(iStatus, KTransmitEndpoint, iLocation, iLocation.Length(), ETrue);
	SetActive();
	}


/**
Cancels an outstanding write.
*/
void CObexUsbActiveWriter::DoCancel()
	{
	LOG_FUNC

	//Only cancel the write if there's more than 1 packet left to write.
	//This restriction is imposed to prevent the SUCCESS response being
	//cancelled when an ObexServer is handling a disconnect and bringing 
	//the transport down.
	if(Remaining() > iPacketSize)
		{
		iUsb.WriteCancel(KTransmitEndpoint);
		}		
	}
		

//

/**
CObexUsbActiveReader factory function

@param	aOwner		Reference to a MObexTransportNotify object
@param	aUsb		Reference to a RDevUsbcClient object
@param	aInfo		Reference to a TObexConnectionInfo object
@return Ownership of a new CObexUsbActiveWriter.
*/
CObexUsbActiveReader* CObexUsbActiveReader::NewL(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb,
										   TObexConnectionInfo& aInfo)
	{
	CObexUsbActiveReader* self = new(ELeave) CObexUsbActiveReader(aOwner, aUsb, aInfo);
	CleanupStack::PushL(self);
	self->BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
CObexUsbActiveReader constructor

@param	aOwner		Reference to a MObexTransportNotify object
@param	aUsb		Reference to a RDevUsbcClient object
@param	aInfo		Reference to a TObexConnectionInfo object
*/
CObexUsbActiveReader::CObexUsbActiveReader(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb,
										   TObexConnectionInfo& aInfo)
: CObexReaderBase(EPriorityStandard, aOwner, aInfo), iUsb(aUsb)
	{
	}


/**
CObexUsbActiveReader destructor.
*/
CObexUsbActiveReader::~CObexUsbActiveReader()
	{
	LOG_FUNC

	Cancel();
	}


/**
Start actual transfer.  May be called several times by CObexActiveRW::RunL.
Queues a read on USB endpoint.
*/
void CObexUsbActiveReader::DoTransfer()
	{
	LOG_FUNC
	LOG1(_L("CObexUsbActiveReader::DoTransfer [maxLength=%d]"),iLocation.MaxLength());

	iUsb.ReadUntilShort(iStatus, KReceiveEndpoint, iLocation);
	SetActive();
	}


/**
Cancels an outstanding read,
 */
void CObexUsbActiveReader::DoCancel()
	{
	FLOG(_L("CObexUsbActiveReader::DoCancel"));

	iUsb.ReadCancel(KReceiveEndpoint);
	}


/**
Return the maximum packet size
@return TInt the maximum packet size for this transport
*/
TInt CObexUsbActiveReader::GetMaxPacketSize()
	{
	LOG_FUNC

	return GetObexPacketDataLimit();
	}


/**
Returns a initial packet size when the packet size of iPacket is not know.  This is used
when determining the remaining bytes to be read.  
@return TInt the initial packet size
*/
TInt CObexUsbActiveReader::GetInitialPacketSize ()
	{
	LOG_FUNC
	
	return GetObexPacketBufferSize();
	}
//

/**
CObexUsbConnector constructor.
*/
CObexUsbConnector::CObexUsbConnector(MObexTransportNotify& aObserver, TObexUsbTransportInfo& aUsbTransportInfo)
 :	CObexConnector(aObserver), 
	iObexTransportInfo(aUsbTransportInfo), 
	iMode(EDefaultClient)
	{
	LOG_FUNC
	// Note iObexTransportInfo reference is only kept so that it can be used in the ConstructL
	// It may not be safe to use this variables outside of the construction phases
	}


/*
Returns an instance of CObexUsbConnector

@param	aController  	Reference to a MObexTransportNotify object (the CObexUsbTransportController that owns this)
@param	aTransportInfo 	Reference to a TObexUsbTransportInfo object containing the transport information

@return	A pointer to a CObexUsbConnector object
*/
CObexUsbConnector* CObexUsbConnector::NewL(MObexTransportNotify& aController, TObexUsbTransportInfo& aTransportInfo)
	{
	CObexUsbConnector* self = new(ELeave) CObexUsbConnector(aController, aTransportInfo);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return(self);
	}


/**
Second phase construction.
*/
void CObexUsbConnector::ConstructL()
	{
	LOG_FUNC

	OpenUsbL(0);

	FLOG(_L("USB opened"));

	iHandler  = CObexUsbHandler::NewL(iObexTransportInfo, *this, iUsb);
	iCallback = new(ELeave) CAsyncCallBack(EPriorityHigh);
	iStaller = CObexUsbStaller::NewL(iUsb, *this);
	iHandler->Start();

	BaseConstructL();
	}


/**
CObexUsbConnector destructor. Releases endpoint and closes USB object if we have an open connection.
*/
CObexUsbConnector::~CObexUsbConnector()
	{
	LOG_FUNC

	delete iCallback;
	delete iHandler;
	delete iStaller;
	
	if (iUsb.Handle())
		{	
		iUsb.Close();
		}
	}


/**
Attempt to open a client-type connection.
This will succeed if the USB host has already opened a connection to us
*/
void CObexUsbConnector::ConnectL()
	{
	LOG_FUNC

	if (iMode == EServer)
		{
		// Already performed a server-type connect, so can't act as a
		// client as well.
		FLOG(_L(" -- In server mode, give up"));
		LEAVEIFERRORL(KErrNotSupported);		
		}
		
		
	FLOG(_L(" -- Not in server mode"));

	iMode = EClient;
	if (iTransportLinkAvailable)
		{
		FLOG(_L(" -- Link available, set a callback"));

		TCallBack callback(LinkUp, this);
		iCallback->Set(callback);
		iCallback->CallBack();
		iConnectRequested = ETrue;
		}
	else
		{
		FLOG(_L(" -- No link available, set a callback"));

		TCallBack callback(NoDevice, this);
		iCallback->Set(callback);
		iCallback->CallBack();
		}
	}


/*
@param	aConn	A pointer to a connector object. Will be cast to CObexUsbConnector

@return	KErrNone
*/
TInt CObexUsbConnector::LinkUp(TAny* aConn)
	{
	FLOG(_L("CObexUsbConnector::LinkUp"));

	reinterpret_cast<CObexUsbConnector*>(aConn)->SignalUp();
	return KErrNone;
	}


/*
@param	aConn	A pointer to a connector object. Will be cast to CObexUsbConnector

@return	KErrNone
*/
TInt CObexUsbConnector::NoDevice(TAny* aConn)
	{
	FLOG(_L("CObexUsbConnector::NoDevice"));

	reinterpret_cast<CObexUsbConnector*>(aConn)->SignalDown(KErrIrObexClientPeerDoesNotHaveObex);
	return KErrNone;
	}


/**
Cancel an active connection attempt.  This is unlikely to succeed, as a client
connection over USB will complete almost immediately.
*/
void CObexUsbConnector::CancelConnect()
	{
	LOG_FUNC

	if (iMode == EClient)
		{
		iCallback->Cancel();
		}
	
	iConnectRequested = EFalse;
	}


/**
Start listening for a connection to us.
*/
void CObexUsbConnector::AcceptL()
	{
	LOG_FUNC

	if (iMode == EClient)
		{
		// Already carried out a client side connection.  Therefore we
		// can't be a server as well.
		FLOG(_L(" -- In client mode, give up"));

		LEAVEIFERRORL(KErrNotSupported);		
		}
	
	FLOG(_L(" -- Not in client mode"));

	iMode = EServer;
	if (iTransportLinkAvailable)
		{
		FLOG(_L(" -- Transport link currently available, set a callback"));

		TCallBack callback(LinkUp, this);
		iCallback->Set(callback);
		iCallback->CallBack();
		}
	else
		{
		FLOG(_L(" -- No link available, wait for one"));

		iWaitingForLink = ETrue;
		}
	}

/**
Stop listening for connections.
*/
void CObexUsbConnector::CancelAccept()
	{
	LOG_FUNC

	if (iMode == EServer)
		{
		iWaitingForLink = EFalse;
		iCallback->Cancel();
		}
	}


/**
Called to indicate transport layer has come up.
*/
void CObexUsbConnector::TransportUp()
	{
	LOG_FUNC
	
	SetTransportPacketSize();

	iTransportLinkAvailable = ETrue;
	
	if ((iMode == EServer) && (iWaitingForLink))
		{
		FLOG(_L(" -- Signalling link available"));

		iWaitingForLink = EFalse;
		SignalUp();
		}
	}

/**
Called to indicate transport layer has gone down.

@param	aErr 	Any error code associated with disconnection. Defaults to KErrDisconnected.
*/
void CObexUsbConnector::TransportDown(TInt aErr)
	{
	LOG_FUNC
	
	ResetTransportPacketSize();
	
	iStaller->Cancel();

	iTransportLinkAvailable = EFalse;
		
	if (iMode == EServer)
		{
		FLOG(_L(" -- In server mode, signal link down"));

		SignalDown(aErr);
		}
	else
		{
		FLOG(_L(" -- In client mode"));

		// Client or default client state
		if (iConnectRequested)
			{
			FLOG(_L(" -- Outstanding client connection. Signal link down"));

			iConnectRequested = EFalse;
			SignalDown(aErr);
			}
		}
	}

/**
Signals to the transport controller that the transport is up
*/
void CObexUsbConnector::SignalUp()
	{
	LOG_FUNC

	// Indicate transport now up
	TObexConnectionInfo sockinfo;
	sockinfo.iMaxRecvSize = 0;	// set these to zero as we can't find them out
	sockinfo.iMaxSendSize = 0;	// and they are not used
	sockinfo.iSocketType = TObexConnectionInfo::EUsbStream;
	Observer().TransportUp(sockinfo);
	}


/**
Signals (with an error code) to the transport controller that the transport has gone down 
*/
void CObexUsbConnector::SignalDown(TInt aErr)
	{
	FLOG(_L("CObexUsbConnector::SignalDown"));

	Observer().Error(aErr);
	}


/**
The Obex server running over USB is not able to bring the transport
down itself. It will only delete the transport object when an error
has occurred (such as the link being brought down) and hence the
transport is no longer valid.

@return	ETrue
*/
TBool CObexUsbConnector::BringTransportDown()
{
	// This cancel function needs to be called here because it
	// doesn't get called anywhere else. Bluetooth and IrDA rely
	// on the socket shutdown to cancel the accept.
	CancelAccept();
	return ETrue;
}


/**
Return the transport layer object for use by USB transport objects

@return Pointer to the USB transport layer object
*/
RDevUsbcClient* CObexUsbConnector::TransportObject()
	{
	return &iUsb;
	}


/**
Sets a bus stall condition on the IN endpoint.  This will cause the USB
host to initiate a CLEAR_FEATURE sequence, which notifies the server that the
Obex link has been reset (equivalent to dropping the Obex level connection)
*/
void CObexUsbConnector::SignalTransportError()
	{
	LOG_FUNC
	
	iStaller->Start();
	iTransportLinkAvailable = EFalse;
	iConnectRequested = EFalse;
	}


/**
Stall cleared => transport available.
This upcall has been separated from the TransportUp upcall for two
reasons---it's called from a different class and it happens when an
error condition has been cleared.  Currently it has no special
meaning, but this could change in future.
*/
void CObexUsbConnector::StallCleared()
	{
	LOG_FUNC
	
	TransportUp();
	}


/**
Opens USB and prepares it for use.

@param	aUnit	According to the RDevUsbcClient documentation, this should be 0. 
*/
void CObexUsbConnector::OpenUsbL(TInt aUnit)
	{
	LOG1(_L("CObexUsbConnector::OpenUsbL called for unit=%d"), aUnit);
	
	// Load ldd
	TInt err = User::LoadLogicalDevice(KUsbLddName);

	if (err != KErrNone && err != KErrAlreadyExists)
		{
		LEAVEIFERRORL(err);
		}

	// Open the requested unit and initialise local state
	LEAVEIFERRORL(iUsb.Open(aUnit));
	}


void CObexUsbConnector::ResetTransportPacketSize()
	{
	iPacketSize = 0;
	}


void CObexUsbConnector::SetTransportPacketSize()
	{
	if (iUsb.CurrentlyUsingHighSpeed())
		{
		iPacketSize = KMaxPacketTypeBulkHS;
		}
	else
		{
		iPacketSize = KMaxPacketTypeBulkFS;
		}
	}


TInt CObexUsbConnector::GetTransportPacketSize()
	{
	return iPacketSize;
	}


//

/**
Constructs a CObexUsbHandler object

@param	aUsbTransportInfo	Reference to a TObexTransportInfo object. Will be cast to TObexUsbTransportInfo.
@param	aOwner				Reference to a MObexUsbConnector that owns this handler
@param	aUsb				Reference to a RDevUsbcClient object.

@return	A new CObexUsbHandler object
*/
CObexUsbHandler* CObexUsbHandler::NewL(TObexUsbTransportInfo& aUsbTransportInfo,
									   MObexUsbConnector& aOwner,
									   RDevUsbcClient& aUsb)
	{
	FLOG(_L("CObexUsbHandler::NewL"));
	
	CObexUsbHandler* self = new (ELeave) CObexUsbHandler(aUsbTransportInfo, aOwner, aUsb);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


/**
CObexUsbHandler constructor.

Note that the iObexTransportInfo may not be valid outside of the construction phases
The constructor copies out most of the information it needs into member variables
apart from the interface string descriptor, which is used from RegInterfacesL

@param	aUsbTransportInfo	Reference to a TObexTransportInfo object. Will be cast to TObexUsbTransportInfo.
@param	aOwner				Reference to a MObexUsbConnector that owns this handler
@param	aUsb				Reference to a RDevUsbcClient object.
*/
CObexUsbHandler::CObexUsbHandler(TObexUsbTransportInfo& aUsbTransportInfo,
								 MObexUsbConnector& aOwner,
								 RDevUsbcClient& aUsb)
 : CActive(EPriorityStandard), iObexTransportInfo(aUsbTransportInfo),
   iOwner(aOwner), iUsb(aUsb)
	{
	LOG_FUNC
	
	CActiveScheduler::Add(this);

	// Set interface performance configuration parameters
	if (aUsbTransportInfo.iTransportName == KObexUsbProtocol)
		// Default values if transport is normal USB - no extra bandwidth, no DMA
		{
		FLOG(_L("CObexUsbHandler - Using default USB protocol info"));
		iBandwidthPriority = EUsbcBandwidthOUTDefault | EUsbcBandwidthINDefault;
		iRequestDmaOnOutEndpoint = EFalse;
		iRequestDmaOnInEndpoint = EFalse;
		}
	else if (aUsbTransportInfo.iTransportName == KObexUsbProtocolV2)
		// If transport is extended USB take values from the protocol info
		{
		FLOG(_L("CObexUsbHandler - Using USB protocol extended info"));
		TObexUsbV2TransportInfo& infoV2 = static_cast<TObexUsbV2TransportInfo&>(aUsbTransportInfo);
		iBandwidthPriority       = infoV2.iBandwidthPriority;
		iRequestDmaOnOutEndpoint = infoV2.iDmaOnOutEndpoint;
		iRequestDmaOnInEndpoint  = infoV2.iDmaOnInEndpoint;
		}
	else
		{
		//aUsbTransportInfo.iTransportName is used to load the correct plug-in, so if this code
		//is being executed it must have contained one of the options above. Else something
		//has gone badly wrong.
		__ASSERT_DEBUG(false, PANIC(KPanicCat, EUnknownUsbTransport));
		}

	LOG3(_L("CObexUsbHandler - iBandwidthPriority 0x%X, iDmaOnOutEndpoint, %d iDmaOnInEndpoint %d"),
						iBandwidthPriority, iRequestDmaOnOutEndpoint, iRequestDmaOnInEndpoint);
	}


/**
Second phase construction.
*/
void CObexUsbHandler::ConstructL()
	{

	// Check bandwidth priority to make sure it makes sense
	// Reasoning is that the bitwise OR of the maximum values of IN and OUT priorities
	// covers all the bits that can be set in iBandwidthPriority so, using this as a mask,
	// anything set outside of this indicates that iBandwidthPriority is corrupt
	if ( ( iBandwidthPriority & ~( EUsbcBandwidthOUTMaximum | EUsbcBandwidthINMaximum ) ) != 0 )
		{
		LEAVEIFERRORL(KErrArgument);
		}

	FLOG(_L("About to open USB comms interface connection"));

	LEAVEIFERRORL(iUsbComms.Open(0));

	FLOG(_L("Registering interfaces"));

	RegInterfacesL();
	}


/**
CObexUsbHandler destructor.
*/
CObexUsbHandler::~CObexUsbHandler()
	{
	LOG_FUNC
	Cancel();

	// Must release interfaces from highest alternate setting downwards
	if (iUsb.Handle())
		{
#ifndef NO_ALTERNATE_USB_INTERFACE_SUPPORT
		iUsb.ReleaseInterface(1 /*alternate setting*/);
#endif
		iUsb.ReleaseInterface(0 /*alternate setting*/);
		}

	if (iUsbComms.Handle())
		{	
		iUsbComms.ReleaseInterface(0 /*alternate setting*/);
		}

	iUsbComms.Close();
	}


/**
Standard active object error function.

@return	KErrNone because currently nothing should cause this to be called.
*/
TInt CObexUsbHandler::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}


/**
This function will be called upon a change in the state of the device
(as set up in AcceptL).
*/
void CObexUsbHandler::RunL()
	{
	LOG1(_L("CObexUsbHandler::RunL called state=0x%X"), iUsbState);
	
	if (iStatus != KErrNone)
		{
		LOG1(_L("CObexUsbHandler::RunL() - Error = %d"),iStatus.Int());

		iTransportUp = EFalse;
		iOwner.TransportDown(iStatus.Int());
		return;
		}

	// Check whether the alternate setting to use or the device state has changed.
	// We have to check the device state to handle the unplugging of the cable. We can't
	// rely on the always-outstanding read to complete with an error, because the driver
	// doesn't have a well-defined error code for this.

	if (iUsbState & KUsbAlternateSetting)
		{
		iUsbState &= ~KUsbAlternateSetting;

		LOG1(_L("switched to alternate setting %d"), iUsbState);

		if (iUsbState == KObexAlt0)
			{
			// alternate setting 0 - function inactive
			if (iTransportUp)
				{
				FLOG(_L("CObexUsbHandler::RunL - transport layer going down..."));

				iTransportUp = EFalse;
				iOwner.TransportDown();
				}
			}
		else if (iUsbState == KObexAlt1)
			{
			// alternate setting 1 - function active
			if (!iTransportUp)
				{
				FLOG(_L("CObexUsbHandler::RunL - transport layer coming up..."));

				// Now the transport is selected, DMA can be allocated to the endpoints, if required
				AllocateDma();

				iTransportUp = ETrue;

				iOwner.TransportUp();
				}
			}
		else
			{
			FLOG(_L("WARNING: unknown setting!"));
			}
		}
	else
		{
		TUsbcDeviceState deviceState = static_cast<TUsbcDeviceState>(iUsbState);

		switch(deviceState)
			{
			case EUsbcDeviceStateUndefined:
			case EUsbcDeviceStateAttached:
			case EUsbcDeviceStatePowered:
			case EUsbcDeviceStateDefault:
			case EUsbcDeviceStateAddress:
#ifdef ERRONEOUS_SUSPEND_INDICATIONS
			// On Lubbock, we sometimes get "suspended" when the cable is unplugged.
			case EUsbcDeviceStateSuspended:
#endif // ERRONEOUS_SUSPEND_INDICATIONS
				if (iTransportUp)
					{
					FLOG(_L("CObexUsbHandler::RunL - transport layer going down..."));

					iTransportUp = EFalse;
					iOwner.TransportDown();
					}
				break;

#ifndef ERRONEOUS_SUSPEND_INDICATIONS
			case EUsbcDeviceStateSuspended:
				break;
#endif // !ERRONEOUS_SUSPEND_INDICATIONS

			case EUsbcDeviceStateConfigured:
				// Normally, we don't do anything here, because the transport only goes up when
				// we actually receive the alternate interface change.
#ifdef NO_ALTERNATE_USB_INTERFACE_SUPPORT
				if (!iTransportUp)
					{
					FLOG(_L("Device now configured: transport layer coming up..."));

					iTransportUp = ETrue;

					// DMA can be allocated to the endpoints, if required
					// This has to be done here for no alternate interface devices
					// as they will not receive interface change notification
					AllocateDma();

					iOwner.TransportUp();
					}
#endif // NO_ALTERNATE_USB_INTERFACE_SUPPORT
				break;

			default:
				__ASSERT_DEBUG(false, PANIC(KPanicCat, EUnknownUsbState));
				break;
			}
		}

	// Await further notification of a state change. We may have called Error(), which
	// would already have set this notification request.
	if (!IsActive())
		{
		iUsb.AlternateDeviceStatusNotify(iStatus, iUsbState);
		SetActive();
		}

	FLOG(_L("CObexUsbHandler::RunL finished"));
	}


/**
Standard active object cancellation function.
*/
void CObexUsbHandler::DoCancel()
	{
	LOG_FUNC

	iUsb.AlternateDeviceStatusNotifyCancel();
	}


/**
Registers the required comms and data interfaces.
*/
void CObexUsbHandler::RegInterfacesL()
	{
	LOG_FUNC

	// Setup a CDC Communication Class interface
	TUsbcInterfaceInfoBuf ifc;

	// Extract the interface string from the transport info structure we were
	// passed on construction. This isn't const because the SetInterface API
	// requires it not to be.
	// This is the only point where iObexTransportInfo is used directly
	// Do not use iObexTransportInfo outside of construction phases
	TPtrC string(iObexTransportInfo.iInterfaceStringDescriptor);

	LOG1(_L("Using interface string \"%S\""), &string);
		
	ifc().iString = &string;
	ifc().iClass.iClassNum = KObexClassNumber;		
	ifc().iClass.iSubClassNum = KObexSubClassNumber;
	ifc().iClass.iProtocolNum = KObexProtocolNumber;
	ifc().iTotalEndpointsUsed = 0;
	
	// Indicate that this interface does not expect any control transfers 
	// from EP0.
	ifc().iFeatureWord |= KUsbcInterfaceInfo_NoEp0RequestsPlease;
	
	FLOG(_L("CObexUsbListeningConnector::RegInterfacesL - setting comms interface")); 
	
	TInt err = iUsbComms.SetInterface(0, ifc);

	if (err != KErrNone) 
		{ 
		LOG1(_L("SetInterface failed with error %d"), err);
		LEAVEIFERRORL(err);
		}

	// Get the interface number for later 
	TInt intDescSize;
	err = iUsbComms.GetInterfaceDescriptorSize(0,intDescSize);	
	if(err != KErrNone)
		{
		LOG1(_L("Failed to get interface descriptor size. Err = %d"),err);
		intDescSize = KObexDefaultInterfaceDescriptorLength; //Default is 100
		}
	HBufC8* interfaceDescriptor;
	interfaceDescriptor = HBufC8::NewLC(intDescSize);
	TPtr8 pIntDesc = interfaceDescriptor->Des();
	iUsbComms.GetInterfaceDescriptor(0, pIntDesc);
	TUint8 obexIntNo = interfaceDescriptor->Ptr()[2];
	CleanupStack::PopAndDestroy();

	TBuf8<KObexInterfaceDescriptorBlockLength> desc;

	// Comms Class Header Functional Descriptor

	desc.Append(KObexFunctionalDescriptorLength);
	desc.Append(KUsbDescType_CS_Interface);
	desc.Append(KHeaderFunctionalDescriptor);
	desc.Append(KCdcVersionNumber[0]);
	desc.Append(KCdcVersionNumber[1]);

	// Obex Functional Descriptor

	desc.Append(KObexFunctionalDescriptorLength);
	desc.Append(KUsbDescType_CS_Interface);
	desc.Append(KObexFunctionalDescriptor);
	desc.Append(KWmcdcVersionNumber[0]);
	desc.Append(KWmcdcVersionNumber[1]);

	// Union Functional Descriptor
	
	desc.Append(KObexFunctionalDescriptorLength);
	desc.Append(KUsbDescType_CS_Interface); 
	desc.Append(KUnionFunctionalDescriptor);
	desc.Append(obexIntNo); 
	TInt dataInt = obexIntNo + 1;
	desc.Append(static_cast<TUint8>(dataInt));		

	err = iUsbComms.SetCSInterfaceDescriptorBlock(0, desc);
	if (err != KErrNone)
		{
		LOG1(_L("SetCSInterfaceDescriptorBlock failed with error %d"), err);
		LEAVEIFERRORL(err);
		}

	// Setup CDC Data Class interfaces

	// Only set the "function inactive" interface (alternate setting 0)
	// if the device can support alternate interfaces
#ifndef NO_ALTERNATE_USB_INTERFACE_SUPPORT
	TUsbcInterfaceInfoBuf dataifc;

	dataifc().iString = NULL;
	dataifc().iClass.iClassNum = KObexDataClass;		
	dataifc().iClass.iSubClassNum = KObexDataSubClass;
	dataifc().iClass.iProtocolNum = 0;
	dataifc().iTotalEndpointsUsed = 0;

	// Indicate that this interface does not expect any control transfers 
	// from EP0.
	dataifc().iFeatureWord |= KUsbcInterfaceInfo_NoEp0RequestsPlease;

	FLOG(_L("Setting data class interface (alt setting 0")); 
	
	err = iUsb.SetInterface(0 /*alternate setting*/, dataifc, iBandwidthPriority);
	if (err != KErrNone) 
		{
		LOG1(_L("Cannot set data interface (alternate setting 0): error %d"), err);
		LEAVEIFERRORL(err);
		}

	// If anything below this point leaves, make sure this alternate setting
	// gets released.
	CleanupStack::PushL(TCleanupItem(CleanupReleaseInterface0, &iUsb));
#endif // NO_ALTERNATE_USB_INTERFACE_SUPPORT

	// Check the device has enough endpoints for the "function active" data interface
	TUsbDeviceCaps dCaps;
	LEAVEIFERRORL(iUsb.DeviceCaps(dCaps));
	
	TInt n = dCaps().iTotalEndpoints;
	if (n < KObexMinNumEndpoints) 
		{
		LEAVEIFERRORL(KErrOverflow);
		}
	
	// Use the hardware's HS capability to determine maximum bulk transfer packet size
	TInt maxPacketSize = (dCaps().iHighSpeed) ? KMaxPacketTypeBulkHS : KMaxPacketTypeBulkFS;
	
		
	// Get information on the available endpoints from the driver
	TUsbcEndpointData data[KUsbcMaxEndpoints];
	TPtr8 dataptr(REINTERPRET_CAST(TUint8*, data), sizeof(data), sizeof(data));
	LEAVEIFERRORL(iUsb.EndpointCaps(dataptr));

	// Check to see if there are suitably capable IN and OUT endpoints available
	// and fill dataifc2 structure accordingly.
	//
	// NOTE: The order the iEndpointData array is filled matches the order used for the
	// virtual endpoint numbers KTransmitEndpoint and KReceiveEndpoint - so change with caution!
	//
	// The virtual endpoint numbers are 1-based, whereas the iEndpointData array is 0 based
	// hence the subtraction in the array indices
	//
	// NOTE: IN and OUT refer to the host so in this case:
	//			IN  => device to PC = KTransmitEndpoint
	//			OUT => PC to device = KReceiveEndpoint
	TUsbcInterfaceInfoBuf dataifc2;
	TBool foundIn = EFalse;
	TBool foundOut = EFalse;
	for (TInt i = 0; !(foundIn && foundOut) && i < n; i++)
		{
		const TUsbcEndpointCaps* caps = &data[i].iCaps;
		if (data[i].iInUse)
			{
			continue;
			}
			
		const TUint KBulkInFlags = KUsbEpTypeBulk | KUsbEpDirIn;
		const TUint KBulkOutFlags = KUsbEpTypeBulk | KUsbEpDirOut;

		if (!foundIn && (caps->iTypesAndDir & KBulkInFlags) == KBulkInFlags)
			{
			dataifc2().iEndpointData[KTransmitEndpoint - 1].iType  = KUsbEpTypeBulk;
			dataifc2().iEndpointData[KTransmitEndpoint - 1].iDir   = KUsbEpDirIn;	
			TInt maxSize = caps->MaxPacketSize();
			if (maxSize > maxPacketSize)
				{
				maxSize = maxPacketSize;
				}
			dataifc2().iEndpointData[KTransmitEndpoint - 1].iSize  = maxSize;
			
			// Allocate dma if requested and the device support resource allocation scheme version 2
			// for resource allocation scheme version1, refer to AllocateDma()
			if (iRequestDmaOnInEndpoint && 
				((dCaps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) != 0))
				{
				dataifc2().iEndpointData[KTransmitEndpoint - 1].iFeatureWord1 |= KUsbcEndpointInfoFeatureWord1_DMA;
				}

			foundIn = ETrue;
			}
		else if (!foundOut && (caps->iTypesAndDir & KBulkOutFlags) == KBulkOutFlags)
			{
			dataifc2().iEndpointData[KReceiveEndpoint - 1].iType = KUsbEpTypeBulk;
			dataifc2().iEndpointData[KReceiveEndpoint - 1].iDir = KUsbEpDirOut;
			TInt maxSize = caps->MaxPacketSize();
			if (maxSize > maxPacketSize)
				{
				maxSize = maxPacketSize;
				}
			dataifc2().iEndpointData[KReceiveEndpoint - 1].iSize  = maxSize;
			
			//Allocate dma here if requested and the device support resource allocation scheme version 2
			// for resource allocation scheme version1, refer to AllocateDma()
			if (iRequestDmaOnOutEndpoint && 
				((dCaps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) != 0))
				{
				dataifc2().iEndpointData[KReceiveEndpoint - 1].iFeatureWord1 |= KUsbcEndpointInfoFeatureWord1_DMA;
				}
			
			foundOut = ETrue;
			}
		}
		
	if (!(foundIn && foundOut)) 
		{
		LEAVEIFERRORL(KErrHardwareNotAvailable);
		}
	
	// Set the active interface.
	dataifc2().iString = NULL;
	dataifc2().iClass.iClassNum = KObexDataClass;		
	dataifc2().iClass.iSubClassNum = KObexDataSubClass;
	dataifc2().iClass.iProtocolNum = 0;
	dataifc2().iTotalEndpointsUsed = KObexTotalEndpoints;
	
	// Indicate that this interface does not expect any control transfers 
	// from EP0.
	dataifc2().iFeatureWord |= KUsbcInterfaceInfo_NoEp0RequestsPlease;
		
#ifdef NO_ALTERNATE_USB_INTERFACE_SUPPORT
	// For devices that don't suport alternate interfaces, have to place the "function active"
	// interface at alternate setting 0, although the CDC specification says if should be alternate setting 1
	FLOG(_L("Setting data class interface (no alternate interface support)"));
	
	err = iUsb.SetInterface(0 /*alternate setting*/, dataifc2, iBandwidthPriority);

	if (err != KErrNone) 
		{
		//FTRACE(FPrint(_L("Cannot set data interface (no alternate interface support): error %d"), err));
		LOG1(_L("Cannot set data interface (no alternate interface support): error %d"), err);
		LEAVEIFERRORL(err);
		}
#else
	FLOG(_L("Setting data class interface (alternate setting 1)"));
	
	err = iUsb.SetInterface(1 /*alternate setting*/, dataifc2, iBandwidthPriority); 

	if (err != KErrNone) 
		{
		//FTRACE(FPrint(_L("Cannot set data interface (alternate setting 1): error %d"), err));
		LOG1(_L("Cannot set data interface (alternate setting 1): error %d"), err);
		LEAVEIFERRORL(err);
		}

	CleanupStack::Pop(); // ReleaseInterface0
#endif

	FLOG(_L("CObexUsbHandler::RegInterfacesL - finished."));
	}


/**
Utility function which releases the first alternate setting of the specified
interface. Used when setting up Obex interfaces.

@param	aInterface	The interface to release
*/
void CObexUsbHandler::CleanupReleaseInterface0(TAny* aInterface)
	{
	reinterpret_cast<RDevUsbcClient*>(aInterface)->ReleaseInterface(0 /*alternate setting*/);
	}


/**
Accept an incoming connection.
*/
void CObexUsbHandler::Start()
	{
	LOG_FUNC

	// Await notification of a state change (this is like waiting for a connect...).
	iUsb.AlternateDeviceStatusNotify(iStatus, iUsbState);
	SetActive();
	}

	
/**
Utility function to allocate DMA to the bulk endpoints - if they have been requested
This function is called during a connect, once the device is configured,
so if the allocations fail it wont complain in release builds but debug builds will
panic with either EDmaAllocationFailedEndpointIn or EDmaAllocationFailedEndpointOut. 
*/
void CObexUsbHandler::AllocateDma()
	{
	LOG_FUNC

	TUsbDeviceCaps dCaps;
	iUsb.DeviceCaps(dCaps);
	if ((dCaps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) != 0)
		{
		// for resource allocation version2, refer to CObexUsbHandler::RegInterfacesL()
		return;
		}

	if (iRequestDmaOnInEndpoint)
		{
		TInt err = iUsb.AllocateEndpointResource(KTransmitEndpoint, EUsbcEndpointResourceDMA);
		// The following log message is checked for in the test code - change with caution!
		LOG1(_L("IN Endpoint DMA resource allocation result %d"), err);
		(void)err; // to stop compilers complaining about unused variables
		}

	if (iRequestDmaOnOutEndpoint)
		{
		TInt err = iUsb.AllocateEndpointResource(KReceiveEndpoint, EUsbcEndpointResourceDMA);
		// The following log message is checked for in the test code - change with caution!
		LOG1(_L("OUT Endpoint DMA resource allocation result %d"), err);
		(void)err; // to stop compilers complaining about unused variables
		}
	}


//

/**
Constructs a CObexUsbStaller object

@param	aUsb			Reference to a RDevUsbcClient object.
@param	aOwner			Reference to a MObexUsbConnector that owns this handler

@return	A new CObexUsbStaller object
*/
CObexUsbStaller* CObexUsbStaller::NewL(RDevUsbcClient& aUsb, MObexUsbConnector& aOwner)
	{
	CObexUsbStaller* self = new(ELeave) CObexUsbStaller(aUsb, aOwner);
	return self;
	}

/**
CObexUsbStaller constructor.

@param	aUsb			Reference to a RDevUsbcClient object.
@param	aOwner			Reference to a MObexUsbConnector that owns this handler
*/
CObexUsbStaller::CObexUsbStaller(RDevUsbcClient& aUsb, MObexUsbConnector& aOwner)
	: CActive(EPriorityStandard), iUsb(aUsb), iOwner(aOwner)
	{
	CActiveScheduler::Add(this);
	}


/**
CObexUsbStaller destructor.
*/
CObexUsbStaller::~CObexUsbStaller()
	{
	Cancel();
	}


/**
Starts the staller
*/
void CObexUsbStaller::Start()
	{
	LOG_FUNC
	
	if (!IsActive())
		{
		FLOG(_L("Halting transmit endpoint..."));
		iUsb.HaltEndpoint(KTransmitEndpoint);
		iUsb.EndpointStatusNotify(iStatus, iEndpointStatus);
		SetActive();
		}
	}
	

/**
Called when the transmit endpoint has changed state
*/
void CObexUsbStaller::RunL()
	{
	LOG_FUNC
	
	TEndpointState endpointState;
	iUsb.EndpointStatus(KTransmitEndpoint, endpointState);	
#ifdef __FLOG_ACTIVE
	switch (endpointState)
		{
		case EEndpointStateNotStalled:
			{
			FLOG(_L("Receive endpoint not stalled"));
			break;
			}
		case EEndpointStateStalled:
			{
			FLOG(_L("Receive endpoint stalled"));
			break;
			}
		case EEndpointStateUnknown:
			{
			FLOG(_L("Receive endpoint unknown state"));
			break;
			}
		}
#endif		 
	if (endpointState == EEndpointStateNotStalled)
		{
		LOG1(_L("CObexUsbStallWatcher::RunL -- endpoint no longer stalled (0x%08x)"), iEndpointStatus);
		iOwner.StallCleared();
		}
	else
		{
		LOG1(_L("CObexUsbStallWatcher::RunL -- endpoint still stalled (0x%08x)"), iEndpointStatus);
		iUsb.EndpointStatusNotify(iStatus, iEndpointStatus);
		SetActive();
		}
	}


/**
Standard active object error function.

@return	KErrNone because currently nothing should cause this to be called.
*/
TInt CObexUsbStaller::RunError(TInt /*aError*/)
	{
	LOG_FUNC
	
	return KErrNone;
	}
	

/**
Standard active object cancellation function.
*/	
void CObexUsbStaller::DoCancel()
	{
	FLOG(_L("CObexUsbStaller -- Cancelling status notification"));
	iUsb.EndpointStatusNotifyCancel();
	}
	
