// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// cptpipsockethandler.cpp
// 
//

/**
 @internalComponent
*/

#include "mtpdebug.h"
#include "cptpipsockhandlerbase.h"
#include "cptpipconnection.h"
#include "ptpippanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpipsockhandlerbaseTraces.h"
#endif


const TUint32 KPTPIPHeaderSize = 8;
const TUint KMaxPacketSixe   = 16;
const TUint64 KMaxPTPIPPacketSize = 0x18FF4; // 100 KB - 12

//#define MTP_DEBUG_OST_HEX_DUMP 

#define UNUSED_VAR(a) (a)=(a)

/**
Constructor 
*/
CPTPIPSocketHandlerBase::CPTPIPSocketHandlerBase(CPTPIPConnection& aConnection, TPriority priority ) : 
						CActive(priority),
						iChunkStatus(0),
						iSendChunkData(NULL, 0),
						iSendData(NULL, 0),
						iReceiveChunkData(NULL, 0),
						iPTPPacketLength(0),
						iState(EIdle),
						iPacketBuffer(NULL),
						iPacketSizeMax(NULL),
						iReceiveData(NULL, 0),
						iIsFirstChunk(EFalse),
						iPTPPacketLengthReceived(0),
						iReceiveDataCommit(EFalse),
						iSendHeaderData(NULL, 0),
						iPTPIPDataHeader(NULL),
						iConnection(aConnection),
						iReceiveDataSink(NULL),
						iSendDataSource(NULL),
						iCurrentChunkData(NULL,0,0)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CPTPIPSOCKETHANDLERBASE_ENTRY );
	CActiveScheduler::Add(this);
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CPTPIPSOCKETHANDLERBASE_EXIT );
	}



/**
Second phase constructor.
*/
void CPTPIPSocketHandlerBase::ConstructL()
    {
    OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CONSTRUCTL_ENTRY );
    iPacketSizeMax = KMaxPacketSixe; 
    iPacketBuffer.ReAllocL(iPacketSizeMax);
    iPTPIPDataHeader = CPTPIPDataContainer::NewL();
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CONSTRUCTL_EXIT );
    }
  
 /**
 Destructor
 */  
 CPTPIPSocketHandlerBase::~CPTPIPSocketHandlerBase()
 {
    OstTraceFunctionEntry0( DUP1_CPTPIPSOCKETHANDLERBASE_CPTPIPSOCKETHANDLERBASE_ENTRY );
    Cancel();
    iPacketBuffer.Close();
    iSocket.Close();
    delete iPTPIPDataHeader;
    OstTraceFunctionExit0( DUP1_CPTPIPSOCKETHANDLERBASE_CPTPIPSOCKETHANDLERBASE_EXIT );
 }


//
// CActive Functions
//

