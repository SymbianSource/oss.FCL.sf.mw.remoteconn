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


#ifndef __USBCONN_H__
#define __USBCONN_H__

#include <e32base.h>
#include <usb.h>
#include <d32usbc.h>
#include <obex/transport/obexreaderbase.h>
#include <obex/transport/obexwriterbase.h>
#include <obex/transport/obexconnector.h>
#include <obex/transport/mobextransportnotify.h>
#include <obexconstants.h>
#include <obexusbtransportinfo.h>
#include "ObexUsbTransportController.h"

//
// LDD name
//
_LIT(KUsbLddName, "eusbc");

// Transmit and receive endpoints
// Note that these are the virtual endpoint numbers for the bulk interface and
// should be put in the same order in the TUsbcEndpointInfo array in RegInterfacesL
const TEndpointNumber KTransmitEndpoint = EEndpoint1;
const TEndpointNumber KReceiveEndpoint  = EEndpoint2;

// These are taken from section 6.5 of the WMCDC spec.
const TUint KObexDescriptorLength = 18;
const TUint KObexMinNumEndpoints = 3;
const TUint KObexClassNumber = 0x02;
const TUint KObexNumInterfaces = 2;
const TUint KObexSubClassNumber = 0x0b;
const TUint KObexProtocolNumber = 0x0;
const TUint KObexTotalEndpoints = 2;

const TUint KObexDataClass = 0x0A;
const TUint KObexDataSubClass = 0;
const TUint KObexAlt0 = 0;
const TUint KObexAlt1 = 1;
const TUint KObexFunctionalDescriptorLength = 5;
const TUint KObexDefaultInterfaceDescriptorLength = 100; //Default value used prior to OS 9.2
const TUint KObexInterfaceDescriptorBlockLength = 16;

const TUint8 KCdcVersionNumber[2] = { 0x10, 0x01 }; // CDC v1.1
const TUint8 KHeaderFunctionalDescriptor = 0x00;
const TUint8 KObexFunctionalDescriptor = 0x15;
const TUint8 KWmcdcVersionNumber[2] = { 0x00, 0x01 }; // WMCDC v1.0
const TUint8 KUnionFunctionalDescriptor = 0x06;

const TInt KMaxPacketTypeBulkFS = 64;
const TInt KMaxPacketTypeBulkHS = 512;


/**
USB Active Writer class
*/
NONSHARABLE_CLASS(CObexUsbActiveWriter) : public CObexWriterBase
//------------------------------------------------------------------------------------
	{
public:
	static CObexUsbActiveWriter* NewL(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo, TInt aPacketSize);
	~CObexUsbActiveWriter();

private:
	virtual void DoCancel();
	virtual void DoTransfer();

private:
	CObexUsbActiveWriter(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo, TInt aPacketSize);

private:
	RDevUsbcClient& iUsb;       // USB logical device driver
	TInt iPacketSize;
	};


/**
USB Active Reader class
*/
NONSHARABLE_CLASS(CObexUsbActiveReader) : public CObexReaderBase
//------------------------------------------------------------------------------------
    {
public:
	static CObexUsbActiveReader* NewL(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo);
	~CObexUsbActiveReader();

private:
	virtual void DoCancel();
	virtual void DoTransfer();
	virtual TInt GetMaxPacketSize();
	virtual TInt GetInitialPacketSize();	
private:
	CObexUsbActiveReader(MObexTransportNotify& aOwner, RDevUsbcClient& aUsb, TObexConnectionInfo& aInfo);
private:
	RDevUsbcClient& iUsb;       // USB logical device driver
	TBool iGotHeader;
    };
    

/**
Used by the USB underlying object to notify changes in the transport link.
*/
NONSHARABLE_CLASS(MObexUsbConnector)
//------------------------------------------------------------------------------------
	{
public:
	/**
	@internalComponent
	Called in response to the USB transport coming up.
	*/
	virtual void TransportUp() =0;
	
	/**
	@internalComponent
	Called in response to the USB transport going down.
	@param aErr An error value associated with the transport down event.
	*/
	virtual void TransportDown(TInt aErr = KErrDisconnected) =0;
	
	/**
	@internalComponent
	Called when an endpoint stall has been cleared.
	*/
	virtual void StallCleared() =0;
	};


