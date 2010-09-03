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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbepbaseTraces.h"
#endif


const TUint KUSBHeaderSize = 12;
#define UNUSED_VAR(a) (a)=(a)

/**
Destructor
*/
CMTPUsbEpBase::~CMTPUsbEpBase()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_CMTPUSBEPBASE_DES_ENTRY );
    Cancel();
    iPacketBuffer.Close();
    OstTraceFunctionExit0( CMTPUSBEPBASE_CMTPUSBEPBASE_DES_EXIT );
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
void CMTPUsbEpBase::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPUSBEPBASE_CONSTRUCTL_EXIT );
    }
    
/**
Sets the MaxPacketSize for the endpoint.
@param aSize The maximum packet size.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::SetMaxPacketSizeL(TUint aSize)
	{
	OstTraceFunctionEntry0( CMTPUSBEPBASE_SETMAXPACKETSIZEL_ENTRY );
	iPacketSizeMax = aSize;
	OstTraceExt2( TRACE_NORMAL, CMTPUSBEPBASE_SETMAXPACKETSIZEL, "Endpoint %d maximum packetsize = %u", iId, iPacketSizeMax );
	// Allocate the packet buffer.
    iPacketBuffer.ReAllocL(iPacketSizeMax);
	OstTraceFunctionExit0( CMTPUSBEPBASE_SETMAXPACKETSIZEL_EXIT );
	}    

/**
Creates a stall condition on the endpoint.
*/  
void CMTPUsbEpBase::Stall()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_STALL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_STALL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    Cancel();
    RDevUsbcClient& ldd(Connection().Ldd());
    const TEndpointNumber number(EndpointNumber());
    TEndpointState state;
    ldd.EndpointStatus(number, state);
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_STALL, "EndpointStatus = %d", state );
    if (state != EEndpointStateStalled)
        {
        OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_STALL, "Halting endpoint = %d", number );
        ldd.HaltEndpoint(number);
        }
    SetStreamState(EStalled);
    OstTraceFunctionExit0( CMTPUSBEPBASE_STALL_EXIT );
    }

/**
Clears a stall condition on the endpoint.
*/    
void CMTPUsbEpBase::StallClear()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_STALLCLEAR_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_STALLCLEAR, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    RDevUsbcClient& ldd(Connection().Ldd());
    const TEndpointNumber number(EndpointNumber());
    TEndpointState state;
    ldd.EndpointStatus(number, state);
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_STALLCLEAR, "EndpointStatus = %d", state );
    if (state != EEndpointStateNotStalled)
        {
        OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_STALLCLEAR, "Clearing halt on endpoint = %d", number );
        Connection().Ldd().ClearHaltEndpoint(number);
        }
    SetStreamState(EIdle);
    OstTraceFunctionExit0( CMTPUSBEPBASE_STALLCLEAR_EXIT );
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
	OstTraceFunctionEntry0( CMTPUSBEPBASE_CANCELRECEIVEL_ENTRY );
     
    if (DataStreamDirection() == EReceivingState)
	    {
		OstTrace0( TRACE_NORMAL, CMTPUSBEPBASE_CANCELRECEIVEL, "Cancel in EReceivingState" );
	    // Cancel any outstanding request.
    	Cancel();  

        // Notify the connection and reset the receive data stream.
        ResetReceiveDataStream();
        ReceiveDataCompleteL(aReason, *iReceiveDataSink);
        // Flush incoming data, otherwise device and PC may get out of sync
        FlushRxDataL();
	    }
	    
	OstTraceFunctionExit0( CMTPUSBEPBASE_CANCELRECEIVEL_EXIT );
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
	OstTraceFunctionEntry0( CMTPUSBEPBASE_CANCELSENDL_ENTRY );
    
	if (DataStreamDirection() == ESendingState)
		{
 		OstTrace0( TRACE_NORMAL, CMTPUSBEPBASE_CANCELSENDL, "Cancel in ESendingState" );
 		// Cancel any outstanding request.
    	Cancel();  
        // Notify the connection and reset the send data stream.
        ResetSendDataStream();
        SendDataCompleteL(aReason, *iSendDataSource);
		}

	OstTraceFunctionExit0( CMTPUSBEPBASE_CANCELSENDL_EXIT );
	}