/**
 This will be called after every chunk is sent or received over the socket. 
*/
void CPTPIPSocketHandlerBase::RunL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RUNL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_RUNL, "Current State is 0x%08X, and status is %d",
            iState, iStatus.Int());
    
    
	switch ( DataStreamDirection() )
		{
	case EReceivingState:
	
        OstTrace1( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_RUNL, "Receive data completion status = %d",
                iStatus.Int());
        
        if (iState == EReceiveFastInProgress)
        	{
        	InitiateFirstChunkReceiveL(); 
        	}
        
        else if (iStatus != KErrNone)
            {
            // Abnormal completion.
            OstTrace1( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_RUNL, "PTPIP Error: Receive data completed with error = %d",
                    iStatus.Int());
            
            SetState(EReceiveComplete);
            }
        else if (iState != EReceiveComplete)
            {
	        // Update the chunk data length. We use the iReceiveData variable with the socket 
	        // so only its length gets updated on receive. 
	        
	        // However during remaining processing we use iReceiveChunkData, so update its len.
            iReceiveChunkData.SetLength(iReceiveChunkData.Length() + iReceiveData.Length());
            if (iIsFirstChunk)
	            {
	            // process the first chunk.
	            ProcessFirstReceivedChunkL();  
	            }
	        else
		        {
		        ResumeReceiveDataStreamL(); 
		        }
            }
            
        if (iState == EReceiveComplete)
            {             
            // Save the pointer to the last populated location on the current chunk            
            iCurrentChunkData.Set(iReceiveChunkData);             
            // Reset the receive data stream and notify the connection.
            MMTPType& data(*iReceiveDataSink);
            ResetReceiveDataStream();
            TRAPD(err, ReceiveDataCompleteL(iStatus.Int(), data));
            if (err != KErrNone)
            	{
            	OstTrace1( TRACE_NORMAL, DUP3_CPTPIPSOCKETHANDLERBASE_RUNL, "Framework threw an error from ReceiveDataCompleteL = %d",
            	        err);
            	
            	ReceiveDataCompleteL(err, data);
            	}
            }
        break;
        
	case ESendingState:
		// Special case for handling the sending of init ack, handled in the derived class.
		if (HandleInitAck())
			{
			ResetSendDataStream();
			break;
			}

        OstTrace1( TRACE_NORMAL, DUP4_CPTPIPSOCKETHANDLERBASE_RUNL, "Send data stream completion status = %d",
                iStatus.Int());
        
        if (iStatus != KErrNone)
            {
            // Abnormal completion.
            SetState(ESendComplete);
            }
		else
			{
            ProcessSendDataL(); 
            }
         if (iState == ESendComplete)
            {
            // Reset the send data stream and notify the connection.
            const MMTPType& data(*iSendDataSource);
            ResetSendDataStream();
            TRAPD(err, SendDataCompleteL(iStatus.Int(), data));
            if (err != KErrNone)
            	{
            	OstTrace1( TRACE_NORMAL, DUP5_CPTPIPSOCKETHANDLERBASE_RUNL, "Framework threw an error from SendDataCompleteL = %d",
            	        err);
            	
            	SendDataCompleteL(err, data);
            	}
            }
        break;
        
	case ESendDataState:
		if (iStatus != KErrNone)
            {
            // Abnormal completion.
            SetState(ESendDataComplete);
            }
 		else if (iState == ESendingDataHeader)
			{
			SetState(ESendingDataPacket);	
			}
		else if (iState == ESendingDataPacket)
			{
			SetState(ESendingDataHeader);		
			}
		else if (iState == ESendDataPacketCompleting)
			{
			SetState(ESendDataComplete);	
			}
		
		if (iState == ESendDataComplete || iState == ESendDataCancelled)
            {
            // Reset the send data stream and notify the connection.
            const MMTPType& data(*iSendDataSource);
            ResetSendDataStream();
            SendDataCompleteL(iStatus.Int(), data);
            }
   		else
	        {
	        CreateAndSendDataPacketsL();	
	        }
		break;
		
	default:
		
		OstTrace1( TRACE_NORMAL, DUP6_CPTPIPSOCKETHANDLERBASE_RUNL, "PTPIP ERROR: Invalid state of the sockethandler: RunL should not be called with 0x%08X state",
		        iState);
		
        Panic(EPTPIPBadState);
		break;
		
		}// switch
    OstTrace1( TRACE_NORMAL, DUP7_CPTPIPSOCKETHANDLERBASE_RUNL, "IsActive = %d",
            IsActive());
    
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RUNL_EXIT );
	}

/**
Tell the Asynchronous Service provider to cancel all outstanding operations. 
*/
void CPTPIPSocketHandlerBase::DoCancel()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_DOCANCEL_ENTRY );
    switch (iState & EStateDirection)
        {
    case EReceivingState:
        OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_DOCANCEL, "Cancelling receive on the socket" );
        
        iSocket.CancelRecv();
        ResetReceiveDataStream();
        break;

    case ESendingState:    
        OstTrace0( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_DOCANCEL, "Cancelling send on the socket" );
        
        iSocket.CancelSend();
        ResetSendDataStream();
        break;
        
    case ESendDataState:
	    OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_DOCANCEL, "Cancelling send on the socket" );
	    
	    iSocket.CancelSend();
	    ResetSendDataStream();
	    break;
	    
    default:
        break;
		}
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_DOCANCEL_EXIT );
	}

/**
This is invoked when a panic occurs during RunL. 
To handle this , all operations are cancelled, and if we were processing
a send or recv operation, we complete it with the error code. 
@param - The error code with which RunL left
*/
TInt CPTPIPSocketHandlerBase::RunError(TInt aError)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RUNERROR_ENTRY );
    OstTraceExt3( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_RUNERROR, "Error reported is  %d and state is 0x%08X, and status is %d",
            aError, iState, iStatus.Int());
    
 
    // Cancel any outstanding request.
    Cancel();  
    
    // Notify the protocol layer of the error.
    TInt32 streamDirection = DataStreamDirection();
    if (streamDirection == EReceivingState)
	    {
		OstTrace0( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_RUNERROR, "Error in EReceivingState" );
		
        // Notify the connection and reset the receive data stream.
        MMTPType& data(*iReceiveDataSink);
        ResetReceiveDataStream();
        TRAPD(err, ReceiveDataCompleteL(aError, data));
        UNUSED_VAR(err);
	    }
	else if (streamDirection == ESendingState || (streamDirection == ESendDataState))
		{
		OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_RUNERROR, "Error in ESendingState" );
		
        // Notify the connection and reset the send data stream.
        const MMTPType& data(*iSendDataSource);
        ResetSendDataStream();
        TRAPD(err, SendDataCompleteL(aError, data));
        UNUSED_VAR(err);
		}
    // We are neither in sending nor receiving state, the error in RunL
    // must have originated from the MTP framework, tell the connection 
    // to stop.
	else 
		{
		Connection().HandleError(aError);
		}
 
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RUNERROR_EXIT );
	return KErrNone;
	}
	
