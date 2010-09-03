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

#include "mtpdebug.h"

#include "cptpipsockethandler.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipsockethandlerTraces.h"
#endif
	//CPTPIPSockethandler



CPTPIPSocketHandler* CPTPIPSocketHandler::NewLC()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_NEWLC_ENTRY );
	CPTPIPSocketHandler* self = new (ELeave) CPTPIPSocketHandler;
	CleanupStack::PushL(self);
	self->ConstructL();
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_NEWLC_EXIT );
	return self;
	}

/*
 Factory Method
*/
CPTPIPSocketHandler* CPTPIPSocketHandler::NewL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_NEWL_ENTRY );
	CPTPIPSocketHandler* self = CPTPIPSocketHandler::NewLC();
	CleanupStack::Pop(self);
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_NEWL_EXIT );
	return self;
	}


CPTPIPSocketHandler::CPTPIPSocketHandler():
		CActive(EPriorityStandard),iReceiveChunk(NULL,0),iWriteChunk(NULL,0)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_CPTPIPSOCKETHANDLER_ENTRY );
	CActiveScheduler::Add(this);
	iState=EReadState;
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_CPTPIPSOCKETHANDLER_EXIT );
	}

void CPTPIPSocketHandler::ConstructL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_CONSTRUCTL_ENTRY );
	
			
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_CONSTRUCTL_EXIT );
	}
	
	
RSocket& CPTPIPSocketHandler::Socket()
	{			
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_SOCKET_ENTRY );
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_SOCKET_EXIT );
	return	iSocket;	
	}
	
TSocketHandlerState& CPTPIPSocketHandler::State()
{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_STATE_ENTRY );
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_STATE_EXIT );
	return iState;
}
	
   
	
CPTPIPSocketHandler::~CPTPIPSocketHandler()
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPSOCKETHANDLER_CPTPIPSOCKETHANDLER_ENTRY );
	Socket().Close();
	OstTraceFunctionExit0( DUP1_CPTPIPSOCKETHANDLER_CPTPIPSOCKETHANDLER_EXIT );
	}

/*
 Reads from the current accepted socket
 @param TRequestStatus passed from PTPIP Controller
 */
void CPTPIPSocketHandler::ReadFromSocket(MMTPType& aData,TRequestStatus& aCallerStatus)
	{
    OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_READFROMSOCKET_ENTRY );
    iReadData = &aData;
	iChunkStatus = aData.FirstWriteChunk(iReceiveChunk);
	//ToDo check this works or not
	iCallerStatus=&aCallerStatus;
	  
	/* complete the reading of whole packet and then hit RunL
	*/          
	Socket().Recv(iReceiveChunk,0,iStatus);
	
	//start timer	
    SetActive();
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_READFROMSOCKET_EXIT );
	}

/*Writes to the current socket
@param TRequestStatus passed from PTPIP Controller
*/
void CPTPIPSocketHandler::WriteToSocket(MMTPType& aData,TRequestStatus& aCallerStatus)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_WRITETOSOCKET_ENTRY );
	iWriteData=&aData;
	iChunkStatus = aData.FirstReadChunk(iWriteChunk);

	iCallerStatus=&aCallerStatus;		
	Socket().Write(iWriteChunk,iStatus);	
	SetActive();	
    
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_WRITETOSOCKET_EXIT );
	}


	
void CPTPIPSocketHandler::RunL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_RUNL_ENTRY );
	TInt err = iStatus.Int();
	LEAVEIFERROR( err, 
	        OstTrace1( TRACE_ERROR, CPTPIPSOCKETHANDLER_RUNL, "error code is %d", err ));
	
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
OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_RUNL_EXIT );
}

TInt CPTPIPSocketHandler::RunError(TInt aErr)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_RUNERROR_ENTRY );
	User ::RequestComplete(iCallerStatus,aErr);
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_RUNERROR_EXIT );
	return KErrNone;
	}



void CPTPIPSocketHandler::DoCancel()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLER_DOCANCEL_ENTRY );
	if(iState==EReadState)	
	Socket().CancelRecv();
	else if(iState==EWriteState)
	Socket().CancelWrite();	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLER_DOCANCEL_EXIT );
	}





