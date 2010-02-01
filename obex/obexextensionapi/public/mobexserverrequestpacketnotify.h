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

/**
 @file
 @publishedPartner
 @released
*/

#ifndef OBEXSERVERREQUESTPACKETNOTIFY_H
#define OBEXSERVERREQUESTPACKETNOTIFY_H

#include <e32std.h>
#include <obexbase.h>

class TObexConnectInfo;

typedef TUint8 TObexRequestCode;

const TObexRequestCode KObexRequestFinalConnect		= 0x80;
const TObexRequestCode KObexRequestFinalDisconnect	= 0x81;
const TObexRequestCode KObexRequestPut				= 0x02;
const TObexRequestCode KObexRequestFinalPut			= 0x82;
const TObexRequestCode KObexRequestGet				= 0x03;
const TObexRequestCode KObexRequestFinalGet			= 0x83;
const TObexRequestCode KObexFinalSetPath			= 0x85;
const TObexRequestCode KObexFinalAbort				= 0xFF;

/**
A Mixin to provide a notification interface for the receipt of request
packets by an OBEX server.

@publishedPartner
@released
*/
class MObexServerRequestPacketNotify
	{
public:
	
	/**
	Receipt of a generic request packet.
	
	@param aRequest 	The opcode of the request packet received.
	@param aResponse 	The implementer should set this to the response they
						want to send to the client if they are abandoning
						processing.
	@return EFalse to abandon processing the packet, ETrue to process as normal 
	*/
	virtual TBool RequestPacket(TObexRequestCode aRequest, TObexResponse& aResponse) = 0;
	
	/**
	Receipt of a Connect Request packet. Overload of the generic case.
	
	@param aRequest 	The opcode of the Connect Request packet received.
	@param aConnectInfo	The Connect Info of the Connect Request packet.
	@param aResponse 	The implementer should set this to the response they
						want to send to the client if they are abandoning
						processing.
	@return EFalse to abandon processing the packet, ETrue to process as normal 
	*/
	virtual TBool RequestPacket(TObexRequestCode aRequest, TObexConnectInfo& aConnectInfo, TObexResponse& aResponse) = 0;
	
	/**
	Receipt of a SetPath Request packet. Overload of the generic case.
	
	@param aRequest 	The opcode of the SetPath Request packet received.
	@param aSetPathInfo	The SetPath Info of the SetPath Request packet.
	@param aResponse 	The implementer should set this to the response they
						want to send to the client if they are abandoning
						processing.
	@return EFalse to abandon processing the packet, ETrue to process as normal 
	*/
	virtual TBool RequestPacket(TObexRequestCode aRequest, CObex::TSetPathInfo& aSetPathInfo, TObexResponse& aResponse) = 0;
	
	};

#endif // OBEXSERVERREQUESTPACKETNOTIFY_H