//
// Send Data Functions
//

/**
This sends the data over the socket. The buffer is provided by the MTP fw.
@param aData - The data buffer to be sent
@param aTransactionId - The id of the current onging transaction. This is needed 
 in case the data in R to I phase has to be split into multiple PTPIP data packets
 In such a case the PTPIP Data packet header needs to be created which contains the tran id. 
*/
void CPTPIPSocketHandlerBase::SendDataL(const MMTPType& aData, TUint32 aTransactionId )
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SENDDATAL_ENTRY );
    iSendDataSource = &aData;
	TUint64 size = iSendDataSource->Size();
    OstTrace1( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_SENDDATAL, "Size of total data to be sent = %ld bytes",
            size);
    
	
	// if the data is less than KMaxPTPIPPacketSize then it can be sent in a shot, 
	// Currently the ptp ip packet has the end data packet already, so it can be sent directly. 
	if ( size < KMaxPTPIPPacketSize)
		{
		OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_SENDDATAL, "Size of data is less than KMaxPTPIPPacketSize, sending as one ptpip packet." );
		
	    SetState(ESendInitialising);
		ProcessSendDataL();
		}
		
	// if the data is greater than KMaxPTPIPPacketSize then it needs to be split into 
	// multiple packets. The PTPIP header will be locally created , and sent
	// and then one chunk will be sent as one PTPIP packet.
	else 
		{
		OstTrace0( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_SENDDATAL, "Size of data is more than KMaxPTPIPPacketSize, sending as multiple ptpip packets." );
		
		iPTPIPDataHeader->SetUint32L(CPTPIPDataContainer::ETransactionId, aTransactionId);
		iPTPIPDataHeader->SetPayloadL(NULL);
		
		// We can ignore this header, since we create our own while sending each packet.
		iChunkStatus = iSendDataSource->FirstReadChunk(iSendChunkData);
        
		SetState(ESendingDataHeader);
		CreateAndSendDataPacketsL();
		}
	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SENDDATAL_EXIT );
	}