/**
Initiates an asynchronous data receive sequence. 
@param aSink The receive data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::ReceiveDataL(MMTPType& aSink)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_RECEIVEDATAL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_RECEIVEDATAL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    
    if(iState != EIdle)
      {
    	Cancel();
      }
    
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iReceiveDataSink    = &aSink;
    iReceiveDataCommit  = iReceiveDataSink->CommitRequired();
    SetStreamState(EReceiveInitialising);
    InitiateFirstChunkReceiveL(); 

    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_RECEIVEDATAL, "CMTPUsbEpBase state on exit = 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBEPBASE_RECEIVEDATAL_EXIT );
    }

/**
Resumes a halted data receive sequence.
@param aSink The receive data sink buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/

void CMTPUsbEpBase::ResumeReceiveDataL(MMTPType& aSink)
	{
	OstTraceFunctionEntry0( CMTPUSBEPBASE_RESUMERECEIVEDATAL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_RESUMERECEIVEDATAL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iReceiveDataSink    = &aSink;
    iReceiveDataCommit  = iReceiveDataSink->CommitRequired();
    
    iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData);
    // The first chunk is going to be read.
    iReceiveData.Set(iReceiveChunkData);
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_RESUMERECEIVEDATAL, "Issuing ReadUntilShort request on endpoint %d", EndpointNumber());
    OstTraceExt2( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_RESUMERECEIVEDATAL, 
            "Receive chunk capacity = %d bytes, length = %d bytes", iReceiveChunkData.MaxLength(), iReceiveChunkData.Length());
    OstTrace1( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_RESUMERECEIVEDATAL, "Chunk status = %d", iChunkStatus );
    Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
    SetStreamState(EReceiveInProgress);
    SetActive();
	OstTraceFunctionExit0( CMTPUSBEPBASE_RESUMERECEIVEDATAL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPBASE_RECEIVEDATACOMPLETEL_ENTRY );
    __DEBUG_ONLY(Panic(EMTPUsbNotSupported));
    OstTraceFunctionExit0( CMTPUSBEPBASE_RECEIVEDATACOMPLETEL_EXIT );
    }

/**
Initiates an asynchronous data send sequence. 
@param aSource The send data source buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbEpBase::SendDataL(const MMTPType& aSource)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_SENDDATAL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_SENDDATAL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    __ASSERT_DEBUG(iState == EIdle, Panic(EMTPUsbBadState));
    
    iSendDataSource = &aSource;
    SetStreamState(ESendInitialising);
    ProcessSendDataStreamL();

    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_SENDDATAL, "CMTPUsbEpBase state on exit = 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBEPBASE_SENDDATAL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPBASE_SENDDATACOMPLETEL_ENTRY );
    __DEBUG_ONLY(Panic(EMTPUsbNotSupported));
    OstTraceFunctionExit0( CMTPUSBEPBASE_SENDDATACOMPLETEL_EXIT );
    }

void CMTPUsbEpBase::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_DOCANCEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_DOCANCEL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    
    switch (iState & EStateDirection)
        {
    case EReceivingState:
        OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_DOCANCEL, "Issuing ReadCancel on endpoint %d", EndpointNumber() );
        Connection().Ldd().ReadCancel(EndpointNumber());
        ResetReceiveDataStream();
        break;

    case ESendingState:    
        OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_DOCANCEL, "Issuing WriteCancel on endpoint %d", EndpointNumber());
        Connection().Ldd().WriteCancel(EndpointNumber());
        ResetSendDataStream();
        break;

    default:
        break;
        }
    OstTraceFunctionExit0( CMTPUSBEPBASE_DOCANCEL_EXIT );
    }
    
TInt CMTPUsbEpBase::RunError(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, 
            CMTPUSBEPBASE_RUNERROR, "error code %d", aError);
    
    // Cancel any outstanding request.
    Cancel();  
    
    // Notify the protocol layer of the error.
    TInt32 streamDirection = DataStreamDirection();
    if (streamDirection == EReceivingState)
	    {
		OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_RUNERROR, "Error in EReceivingState" );
        // Notify the connection and reset the receive data stream.
        MMTPType& data(*iReceiveDataSink);
        ResetReceiveDataStream();
        TRAPD(err, ReceiveDataCompleteL(aError, data));
        UNUSED_VAR(err);
	    }
	else if (streamDirection == ESendingState)
		{
		OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_RUNERROR, "Error in ESendingState" );
        // Notify the connection and reset the send data stream.
        const MMTPType& data(*iSendDataSource);
        ResetSendDataStream();
        TRAPD(err, SendDataCompleteL(aError, data));
        UNUSED_VAR(err);
		}

    OstTraceFunctionExit0( CMTPUSBEPBASE_RUNERROR_EXIT );
    return KErrNone;
    }

void CMTPUsbEpBase::RunL()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_RUNL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_RUNL, "Current endpoint is %d", EndpointNumber());
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_RUNL, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    
    switch (DataStreamDirection())
        {
    case EReceivingState:
        OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_RUNL, "Receive data completion status = %d", iStatus.Int() );
        
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
        		OstTraceFunctionExit0( CMTPUSBEPBASE_RUNL_EXIT );
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
        OstTrace1( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_RUNL, "Send data stream completion status = %d", iStatus.Int() );
        
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
        OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP4_CMTPUSBEPBASE_RUNL, "Invalid data stream state, status = %d", iStatus.Int());
        Panic(EMTPUsbBadState);
        break;
        }

    OstTrace1( TRACE_NORMAL, DUP5_CMTPUSBEPBASE_RUNL, "IsActive = %d", IsActive() );
    OstTraceFunctionExit0( DUP1_CMTPUSBEPBASE_RUNL_EXIT );
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
	OstTraceFunctionEntry0( CMTPUSBEPBASE_RESETRECEIVEDATASTREAM_ENTRY );
    iReceiveChunkData.Set(NULL, 0, 0);
    iReceiveData.Set(NULL, 0, 0);
    iReceiveDataSink = NULL;
    SetStreamState(EIdle);
    OstTraceFunctionExit0( CMTPUSBEPBASE_RESETRECEIVEDATASTREAM_EXIT );
    }

/**
Resets the receive data stream by clearing all receive buffer pointers and
setting the stream state to EIdle.
*/    
void CMTPUsbEpBase::ResetSendDataStream()
    {
	OstTraceFunctionEntry0( CMTPUSBEPBASE_RESETSENDDATASTREAM_ENTRY );
    iSendChunkData.Set(NULL, 0);
    iSendData.Set(NULL, 0);
    iSendDataSource = NULL;
    SetStreamState(EIdle);
    OstTraceFunctionExit0( CMTPUSBEPBASE_RESETSENDDATASTREAM_EXIT );
    }
    
