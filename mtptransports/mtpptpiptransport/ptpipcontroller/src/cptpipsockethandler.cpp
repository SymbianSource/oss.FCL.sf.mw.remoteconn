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


#include "cptpipsockethandler.h"	//CPTPIPSockethandler



CPTPIPSocketHandler* CPTPIPSocketHandler::NewLC()
	{
	CPTPIPSocketHandler* self = new (ELeave) CPTPIPSocketHandler;
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/*
 Factory Method
*/
CPTPIPSocketHandler* CPTPIPSocketHandler::NewL()
	{
	CPTPIPSocketHandler* self = CPTPIPSocketHandler::NewLC();
	CleanupStack::Pop(self);
	return self;
	}


CPTPIPSocketHandler::CPTPIPSocketHandler():
		CActive(EPriorityStandard),iReceiveChunk(NULL,0),iWriteChunk(NULL,0)
	{
	CActiveScheduler::Add(this);
	iState=EReadState;
	}

void CPTPIPSocketHandler::ConstructL()
	{
	
			
	}
	
	
RSocket& CPTPIPSocketHandler::Socket()
	{			
	return	iSocket;	
	}
	
TSocketHandlerState& CPTPIPSocketHandler::State()
{
	return iState;
}
	
   
	
CPTPIPSocketHandler::~CPTPIPSocketHandler()
	{
	Socket().Close();
	}

/*
 Reads from the current accepted socket
 @param TRequestStatus passed from PTPIP Controller
 */
void CPTPIPSocketHandler::ReadFromSocket(MMTPType& aData,TRequestStatus& aCallerStatus)
	{
    iReadData = &aData;
	iChunkStatus = aData.FirstWriteChunk(iReceiveChunk);
	//ToDo check this works or not
	iCallerStatus=&aCallerStatus;
	  
	/* complete the reading of whole packet and then hit RunL
	*/          
	Socket().Recv(iReceiveChunk,0,iStatus);
	
	//start timer	
    SetActive();
	}

/*Writes to the current socket
@param TRequestStatus passed from PTPIP Controller
*/
void CPTPIPSocketHandler::WriteToSocket(MMTPType& aData,TRequestStatus& aCallerStatus)
	{
	iWriteData=&aData;
	iChunkStatus = aData.FirstReadChunk(iWriteChunk);

	iCallerStatus=&aCallerStatus;		
	Socket().Write(iWriteChunk,iStatus);	
	SetActive();	
    
	}


	
void CPTPIPSocketHandler::RunL()
	{
	TInt err = iStatus.Int();
	User::LeaveIfError(err);
	
	switch(iState)
	{
		case EReadState:
			if(iChunkStatus!=KMTPChunkSequenceCompletion)  
			{
        	if(iReadData->CommitRequired())
	        	{
	        	iReadData->CommitChunkL(iReceiveChunk);	
	        	}		
			iChunkStatus = iReadData->NextWriteChunk(iReceiveChunk);		
			Socket().Recv(iReceiveChunk,0,iStatus,iLen);	
	    	SetActive();	
			}
	
		else
			{

				   
			User ::RequestComplete(iCallerStatus,err);
			iState=EWriteState;	         	  		
			}
			break;
			
	case EWriteState:
								
			
			if(iChunkStatus!=KMTPChunkSequenceCompletion) 
			{       	        	        	        
        		iChunkStatus = iWriteData->NextReadChunk(iWriteChunk);
	        	Socket().Write(iWriteChunk,iStatus);	
		    	SetActive();
			}
		
			else
			{			
				   
			User ::RequestComplete(iCallerStatus,err);	  
			iState=EReadState;      	  							
			}
			break;
	}
}

TInt CPTPIPSocketHandler::RunError(TInt aErr)
	{
	User ::RequestComplete(iCallerStatus,aErr);
	return KErrNone;
	}



void CPTPIPSocketHandler::DoCancel()
	{
	if(iState==EReadState)	
	Socket().CancelRecv();
	else if(iState==EWriteState)
	Socket().CancelWrite();	
	}