/**
Using the same algo as the USB send, to buffer into a local buffer, and sending 
only when buffer exceeds the max socket send size, or all chunks have been buffered. 
*/
void CPTPIPSocketHandlerBase::ProcessSendDataL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL_ENTRY );
	
    iSendData.Set(KNullDesC8);

    TUint chunkAvailableLen(iSendChunkData.Length());
    if (!chunkAvailableLen)
        {    
        // Fetch the next read data chunk. 
        switch (iState)
            {
        case ESendInitialising:
            OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Fetching first read data chunk" );
            
            iChunkStatus = iSendDataSource->FirstReadChunk(iSendChunkData);
            iPacketBuffer.Zero();
            break;
            
        case ESendInProgress:
            OstTrace0( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Fetching next read data chunk" );
            
            iChunkStatus = iSendDataSource->NextReadChunk(iSendChunkData);
            break;
            
        case ESendCompleting:
            break;            
           
        case EIdle:
        default:
            OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Invalid send data stream state" );
            
            Panic(EPTPIPBadState);
            break;
            }
        
        // Fetch the new chunk data size available.
        chunkAvailableLen = iSendChunkData.Length();
        
        // Update data stream state.
        switch (iChunkStatus)
            {
        case KErrNone:
            SetState(ESendInProgress);
            break;
            
        case KMTPChunkSequenceCompletion:
            if (iState == ESendCompleting)
                {
                SetState(ESendComplete);
                }
            else
                {
                SetState(ESendCompleting);
                }
            break;
            
          case KErrNotFound:// CMTPTypeFile returns this when it has zero length
        	if( 0 == iSendChunkData.Length() )
        		{
                if (iState == ESendCompleting)
                {
                SetState(ESendComplete);
                }
            else
                {
                SetState(ESendCompleting);
                }
                }  
                break;
        default:
            LEAVEIFERROR(iChunkStatus, 
                                OstTrace1( TRACE_ERROR, DUP17_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "iChunkStatus is %d", iChunkStatus ));

            break;
            }          
        }
    OstTrace1( TRACE_NORMAL, DUP3_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Chunk status = %d",
            iChunkStatus);
    
    
    // Process the buffered residual and/or available chunk data.
    TUint bufferedLen(iPacketBuffer.Length());
    TUint chunkIntegralLen((chunkAvailableLen / iPacketSizeMax) * iPacketSizeMax);
    TUint chunkResidualLen(chunkAvailableLen % iPacketSizeMax);
    OstTrace1( TRACE_NORMAL, DUP4_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Buffered residual data = %d bytes",
            bufferedLen);
    OstTrace1( TRACE_NORMAL, DUP5_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Chunk data available = %d bytes",
            chunkAvailableLen);
    OstTrace1( TRACE_NORMAL, DUP6_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Chunk data packet integral portion = %d bytes",
            chunkIntegralLen);
    OstTrace1( TRACE_NORMAL, DUP7_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Chunk data packet residual portion = %d bytes",
            chunkResidualLen);
    
    if (bufferedLen)
        {
        // Data is buffered in the packet buffer. Fill the available packet buffer space.
        if (chunkAvailableLen)
            {
            // Fill the packet buffer.
            TUint consumedLen(0);
            TUint unconsumedLen(0);
            TUint capacity(iPacketBuffer.MaxLength() - iPacketBuffer.Length());
            if (chunkAvailableLen > capacity)
                {
                consumedLen     = capacity;
                unconsumedLen   = (chunkAvailableLen - consumedLen);              
                }
            else
                {
                consumedLen = chunkAvailableLen;
                }
            OstTrace1( TRACE_NORMAL, DUP8_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Buffering %d bytes",
                    consumedLen);
            
            iPacketBuffer.Append(iSendChunkData.Left(consumedLen));
            
            // Update the available chunk data to reflect only the unconsumed portion.
            OstTrace0( TRACE_NORMAL, DUP9_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "CPTPIPSocketHandlerBase::ProcessSendDataL" );
            
            if (unconsumedLen)
                {
                iSendChunkData.Set(iSendChunkData.Right(unconsumedLen));
                }
            else
                {
                iSendChunkData.Set(NULL, 0);                    
                }
            }
        
        // Send the packet buffer when full.
        if ((iState == ESendCompleting) || (iPacketBuffer.Size() == iPacketBuffer.MaxSize()))
            {
            iSendData.Set(iPacketBuffer);
            iPacketBuffer.Zero();
            }
       
        }
    else if (iState == ESendInProgress)
        {
        // Send the chunk data packet integral portion.
        if (chunkIntegralLen)
            {
            iSendData.Set(iSendChunkData.Left(chunkIntegralLen));   
            }
    
        // Buffer the chunk data packet residual portion.
        if (chunkResidualLen)
            {
            OstTrace1( TRACE_NORMAL, DUP10_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Buffering %d bytes",
                    chunkResidualLen);
            
            
            iPacketBuffer.Append(iSendChunkData.Right(chunkResidualLen));  
            }
            
        // All data has been consumed and/or buffered.
        iSendChunkData.Set(NULL, 0);
        }
    else if (iState == ESendCompleting)
        {
        // Send all available chunk data.
        iSendData.Set(iSendChunkData);
            
        // All data has been consumed.
        iSendChunkData.Set(NULL, 0);
        }

    // Send the available data or reschedule to process the next chunk.
    TUint sendBytes(iSendData.Length());
    if (sendBytes)
        {
        OstTrace1( TRACE_NORMAL, DUP16_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Send data length = %d bytes", 
                iSendData.Length());
        
        OstTrace1( TRACE_NORMAL, DUP11_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Send data length = %d bytes",
                iSendData.Length());
        
#ifdef MTP_DEBUG_OST_HEX_DUMP
        OstTrace0( TRACE_NORMAL, DUP12_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Sending data on socket " );
        OstTraceData( TRACE_DUMP, DUP13_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, 
                "%x", iSendData.Ptr(), iSendData.Size());
#endif

        iSocket.Send(iSendData, 0, iStatus);
        SetActive(); 
        OstTrace0( TRACE_NORMAL, DUP14_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "Request issued" );
        
        }
    else if (iState != ESendComplete)
        {    
        iStatus = KRequestPending;
		CompleteSelf(KErrNone);      
		}

    OstTrace1( TRACE_NORMAL, DUP15_CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL, "CSocketHandler state on exit = 0x%08X", iState );
    
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_PROCESSSENDDATAL_EXIT );
	}


	
/**
first the data header will be sent, 
next the packet will be sent. 

in case this is the last chunk, then the data header will have the last data 
*/
	