/**
This method verify if the received first chunk data is a valid
USB header for BulkOut EP.
@pre this method should only be called after the USB header is received.
@return ETrue if the received first chunk data is a vaild USB header, otherwise EFalse.
*/
TBool CMTPUsbEpBase::ValidateUSBHeaderL()
	{	
	OstTraceFunctionEntry0( CMTPUSBEPBASE_VALIDATEUSBHEADERL_ENTRY );
	
	TBool result(EFalse);
	TUint16 containerType(Connection().BulkContainer().Uint16L(CMTPUsbContainer::EContainerType));
	iDataLength = Connection().BulkContainer().Uint32L(CMTPUsbContainer::EContainerLength);
	
#ifdef OST_TRACE_COMPILER_IN_USE
    TUint32 transactionId(Connection().BulkContainer().Uint32L(CMTPUsbContainer::ETransactionID));
	TUint16 code(Connection().BulkContainer().Uint16L(CMTPUsbContainer::ECode));
    OstTraceExt4( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_VALIDATEUSBHEADERL, 
            "ContainerLength = %lu, containerType = 0x%x, code = 0x%x, transactionID = 0x%x", iDataLength, containerType, code, transactionId );
#endif
	
	//Due to an issue of Windows OS, the value of CMTPUsbContainer::EContainerLength is incorrect if the
	//object >= 4G-12. The value should be KMaxTUint32 in this kind of cases, but in current Windows
	//implementation it will be a value between 0 and 11.
	//Here we reset the iDateLength to the actual size of iReceiveDataSink as a walkaround.
	if(containerType == 2 && (iDataLength <= 11 || iDataLength == KMaxTUint32))
		{	
	    OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_VALIDATEUSBHEADERL, "iDataLength <= 11, change to size of receive data sink" );	
		iDataLength = iReceiveDataSink->Size();
		}
	
	OstTraceExt2( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_VALIDATEUSBHEADERL, 
	        "containerType = %u , dataLength = %lu bytes", (TUint)containerType, iDataLength );

    if (iDataLength >= KUSBHeaderSize && 
        (containerType == EMTPUsbContainerTypeCommandBlock || containerType == EMTPUsbContainerTypeDataBlock))
        {	
        result = ETrue;
        iDataCounter = 0;
        }
	
	OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_VALIDATEUSBHEADERL, "Exit with the result of %d", result );
	OstTraceFunctionExit0( CMTPUSBEPBASE_VALIDATEUSBHEADERL_EXIT );
	return result;
	}

