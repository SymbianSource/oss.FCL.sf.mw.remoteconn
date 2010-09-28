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
// This file defines the API for cptpipcontroller.dll
// 
//

/**
 @file
 @internalComponent
*/

#ifndef __CPTPIPCONTROLLER_H__
#define __CPTPIPCONTROLLER_H__

#include <e32base.h>	

#include <e32property.h>
#include "centralrepository.h"
#include "cptpipsockethandler.h"
#include <mtp/cptpiphostfilterinterface.h>
#include <mtp/rmtpclient.h>
#include "mptpipcontroller.h"
#include "cptpipinitcmdreq.h"
#include "cptpipinitcmdack.h"
#include "tptpipinitevtreq.h"
#include "tptpipinitfailed.h"
#include <mtp/cmtptypestring.h>
#include "cptpiptimer.h"


  enum TInitFailReason
        {
        /**
        Responder impliments a device bonding mechanism and initiator requesting 
        the connection is not one of the 'allowed' device. see annex 5.3 for more 
        detail on bonding mechanism.
        */
        EInitFailRejected,        
        /**
         Responder has too many active connections. The Initiator  may try to 
         establish a connection latter.
        */
        EInitFailBusy,
        /**
        Covers all other rejection cases.
        */
        EInitFailUnSpecified,

 		};
 		
enum TParameter
{
	/*
	This parameter defines the Device Friendly Name MTP device property 
	value. It is implemented as a string and has a maximum length of 254 
	characters. The parameter value specified in the initialisation file 
	may be overwritten in the course of MTP operational processing, if an 
	MTP SetDevicePropValue specifying the Device Friendly Name device 
	property is received.
	*/
	EDeviceFriendlyName  = 0x00000000
}; 		


class CPTPIPController : public CActive, public MPTPIPController
{
public:	 
	IMPORT_C static CPTPIPController* NewL();
	IMPORT_C static CPTPIPController* NewLC();
	IMPORT_C ~CPTPIPController();
			
	IMPORT_C void SocketAccepted(TRequestStatus& aStatus);		
	IMPORT_C void StopTransport();						
	IMPORT_C TInt SetDeviceGUID(TDesC8& aDeviceGUID);
	IMPORT_C void SetDeviceFriendlyName(TDesC16* aDeviceFriendlyName);
	IMPORT_C RSocket& NewSocketL();
			
	IMPORT_C void OnTimeOut();
										
private:
	TBool CompareHost(RSocket& aSocket);				
	TInt PublishSocketNamePair();		
	void CheckAndHandleErrorL(TInt aError);
	TInt EnableSocketTransfer();		
	TBool Validate();
	void Schedule();
			
	TInt ParseInitPacketL();
	TInt ParseEvtPacket();
			
	void RunL();
	void DoCancel();	    	    
	TInt RunError(TInt aErr);
			
	void CheckInitFailL(TInt aError);	    
	CPTPIPController();
	void ConstructL();
	void BuildInitAckL();
	void BuildInitFailL(TInitFailReason aReason);
	void StartTimer(TInt aSecond);
	TInt CheckMTPConnection();
	void Reset();
			
	 enum TPTPIPControllerStates
		{
		/*EIdle:The Controller just got constructed, and any time
		Listener will notify about new socket. */ 
		EIdle,
		
	  /*EInitCommandAwaited:When SocketHandler is waiting for 
	   InitCommandRequest to be sent over Command socket
	   No socket connection request is validated at this state*/
		EInitCommandAwaited,
	 
	 /*EInitCommandRead:When the InitCommandRequest is successfully
	  read from the socket
	  */
		EInitCommandRead,
		
		
		/*EFilterConsentAwaited:Controller is waiting for approval from Filter*/
		EFilterConsentAwaited,
		
		
		/*EWaitForInitFail:Filter has rejected the Connection request.Init Nack
		built,Wait for Init NAck to be written and failure to connect reported*/
		EWaitForInitFail,
		
		/*EWaitForInitCommandAck:Filter has accepted the conenction request.InitAck
		built,Wait for Init Ack to be written*/
		EWaitForInitCommandAck,       		                            
		
	 
	  /*EInitEvtAwaited:When Socket Handler is waiting for InitEventRequest
	   * No socket connection request is validated at this state*/
		EInitEvtAwaited,
		
	  
	 /*EInitEventRead:InitEventRequest read
	  *No socket connection request is validated at this state */           
		EInitEventRead,       		       	       	 
						
						 
	 /*EConnected:Socket connection requests are not to be validated in this 
	  state and the message Device Busy to be sent*/
		EPTPIPConnected           
		};
		   
private:
		
	CPTPIPSocketHandler* iCmdHandler,*iEvtHandler;
	CPTPIPHostFilterInterface* iFilter;
	TPTPIPControllerStates iCtrlState;
	RProperty iProperty,iConnectionState;		
	RMTPClient iMTP;
	TBool iIsConnectedToMTP;
	TMTPTypeUint128 iHostGUID;	
	TDesC* iHostFriendlyName;							
	TRequestStatus* iCallerStatus;		
	CPTPIPInitCmdRequest* iInitCmdReq;
	TPTPIPInitEvtRequest iInitEvtReq;
	TPTPIPInitFailed iInitFailed;
	CPTPIPInitCmdAck* iInitCmdAck;	
	CRepository* iRepository;							
	TMTPTypeUint128 iDeviceGUID;
	HBufC16* iDeviceFriendlyName;		
	RSocket iDummySocket;
	TInt iCounter;
	CPTPIPTimer* iTimer;
	TUid iTransportId;			    			
};


#endif  // __CPTPIPCONTROLLER_H__