void CPTPIPSocketHandlerBase::CreateAndSendDataPacketsL()	
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL_ENTRY );
	
	// Create the data header and prepare to send it.
	if (iState == ESendingDataHeader)
		{
		// if we've received a Cancel, then don't send further packets and return to the connection. 
		if (iCancelReceived)
			{
			iState = ESendDataCancelled;
			CompleteSelf(KErrNone);
			OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Sending the PTPIP data " );
			
			OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL_EXIT );
			return;
			}
		else 
			{
			iChunkStatus = iSendDataSource->NextReadChunk(iSendChunkData);
			TInt32 size = iPTPIPDataHeader->Size() + iSendChunkData.Size();
			iPTPIPDataHeader->SetUint32L(CPTPIPDataContainer::EPacketLength, size);
	   		OstTrace1( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Size of ptpip packet data to be sent = %d bytes", size );
	   		
	
			switch (iChunkStatus)
		        {
		    case KErrNone:
		        iPTPIPDataHeader->SetUint32L(CPTPIPDataContainer::EPacketType, EPTPIPPacketTypeData);
		        break;
		        
		    case KMTPChunkSequenceCompletion:
		    	iPTPIPDataHeader->SetUint32L(CPTPIPDataContainer::EPacketType, EPTPIPPacketTypeEndData);
		        break;
		        
		    default:
		    	OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "PTPIP Error :chunkStatus returned an error" );
		    	
		        LEAVEIFERROR(iChunkStatus, 
		                            OstTrace1( TRACE_ERROR, CINTERNETCONNECTIONHANDLER_CONSTRUCTL, "iChunkStatus is %d", iChunkStatus ));

		        break;
		        } 
			// Set the iSendData to point to the ptpip header. 
			TInt ret = iPTPIPDataHeader->FirstReadChunk(iSendData);
			}
 		}
	
	// Set the iSendData to point to the actual data chunk. 
	else if (iState == ESendingDataPacket)
		{
		iSendData.Set(iSendChunkData);
    	OstTrace0( TRACE_NORMAL, DUP3_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Sending the PTPIP data " );
    	
		// if this is the last packet then set state. 	
		if (iChunkStatus == KMTPChunkSequenceCompletion)
			SetState(ESendDataPacketCompleting);
		}
	// We exited due to an error condition, 
	else 
		{
        Panic(EPTPIPBadState);
		}
		
    OstTrace1( TRACE_NORMAL, DUP4_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Send data length = %d bytes",
            iSendData.Length());
    
#ifdef MTP_DEBUG_OST_HEX_DUMP
        OstTrace0( TRACE_NORMAL, DUP5_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Sending data on socket " );
        OstTraceData( TRACE_DUMP, DUP6_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, 
                "%x", iSendData.Ptr(), iSendData.Size());
#endif

    iSocket.Send(iSendData, 0, iStatus);
    SetActive();
    OstTrace0( TRACE_NORMAL, DUP7_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL, "Request issued" );
    
	OstTraceFunctionExit0( DUP1_CPTPIPSOCKETHANDLERBASE_CREATEANDSENDDATAPACKETSL_EXIT );
	}
	
	
	
/**
Signals tthe data transfer controller that an asynchronous data send sequence 
has completed.
@leave One of the system wide error codes, if a processing failure occurs.
@panic EPTPIPNotSupported In debug builds only, if the derived class has not fully 
implemented the send data path.
*/
void CPTPIPSocketHandlerBase::SendDataCompleteL(TInt /*aError*/, const MMTPType& /*aSource*/)
    {
    OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SENDDATACOMPLETEL_ENTRY );
    __DEBUG_ONLY(Panic(EPTPIPNotSupported));
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SENDDATACOMPLETEL_EXIT );
    }	

/**
Forces the completion of a transfer in progress. 

@param aReason error code describing the reason for cancelling.
@leave Any of the system wide error codes.
*/
void CPTPIPSocketHandlerBase::CancelSendL(TInt aReason)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CANCELSENDL_ENTRY );
    
	if ( (DataStreamDirection() == ESendingState) || (DataStreamDirection() == ESendDataState))
		{
		OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_CANCELSENDL, "Cancel in ESendingState" );
		
		// Cancel any outstanding request.
		Cancel();
		ResetSendDataStream();
		SendDataCompleteL(aReason, *iSendDataSource);
		}
		
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CANCELSENDL_EXIT );
	}

	
//
// Receive Data Functions
//

/**
This receives the data over the socket. The data may be event or command data. 
The buffer is provided by the fw in case its a data in.
@param aSink The buffer into which the data from initiator will be received. 
*/
void CPTPIPSocketHandlerBase::ReceiveDataL(MMTPType& aSink)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RECEIVEDATAL_ENTRY );
	
	// Set the state 
	SetState(EReceiveInProgress);
	
	// The memory has come from the conection. In case it was a command receive
	// then the connection itself had put the memory for the payload 
	// from its members, 
	// in case it was a data receive , then the payload buffer was provided by
	// the framework. 
	
    iReceiveDataSink    = &aSink;
    iReceiveDataCommit  = iReceiveDataSink->CommitRequired();
    
    InitiateFirstChunkReceiveL(); 
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RECEIVEDATAL_EXIT );
	}