/**
Initiates the first chunk received data.
*/
void CMTPUsbEpBase::InitiateFirstChunkReceiveL()
	{
	OstTraceFunctionEntry0( CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL_ENTRY );

	OstTrace0( TRACE_NORMAL, CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL, "Fetching first write data chunk" );
    iChunkStatus = iReceiveDataSink->FirstWriteChunk(iReceiveChunkData);
    // The first chunk is going to be read.
    iIsFirstChunk = ETrue;
    iReceiveData.Set(iReceiveChunkData);
    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL, 
            "Issuing ReadUntilShort request on endpoint %d", EndpointNumber());
    OstTraceExt2( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL, 
            "Receive chunk capacity = %d bytes, length = %d bytes", iReceiveChunkData.MaxLength(), iReceiveChunkData.Length());
    OstTrace1( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL, "Chunk status = %d", iChunkStatus);
    Connection().Ldd().ReadUntilShort(iStatus, EndpointNumber(), iReceiveData);
    SetStreamState(EReceiveInProgress);
    SetActive();

    OstTrace0( TRACE_NORMAL, DUP4_CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL, "Request issued" );
	OstTraceFunctionExit0( CMTPUSBEPBASE_INITIATEFIRSTCHUNKRECEIVEL_EXIT );
	}
	
