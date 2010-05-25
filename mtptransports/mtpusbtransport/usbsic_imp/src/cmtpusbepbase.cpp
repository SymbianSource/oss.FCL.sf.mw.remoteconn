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
//

/**
 @file
 @internalComponent
*/

#include "cmtpusbconnection.h"
#include "cmtpusbepbase.h"
#include "mtpusbpanic.h"
#include "cmtpusbcontainer.h"
#include "mtpbuildoptions.hrh"
#include "mtpusbprotocolconstants.h"
#include <e32debug.h>

const TUint KUSBHeaderSize = 12;
#define UNUSED_VAR(a) (a)=(a)

/**
Destructor
*/
CMTPUsbEpBase::~CMTPUsbEpBase()
    {
    __FLOG(_L8("CMTPUsbEpBase::~CMTPUsbEpBase - Entry"));
    Cancel();
    iPacketBuffer.Close();
    __FLOG(_L8("CMTPUsbEpBase::~CMTPUsbEpBase - Exit"));
    __FLOG_CLOSE;
    }
    
/**
Provides the logical endpoint number of the endpoint.
@return The logical endpoint number. 
*/
TEndpointNumber CMTPUsbEpBase::EndpointNumber() const
    {
    return iConnection.EndpointNumber(iId);
    }
    
/**
Provides the internal endpoint identifier of the endpoint.
@return The internal endpoint identifier. 
*/
TUint CMTPUsbEpBase::Id() const
    {
    return iId;
    }

/**
Constructor
@param aId The internal endpoint identifier of the endpoint.
@param aPriority the priority of the active object assigned. 
@param aConnection MTP USB device class transport connection which controls 
the endpoint.
*/
CMTPUsbEpBase::CMTPUsbEpBase(TUint aId, TPriority aPriority, CMTPUsbConnection& aConnection) :
    CActive(aPriority),
    iId(aId),
    iReceiveChunkData(NULL, 0),
    iReceiveData(NULL, 0),
    iSendChunkData(NULL, 0),
    iSendData(NULL, 0),
    iIsFirstChunk(EFalse),
    iConnection(aConnection)
    {
    CActiveScheduler::Add(this);
    }
    
/**
Second phase constructor.
*/
#ifdef __FLOG_ACTIVE    
void CMTPUsbEpBase::ConstructL(const TDesC8& aComponentName)
#else
void CMTPUsbEpBase::ConstructL()
#endif
    {
    __FLOG_OPEN(KMTPSubsystem, aComponentName);
    __FLOG(_L8("CMTPUsbEpBase::ConstructL - Entry"));
    __FLOG(_L8("CMTPUsbEpBase::ConstructL - Exit"));
    }
    