/**
Reads the first chunk. Then validates what was read and adjusts the buffer accordingly. 
*/
void CPTPIPSocketHandlerBase::InitiateFirstChunkReceiveL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL_ENTRY );
	
	// sink refers to the buffer from connection.
	// Now the ptr iReceiveChunkData is set to it first chunk.
	iChunkStatus = iReceiveDataSink->FirstWriteChunk(iReceiveChunkData);
	
	// The first chunk is going to be read.
	iIsFirstChunk = ETrue;
    OstTraceExt2( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL, "Receive chunk capacity = %d bytes, length = %d bytes",
            iReceiveChunkData.MaxLength(), iReceiveChunkData.Length());
    
    OstTrace1( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL, "Chunk status = %d",
            iChunkStatus);
    

	iReceiveData.Set(iReceiveChunkData);
	
	// start the timer. 
	
	// Make the async request to read on the socket and set ourselves active.
	// once data is read on the socket, the iStatus will be changed by the comms framework. 
 	iSocket.Recv(iReceiveData, 0, iStatus);
 	
#ifdef MTP_DEBUG_OST_HEX_DUMP
        OstTrace0( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL, "Received data on socket" );
        OstTraceData( TRACE_DUMP, DUP3_CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL, 
                "%x", iReceiveData.Ptr(), iReceiveData.Size());
#endif


    SetActive();
    OstTrace0( TRACE_NORMAL, DUP4_CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL, "Request issued" );
    
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_INITIATEFIRSTCHUNKRECEIVEL_EXIT );
	}

/**
The first chunk received will have the length of the data and the ptpip packet type.
Validate the ptp packet type, and set the correct packet type in the generic container.
Set the state to complete or read further depending on the size.
*/
void CPTPIPSocketHandlerBase::ProcessFirstReceivedChunkL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL_ENTRY );
    iIsFirstChunk = EFalse;
    // Reset the data counter, This will be filled in later in the ResumeReceiveDataStreamL.
    iPTPPacketLengthReceived = 0; 
    iType = EPTPIPPacketTypeUndefined;
    
    // Parse and get the packet type, and packet length into type and iPTPPacketLength.
    // This will also validate that the correct payload is set to get the remaining data. 
    iType = ParsePTPIPHeaderL();
	
	// If the PTPIP type is not one of the standard expected types, 
	// or not expected in the current transaction phase then we stop further receiving
	// This current RunL will 
	if (EPTPIPPacketTypeUndefined == iType ) 
		{
		OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL, "PTPIP ERROR: Unexpected value in the type field of PTPIP header, appears corrupt" );
		
		SetState(EReceiveComplete);
		iStatus = KErrCorrupt;
		}
	
	// If there is nothing to receive after the header, possible for probe packet.
	else if (KPTPIPHeaderSize == iPTPPacketLength )
		{
		SetState(EReceiveComplete);
		}
	   
    if (iState == EReceiveComplete)
		{     
#ifdef MTP_DEBUG_OST_HEX_DUMP 
        OstTrace0( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL, "Received data" );
        OstTraceData( TRACE_DUMP, DUP2_CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL, 
                "%x", iReceiveChunkData.Ptr(), iReceiveChunkData.Size());
#endif


		// Commit the received data if required.
        if (iReceiveDataCommit)
		    {
		 	OstTrace0( TRACE_NORMAL, DUP3_CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL, "Commiting write data chunk" );
		 	
	        iReceiveDataSink->CommitChunkL(iReceiveChunkData);       
		    }
		}
	else
		{
		ResumeReceiveDataStreamL();
		}

	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_PROCESSFIRSTRECEIVEDCHUNKL_EXIT );
	}



/**
Called after the first chunk has been received and we are expecting more packets. 
This will be called repeatedly until all the packets have been received.
*/
void CPTPIPSocketHandlerBase::ResumeReceiveDataStreamL()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML_ENTRY );
	TBool endStream(EFalse);
	MMTPType *needCommit = NULL;
		
    // Process the received chunk (if any).
   	iPTPPacketLengthReceived += iReceiveData.Length();
   	OstTraceExt2( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Data received = iPTPPacketLengthReceived = %d bytes, Data expected = iPTPPacketLength = %d",
   	     iPTPPacketLengthReceived, iPTPPacketLength);
   	
   	
	if (iPTPPacketLengthReceived == iPTPPacketLength)
	   	{
	   	SetState(EReceiveComplete);
	   	endStream = ETrue;
	   	
	   	//if type  is data then end stream is not true
	   	if (iType == EPTPIPPacketTypeData)
	   	endStream = EFalse;
	   	}

    OstTraceExt2( TRACE_NORMAL, DUP2_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Received = %d bytes, write data chunk capacity = %d bytes",
            iReceiveChunkData.Length(), iReceiveChunkData.MaxLength());
    