/**
Processes the first received chunk data.
*/
void CMTPUsbEpBase::ProcessFirstReceivedChunkL()
	{
	OstTraceFunctionEntry0( CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL_ENTRY );
	    
    // Reset it back.
    iIsFirstChunk = EFalse;
        
    if (iReceiveChunkData.MaxLength() == KUSBHeaderSize
      && iReceiveChunkData.Length() == KUSBHeaderSize)
	    {
	    // USB header received from BulkOut EP.
	    // USB Header validation
	    if (!ValidateUSBHeaderL())
		    {
            //trash data, continue to flush by one packet.
            FlushOnePacketL();
            
            InitiateFirstChunkReceiveL();  
            OstTraceFunctionExit0( CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL_EXIT );
            return;
			}
			
		if ((iDataLength - KUSBHeaderSize) == 0)
			{
			// only USB header.
			SetStreamState(EReceiveComplete);
			}	        
	    }
    else if (iReceiveChunkData.MaxLength() == KUSBHeaderSize
            && iReceiveChunkData.Length() < KUSBHeaderSize)
        {
        //trash data received, just diacard it and initiate next receiving
        InitiateFirstChunkReceiveL();  
        OstTraceFunctionExit0( DUP1_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL_EXIT );
        return;
        }
    else if (iReceiveChunkData.MaxLength() == iReceiveChunkData.Length())
		{
		// USB Control request setup or data packet is received from Control EP.		    		    		    
	    // All the desired data should be received. 
		SetStreamState(EReceiveComplete);			
		}

	OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, "CMTPUsbEpBase state = 0x%08X", iState );
	
	
	if (iState == EReceiveComplete)
		{
		// All data is received just using the first chunk. It could be a USB Command block without parameters
		// or USB control request setup or data. 
		OstTraceExt2( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, 
		        "Received = %d bytes, write data chunk capacity = %d bytes", iReceiveChunkData.Length(), iReceiveChunkData.MaxLength() );
		
#ifdef MTP_DEBUG_OST_HEX_DUMP 
		OstTrace0( TRACE_DUMP, DUP4_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, "<recved_chunk>" );
        OstTraceData( TRACE_DUMP, DUP3_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, 
                "%x", iReceiveChunkData.Ptr(), iReceiveChunkData.Size());
        OstTrace0( TRACE_DUMP, DUP5_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, "</recved_chunk>" );
#endif

		// Commit the received data if required.
        if (iReceiveDataCommit)
		    {
		 	OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL, "Commiting write data chunk" );
	        iReceiveDataSink->CommitChunkL(iReceiveChunkData);       
		    }
		}
	// Receive more data.
	else
		{
		ResumeReceiveDataStreamL();
		}
	
	OstTraceFunctionExit0( DUP2_CMTPUSBEPBASE_PROCESSFIRSTRECEIVEDCHUNKL_EXIT );
	}

/**
Implements the receive data streaming algorithm. It is called after the first chunk data is received provided
there is still more data to be received. 
*/
void CMTPUsbEpBase::ResumeReceiveDataStreamL()
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    TBool endStream(EFalse);
	TBool lastChunkCommited(EFalse);
	TBool nullPacketReceived(EFalse);
	MMTPType *needCommit = NULL;
    // Process the received chunk (if any).
   	iDataCounter += iReceiveData.Length();
   	OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "iDataLength = %lu bytes", iDataLength );
   	OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "iDataCounter = %lu bytes", iDataCounter );
   	
   	if (iDataCounter == iDataLength)
	   	{
	   	endStream = ETrue;
        nullPacketReceived = ((iState == EReceiveCompleting) && (iReceiveData.Length() == 0));
	   	}

   	OstTraceExt2( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
   	        "Received = %d bytes, write data chunk capacity = %d bytes", iReceiveChunkData.Length(), iReceiveChunkData.MaxLength() );
	   
#ifdef MTP_DEBUG_OST_HEX_DUMP 
   	OstTrace0( TRACE_DUMP, DUP24_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "<recved_chunk>" );
    OstTraceData( TRACE_DUMP, DUP23_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
            "%x", iReceiveChunkData.Ptr(), iReceiveChunkData.Size());
    OstTrace0( TRACE_DUMP, DUP25_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "</recved_chunk>" );