/**
Handles the underlying details of a USB connection.  Monitors the state of
the transport and notifies owner as transport goes up and down.
*/
NONSHARABLE_CLASS(CObexUsbHandler) : public CActive
//------------------------------------------------------------------------------------
    {
public:
	virtual ~CObexUsbHandler();

	static CObexUsbHandler* NewL(TObexUsbTransportInfo& aUsbTransportInfo,
		MObexUsbConnector& aOwner, RDevUsbcClient& aUsb);

	void Start();

	// Implements CActive
	void RunL();
	TInt RunError(TInt /*aError*/);

private:
	CObexUsbHandler(TObexUsbTransportInfo& aUsbProtocolInfo, MObexUsbConnector& aOwner, RDevUsbcClient& aUsb);
	void DoCancel();
	void ConstructL();
	void RegInterfacesL();
	void AllocateDma();
	static void CleanupReleaseInterface0(TAny* aInterface);

private:
	TObexUsbTransportInfo& iObexTransportInfo; // Note: Cannot assume this is valid outside of construction phases
	MObexUsbConnector& iOwner;

	RDevUsbcClient& iUsb;           // USB LDD used for Data Class interface
	RDevUsbcClient iUsbComms;		// USB LDD used for Comms Class interface

	TUint iUsbState;		    	// Used by RDevUsbcClient::AlternateDeviceStatusNotify()
	TBool iTransportUp;             // TRUE when device is configured

	// Interface performance configuration parameters
	// (for extended USB Client Driver API)
	TInt iBandwidthPriority;
	TBool iRequestDmaOnOutEndpoint;
	TBool iRequestDmaOnInEndpoint;
    };


/**
Class to stall then monitor an endpoint and notify owner when the halt state has been cleared.
*/
NONSHARABLE_CLASS(CObexUsbStaller) : public CActive
//------------------------------------------------------------------------------------
	{
public:
	static CObexUsbStaller* NewL(RDevUsbcClient& aUsb, MObexUsbConnector& iOwner);
	~CObexUsbStaller();
	
	void Start();
	
private:
	CObexUsbStaller(RDevUsbcClient& aUsb, MObexUsbConnector& iOwner);
	
	virtual void DoCancel();
	virtual TInt RunError(TInt /*aError*/);
	virtual void RunL();
		
private:
	RDevUsbcClient& iUsb;
	MObexUsbConnector& iOwner;
	TUint iEndpointStatus;
	};


/**
Class to support connecting to other USB devices.
Server mode is relatively straightforward as this corresponds
exactly to how USB works from the device perspective.  Obex client
support though requires some 'magic'.
*/
NONSHARABLE_CLASS(CObexUsbConnector) : public CObexConnector, public MObexUsbConnector
//------------------------------------------------------------------------------------
	{
public:
	/**
	This enum holds the three different states which the connector
	can be in.  In DefaultClient mode, the connector is behaving as
	a client but can be switched to server behaviour.  Once the mode
	has been moved out of DefaultClient (to either Client or Server),
	the mode is locked for the lifetime of the object.  Other
	connectors can provide different behaviour for other CObexServer
	or CObexClient objects.
	*/
	enum TObexUsbMode
		{
		EDefaultClient,
		EClient,
		EServer
		};
	static CObexUsbConnector* NewL(MObexTransportNotify& aOwner, TObexUsbTransportInfo& aUsbTransportInfo);	
	CObexUsbConnector (MObexTransportNotify& aOwner, TObexUsbTransportInfo& aUsbTransportInfo);

	~CObexUsbConnector();

	virtual void ConstructL();

	//Implement MObexUsbConnector interface
	virtual void TransportUp();
	virtual void TransportDown(TInt aErr = KErrDisconnected);
	virtual void StallCleared();	

	//Overrides for pure virtuals in CObexConnector
	virtual void ConnectL();
	virtual void CancelConnect();
	virtual void AcceptL();
	virtual void CancelAccept();
	virtual TBool BringTransportDown();
	virtual void SignalTransportError();
	
	//required to pass the RDevUsbcClient to the activer reader and writer
	RDevUsbcClient* TransportObject();
	TInt GetTransportPacketSize();

private:
	void OpenUsbL(TInt aUnit);	
	static TInt LinkUp(TAny* aConn);
	static TInt NoDevice(TAny* aConn);
	void SignalUp();
	void SignalDown(TInt aErr);
	
	void ResetTransportPacketSize();
	void SetTransportPacketSize();

// Data
private:
	RDevUsbcClient iUsb;	// USB logical device driver
							// Note: This is the handle used for reads and writes.
	TObexUsbTransportInfo& iObexTransportInfo;	// Reference to protocol info to pass on to CObexUsbHandler
												// Note: Cannot assume this is valid outside of construction phases
	TObexUsbMode iMode;
	
	CAsyncCallBack* iCallback;
	TBool iTransportLinkAvailable;
	TBool iConnectRequested;
	TBool iWaitingForLink;
	CObexUsbHandler* iHandler;
	CObexUsbStaller* iStaller;
	
	TInt iPacketSize;
	};


#endif /* __USBCONN_H__ */