#ifdef MTP_DEBUG_OST_HEX_DUMP
        OstTrace0( TRACE_NORMAL, DUP3_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Received data " );
        OstTraceData( TRACE_DUMP, DUP4_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, 
                "%x", iReceiveChunkData.Ptr(), iReceiveChunkData.Size());
#endif



    // Commit the received data if required.
	if (iReceiveDataCommit)
         {
         // if the chunk has been read completely.
         // or if the end of stream has been reached. 
         if (iReceiveChunkData.Length() == iReceiveChunkData.MaxLength()
         	|| endStream )
        	 {
        	 needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData);
        	 }   
         
         
         } 

    // Fetch the next read data chunk.  
    switch (iState)
        {
    case EReceiveInProgress:
   	
	// If we're in the middle of receiving a ptp data packet, 
	// then at this stage we might have already used some of this chunk. 
	// so reset its length to previously saved value
    if (iUseOffset)
		
		{		
		iUseOffset = EFalse;	
		iReceiveChunkData.Set(iCurrentChunkData);
		iChunkStatus = KErrNone;		
		}
    
    	// get the next chunk only if this chunk has been read completely. 
    	if (iReceiveChunkData.Length() == iReceiveChunkData.MaxLength())
    		{
    		
    		iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData);
    		
    		}
    	OstTrace1( TRACE_NORMAL, DUP5_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "iReceiveChunkData pointer address is %08x",
    	        iReceiveChunkData.Ptr());
    	
	    break;
        
    case EReceiveComplete:
        OstTrace0( TRACE_NORMAL, DUP6_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Write data chunk sequence complet" );
        
        break;
                  
    case EIdle:
    default:
        OstTrace0( TRACE_NORMAL, DUP7_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Invalid stream state" );
        
        Panic(EPTPIPBadState);
        break;
        }
        
    OstTrace1( TRACE_NORMAL, DUP8_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Chunk status = %d",
            iChunkStatus);
    
        
        
    // If necessary, process the next chunk. 
    if (iState != EReceiveComplete)
        {
        OstTraceExt2( TRACE_NORMAL, DUP9_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Receive chunk capacity = %d bytes, length = %d bytes",
                iReceiveChunkData.MaxLength(), iReceiveChunkData.Length());
        
        OstTrace1( TRACE_NORMAL, DUP10_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "iReceiveChunkData pointer address is %08x",
                iReceiveChunkData.Ptr());
        
        
        
	        // When we reach the end of receiving a PTPIP packet, it is possible that our PTPIP chunk
	        // length is greater than the actual data that is expected. 
	        // eg when parameters 1to 5 are not set in a PTPIP request. In this case 
	        // we need to read the remaining length which will be less than the max length of the chunk.
	        TUint32 remainingLen = iPTPPacketLength - iPTPPacketLengthReceived;
	        if (remainingLen < (iReceiveChunkData.MaxLength() - iReceiveChunkData.Length()))
	        	{
	        	iReceiveData.Set(const_cast<TUint8*>(iReceiveChunkData.Ptr() + iReceiveChunkData.Length()), 
	        	                 0,
	        					remainingLen); 
	        	}
	        else
	        	{
	        	iReceiveData.Set(const_cast<TUint8*>(iReceiveChunkData.Ptr() + iReceiveChunkData.Length()), 
	        					0, 
	        					iReceiveChunkData.MaxLength() - iReceiveChunkData.Length());
	        	}        	
	       
        	
	        OstTrace1( TRACE_NORMAL, DUP11_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Length read this time is= %d", iReceiveData.MaxLength());
	        
        	
        
 		iSocket.Recv(iReceiveData, 0, iStatus);

#ifdef MTP_DEBUG_OST_HEX_DUMP 
        OstTrace0( TRACE_NORMAL, DUP12_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Received data on socket " );
        OstTraceData( TRACE_DUMP, DUP13_CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, 
                "%x", iReceiveData.Ptr(), iReceiveData.Size());
#endif

        SetActive();
        OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML, "Request issued" );
        
        }
        if(needCommit != NULL)
            {
            TPtr8 tmp(NULL, 0, 0);
            needCommit->CommitChunkL(tmp);
            }
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RESUMERECEIVEDATASTREAML_EXIT );
	}