/**
Sets the MaxPacketSize for the endpoint.
@param aSize The maximum packet size.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::SetMaxPacketSizeL(TUint aSize)
	{
	__FLOG(_L8("CMTPUsbEpBase::SetMaxPacketSizeL - Entry"));
	iPacketSizeMax = aSize;
	__FLOG_VA((_L8("Endpoint %d maximum packetsize = %u"), iId, iPacketSizeMax));
	// Allocate the packet buffer.
    iPacketBuffer.ReAllocL(iPacketSizeMax);
    __FLOG(_L8("CMTPUsbEpBase::SetMaxPacketSizeL - Exit"));
	}    

/**
Creates a stall condition on the endpoint.
*/  
void CMTPUsbEpBase::Stall()
    {
    __FLOG(_L8("CMTPUsbEpBase::Stall - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    Cancel();
    RDevUsbcClient& ldd(Connection().Ldd());
    const TEndpointNumber number(EndpointNumber());
    TEndpointState state;
    ldd.EndpointStatus(number, state);
    __FLOG_VA((_L8("EndpointStatus = %d"), state));
    if (state != EEndpointStateStalled)
        {
        __FLOG_VA((_L8("Halting endpoint = %d"), number));
        ldd.HaltEndpoint(number);
        }
    SetStreamState(EStalled);
    __FLOG(_L8("CMTPUsbEpBase::Stall - Exit"));
    }

/**
Clears a stall condition on the endpoint.
*/    
void CMTPUsbEpBase::StallClear()
    {
    __FLOG(_L8("CMTPUsbEpBase::StallClear - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    RDevUsbcClient& ldd(Connection().Ldd());
    const TEndpointNumber number(EndpointNumber());
    TEndpointState state;
    ldd.EndpointStatus(number, state);
    __FLOG_VA((_L8("EndpointStatus = %d"), state));
    if (state != EEndpointStateNotStalled)
        {
        __FLOG_VA((_L8("Clearing halt on endpoint = %d"), number));
        Connection().Ldd().ClearHaltEndpoint(number);
        }
    SetStreamState(EIdle);
    __FLOG(_L8("CMTPUsbEpBase::StallClear - Exit"));
    }

/**
Indicates whether the endpoint is currently in a stalled condition.
@return ETrue if the endpoint is in a stalled condition, otherwise EFalse.
*/
TBool CMTPUsbEpBase::Stalled() const
    {
    return (iState == EStalled);
    }

/**
Determines the relative order of the two endpoints based on their IDs. 
@return Zero, if the two objects are equal; a negative value, if the first 
endpoint's ID is less than the second, or; a positive value, if the first 
endpoint's ID is greater than the second.
*/    
TInt CMTPUsbEpBase::LinearOrder(const CMTPUsbEpBase& aL, const CMTPUsbEpBase& aR)
    {
    return (aL.iId - aR.iId);
    }
    
/**
Provides the MTP USB device class transport connection which controls the 
endpoint.
@return The MTP USB device class transport connection. 
*/
CMTPUsbConnection& CMTPUsbEpBase::Connection() const
    {
    return iConnection;
    }

/**
Forces the completion of a transfer in progress. This will
reset the data streams without having to stall the endpoints.

This is needed because Windows does not expect endpoints to 
stall on error conditions.

@param aReason error code describing the reason for cancelling.
@leave Any of the system wide error codes.
*/

void CMTPUsbEpBase::CancelReceiveL(TInt aReason)
	{
	__FLOG(_L8("CMTPUsbEpBase::CancelReceiveL - Entry"));
     
    if (DataStreamDirection() == EReceivingState)
	    {
		__FLOG(_L8("Cancel in EReceivingState"));            
	    // Cancel any outstanding request.
    	Cancel();  

        // Notify the connection and reset the receive data stream.
        ResetReceiveDataStream();
        ReceiveDataCompleteL(aReason, *iReceiveDataSink);
        // Flush incoming data, otherwise device and PC may get out of sync
        FlushRxDataL();
	    }
	    
    __FLOG(_L8("CMTPUsbEpBase::CancelReceiveL - Exit"));	
	}

/**
Forces the completion of a transfer in progress. This will
reset the data streams without having to stall the endpoints.

This is needed because Windows does not expect endpoints to 
stall on error conditions.

@param aReason error code describing the reason for cancelling.
@leave Any of the system wide error codes.
*/

void CMTPUsbEpBase::CancelSendL(TInt aReason)
	{
	__FLOG(_L8("CMTPUsbEpBase::CancelSendL - Entry"));
    
	if (DataStreamDirection() == ESendingState)
		{
 		__FLOG(_L8("Cancel in ESendingState"));
 		// Cancel any outstanding request.
    	Cancel();  
        // Notify the connection and reset the send data stream.
        ResetSendDataStream();
        SendDataCompleteL(aReason, *iSendDataSource);
		}
		
	__FLOG(_L8("CMTPUsbEpBase::CancelSendL - Exit"));
	}

/**
Initiates an asynchronous data receive sequence. 
@param aSink The receive data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::ReceiveDataL(MMTPType& aSink)
    {
    __FLOG(_L8("CMTPUsbEpBase::ReceiveDataL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    if(iState != EIdle)
      {
    	Cancel();
      }
    
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iReceiveDataSink    = &aSink;
    iReceiveDataCommit  = iReceiveDataSink->CommitRequired();
    SetStreamState(EReceiveInitialising);
    InitiateFirstChunkReceiveL(); 
        
    __FLOG_VA((_L8("CMTPUsbEpBase state on exit = 0x%08X"), iState));
    __FLOG(_L8("CMTPUsbEpBase::ReceiveDataL - Exit")); 
    }

/**
Resumes a halted data receive sequence.
@param aSink The receive data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/

void CMTPUsbEpBase::ResumeReceiveDataL(MMTPType& aSink)
	{
	__FLOG(_L8("CMTPUsbEpBase::ResumeReceiveDataL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iReceiveDataSink    = &aSink;
    iReceiveDataCommit  = iReceiveDataSink->CommitRequired();
    
    iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData);
    // The first chunk is going to be read.
    iReceiveData.Set(iReceiveChunkData);
    __FLOG_VA((_L8("Issuing ReadUntilShort request on endpoint %d"), EndpointNumber()));
    __FLOG_VA((_L8("Receive chunk capacity = %d bytes, length = %d bytes"), iReceiveChunkData.MaxLength(), iReceiveChunkData.Length()));
    __FLOG_VA((_L8("Chunk status = %d"), iChunkStatus)); 
    Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
    SetStreamState(EReceiveInProgress);
    SetActive();
    __FLOG(_L8("CMTPUsbEpBase::ResumeReceiveDataL - Exit")); 
	}

/**
Signals the data transfer controller that an asynchronous data receive 
sequence has completed.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPUsb 2 In debug builds only, if the derived class has not fully 
implemented the receive data path.
*/
void CMTPUsbEpBase::ReceiveDataCompleteL(TInt /*aError*/, MMTPType& /*aSink*/)
    {
    __FLOG(_L8("CMTPUsbEpBase::ReceiveDataCompleteL - Entry"));
    __DEBUG_ONLY(Panic(EMTPUsbNotSupported));
    __FLOG(_L8("CMTPUsbEpBase::ReceiveDataCompleteL - Exit"));
    }

/**
Initiates an asynchronous data send sequence. 
@param aSource The send data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::SendDataL(const MMTPType& aSource)
    {
    __FLOG(_L8("CMTPUsbEpBase::SendDataL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iSendDataSource = &aSource;
    SetStreamState(ESendInitialising);
    ProcessSendDataStreamL();
        
    __FLOG_VA((_L8("CMTPUsbEpBase state on exit = 0x%08X"), iState));
    __FLOG(_L8("CMTPUsbEpBase::SendDataL - Exit"));
    }

/**
Signals tthe data transfer controller that an asynchronous data send sequence 
has completed.
@leave One of the system wide error codes, if a processing failure occurs.
@panic MTPUsb 2 In debug builds only, if the derived class has not fully 
implemented the send data path.
*/
void CMTPUsbEpBase::SendDataCompleteL(TInt /*aError*/, const MMTPType& /*aSource*/)
    {
    __FLOG(_L8("CMTPUsbEpBase::SendDataCompleteL - Entry"));
    __DEBUG_ONLY(Panic(EMTPUsbNotSupported));
    __FLOG(_L8("CMTPUsbEpBase::SendDataCompleteL - Exit"));
    }

void CMTPUsbEpBase::DoCancel()
    {
    __FLOG(_L8("CMTPUsbEpBase::DoCancel - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    switch (iState & EStateDirection)
        {
    case EReceivingState:
        __FLOG_VA((_L8("Issuing ReadCancel on endpoint %d"), EndpointNumber()));
        Connection().Ldd().ReadCancel(EndpointNumber());
        ResetReceiveDataStream();
        break;

    case ESendingState:    
        __FLOG_VA((_L8("Issuing WriteCancel on endpoint %d"), EndpointNumber()));
        Connection().Ldd().WriteCancel(EndpointNumber());
        ResetSendDataStream();
        break;

    default:
        break;
        }
    __FLOG(_L8("CMTPUsbEpBase::DoCancel - Exit"));
    }
    
TInt CMTPUsbEpBase::RunError(TInt aError)
    {
    __FLOG(_L8("CMTPUsbEpBase::RunError - Entry"));
    __FLOG_VA((_L8("Error = %d"), aError));  
    
    // Cancel any outstanding request.
    Cancel();  
    
    // Notify the protocol layer of the error.
    TInt32 streamDirection = DataStreamDirection();
    if (streamDirection == EReceivingState)
	    {
		__FLOG(_L8("Error in EReceivingState"));            
        // Notify the connection and reset the receive data stream.
        MMTPType& data(*iReceiveDataSink);
        ResetReceiveDataStream();
        TRAPD(err, ReceiveDataCompleteL(aError, data));
        UNUSED_VAR(err);
	    }
	else if (streamDirection == ESendingState)
		{
		__FLOG(_L8("Error in ESendingState"));
        // Notify the connection and reset the send data stream.
        const MMTPType& data(*iSendDataSource);
        ResetSendDataStream();
        TRAPD(err, SendDataCompleteL(aError, data));
        UNUSED_VAR(err);
		}
	    
    __FLOG(_L8("CMTPUsbEpBase::RunError - Exit"));
    return KErrNone;
    }

void CMTPUsbEpBase::RunL()
    {
    __FLOG(_L8("CMTPUsbEpBase::RunL - Entry"));
    __FLOG_VA((_L8("Current endpoint is %d"), EndpointNumber()));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    
    switch (DataStreamDirection())
        {
    case EReceivingState:
        __FLOG_VA((_L8("Receive data completion status = %d"), iStatus.Int()));
        if (iStatus != KErrNone)
            {
            // Abnormal completion.
            SetStreamState(EReceiveComplete);
            }
        else
            {
            // Reissue the request if we got a null packet because the upper layers are not 
        	// interested in null packets.
        	if ((iReceiveData.Length() == 0) && (iReceiveData.MaxLength() > 0))
        		{
        		Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
        		SetActive();
        		return;
        		}  
            // Update the chunk data length.
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
            // Reset the receive data stream and notify the connection.
            MMTPType& data(*iReceiveDataSink);
            ResetReceiveDataStream();
            ReceiveDataCompleteL(iStatus.Int(), data);
            }
        break;
        
    case ESendingState:   
        __FLOG_VA((_L8("Send data stream completion status = %d"), iStatus.Int())); 
        if (iStatus != KErrNone)
            {
            // Abnormal completion.
            SetStreamState(ESendComplete);
            }
        else
            {
            ProcessSendDataStreamL(); 
            }
            
        if (iState == ESendComplete)
            {
            // Reset the send data stream and notify the connection.
            const MMTPType& data(*iSendDataSource);
            ResetSendDataStream();
            SendDataCompleteL(iStatus.Int(), data);
            }
        break;
        
    default:
        __FLOG_VA((_L8("Invalid data stream state, status = %d"), iStatus.Int())); 
        Panic(EMTPUsbBadState);
        break;
        }
        
    __FLOG_VA((_L8("IsActive = %d"), IsActive()));
    __FLOG(_L8("CMTPUsbEpBase::RunL - Exit"));
    }

/**
Provides the current data stream direction.
@return The current data stream direction (EIdle, EReceivingState, or 
ESendingState).
@see TState.
*/
TInt32 CMTPUsbEpBase::DataStreamDirection() const
    {
    return (iState & EStateDirection);
    }

/**
Resets the receive data stream by clearing all receive buffer pointers and
setting the stream state to EIdle.
*/    
void CMTPUsbEpBase::ResetReceiveDataStream()
    {
	__FLOG(_L8("CMTPUsbEpBase::ResetReceiveDataStream - Entry"));
    iReceiveChunkData.Set(NULL, 0, 0);
    iReceiveData.Set(NULL, 0, 0);
    iReceiveDataSink = NULL;
    SetStreamState(EIdle);
	__FLOG(_L8("CMTPUsbEpBase::ResetReceiveDataStream - Exit"));
    }

/**
Resets the receive data stream by clearing all receive buffer pointers and
setting the stream state to EIdle.
*/    
void CMTPUsbEpBase::ResetSendDataStream()
    {
	__FLOG(_L8("CMTPUsbEpBase::ResetSendDataStream - Entry"));
    iSendChunkData.Set(NULL, 0);
    iSendData.Set(NULL, 0);
    iSendDataSource = NULL;
    SetStreamState(EIdle);
	__FLOG(_L8("CMTPUsbEpBase::ResetSendDataStream - Exit"));
    }
    
/**
This method verify if the received first chunk data is a valid
USB header for BulkOut EP.
@pre this method should only be called after the USB header is received.
@return ETrue if the received first chunk data is a vaild USB header, otherwise EFalse.
*/
TBool CMTPUsbEpBase::ValidateUSBHeaderL()
	{	
	__FLOG(_L8("CMTPUsbEpBase::ValidateUSBHeader - Entry"));
	
	TBool result(EFalse);
	TUint16 containerType(Connection().BulkContainer().Uint16L(CMTPUsbContainer::EContainerType));
	iDataLength = Connection().BulkContainer().Uint32L(CMTPUsbContainer::EContainerLength);
	
#ifdef __FLOG_ACTIVE
    TUint32 transactionId(Connection().BulkContainer().Uint32L(CMTPUsbContainer::ETransactionID));
	TUint16 code(Connection().BulkContainer().Uint16L(CMTPUsbContainer::ECode));
    __FLOG_VA((_L8("ContainerLength = %lu, containerType = 0x%x, code = 0x%x, transactionID = 0x%x"), iDataLength, containerType, code, transactionId));
#endif
	
	//Due to an issue of Windows OS, the value of CMTPUsbContainer::EContainerLength is incorrect if the
	//object >= 4G-12. The value should be KMaxTUint32 in this kind of cases, but in current Windows
	//implementation it will be a value between 0 and 11.
	//Here we reset the iDateLength to the actual size of iReceiveDataSink as a walkaround.
	if(containerType == 2 && (iDataLength <= 11 || iDataLength == KMaxTUint32))
		{
		__FLOG(_L8("iDataLength <= 11, change to size of receive data sink"));			
		iDataLength = iReceiveDataSink->Size();
		}
	
	__FLOG_VA((_L8("containerType = %u , dataLength = %lu bytes"), containerType, iDataLength));
	
    if (iDataLength >= KUSBHeaderSize && 
        (containerType == EMTPUsbContainerTypeCommandBlock || containerType == EMTPUsbContainerTypeDataBlock))
        {	
        result = ETrue;
        iDataCounter = 0;
        }
	
	__FLOG_VA((_L8("CMTPUsbEpBase::ValidateUSBHeader - Exit with the result of %d"), result));
	return result;
	}

/**
Initiates the first chunk received data.
*/
void CMTPUsbEpBase::InitiateFirstChunkReceiveL()
	{
	__FLOG(_L8("CMTPUsbEpBase::InitiateFirstChunkReceiveL - Entry"));
	
	__FLOG(_L8("Fetching first write data chunk"));
    iChunkStatus = iReceiveDataSink->FirstWriteChunk(iReceiveChunkData);
    // The first chunk is going to be read.
    iIsFirstChunk = ETrue;
    iReceiveData.Set(iReceiveChunkData);
    __FLOG_VA((_L8("Issuing ReadUntilShort request on endpoint %d"), EndpointNumber()));
    __FLOG_VA((_L8("Receive chunk capacity = %d bytes, length = %d bytes"), iReceiveChunkData.MaxLength(), iReceiveChunkData.Length()));
    __FLOG_VA((_L8("Chunk status = %d"), iChunkStatus)); 
    Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
    SetStreamState(EReceiveInProgress);
    SetActive();
    
    __FLOG(_L8("Request issued"));
	__FLOG(_L8("CMTPUsbEpBase::InitiateFirstChunkReceiveL - Exit"));
	}
	
/**
Processes the first received chunk data.
*/
void CMTPUsbEpBase::ProcessFirstReceivedChunkL()
	{
	__FLOG(_L8("CMTPUsbEpBase::ProcessFirstReceivedChunkL - Entry"));	
	    
    // Reset it back.
    iIsFirstChunk = EFalse;
        
    if (iReceiveChunkData.MaxLength() == KUSBHeaderSize
      && iReceiveChunkData.Length() == KUSBHeaderSize)
	    {
	    // USB header received from BulkOut EP.
	    // USB Header validation
	    if (!ValidateUSBHeaderL())
		    {
            //trash data, continue to flush.
            TRequestStatus status;
            RBuf8 readBuf;
            readBuf.CreateL(KMaxPacketTypeBulkHS);
            Connection().Ldd().ReadPacket(status, EndpointNumber(), readBuf, KMaxPacketTypeBulkHS);
            User::WaitForRequest(status);    
            RDebug::Print(_L("CMTPUsbEpBase::ProcessFirstReceivedChunkL(), trash data length = %d"), readBuf.Length());
            readBuf.Close();
            
            InitiateFirstChunkReceiveL();  
            return;
            

			}
			
		if ((iDataLength - KUSBHeaderSize) == 0)
			{
			// only USB header.
			SetStreamState(EReceiveComplete);
			}	        
	    }
     else if (iReceiveChunkData.MaxLength() == iReceiveChunkData.Length())
		{
		// USB Control request setup or data packet is received from Control EP.		    		    		    
	    // All the desired data should be received. 
		SetStreamState(EReceiveComplete);			
		}
		
	__FLOG_VA((_L8("CMTPUsbEpBase state = 0x%08X"), iState));
	
	if (iState == EReceiveComplete)
		{
		// All data is received just using the first chunk. It could be a USB Command block without parameters
		// or USB control request setup or data. 
		__FLOG_VA((_L8("Received = %d bytes, write data chunk capacity = %d bytes"), iReceiveChunkData.Length(), iReceiveChunkData.MaxLength()));
		
#ifdef MTP_DEBUG_FLOG_HEX_DUMP 
        __FLOG_HEXDUMP((iReceiveChunkData, _L8("Received data ")));
#endif

		// Commit the received data if required.
        if (iReceiveDataCommit)
		    {
		 	__FLOG(_L8("Commiting write data chunk"));
	        iReceiveDataSink->CommitChunkL(iReceiveChunkData);       
		    }
		}
	// Receive more data.
	else
		{
		ResumeReceiveDataStreamL();
		}
	
	__FLOG(_L8("CMTPUsbEpBase::ProcessFirstReceivedChunkL - Exit"));
	}

/**
Implements the receive data streaming algorithm. It is called after the first chunk data is received provided
there is still more data to be received. 
*/
void CMTPUsbEpBase::ResumeReceiveDataStreamL()
    {
    __FLOG(_L8("CMTPUsbEpBase::ResumeReceiveDataStreamL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    TBool endStream(EFalse);
	TBool lastChunkCommited(EFalse);
	TBool nullPacketReceived(EFalse);
	MMTPType *needCommit = NULL;
    // Process the received chunk (if any).
   	iDataCounter += iReceiveData.Length();
   	__FLOG_VA((_L8("iDataLength = %lu bytes"), iDataLength));
   	__FLOG_VA((_L8("iDataCounter = %lu bytes"), iDataCounter));   	
   	
   	if (iDataCounter == iDataLength)
	   	{
	   	endStream = ETrue;
        nullPacketReceived = ((iState == EReceiveCompleting) && (iReceiveData.Length() == 0));
	   	}

    __FLOG_VA((_L8("Received = %d bytes, write data chunk capacity = %d bytes"), iReceiveChunkData.Length(), iReceiveChunkData.MaxLength()));		   
#ifdef MTP_DEBUG_FLOG_HEX_DUMP 
    __FLOG_HEXDUMP((iReceiveChunkData, _L8("Received data ")));
#endif
    __FLOG_VA((_L8("End of stream = %d"), endStream)); 		   
        
      // Commit the received data if required.
	if (iReceiveDataCommit)
         {
         if ((iChunkStatus != KMTPChunkSequenceCompletion)
       		&& !endStream
          	&& (iReceiveChunkData.Length() == iReceiveChunkData.MaxLength()))
	           {
	           	// Two cases are covered here:
	           	// 1. MTP file receiving: MTP type file never returns KMTPChunkSequenceCompletion,It can be received        
          		//    one part after another. Also it can be commited mutiple times.
            	// 2. Other MTP datatype receiving during the middle of data stream
	           __FLOG(_L8("Commiting write data chunk"));
	           needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData);
	           lastChunkCommited = ETrue;   
	           }
		else if ((iChunkStatus != KMTPChunkSequenceCompletion)
	        	&& endStream
	        	&& !nullPacketReceived)
		      {
		      // It should be the end of MTP type file receiving since it never returns KMTPChunkSequenceCompletion.
	 	      // it can be commited mutiple times.
		      __FLOG(_L8("Commiting write data chunk"));
		      needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData);
		      }
		else if ((iChunkStatus == KMTPChunkSequenceCompletion)
	        	&& endStream
	        	&& !nullPacketReceived)
		      {
		      // The last chunk data which type is any other MTP data type than MTP file type. 
		      // It will not be commited until all the chunk data is received.
		      __FLOG(_L8("Commiting write data chunk"));
		      needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData); 
		      }          
         }  

    // Fetch the next read data chunk.  
    switch (iState)
        {
    case EReceiveInProgress:
    	if (iReceiveDataCommit) // Commiting the received data is required 
		    {
		    if (lastChunkCommited)
			    {
			    __FLOG(_L8("Fetching next write data chunk"));
	    	 	iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData, iDataLength - KUSBHeaderSize);
			    }
		    }
		else
			{
			__FLOG(_L8("Fetching next write data chunk"));
    	 	iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData, iDataLength - KUSBHeaderSize);	
			}	         		    					      
        break;
        
    case EReceiveCompleting:
        __FLOG(_L8("Write data chunk sequence completing"));
        __FLOG_VA((_L8("Null packet received = %d"), nullPacketReceived)); 
        break;
                  
    case EIdle:
    default:
        __FLOG(_L8("Invalid receive data stream state"));
        Panic(EMTPUsbBadState);
        break;
        }
    __FLOG_VA((_L8("Chunk status = %d"), iChunkStatus)); 
        
    // Update the data stream state.
    switch (iChunkStatus)
        {
    case KErrNone:
        if (endStream)
            {
            __FLOG(_L8("Terminating packet received."));
            SetStreamState(EReceiveComplete);
            }
        else
            {
            // Full (intermediate) packet data received.
            SetStreamState(EReceiveInProgress);
            }
        break;
        
    case KMTPChunkSequenceCompletion:

        if (endStream)
            {
            __FLOG(_L8("Terminating packet received."));
            SetStreamState(EReceiveComplete);
            }
        else
            {           
            // Processing the last received data chunk.
            // It will be processed once or mutiple times. 
            SetStreamState(EReceiveCompleting);            
            }
        break;
        
    default:
        User::Leave(iChunkStatus);
        break;
        } 
        
    // If necessary, process the next chunk. 
    if (iState != EReceiveComplete)
        {
        __FLOG_VA((_L8("Issuing ReadUntilShort request on endpoint %d"), EndpointNumber()));
        __FLOG_VA((_L8("Receive chunk capacity = %d bytes, length = %d bytes"), iReceiveChunkData.MaxLength(), iReceiveChunkData.Length()));
        __FLOG_VA((_L8("iReceiveChunkData pointer address is %08x"), iReceiveChunkData.Ptr()));
        // TDesC8's Right() method is not used here, because the parameter passed in like iReceiveChunkData.MaxLength() - iReceiveChunkData.Length()is greater than 
        // the length of the descriptor, the function extracts the whole of the descriptor.
        if(iDataLength-iDataCounter < iReceiveChunkData.MaxLength() - iReceiveChunkData.Length())
			{
			iReceiveData.Set(const_cast<TUint8*>(iReceiveChunkData.Ptr() + iReceiveChunkData.Length()), 0, iDataLength - iDataCounter);
			}
		else
			{
			iReceiveData.Set(const_cast<TUint8*>(iReceiveChunkData.Ptr() + iReceiveChunkData.Length()), 0, iReceiveChunkData.MaxLength() - iReceiveChunkData.Length());
			}  
        Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
        SetActive();
        if(needCommit != NULL)
        	{
        	TPtr8 tmp(NULL, 0, 0);
        	needCommit->CommitChunkL(tmp);
        	}
        __FLOG(_L8("Request issued"));
        }
        
    __FLOG_VA((_L8("CMTPUsbEpBase state on exit = 0x%08X"), iState));
    __FLOG(_L8("CMTPUsbEpBase::ResumeReceiveDataStreamL - Exit"));
    }
    
/**
Implements the send data streaming algorithm. This algorithm regulates the 
sequence of data chunks making up the send data stream to ensure that data is
passed to the USB device interface in units of wMaxPacketSize integral length. 
The algorithm attempts to avoid re-buffering unless absolutely necessary, as 
follows:
    1.  If the data chunk size is greater than or equal to the endpoint's 
        wMaxPacketSize, then the maximum wMaxPacketSize integral data portion 
        is sent directly. Any residual data is buffered in a wMaxPacketSize 
        packet buffer.
    2.  If the data chunk size is less than the endpoint's wMaxPacketSize, then
        the data is buffered in the packet buffer. As soon as the packet buffer 
        is filled it is sent.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::ProcessSendDataStreamL()
    {
    __FLOG(_L8("CMTPUsbEpBase::ProcessSendDataStreamL - Entry"));
    __FLOG_VA((_L8("CMTPUsbEpBase state on entry = 0x%08X"), iState));
    
    // Clear the send data stream data pointer.
    iSendData.Set(KNullDesC8);
    
    TUint chunkAvailableLen(iSendChunkData.Length());
    if (!chunkAvailableLen)
        {    
        // Fetch the next read data chunk. 
        switch (iState)
            {
        case ESendInitialising:
            __FLOG(_L8("Fetching first read data chunk"));
            iChunkStatus = iSendDataSource->FirstReadChunk(iSendChunkData);
            iPacketBuffer.Zero();
            break;
            
        case ESendInProgress:
            __FLOG(_L8("Fetching next read data chunk"));
            iChunkStatus = iSendDataSource->NextReadChunk(iSendChunkData);
            break;
            
        case ESendCompleting:
            break;
            
        case EIdle:
        default:
            __FLOG(_L8("Invalid send data stream state"));
            Panic(EMTPUsbBadState);
            break;
            }
        
        // Fetch the new chunk data size available.
        chunkAvailableLen = iSendChunkData.Length();
        
        // Update data stream state.
        switch (iChunkStatus)
            {
        case KErrNone:
            SetStreamState(ESendInProgress);
            break;
            
        case KMTPChunkSequenceCompletion:
            if (iState == ESendCompleting)
                {
                SetStreamState(ESendComplete);
                }
            else
                {
                SetStreamState(ESendCompleting);
                }
            break;
            
        default:
            User::Leave(iChunkStatus);
            break;
            }          
        }
        
    __FLOG_VA((_L8("Chunk status = %d"), iChunkStatus));
    
    // Process the buffered residual and/or available chunk data.
    TUint bufferedLen(iPacketBuffer.Length());
    TUint chunkIntegralLen((chunkAvailableLen / iPacketSizeMax) * iPacketSizeMax);
    TUint chunkResidualLen(chunkAvailableLen % iPacketSizeMax);
    TBool zlp(EFalse);
    __FLOG_VA((_L8("Buffered residual data = %u bytes"), bufferedLen));
    __FLOG_VA((_L8("Chunk data available = %u bytes"), chunkAvailableLen));
    __FLOG_VA((_L8("Chunk data packet integral portion = %u bytes"), chunkIntegralLen));
    __FLOG_VA((_L8("Chunk data packet residual portion = %u bytes"), chunkResidualLen));
    
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
            __FLOG_VA((_L8("Buffering %u bytes"), consumedLen));
            iPacketBuffer.Append(iSendChunkData.Left(consumedLen));
            
            // Update the available chunk data to reflect only the unconsumed portion.
            __FLOG_VA((_L8("Residual chunk data = %u bytes"), unconsumedLen));
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
        
        // Set the end of stream flag.
        zlp = ((iState == ESendCompleting) && (iSendChunkData.Length() == 0));
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
            __FLOG_VA((_L8("Buffering %u bytes"), chunkResidualLen));
            iPacketBuffer.Append(iSendChunkData.Right(chunkResidualLen));  
            }
            
        // All data has been consumed and/or buffered.
        iSendChunkData.Set(NULL, 0);
        }
    else if (iState == ESendCompleting)
        {
        // Send all available chunk data.
        iSendData.Set(iSendChunkData);
        zlp = ETrue;
            
        // All data has been consumed.
        iSendChunkData.Set(NULL, 0);
        }

    // Send the available data or reschedule to process the next chunk.
    TUint sendBytes(iSendData.Length());
    if ( sendBytes||zlp )
        {
        __FLOG_VA((_L8("Issuing Write request on endpoint %d, Zlp = %d"), EndpointNumber(), zlp));
        __FLOG_VA((_L8("Send data length = %d bytes"), iSendData.Length()));
        Connection().Ldd().Write(iStatus, EndpointNumber(), iSendData, sendBytes, zlp);
        SetActive(); 
        __FLOG(_L8("Request issued"));
        }
    else if (iState != ESendComplete)
        {    
        iStatus = KRequestPending;
        TRequestStatus* status = &iStatus;
        SetActive();
        User::RequestComplete(status, KErrNone);
        }

    __FLOG_VA((_L8("CMTPUsbEpBase state on exit = 0x%08X"), iState));
    __FLOG(_L8("CMTPUsbEpBase::ProcessSendDataStreamL - Exit"));   
    }

/**
Sets the data stream state variable.
@param aState The new data stream state.
*/
void CMTPUsbEpBase::SetStreamState(TInt aState)
    {
    __FLOG(_L8("SetStreamState - Entry"));
    iState = aState;
    __FLOG_VA((_L8("Stream state set to 0x%08X"), iState));
    __FLOG(_L8("SetStreamState - Exit"));
    }

// Fix so that cancelling works.
/*
 * Flush USB driver received data
 * 
 */
const TInt KFlushBufferMaxLen = 50*1024; // 50K bytes
#define INTERVAL_FOR_READ_TRASH_DATA   (1000*50)  // 50 Miliseconds
#define INTERVAL_FOR_FLUSH_TRASH_DATA  (9*INTERVAL_FOR_READ_TRASH_DATA)  // 450 Miliseconds
// if there is no data read in flushRxData, wait for 1.5 second at most in case forever waiting
#define INTERVAL_FOR_FLUSH_TRASH_DATA_IF_NO_DATA_READ  (30*INTERVAL_FOR_READ_TRASH_DATA) //1.5 SECOND 

void CMTPUsbEpBase::FlushRxDataL()
    {
    __FLOG(_L8("FlushRxDataL - Entry"));    				  
    // create the read buff
    RBuf8 readBuf;
    readBuf.CreateL(KFlushBufferMaxLen);
		      
    TUint32 uRestTimeToWait = INTERVAL_FOR_FLUSH_TRASH_DATA_IF_NO_DATA_READ;
 			      
    do{
 			    	
      // get the data size in the receive buffer ready to read
      TInt nbytes = 0;
      TInt err = Connection().Ldd().QueryReceiveBuffer(EndpointNumber(), nbytes);

      __FLOG_VA((_L8("FlushRxDataL()--1---err is %d , nbytes is %d"), err, nbytes));	  
 					  
      // has data, read it
      if( (err == KErrNone) && (nbytes > 0) )
         {   
         // synchronously read the data
         TRequestStatus status;
         Connection().Ldd().ReadOneOrMore(status, EndpointNumber(), readBuf);
         User::WaitForRequest(status);
	 		 
         if(status.Int() != KErrNone)  break;

#ifdef __FLOG_ACTIVE
         TInt length =  readBuf.Length();
         __FLOG_VA((_L8("The length of trash data is %d"), length));
         
         __FLOG(_L8("Begining of trash data"));       
         for (int i=0; i<4&&(i*4+4)<=length; i++)
             {
             __FLOG_VA((_L8("0x%x 0x%x 0x%x 0x%x"), readBuf[i*4], readBuf[i*4+1], readBuf[i*4+2], readBuf[i*4+3]));            
             }
         
         __FLOG(_L8("Residual of trash data if any"));          
         TInt residualLength = length%512;
         for (int i=0; i<4&&(i*4+4)<=residualLength; i++)
             {
             TInt beginIndex = length - residualLength;
             __FLOG_VA((_L8("0x%x 0x%x 0x%x 0x%x"), readBuf[beginIndex + i*4], readBuf[beginIndex + i*4+1], readBuf[beginIndex + i*4+2], readBuf[beginIndex + i*4+3]));            
             }
#endif
         
         // whenever some data read, reset the rest wait time.
         uRestTimeToWait = INTERVAL_FOR_FLUSH_TRASH_DATA;
 			                          
         __FLOG(_L8("FlushRxDataL()---Reset the rest wait time"));	          
         }
       else 
         {	
         // wait for the data from the usb channel.
         User::After(INTERVAL_FOR_READ_TRASH_DATA);
         // reduce the rest time to wait 
         uRestTimeToWait -=  INTERVAL_FOR_READ_TRASH_DATA ;
         }	
 	    
      __FLOG_VA((_L8("FlushRxDataL()---uRestTimeToWait is %d"), uRestTimeToWait));
 			    	
    }while( uRestTimeToWait > 0);
			    	
    readBuf.Close();
    __FLOG(_L8("FlushRxDataL - Exit"));    
}
