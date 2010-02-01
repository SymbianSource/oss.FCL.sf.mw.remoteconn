// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Initial Contributors:
// Nokia Corporation - initial contribution.
// Contributors:
// Provides an interface to operate with PTP/IP Controller like., Notify the 
// PTP/IP Controller of incoming requests, Stop the existing transport, Set the 
// current Device GUID and Device Friendly Name,Get a new blank socket everytime 
// there is a connection request.
//



/**
 @publishedPartner
 @released
*/

#ifndef __MPTPIPCONTROLLER_H__
#define __MPTPIPCONTROLLER_H__

#include <e32cmn.h>
#include <es_sock.h>

class MPTPIPController
{
	
	public:
	
	
		/*
		Stops the existing PTPIP transport,In-turn closes the client-side session
		with MTP server,Before calling StopTransport(),any outstanding Listen call
		over socket,has to be cancelled.
		*/
		virtual void StopTransport()=0;

	   /*
	   	This method is called to notify the PTP/IP Controller that there is an incoming connection 
	   	request.It hands over the Command or Event Sockets to the appropriate Socket Handlers.
	   	There is a validation happening as to whether both of the above requests are coming
	   	from the same host; it validates if it is from the same host; otherwise rejects the 
	   	request and starts listening on the socket again.
     	@param TRequestStatus of the caller.
	   */
		virtual void SocketAccepted(TRequestStatus& aStatus)=0;

		/*
		Saves the current Device GUID(Globally Unique Identifier)
		@param GUID of the device which is obtained from the Device as a part of InitCommandAck Packet
		@return KErrNone if GUID is valid, otherwise KErrArgument
		*/
		virtual TInt SetDeviceGUID(TDesC8& aDeviceGUID)=0;

		/*
		Saves the current Device Friendly Name
		@param Friendly Name of the device which is obtained from the Device as a part of InitCommandAck Packet
		*/
		virtual void SetDeviceFriendlyName(TDesC16* aDeviceFriendlyName)=0;

		/*
		Issues a new blank socket.NewSocketL() has to be called when a socket is required to accept
		the incoming connection request.
		@return Reference to the socket the PTPIP/Controller owns.
		@leave when it cannot return any more sockets.It leaves when there are more than 3 connection requests
		per session
		*/
		virtual RSocket& NewSocketL()=0;

	
};

#endif