/**
Signals the data transfer controller that an asynchronous data receive sequence 
has completed.
@leave One of the system wide error codes, if a processing failure occurs.
@panic EPTPIPNotSupported In debug builds only, if the derived class has not fully 
implemented the receive data path.
*/
void CPTPIPSocketHandlerBase::ReceiveDataCompleteL(TInt /*aError*/, MMTPType& /*aSink*/)
    {
    OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RECEIVEDATACOMPLETEL_ENTRY );
    __DEBUG_ONLY(Panic(EPTPIPNotSupported));
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RECEIVEDATACOMPLETEL_EXIT );
    }

/**
Forces the completion of a transfer in progress. 

@param aReason error code describing the reason for cancelling.
@leave Any of the system wide error codes.
*/
void CPTPIPSocketHandlerBase::CancelReceiveL(TInt aReason)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CANCELRECEIVEL_ENTRY );
     
    if (DataStreamDirection() == EReceivingState)
	    {
		OstTrace0( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_CANCELRECEIVEL, "Cancel in EReceivingState" );
		
	    // Cancel any outstanding request.
    	Cancel();  

        // Notify the connection and reset the receive data stream.
		ResetReceiveDataStream();
        ReceiveDataCompleteL(aReason, *iReceiveDataSink);
	    }
	    	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CANCELRECEIVEL_EXIT );
	}

//
// Getters , Setters and other helper functions
//

CPTPIPConnection& CPTPIPSocketHandlerBase::Connection()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_CONNECTION_ENTRY );
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_CONNECTION_EXIT );
	return iConnection;
	}

RSocket& CPTPIPSocketHandlerBase::Socket()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SOCKET_ENTRY );
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SOCKET_EXIT );
	return iSocket;	
	}


void CPTPIPSocketHandlerBase::ResetSendDataStream()
    {
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RESETSENDDATASTREAM_ENTRY );
    iSendChunkData.Set(NULL, 0);
    iSendData.Set(NULL, 0);
    iSendDataSource = NULL;
    iCancelReceived = EFalse;
    SetState(EIdle);
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RESETSENDDATASTREAM_EXIT );
    }
    
void CPTPIPSocketHandlerBase::ResetReceiveDataStream()
    {
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_RESETRECEIVEDATASTREAM_ENTRY );
    iReceiveChunkData.Set(NULL, 0, 0);
    iReceiveData.Set(NULL, 0, 0);
    iReceiveDataSink = NULL;
    iCancelReceived = EFalse;
    SetState(EIdle);
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_RESETRECEIVEDATASTREAM_EXIT );
    }

    
void CPTPIPSocketHandlerBase::SetState(TSocketState aState)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SETSTATE_ENTRY );
	iState = aState;
	OstTrace1( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_SETSTATE, "state = 0x%08X",
	        iState);
	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SETSTATE_EXIT );
	}
	
void CPTPIPSocketHandlerBase::CompleteSelf(TInt aCompletionCode)
     {
     OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_COMPLETESELF_ENTRY );
     SetActive();
     TRequestStatus* stat = &iStatus;
     User::RequestComplete(stat, aCompletionCode);
     OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_COMPLETESELF_EXIT );
     }	

/**
Provides the current data stream direction, sending or receiving
*/
TInt32 CPTPIPSocketHandlerBase::DataStreamDirection() const
    {
    OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_DATASTREAMDIRECTION_ENTRY );
    OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_DATASTREAMDIRECTION_EXIT );
    return (iState & EStateDirection);
    }
    
void CPTPIPSocketHandlerBase::SetSocket(RSocket& aSocket)
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SETSOCKET_ENTRY );
	iSocket = aSocket;	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SETSOCKET_EXIT );
	}

/**
Ignore any errors in setting the socket options
*/
void CPTPIPSocketHandlerBase::SetSocketOptions()
	{
	OstTraceFunctionEntry0( CPTPIPSOCKETHANDLERBASE_SETSOCKETOPTIONS_ENTRY );
	TInt error=iSocket.SetOpt(KSoTcpKeepAlive,KSolInetTcp,1);
	OstTrace1( TRACE_NORMAL, CPTPIPSOCKETHANDLERBASE_SETSOCKETOPTIONS, "setting the keep alive option returned = %d", error );
	
	error=iSocket.SetOpt(KSoTcpNoDelay,KSolInetTcp,1);
	OstTrace1( TRACE_NORMAL, DUP1_CPTPIPSOCKETHANDLERBASE_SETSOCKETOPTIONS, "setting the no delay to disable Nagle's algo returned %d", error );
	
	OstTraceFunctionExit0( CPTPIPSOCKETHANDLERBASE_SETSOCKETOPTIONS_EXIT );
	}
