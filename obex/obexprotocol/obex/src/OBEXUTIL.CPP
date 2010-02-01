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

#include <e32std.h>
#include <obextransportinfo.h>
#include <obexirtransportinfo.h>
#include <obexbttransportinfo.h>
#include <obexusbtransportinfo.h>
#include "obexcommon.h"
#include "OBEXUTIL.H"
#include "obexfaults.h"


void IrOBEXUtil::Panic(TObexPanicCode aCode)
	{
	User::Panic(KObexPanicCategory, aCode);
	}


void IrOBEXUtil::Fault(TObexFaultCode aCode)
	{
	User::Panic(KObexFaultCategory, aCode);
	}

/**
Converts aResp to an EPOC error number 
*/
TInt IrOBEXUtil::EpocError(TUint8 aResp)
	{
	switch (aResp)
		{
	case ERespContinue:
	case ERespSuccess:
	case ERespCreated:
	case ERespAccepted:
		return (KErrNone);
	case ERespBadRequest:
	case ERespUnauthorized:
	case ERespForbidden:
	case ERespNotAcceptable:
	case ERespMethodNotAllowed:
		return (KErrArgument);
	case ERespNotFound:
		return (KErrNotFound);
	case ERespTimedOut:
		return (KErrTimedOut);
	case ERespConflict:
		return (KErrInUse);
	case ERespNotImplemented:
		return (KErrNotSupported);
	default:
		return (KErrIrObexRespBase - aResp);
		}
	}

/**
Returns a TObexTransportInfo created from the received aObexProtocolInfoPtr and aObexProtocolPolicy
to be used in the transport layer
*/
TObexTransportInfo* IrOBEXUtil::CreateTransportInfoL(TObexProtocolInfo& aObexProtocolInfoPtr, TObexProtocolPolicy& aObexProtocolPolicy)
	{
	TObexTransportInfo* transportInfo = NULL;

	if ( aObexProtocolInfoPtr.iTransport == KObexIrTTPProtocol )
		{
		transportInfo = new(ELeave) TObexIrTransportInfo;
		TObexIrProtocolInfo& irProtocolInfo = static_cast<TObexIrProtocolInfo&>(aObexProtocolInfoPtr);
		static_cast<TObexIrTransportInfo*>(transportInfo)->iAddr  = irProtocolInfo.iAddr;
		static_cast<TObexIrTransportInfo*>(transportInfo)->iClassName = irProtocolInfo.iClassName;
		static_cast<TObexIrTransportInfo*>(transportInfo)->iAttributeName = irProtocolInfo.iAttributeName;		
		}
	else if ( aObexProtocolInfoPtr.iTransport == KObexIrTTPProtocolV2 )
		{
		transportInfo = new(ELeave) TObexIrV2TransportInfo;
		TObexIrProtocolInfo& irProtocolInfo = static_cast<TObexIrProtocolInfo&>(aObexProtocolInfoPtr);
		static_cast<TObexIrV2TransportInfo*>(transportInfo)->iAddr  = irProtocolInfo.iAddr;
		static_cast<TObexIrV2TransportInfo*>(transportInfo)->iClassName = irProtocolInfo.iClassName;
		static_cast<TObexIrV2TransportInfo*>(transportInfo)->iAttributeName = irProtocolInfo.iAttributeName;
		static_cast<TObexIrV2TransportInfo*>(transportInfo)->iDiscoverySlots = irProtocolInfo.iDiscoverySlots;
		static_cast<TObexIrV2TransportInfo*>(transportInfo)->iDiscoveryAttempts = irProtocolInfo.iDiscoveryAttempts;
		}
	else if ( aObexProtocolInfoPtr.iTransport == KObexRfcommProtocol )
		{
		transportInfo = new(ELeave) TObexBtTransportInfo;
		TObexBluetoothProtocolInfo& btProtocolInfo = static_cast<TObexBluetoothProtocolInfo&>(aObexProtocolInfoPtr);
		static_cast<TObexBtTransportInfo*>(transportInfo)->iAddr = btProtocolInfo.iAddr;
		}
#ifdef __WINS__
	else if ( aObexProtocolInfoPtr.iTransport == KObexUsbProtocol || aObexProtocolInfoPtr.iTransport == KObexWin32UsbProtocol )
#else
	else if ( aObexProtocolInfoPtr.iTransport == KObexUsbProtocol)
#endif
		{
		transportInfo = new(ELeave) TObexUsbTransportInfo();
		TObexUsbProtocolInfo& usbTransportInfo = static_cast<TObexUsbProtocolInfo&>(aObexProtocolInfoPtr);
		static_cast<TObexUsbTransportInfo*>(transportInfo)->iInterfaceStringDescriptor = usbTransportInfo.iInterfaceStringDescriptor;			
		}
	else if ( aObexProtocolInfoPtr.iTransport == KObexUsbProtocolV2 )
		{
		transportInfo = new(ELeave) TObexUsbV2TransportInfo();
		TObexUsbProtocolInfoV2& usbV2TransportInfo = static_cast<TObexUsbProtocolInfoV2&>(aObexProtocolInfoPtr);
		static_cast<TObexUsbV2TransportInfo*>(transportInfo)->iInterfaceStringDescriptor = usbV2TransportInfo.iInterfaceStringDescriptor;	
		static_cast<TObexUsbV2TransportInfo*>(transportInfo)->iBandwidthPriority = usbV2TransportInfo.iBandwidthPriority;
		static_cast<TObexUsbV2TransportInfo*>(transportInfo)->iDmaOnOutEndpoint = usbV2TransportInfo.iDmaOnOutEndpoint;
		static_cast<TObexUsbV2TransportInfo*>(transportInfo)->iDmaOnInEndpoint = usbV2TransportInfo.iDmaOnInEndpoint;	
		}
		
	__ASSERT_DEBUG(transportInfo, Fault(EUtilNoTransportInfo));
	transportInfo->iTransportName = aObexProtocolInfoPtr.iTransport;
	transportInfo->iReceiveMtu = aObexProtocolPolicy.ReceiveMtu();
	transportInfo->iTransmitMtu = aObexProtocolPolicy.TransmitMtu();
	transportInfo->iVersion = KObexTransportInfoVersion0;
	return transportInfo;
	}

