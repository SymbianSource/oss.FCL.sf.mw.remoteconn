// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cptpipcontroller.h"	// Cptpipcontroller	

#include <mtp/tmtptypeuint128.h>
#include "ptpipsocketpublish.h"
#include <in_sock.h>
#include "ptpipprotocolconstants.h"


 
_LIT_SECURITY_POLICY_PASS(KAllowReadAll);
_LIT_SECURITY_POLICY_C1(KProcPolicy,ECapability_None);
__FLOG_STMT(_LIT8(KComponent,"PTPIPController");)


#define PTPIP_INIT_COMMAND_REQUEST	1
#define PTPIP_INIT_COMMAND_ACK		2
#define PTPIP_INIT_EVENT_REQUEST	3
#define PTPIP_INIT_FAIL				5
#define PTPIP_FIXED_CONNECTION_ID	1
#define PTPIP_PRPTOCOL_VERSION		0x00000001


enum TPTPIPControllerPanicReasons
    {
    EPanicTransportNotStarted     = 0,
    };


EXPORT_C CPTPIPController* CPTPIPController::NewLC()
	{
	CPTPIPController* self = new (ELeave) CPTPIPController;
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/*
 Factory Method
*/
EXPORT_C CPTPIPController* CPTPIPController::NewL()
	{
	CPTPIPController* self = CPTPIPController::NewLC();
	CleanupStack::Pop(1);
	return self;
	}


CPTPIPController::CPTPIPController():
		CActive(EPriorityStandard),iDeviceGUID()
	{
	iCtrlState=EIdle;
	iTransportId=TUid::Uid(KMTPPTPIPTransportImplementationUid);
	iCounter=0;
	CActiveScheduler::Add(this);
	}

/*
Creates two PTPIP SocketHandlers and handles the sockets to 
PTPIP SocketHandler & loads the Filter plugin.
*/
void CPTPIPController::ConstructL()
	{
   	 __FLOG_OPEN(KMTPSubsystem, KComponent);	 
	 iCmdHandler = CPTPIPSocketHandler::NewL();
	 iEvtHandler = CPTPIPSocketHandler::NewL();
	 iFilter=CPTPIPHostFilterInterface::NewL();
	 iInitCmdAck   = CPTPIPInitCmdAck::NewL(); 
	 iTimer=CPTPIPTimer::NewL(*this);	 
	 	
	TParameter param=EDeviceFriendlyName;
	
	iIsConnectedToMTP = EFalse;
	iDeviceGUID.Set(0,0);
	const TUint32 KUidMTPRepositoryValue(0x10282FCC);
	const TUid KUidMTPRepository = {KUidMTPRepositoryValue};
	iRepository = CRepository::NewL(KUidMTPRepository);		
	iDeviceFriendlyName = HBufC16::NewL(100);
	TPtr16 name = iDeviceFriendlyName->Des();	
	TInt result=iRepository->Get(param,name);		
	}

/*
Destructor
*/
EXPORT_C CPTPIPController::~CPTPIPController()
	{
	delete iCmdHandler;
   	delete iEvtHandler;
   	delete iFilter; 
   	delete iTimer;
   	delete iDeviceFriendlyName;  
	delete iRepository;
	delete iInitCmdReq;
	delete iInitCmdAck;
   	iMTP.Close();
	iIsConnectedToMTP = EFalse;
	iProperty.Close();
	iConnectionState.Close();
   	__FLOG_CLOSE;
	}



EXPORT_C RSocket& CPTPIPController::NewSocketL()
	{
	iCounter++;
	if(iCounter==1)
	return iCmdHandler->Socket();
	else if(iCounter==2)
	return iEvtHandler->Socket();
	else
	return iDummySocket;
	//Issue :If Newsocket is called 4 time then we are going to give 
	 //the same socket that we have given him third time.
	
	}
TInt  CPTPIPController::CheckMTPConnection()
{
	TInt error = KErrNone;
	if(iIsConnectedToMTP == EFalse)
		{
		error = iMTP.Connect();	
		}	
	if(error ==KErrNone) 
		{
		iIsConnectedToMTP = ETrue;		
		error = iMTP.IsAvailable(iTransportId);
		}
    return error;
}
/*
Validates the socket connection based on the current value of iCtrlState
@return ETrue on succes EFalse on failure*/
TBool CPTPIPController::Validate()
	{
	if(iCtrlState==EIdle || iCtrlState==EInitEvtAwaited)
	return ETrue;
	else 
	return EFalse;
	}


/*Saves the CommandSocket and EventSocket;The respective SocketHandlers are given the sockets
and Calls itself on a CompleteSelf()
 @param aSocket Pointer to socket
 @param TRequestStatus of the caller i.e., CPTPIPAgent
*/ 
EXPORT_C void CPTPIPController::SocketAccepted(TRequestStatus& aStatus)
	{
   	iCallerStatus=&aStatus;
   	aStatus=KRequestPending;    	   		
	TBool result=Validate();
	if(result==EFalse)
		{
		User::RequestComplete(iCallerStatus,KErrServerBusy);
		return;
		}
	
    if(iCtrlState==EInitEvtAwaited)  
    	{ // we are not gracefully rejecting a new PTPIP connection from other  host
    	  // as of now just socket closed.
        if(CompareHost(iEvtHandler->Socket())==EFalse)
        	{       
        	User::RequestComplete(iCallerStatus,KErrServerBusy);	
        	return;
 			}
    	}
	if(iCtrlState==EPTPIPConnected)  
    	{ 
         /*if(CompareHost(iDummySocket->Socket())==EFalse)
          *	User::RequestComplete(status,KErrServerBusy);	
          *	Note : As of now this check is not required because 
          * call to Validate(0) will fail. 	
         */
 		}
	 			
	/*
	Check whether PTPIPController is in a state of accepting new socket connections
	*/
	if (iCtrlState == EIdle)
      	{      
      	iCtrlState = EInitCommandAwaited;
      	}       
    else if (iCtrlState == EInitEvtAwaited)
      	{        
       	iCtrlState = EInitEvtAwaited;
      	}
      	
      Schedule();        
	}
	
	
/*
Compares whether the second connection request is from the same remote host
@param returns ETrue if the second request comes from the same host
*/	
TBool CPTPIPController::CompareHost(RSocket& aSocket)
	{
     
	TInetAddr  thisaddr, newAddr;
     
    iCmdHandler->Socket().RemoteName(thisaddr);     
	aSocket.RemoteName(newAddr);	
    if(newAddr.Address() == thisaddr.Address())
    	{
		return ETrue;
    	}
    else
    	{
   	 	return EFalse;
    	}

	}		
			
/**
Schedules the next request phase or event.
*/
void CPTPIPController::Schedule()
	{	 
 	iStatus = KRequestPending; 
 	TRequestStatus* status(&iStatus);  
    SetActive();
    User::RequestComplete(status, KErrNone);	 
	}

/*
 Defines the two socket names to be published
 @return one of the system wide error codes on failure
*/
TInt  CPTPIPController::PublishSocketNamePair()
	{
	TName iCommandSocketSysName,iEventSocketSysName;			  
   	iCmdHandler->Socket().Name(iCommandSocketSysName);
   	iEvtHandler->Socket().Name(iEventSocketSysName);
	
	
	
	/******************Define command socket system name************************/		
	RProcess serverprocess;
	
	const TUid KPropertyUid= serverprocess.Identity();		
   
    TInt error=iProperty.Define(KPropertyUid,ECommandSocketName,RProperty::EText,KAllowReadAll,KAllowReadAll);
   
    error=iProperty.Attach(KPropertyUid,ECommandSocketName);
   
	error=RProperty::Set(KPropertyUid,ECommandSocketName,iCommandSocketSysName);    

	/*****************Define event socket system name***********************/		
	
	error=iProperty.Define(KPropertyUid,EEventSocketName,RProperty::EText,KAllowReadAll,KAllowReadAll);
	
	error=iProperty.Attach(KPropertyUid,EEventSocketName);
	
    error=RProperty::Set(KPropertyUid,EEventSocketName,iEventSocketSysName);  
	
	return error;
	}
	

/*Makes the sockets Transfer enabled
 @return one of the system wide error codes on failure
 */
TInt CPTPIPController::EnableSocketTransfer()
	{
	TInt err;
	err = iCmdHandler->Socket().SetOpt(KSOEnableTransfer, KSOLSocket,KProcPolicy().Package());	
	
	if(err != KErrNone) return err;
	
	err = iEvtHandler->Socket().SetOpt(KSOEnableTransfer, KSOLSocket,KProcPolicy().Package());
	
	 return err;
	}	

/*
Sets the obtained DeviceGUID as the current DeviceGUID
@param TDesC8& aDeviceGUID
@return TInt KErrArgument if DeviceGUID is invalid or KErrNone
*/
EXPORT_C TInt CPTPIPController::SetDeviceGUID(TDesC8& aDeviceGUID)
	{	     
    TInt size = aDeviceGUID.Size();
    if (size != 16) return KErrArgument;
	TMTPTypeUint128  guid(aDeviceGUID);	
	iDeviceGUID = guid;	
	return KErrNone;
	}

/*
Sets the obtained DeviceFriendlyName
@param TDesC16* aDeviceGUID
*/    
EXPORT_C void CPTPIPController::SetDeviceFriendlyName(TDesC16* aDeviceFreindlyName)
	{
	delete iDeviceFriendlyName;
	
	TRAPD(err, iDeviceFriendlyName=aDeviceFreindlyName->AllocL());
	
	if(err != KErrNone)
		{
		 __FLOG_VA((_L8("CPTPIPController::SetDeviceFriendlyName ERROR = %d\n"), err));	
		}
	
	}
	

void CPTPIPController::Reset()
	{
	iCmdHandler->Socket().Close();
	iEvtHandler->Socket().Close();
	if(iIsConnectedToMTP)
	{
	TInt stopStatus=iMTP.StopTransport(iTransportId);
	if (KErrNone != stopStatus)
	{
	 __FLOG_VA((_L8("CPTPIPController::Reset ERROR = %d\n"), stopStatus));	
	}	
		
	}
				
	iMTP.Close();
	iProperty.Close();
	iConnectionState.Close();
	iCounter=0;
	iIsConnectedToMTP = EFalse;	
	iCtrlState = EIdle;	
	iCmdHandler->State()=EReadState;
	iEvtHandler->State()=EReadState;
	}

EXPORT_C void CPTPIPController::StopTransport()
	{
	Reset();
	}



void CPTPIPController:: CheckAndHandleErrorL(TInt  aError)
	{		
	if(aError != KErrNone)
		{
		Reset();							
		__FLOG_VA((_L8("PTPIP Controller CheckAndHandleErrorL, Error = %d"), aError));	
		User::Leave(aError);
		}
	}
	
void CPTPIPController:: CheckInitFailL(TInt aError)	
	{
	
	TInitFailReason reason = EInitFailUnSpecified;
		
	// we send Init fail packet to Initiator on command channel
	// even after InitEvent is received.
	if(aError!=KErrNone && (iCtrlState==EInitCommandRead|| iCtrlState==EInitEventRead))
		{
		if(aError == KErrAccessDenied)
			{
			reason = EInitFailRejected;
			}
		BuildInitFailL(reason);  				 				 				 		
		iCtrlState= EWaitForInitFail;
		if(iCmdHandler->State()==EWriteState)
		iCmdHandler->WriteToSocket(iInitFailed,iStatus);		
		else
		iEvtHandler->WriteToSocket(iInitFailed,iStatus);	
		StartTimer(30);
		__FLOG_VA((_L8("PTPIP Controller Error, Error = %d"), aError));
		User::Leave(aError);					
		}	
	}	

/*
Cause a Time-Out event to occur
*/
EXPORT_C void CPTPIPController::OnTimeOut()
	{
	TRequestStatus* status(&iStatus);
	User::RequestComplete(status,KErrTimedOut);
	}
	
void CPTPIPController::StartTimer(TInt aSecond)	
	{	
		iTimer->IssueRequest(aSecond);
		iStatus = KRequestPending;	
		SetActive(); 	
	}
	
void CPTPIPController::RunL()
	{
	
	TInt StatusError=iStatus.Int();
		
	if(iCmdHandler->IsActive() || iEvtHandler->IsActive())	
	{		
	if(iCmdHandler->IsActive())
	iCmdHandler->Cancel();
	if(iEvtHandler->IsActive())
	iEvtHandler->Cancel();
	
	}
	else if(iTimer->IsActive())
	{	
	iTimer->Cancel();	
	}

    TPtrC8 hostGUID;
	TInt error;
 	 switch(iCtrlState)
  	{
  		case EIdle:
  			 break;

   		case EInitCommandAwaited :    		   			
   		   				
   				iInitCmdReq   = CPTPIPInitCmdRequest::NewL();  
   											
     			iCmdHandler->ReadFromSocket(*iInitCmdReq,iStatus);        			  							
 				iCtrlState=EInitCommandRead;
 												 										
				StartTimer(30);
  				break;
 
  		case EInitCommandRead:
  				CheckAndHandleErrorL(StatusError);
  				

  				error = ParseInitPacketL();  				
  				CheckInitFailL(error);
  				
  				// Need special error number for MTP not available 
  				// so that licensee can understand it.
  				if(iMTP.IsProcessRunning() != KErrNotFound )
	  				{
					error = CheckMTPConnection();
	                CheckInitFailL(error);  					
	  				}

  				//coverity[unchecked_value]
                iHostGUID.FirstReadChunk(hostGUID);               
                
                iFilter->Accept(*iHostFriendlyName,hostGUID,iStatus);               
                iCtrlState=EFilterConsentAwaited;                
                StartTimer(30);                
  				break;
 
 		 case EFilterConsentAwaited:  		  		 	
 		 			 	 		  		 	
                // Please wriet in function in m class that filter must send KErrAccessDenied
 		 		CheckInitFailL(StatusError );
 				//give error code for rejection.
 				BuildInitAckL();  				 							
 				iCtrlState = EWaitForInitCommandAck; 				
 				iCmdHandler->WriteToSocket(*iInitCmdAck,iStatus); 				 				
 				StartTimer(30); 				
				break;
				
 		case EWaitForInitFail : 
 		       // Do not call any other leaving function here 		                 
 		       // because RunError is going to ignore it.
 		       
 		       // Error code is wrong if this happens due to local error.
                CheckAndHandleErrorL(KErrAccessDenied);                                
                break;
				
 		case EWaitForInitCommandAck :  
                CheckAndHandleErrorL(StatusError);                
				iCtrlState = EInitEvtAwaited;				
  				User::RequestComplete(iCallerStatus,KErrNone); 
				break;
				   				  	
 
  		case EInitEvtAwaited:
  		   				 	  			  		   		   				 				  								
  				iEvtHandler->ReadFromSocket(iInitEvtReq,iStatus);
  				iCtrlState=EInitEventRead;  				
  				StartTimer(30);  			
  				break;
 
  		case EInitEventRead:    			 			
 					 				
               	CheckInitFailL(StatusError);
  				error = ParseEvtPacket();
  				CheckInitFailL(error);
  				               	
				error = EnableSocketTransfer();
				CheckInitFailL(error);
				
  				error = PublishSocketNamePair();
  				CheckInitFailL(error);
  				
  				error = CheckMTPConnection();                             
                CheckInitFailL(error); 
  				error = iMTP.StartTransport(iTransportId);
  	  			CheckAndHandleErrorL(error);
  	  				
  				/************Subscribe to the state of plugin********/  				
  				TInt connState;
  				error = iConnectionState.Attach(KMTPPublishConnStateCat,EMTPConnStateKey);
  				CheckAndHandleErrorL(error);
  				error = iConnectionState.Get(KMTPPublishConnStateCat,EMTPConnStateKey,connState);   				
  				CheckAndHandleErrorL(error);
               	iConnectionState.Subscribe(iStatus);                	            	              	               	
               	iCtrlState = EPTPIPConnected;               	
               	SetActive();               
               	break;
                
               
  		case EPTPIPConnected:  	
  			{
			error=iConnectionState.Get(connState);  				
			if((error!=KErrNone) || (connState==EDisconnectedFromHost))
  				{  					
  				Reset();						
				User::RequestComplete(iCallerStatus,KErrNone);				
  				}
  			else
	  			{
				iConnectionState.Subscribe(iStatus);
	 		    SetActive();
	  			}
			break;
  			}  				
  		default :
	  		{
	  		break;  			
	  		}

  	} 
 	
	}

/*
Over-ridden to return KErrNone by checking the state of PTPIP Controller
*/
TInt CPTPIPController::RunError(TInt aErr)	
	{			
   		if(iCtrlState != EWaitForInitFail)
   		{ 
   		User::RequestComplete(iCallerStatus,aErr);   			
		iCtrlState = EIdle;
		iCmdHandler->State()=EReadState;
		iEvtHandler->State()=EReadState;
		iCounter=0;		
		iMTP.Close();		
		iIsConnectedToMTP = EFalse; 			   	
   		}
   		//Return KErrNone back to RunL()
   		return KErrNone;
	}
	



void CPTPIPController::DoCancel()
	{

	}
	
TInt CPTPIPController::ParseInitPacketL()
{
		TUint32 length(iInitCmdReq->Uint32L(CPTPIPInitCmdRequest::ELength));
		TUint32 type(iInitCmdReq->Uint32L(CPTPIPInitCmdRequest::EPktType));
		if(type != PTPIP_INIT_COMMAND_REQUEST) 
			{
			return KErrBadHandle;	
			}
		
		iInitCmdReq->GetL(CPTPIPInitCmdRequest::EInitiatorGUID,iHostGUID);


		TDesC& name = iInitCmdReq->HostFriendlyName();
		iHostFriendlyName = &name;
		TUint32 version(iInitCmdReq->Uint32L(CPTPIPInitCmdRequest::EVersion));
		return KErrNone;
}

TInt CPTPIPController::ParseEvtPacket()
{
		TUint32 length(iInitEvtReq.Uint32(TPTPIPInitEvtRequest::ELength));
		TUint32 type(iInitEvtReq.Uint32(TPTPIPInitEvtRequest::EType));	
		if(type != PTPIP_INIT_EVENT_REQUEST) return KErrBadHandle;
		TUint32 conNumber(iInitEvtReq.Uint32(TPTPIPInitEvtRequest::EconNumber));	
		if(conNumber !=PTPIP_FIXED_CONNECTION_ID)
			{ 
			// We are supporting only one connection,So connection Id is fixed.
			return KErrBadHandle;
			}

		return KErrNone;
}
void CPTPIPController::BuildInitAckL()
{	
	iInitCmdAck->SetUint32L(CPTPIPInitCmdAck::EPktType,PTPIP_INIT_COMMAND_ACK);
	// We are supporting only one connection,So connection Id is fixed
	iInitCmdAck->SetUint32L(CPTPIPInitCmdAck::EConNumber,PTPIP_FIXED_CONNECTION_ID);

	iInitCmdAck->SetL(CPTPIPInitCmdAck::EResponderGUID,iDeviceGUID);

	iInitCmdAck->SetDeviceFriendlyName(*iDeviceFriendlyName);
	iInitCmdAck->SetUint32L(CPTPIPInitCmdAck::EVersion,PTPIP_PRPTOCOL_VERSION);
	TUint64 size =  iInitCmdAck->Size();
	iInitCmdAck->SetUint32L(CPTPIPInitCmdAck::ELength,(TUint32)size);	
}

void CPTPIPController::BuildInitFailL(TInitFailReason aReason)
{	
	iInitFailed.SetUint32(TPTPIPInitFailed::ELength,iInitFailed.Size());
	iInitFailed.SetUint32(TPTPIPInitFailed::EType,PTPIP_INIT_FAIL);
	iInitFailed.SetUint32(TPTPIPInitFailed::EReason,aReason);		
}

TBool E32Dll()
{
	return ETrue;
}