#endif
    OstTrace1( TRACE_NORMAL, DUP4_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "End of stream = %d", endStream );
        
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
	           OstTrace0( TRACE_NORMAL, DUP5_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Commiting write data chunk - 1" );

	           needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData);
	           lastChunkCommited = ETrue;   
	           }
		else if ((iChunkStatus != KMTPChunkSequenceCompletion)
	        	&& endStream
	        	&& !nullPacketReceived)
		      {
		      // It should be the end of MTP type file receiving since it never returns KMTPChunkSequenceCompletion.
	 	      // it can be commited mutiple times.
		      OstTrace0( TRACE_NORMAL, DUP6_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Commiting write data chunk - 2" );
		      needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData);
		      }
		else if ((iChunkStatus == KMTPChunkSequenceCompletion)
	        	&& endStream
	        	&& !nullPacketReceived)
		      {
		      // The last chunk data which type is any other MTP data type than MTP file type. 
		      // It will not be commited until all the chunk data is received.
		      OstTrace0( TRACE_NORMAL, DUP7_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Commiting write data chunk - 3" );
		      needCommit = iReceiveDataSink->CommitChunkL(iReceiveChunkData); 
		      }
        else if ((iChunkStatus == KMTPChunkSequenceCompletion)
                && !endStream
                && (iReceiveChunkData.Length() == iReceiveChunkData.MaxLength()))
              {
              // The last chunk data is received and chunk has been filled up:
              // just flush one packet and set endStream true and commit received data.
              // This condition tries to make MTP more robust if DP forgets to handle data-out phase. 
              OstTrace0( TRACE_NORMAL, DUP8_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Commiting write data chunk - 4" );

              FlushOnePacketL();  
              endStream = ETrue;
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
			    OstTrace0( TRACE_NORMAL, DUP9_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Fetching next write data chunk" );
	    	 	iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData, iDataLength - KUSBHeaderSize);
			    }
		    }
		else
			{
			OstTrace0( TRACE_NORMAL, DUP10_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Fetching next write data chunk" );
    	 	iChunkStatus = iReceiveDataSink->NextWriteChunk(iReceiveChunkData, iDataLength - KUSBHeaderSize);	
			}	         		    					      
        break;
        
    case EReceiveCompleting:
        OstTrace0( TRACE_NORMAL, DUP11_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Write data chunk sequence completing" );
        OstTrace1( TRACE_NORMAL, DUP12_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Null packet received = %d", nullPacketReceived );
        break;
                  
    case EIdle:
    default:
        OstTraceDef0(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP13_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Invalid receive data stream state" );
        Panic(EMTPUsbBadState);
        break;
        }
    OstTrace1( TRACE_NORMAL, DUP14_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Chunk status = %d", iChunkStatus );

    // Update the data stream state.
    switch (iChunkStatus)
        {
    case KErrNone:
        if (endStream)
            {
            OstTrace0( TRACE_NORMAL, DUP15_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Terminating packet received." );
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
            OstTrace0( TRACE_NORMAL, DUP16_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Terminating packet received." );
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
        OstTrace1( TRACE_ERROR, DUP22_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Invalid iChunkStatus %d", iChunkStatus);
        User::Leave(iChunkStatus);
        break;
        } 
        
    // If necessary, process the next chunk. 
    if (iState != EReceiveComplete)
        {
        OstTrace1( TRACE_NORMAL, DUP17_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
                "Issuing ReadUntilShort request on endpoint %d", EndpointNumber());
        OstTraceExt2( TRACE_NORMAL, DUP18_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
                "Receive chunk capacity = %d bytes, length = %d bytes", iReceiveChunkData.MaxLength(), iReceiveChunkData.Length() );
        OstTrace1( TRACE_NORMAL, DUP19_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
                "iReceiveChunkData pointer address is %08x", iReceiveChunkData.Ptr());

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
        OstTrace0( TRACE_NORMAL, DUP20_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, "Request issued" );
        }

    OstTrace1( TRACE_NORMAL, DUP21_CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML, 
            "CMTPUsbEpBase state on exit = 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBEPBASE_RESUMERECEIVEDATASTREAML_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPBASE_PROCESSSENDDATASTREAML_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "CMTPUsbEpBase state on entry = 0x%08X", iState );
    
    
    // Clear the send data stream data pointer.
    iSendData.Set(KNullDesC8);
    
    TUint chunkAvailableLen(iSendChunkData.Length());
    if (!chunkAvailableLen)
        {    
        // Fetch the next read data chunk. 
        switch (iState)
            {
        case ESendInitialising:
            OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Fetching first read data chunk" );
            iChunkStatus = iSendDataSource->FirstReadChunk(iSendChunkData);
            iPacketBuffer.Zero();
            break;
            
        case ESendInProgress:
            OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Fetching next read data chunk" );
            iChunkStatus = iSendDataSource->NextReadChunk(iSendChunkData);
            break;
            
        case ESendCompleting:
            break;
            
        case EIdle:
        default:
            OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP3_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
                    "Invalid send data stream state. iState %d", iState );
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
            OstTrace1( TRACE_ERROR, DUP16_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Invalid iChunkStatus %d", iChunkStatus);
            User::Leave(iChunkStatus);
            break;
            }          
        }
        
    OstTrace1( TRACE_NORMAL, DUP4_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Chunk status = %d", iChunkStatus );

    
    // Process the buffered residual and/or available chunk data.
    TUint bufferedLen(iPacketBuffer.Length());
    TUint chunkIntegralLen((chunkAvailableLen / iPacketSizeMax) * iPacketSizeMax);
    TUint chunkResidualLen(chunkAvailableLen % iPacketSizeMax);
    TBool zlp(EFalse);
    OstTrace1( TRACE_NORMAL, DUP5_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
            "Buffered residual data = %u bytes", bufferedLen );
    OstTrace1( TRACE_NORMAL, DUP6_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
            "Chunk data available = %u bytes", chunkAvailableLen );
    OstTrace1( TRACE_NORMAL, DUP7_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
            "Chunk data packet integral portion = %u bytes", chunkIntegralLen );
    OstTrace1( TRACE_NORMAL, DUP8_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
            "Chunk data packet residual portion = %u bytes", chunkResidualLen );
    
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
            OstTrace1( TRACE_NORMAL, DUP9_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Buffering %u bytes", consumedLen );
            iPacketBuffer.Append(iSendChunkData.Left(consumedLen));
            
            // Update the available chunk data to reflect only the unconsumed portion.
            OstTrace1( TRACE_NORMAL, DUP10_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
                    "Residual chunk data = %u bytes", unconsumedLen );
            
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
            OstTrace1( TRACE_NORMAL, DUP11_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Buffering %u bytes", chunkResidualLen );
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
        OstTraceExt2( TRACE_NORMAL, DUP12_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
                "Issuing Write request on endpoint %d, Zlp = %d", EndpointNumber(), zlp );
        OstTrace1( TRACE_NORMAL, DUP13_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
                "Send data length = %d bytes", iSendData.Length() );
        Connection().Ldd().Write(iStatus, EndpointNumber(), iSendData, sendBytes, zlp);
        SetActive(); 
        OstTrace0( TRACE_NORMAL, DUP14_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, "Request issued" );
        }
    else if (iState != ESendComplete)
        {    
        iStatus = KRequestPending;
        TRequestStatus* status = &iStatus;
        SetActive();
        User::RequestComplete(status, KErrNone);
        }

    OstTrace1( TRACE_NORMAL, DUP15_CMTPUSBEPBASE_PROCESSSENDDATASTREAML, 
            "CMTPUsbEpBase state on exit = 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBEPBASE_PROCESSSENDDATASTREAML_EXIT );
    }

/**
Sets the data stream state variable.
@param aState The new data stream state.
*/
void CMTPUsbEpBase::SetStreamState(TInt aState)
    {
    OstTraceFunctionEntry0( CMTPUSBEPBASE_SETSTREAMSTATE_ENTRY );
    iState = aState;
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_SETSTREAMSTATE, "Stream state set to 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBEPBASE_SETSTREAMSTATE_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBEPBASE_FLUSHRXDATAL_ENTRY );			  
    // create the read buff
    RBuf8 readBuf;
    readBuf.CreateL(KFlushBufferMaxLen);
		      
    TUint32 uRestTimeToWait = INTERVAL_FOR_FLUSH_TRASH_DATA_IF_NO_DATA_READ;
 			      
    do{
 			    	
      // get the data size in the receive buffer ready to read
      TInt nbytes = 0;
      TInt err = Connection().Ldd().QueryReceiveBuffer(EndpointNumber(), nbytes);
  
      OstTraceExt2( TRACE_NORMAL, CMTPUSBEPBASE_FLUSHRXDATAL, 
              "FlushRxDataL()--1---err is %d , nbytes is %d", err, nbytes );
  
      // has data, read it
      if( (err == KErrNone) && (nbytes > 0) )
         {   
         // synchronously read the data
         TRequestStatus status;
         Connection().Ldd().ReadOneOrMore(status, EndpointNumber(), readBuf);
         User::WaitForRequest(status);
	 		 
         if(status.Int() != KErrNone)  break;

#ifdef OST_TRACE_COMPILER_IN_USE
         TInt length =  readBuf.Length();
         OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBEPBASE_FLUSHRXDATAL, "The length of trash data is %d", length );
         

         OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBEPBASE_FLUSHRXDATAL, "Begining of trash data" );
         
         for (int i=0; i<4&&(i*4+4)<=length; i++)
             {          
             OstTraceExt4( TRACE_NORMAL, DUP3_CMTPUSBEPBASE_FLUSHRXDATAL, 
                     "0x%x 0x%x 0x%x 0x%x", (TUint)readBuf[i*4], readBuf[i*4+1], readBuf[i*4+2], readBuf[i*4+3] );
             }
 
         OstTrace0( TRACE_NORMAL, DUP4_CMTPUSBEPBASE_FLUSHRXDATAL, "Residual of trash data if any" );
         TInt residualLength = length%512;
         for (int i=0; i<4&&(i*4+4)<=residualLength; i++)
             {
             TInt beginIndex = length - residualLength;
             OstTraceExt4( TRACE_NORMAL, DUP5_CMTPUSBEPBASE_FLUSHRXDATAL, 
                     "0x%x 0x%x 0x%x 0x%x", (TUint)readBuf[beginIndex + i*4], readBuf[beginIndex + i*4+1], 
                     readBuf[beginIndex + i*4+2], readBuf[beginIndex + i*4+3] );
             }
#endif
         
         // whenever some data read, reset the rest wait time.
         uRestTimeToWait = INTERVAL_FOR_FLUSH_TRASH_DATA;

         OstTrace0( TRACE_NORMAL, DUP6_CMTPUSBEPBASE_FLUSHRXDATAL, "FlushRxDataL()---Reset the rest wait time" );
         }
       else 
         {	
         // wait for the data from the usb channel.
         User::After(INTERVAL_FOR_READ_TRASH_DATA);
         // reduce the rest time to wait 
         uRestTimeToWait -=  INTERVAL_FOR_READ_TRASH_DATA ;
         }	

      OstTrace1( TRACE_NORMAL, DUP7_CMTPUSBEPBASE_FLUSHRXDATAL, "FlushRxDataL()---uRestTimeToWait is %d", uRestTimeToWait );
      
 			    	
    }while( uRestTimeToWait > 0);
			    	
    readBuf.Close();
    OstTraceFunctionExit0( CMTPUSBEPBASE_FLUSHRXDATAL_EXIT );
}

/*
 * The difference with FlushRxDataL() is FlushBufferedRxDataL only flush current buffered trash data
 * if currently no data is buffered in usb driver, just return rather than wait some time like what 
 * FlushRxDataL() does
 */
void CMTPUsbEpBase::FlushBufferedRxDataL()
    {
    //flush buffered rx data
    TInt  nbytes = 0;
    TInt err = Connection().Ldd().QueryReceiveBuffer(EndpointNumber(), nbytes);

    // has data, read it
    if( (err == KErrNone) && (nbytes > 0) )
        {
        // create the read buff
        RBuf8 readBuf;
        readBuf.CreateL(nbytes);
        // synchronously read the data
        TRequestStatus status;
        Connection().Ldd().ReadOneOrMore(status, EndpointNumber(), readBuf);
        User::WaitForRequest(status);
        readBuf.Close(); 
        }
    }

void CMTPUsbEpBase::FlushOnePacketL()
    {
    //trash data, continue to flush.
    TRequestStatus status;
    RBuf8 readBuf;
    readBuf.CreateL(KMaxPacketTypeBulkHS);
    Connection().Ldd().ReadPacket(status, EndpointNumber(), readBuf, KMaxPacketTypeBulkHS);
    User::WaitForRequest(status);    
    OstTrace1( TRACE_NORMAL, CMTPUSBEPBASE_FLUSHONEPACKETL, 
            "trash data length = %d", readBuf.Length());
    
    readBuf.Close();
    }