/**	
Converts aErr to an obex response.

@return aDefault if aErr == KErrNone, else the appriate obex response
*/	
TObexResponse IrOBEXUtil::ObexResponse (TInt aErr, TObexResponse aDefault)
	{ 
	switch (aErr)
		{
		case KErrNone:
			return (aDefault);
		case KErrNotFound:
		case KErrPathNotFound:
		case KErrUnknown:
		case KErrBadName:
			return (ERespNotFound);
		case KErrNotSupported:
			return (ERespNotImplemented);
		case KErrAlreadyExists:
		case KErrInUse:
			return (ERespConflict);
		case KErrAccessDenied:
			return (ERespUnauthorized);
		case KErrLocked:
			return (ERespForbidden);
		case KErrTimedOut:
			return (ERespTimedOut);
		default:
			{ 
			//check if it's one of the Symbian codes which has been set aside for 
			//mapping to an official OBEX code
			//unfortunately these are not continuous and exist in 4 discrete blocks
			
			if (( (aErr <= KErrIrObexRespSuccess ) && (aErr >= KErrIrObexRespPartialContent))
				||( (aErr <=KErrIrObexRespMultipleChoices) && (aErr >= KErrIrObexRespUseProxy))
				||( (aErr<=KErrIrObexRespBadRequest) && (aErr>=KErrIrObexRespHTTPVerNotSupp) )
				||( (aErr <= KErrIrObexRespDatabaseFull) && (aErr >= KErrIrObexRespDatabaseLocked)))
				{
				
				
				TInt err = aErr * -1;
	
				err += KErrIrObexRespBase;

				
					
				return ( static_cast<TObexResponse>(err) );

				}
			//in all other cases the error code isn't understood
			return (ERespInternalError);
			}
		}
	}

//
// End of file
