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
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperesponse.h>

#include "cmtpusbepbulkin.h"
#include "cmtpusbepbulkout.h"
#include "cmtpusbconnection.h"
#include "cmtpusbcontainer.h"
#include "cmtpusbepcontrol.h"
#include "cmtpusbepinterruptin.h"
#include "mmtpconnectionmgr.h"
#include "mmtpconnectionprotocol.h"
#include "mtpbuildoptions.hrh"
#include "mtpdebug.h"
#include "mtpusbpanic.h"
#include "mtpusbprotocolconstants.h"

#ifdef _DEBUG
#include <e32debug.h>
#endif

#define UNUSED_VAR(a) (a)=(a)


// File type constants.
const TInt KMTPNullChunkSize(0x00020000); // 100KB
const TUint KUSBHeaderSize = 12;

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"UsbConnection");)
    
// Endpoint meta data.
const CMTPUsbConnection::TEpInfo CMTPUsbConnection::KEndpointMetaData[EMTPUsbEpNumEndpoints] = 
    {
        {KMTPUsbControlEpBit,   KMTPUsbControlEpDir,    KMTPUsbControlEpPoll,   KMTPUsbControlEpNAKRate,	KMTPUsbControlEp,   KMTPUsbControlEpType},  // EMTPUsbEpControl
        {KMTPUsbBulkInEpBit,    KMTPUsbBulkInEpDir,     KMTPUsbBulkInEpPoll,    KMTPUsbBulkInEpNAKRate,		KMTPUsbBulkInEp,    KMTPUsbBulkInEpType},   // EMTPUsbEpBulkIn
        {KMTPUsbBulkOutEpBit,   KMTPUsbBulkOutEpDir,    KMTPUsbBulkOutEpPoll,   KMTPUsbBulkOutEpNAKRate,	KMTPUsbBulkOutEp,   KMTPUsbBulkOutEpType},  // EMTPUsbEpBulkOut
        {KMTPUsbInterruptEpBit, KMTPUsbInterruptEpDir,  KMTPUsbInterruptEpPoll, KMTPUsbInterruptEpNAKRate,	KMTPUsbInterruptEp, KMTPUsbInterruptEpType} // EMTPUsbEpInterrupt
    }; 
    
/**
USB MTP USB device class connection factory method.
@param aConnectionMgr The MTP connection manager interface.
@return A pointer to an MTP USB device class connection. Ownership IS 
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbConnection* CMTPUsbConnection::NewL(MMTPConnectionMgr& aConnectionMgr)
    {
    CMTPUsbConnection* self = new (ELeave) CMTPUsbConnection(aConnectionMgr);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
CMTPUsbConnection::~CMTPUsbConnection()
    {
    __FLOG(_L8("~CMTPUsbConnection - Entry"));
    
    // Terminate all endpoint data transfer activity.
    StopConnection();
        
    // Close the device class endpoints and generic container buffers.
    iEndpoints.ResetAndDestroy();
    delete iUsbBulkContainer;
    delete iUsbEventContainer;
    
    // Stop the USB device.
    StopUsb();
    
    iNullBuffer.Close();

    if (iProtocolLayer != NULL)
        {
        BoundProtocolLayer().Unbind(*this);
        }
    iProtocolLayer = NULL;
    
    __FLOG(_L8("~CMTPUsbConnection - Exit"));
    __FLOG_CLOSE;
    }

void CMTPUsbConnection::BindL(MMTPConnectionProtocol& aProtocol)
    {
    __FLOG(_L8("BindL - Entry"));
    __ASSERT_DEBUG(!iProtocolLayer, Panic(EMTPUsbBadState));
    iProtocolLayer = &aProtocol;
    __FLOG(_L8("BindL - Exit"));
    }
    
MMTPConnectionProtocol& CMTPUsbConnection::BoundProtocolLayer()
    {
    __FLOG(_L8("BoundProtocolLayer - Entry"));
    __ASSERT_DEBUG(iProtocolLayer, Panic(EMTPUsbBadState));
    __FLOG(_L8("BoundProtocolLayer - Exit"));
    return *iProtocolLayer;
    }
    
void CMTPUsbConnection::CloseConnection()
    {
    __FLOG(_L8("CloseConnection - Entry"));
    /* 
    Terminate all endpoint data transfer activity, stall all but the control 
    endpoints, and wait for the host to issue a Device Reset Request.
    */
    StopConnection();
    TRAPD(err, BulkEndpointsStallL());
    UNUSED_VAR(err);
    __FLOG(_L8("CloseConnection - Exit"));
    }
    
void CMTPUsbConnection::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/)
    {
    __FLOG(_L8("ReceiveDataL - Entry"));
    
    // Update the transaction state.
    SetBulkTransactionState(EDataIToRPhase);
    
    // Setup the bulk container and initiate the bulk data receive sequence.
    iUsbBulkContainer->SetPayloadL(&aData);
    
    //Expected containerType pre-setup here in case we don't receive IToR dataphase at all so 
    //Cancel operation can trigger right call inside ReceiveBulkDataCompleteL(). 
    iUsbBulkContainer->SetUint16L(CMTPUsbContainer::EContainerType, EMTPUsbContainerTypeDataBlock);
    
    static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->ReceiveBulkDataL(*iUsbBulkContainer);
    
    __FLOG(_L8("ReceiveDataL - Exit"));       
    }

void CMTPUsbConnection::ReceiveDataCancelL(const TMTPTypeRequest& /*aRequest*/)
    {
    __FLOG(_L8("ReceiveDataCancelL - Entry"));
    
    // Store the device status code.
    TUint16 deviceStatus = iDeviceStatusCode;
    
    SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
   	static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->CancelReceiveL(KErrCancel);
   	
   	// Restore it.
   	SetDeviceStatus(deviceStatus);
    __FLOG(_L8("ReceiveDataCancelL - Exit"));
    }

void CMTPUsbConnection::SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("SendDataL - Entry"));  
    ProcessBulkDataInL(aRequest, aData);
    __FLOG(_L8("SendDataL - Exit"));  
    }

void CMTPUsbConnection::SendDataCancelL(const TMTPTypeRequest& /*aRequest*/)
    {
    __FLOG(_L8("SendDataCancelL - Entry"));
    // Store the device status code.
    TUint16 deviceStatus = iDeviceStatusCode;
    
    SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
 	static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->CancelSendL(KErrCancel);
 	// Restore it.
   	SetDeviceStatus(deviceStatus);
    __FLOG(_L8("SendDataCancelL - Exit"));
    }
        
void CMTPUsbConnection::SendEventL(const TMTPTypeEvent& aEvent)
    {    
    __FLOG(_L8("SendEventL - Entry"));
    
    // Reset the event.
    iMTPEvent.Reset(); 
    MMTPType::CopyL(aEvent, iMTPEvent);
    
    switch (ConnectionState())
        {
    case EIdle:
    case EStalled:
        // Drop the event.    
        __FLOG(_L8("Dropping the event")); 
        BoundProtocolLayer().SendEventCompleteL(KErrNone, aEvent);
        break;
        
    case EOpen:
    case EBusy:
        // Process the event.    
        switch (SuspendState())
            {
        case ENotSuspended:
            // Only send event if there are no pending events
            if (!iEventPending)
            	{
            	// Send the event data.
	            __FLOG(_L8("Sending the event"));
    	        BufferEventDataL(aEvent);
        	    SendEventDataL(); 	
            	}
             break;
                
        case ESuspended:
            /* 
            If remote wakeup is enabled then signal remote wakeup and buffer 
            the event. The event will be sent when bus signalling is resumed. 
            Otherwise the event is dropped, and a PTP UnreportedStatus event 
            issued when the host resumes the connection.
            */
            if (iLdd.SignalRemoteWakeup() == KErrNone)
                {
                // Remote wakeup is enabled, buffer the event data.
                __FLOG(_L8("Buffer event data and signal remote wakeup"));
                BufferEventDataL(aEvent);
                }
            else
                {
                // Remote wakeup is not enabled, drop the event.    
                __FLOG(_L8("Dropping the event")); 
                BoundProtocolLayer().SendEventCompleteL(KErrNone, aEvent);
                }
            
            /*
            Update state to trigger the correct processing when the USB connection is resumed.
            */
            SetSuspendState(ESuspendedEventsPending);
            break;
                
        case ESuspendedEventsPending:
            // Drop the event.    
            __FLOG(_L8("Dropping the event")); 
            BoundProtocolLayer().SendEventCompleteL(KErrNone, aEvent);
            break; 
              
        default:
            __FLOG(_L8("Invalid suspend state"));
            Panic(EMTPUsbBadState);
            break;
            }
        break;
                
    default:
        __FLOG(_L8("Invalid connection state"));
        Panic(EMTPUsbBadState);
        break;
        }
    
    __FLOG(_L8("SendEventL - Exit"));
    }
    
void CMTPUsbConnection::SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("SendResponseL - Entry"));
    __FLOG_VA((_L8("DeviceState: 0x%x TransactionState: 0x%x Connection: 0x%x"), iDeviceStatusCode, iBulkTransactionState, ConnectionState()));
    
    // Update the transaction state.
	SetBulkTransactionState(EResponsePhase);
    if (SuspendState() != ESuspended && !iIsCancelReceived)
    	{  	      
   		TUint16 opCode(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
   		TUint16 rspCode(aResponse.Uint16(TMTPTypeResponse::EResponseCode));
   		__FLOG_VA((_L8("ResponseCode = 0x%04X, Operation Code = 0x%04X"), rspCode, opCode));
    
   		if ((opCode == EMTPOpCodeOpenSession) && (rspCode == EMTPRespCodeOK))
        	{        
   	    	// An session has been opened. Record the active SessionID.
       		iMTPSessionId = aRequest.Uint32(TMTPTypeRequest::ERequestParameter1);
       		__FLOG_VA((_L8("Processing OpenSession response, SessionID = %d"), iMTPSessionId));
       		}
   		else if (((opCode == EMTPOpCodeCloseSession) || (opCode == EMTPOpCodeResetDevice))&& (rspCode == EMTPRespCodeOK))
        	{
   	    	// An session has been closed. Clear the active SessionID.        
       		__FLOG_VA((_L8("Processing CloseSession or ResetDevice response, SessionID = %d"), iMTPSessionId));
       		iMTPSessionId = KMTPSessionNone;
       		}

   		/* 
   		Setup the parameter block payload dataset. Note that since this is a 
   		variable length dataset, it must first be reset.
   		*/
   		iUsbBulkParameterBlock.Reset();
        TBool isNullParamValid = EFalse;
        TUint numberOfNullParam = 0;
        iUsbBulkParameterBlock.CopyIn(aResponse, TMTPTypeResponse::EResponseParameter1, TMTPTypeResponse::EResponseParameter1 + aResponse.GetNumOfValidParams(), isNullParamValid, numberOfNullParam);

   		// Setup the bulk container.
   		iUsbBulkContainer->SetPayloadL(const_cast<TMTPUsbParameterPayloadBlock*>(&iUsbBulkParameterBlock));
   		iUsbBulkContainer->SetUint32L(CMTPUsbContainer::EContainerLength, static_cast<TUint32>(iUsbBulkContainer->Size()));
   		iUsbBulkContainer->SetUint16L(CMTPUsbContainer::EContainerType, EMTPUsbContainerTypeResponseBlock);
   		iUsbBulkContainer->SetUint16L(CMTPUsbContainer::ECode, rspCode);
   		iUsbBulkContainer->SetUint32L(CMTPUsbContainer::ETransactionID, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));

    	// Initiate the bulk data send sequence.
   		__FLOG_VA((_L8("Sending response 0x%04X (%d bytes)"), iUsbBulkContainer->Uint16L(CMTPUsbContainer::ECode), iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength)));
   		static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->SendBulkDataL(*iUsbBulkContainer);
    	}
    else
    	{
    	BoundProtocolLayer().SendResponseCompleteL(KErrNone, aResponse, aRequest);
    	}
    
    __FLOG(_L8("SendResponseL - Exit"));
    } 
    
void CMTPUsbConnection::TransactionCompleteL(const TMTPTypeRequest& /*aRequest*/)
    {
    __FLOG(_L8("TransactionCompleteL - Entry"));
   
   	__FLOG_VA((_L8("DeviceState: 0x%x TransactionState: 0x%x"), iDeviceStatusCode, iBulkTransactionState));
   	
   	if (iBulkTransactionState != ERequestPhase)
   	    {
        // Update the transaction state.
        SetBulkTransactionState(EIdlePhase);    
        // Update the device status
        SetDeviceStatus(EMTPUsbDeviceStatusOK);     
        // Clear the cancel flag.
        iIsCancelReceived = EFalse; 
        
        if (ConnectionOpen())
            {
            // Initiate the next request phase bulk data receive sequence.
            InitiateBulkRequestSequenceL();   		    
            }
        else if (iIsResetRequestSignaled)
            {
            iIsResetRequestSignaled = EFalse;
            StartConnectionL();
            }
   	    }
    
    __FLOG(_L8("TransactionCompleteL - Exit"));
    } 

void CMTPUsbConnection::Unbind(MMTPConnectionProtocol& /*aProtocol*/)
    {
    __FLOG(_L8("Unbind - Entry"));
    __ASSERT_DEBUG(iProtocolLayer, Panic(EMTPUsbBadState));
    iProtocolLayer = NULL;
    __FLOG(_L8("Unbind - Exit"));
    } 
    
TAny* CMTPUsbConnection::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    return NULL;    
    }

TUint CMTPUsbConnection::GetImplementationUid()
    {
    return KMTPUsbTransportImplementationUid;
    }

void CMTPUsbConnection::ReceiveBulkDataCompleteL(TInt aError, MMTPType& /*aData*/)
    {
    __FLOG(_L8("ReceiveBulkDataCompleteL - Entry"));  
    if (!BulkRequestErrorHandled(aError))
        { 
        TUint type(iUsbBulkContainer->Uint16L(CMTPUsbContainer::EContainerType));        
	    __FLOG_VA((_L8("Received container type 0x%04X"), type));
	    
	    // The proper behaviour at this point is to stall the end points
	    // but alas Microsoft does not honor this.
	    // The solution is to store the error code, consume all the data
	    // sent and then forward the error code to the upper layers
	    // which will then go through all the phases
	    
	    // Only go here if...
       	if (aError != KErrNone // there is an error
       		&& type == EMTPUsbContainerTypeDataBlock // it is a data transfer
       		&& iDeviceStatusCode != EMTPUsbDeviceStatusTransactionCancelled // we haven't been cancelled by the initiator
            )
       		{
       		__FLOG_VA((_L8("ReceiveBulkDataCompleteL - error: %d"), aError));
       		iXferError = aError;
       		
       		// Update the transaction state.
    		SetBulkTransactionState(EDataIToRPhase);
    
		    // Setup the bulk container and initiate the bulk data receive sequence.
		    iNullBuffer.Close();
		    iNullBuffer.CreateL(KMTPNullChunkSize);
		    iNullBuffer.SetLength(KMTPNullChunkSize);
		    iNull.SetBuffer(iNullBuffer);
    		iUsbBulkContainer->SetPayloadL(&iNull);
    		static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->ResumeReceiveDataL(*iUsbBulkContainer);
       		}            
	    else
	    	{
	    	if (iXferError != KErrNone)
	    		{
	    		aError = iXferError;
	    		iXferError = KErrNone;
	    		iNullBuffer.Close();
	    		}
	    	
   	        switch (type)
	            {
    	    case EMTPUsbContainerTypeCommandBlock:
        	    ProcessBulkCommandL(aError);
           		break;
            
        	case EMTPUsbContainerTypeDataBlock:
            	ProcessBulkDataOutL(aError);
            	break;
            
        	case EMTPUsbContainerTypeResponseBlock:
        	case EMTPUsbContainerTypeEventBlock:
        	default:
            	// Invalid container received, shutdown the bulk data pipe.
            	__FLOG_VA((_L8("Invalid container type = 0x%04X"), type));
            	CloseConnection();
            	}
	        // Reset the bulk container.
			/* A special case for handling MTP framework's synchronous error handling during the request phase. 
				( ie at the beginning of this function, we are in the request phase, the request is sent to the 
				mtp f/w, which may send a synchronous error response, which will be populated into the usb bulk 
				container and sent out by the bulk In AO. When this function is completing, the response phase 
				has already been reached, and the container is in use by the bulk In AO, so the payload is not 
				reset)*/
    	    if((iBulkTransactionState != EResponsePhase)&&(!isCommandIgnored))
	        	{
				iUsbBulkContainer->SetPayloadL(NULL);
				}    	    
			// clear the flag
			isCommandIgnored = false;
	    	}         
        }
        
    __FLOG(_L8("ReceiveBulkDataCompleteL - Exit"));
    }
    
void CMTPUsbConnection::ReceiveControlRequestDataCompleteL(TInt aError, MMTPType& /*aData*/)
    {
    __FLOG(_L8("ReceiveControlRequestDataCompleteL - Entry"));
    if (!ControlRequestErrorHandled(aError))
        {
        // Complete the control request sequence.
        static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->SendControlRequestStatus();
          
        if (iUsbControlRequestSetup.Uint8(TMTPUsbControlRequestSetup::EbRequest) == EMTPUsbControlRequestCancel)
            {
            // Cancel data received.
            __FLOG(_L8("Cancel request data received."));
    
            // Setup the event dataset.
            __FLOG_VA((_L8("Cancellation Code = 0x%04X"), iUsbControlRequestCancelData.Uint16(TMTPUsbControlRequestCancelData::ECancellationCode)));
            __FLOG_VA((_L8("Transaction ID = 0x%08X"), iUsbControlRequestCancelData.Uint32(TMTPUsbControlRequestCancelData::ETransactionID)));
  
			#ifdef _DEBUG            
            // print log about the cacel event
            RDebug::Print(_L("cancel event received!!!!!!!!!!!!!!!!!Transaction phase is %d ----------------\n"), BoundProtocolLayer().TransactionPhaseL(iMTPSessionId));
            RDebug::Print(_L("The Transaction ID want to canceled is %d -------------"), iUsbControlRequestCancelData.Uint32(TMTPUsbControlRequestCancelData::ETransactionID));
            RDebug::Print(_L("Current Transaction ID is %d ----------------"),iMTPRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));
			#endif
            
            isResponseTransactionCancelledNeeded = true;
            if(  BoundProtocolLayer().TransactionPhaseL(iMTPSessionId) > EIdlePhase   ) 
            	{
            	
               	  

	            iMTPEvent.Reset();
	            iMTPEvent.SetUint16(TMTPTypeEvent::EEventCode, iUsbControlRequestCancelData.Uint16(TMTPUsbControlRequestCancelData::ECancellationCode));
	            iMTPEvent.SetUint32(TMTPTypeEvent::EEventSessionID, iMTPSessionId);
	            
	            // replace the transaction id in the event with the current transaction id
	            iMTPEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, iMTPRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));
	            
	            // Set the cancel flag.
	            iIsCancelReceived = ETrue;
	            
	            // Update the device status.
	       		SetDeviceStatus(EMTPUsbDeviceStatusBusy);   
	       		         
	            // Notify the protocol layer.
	            if (ConnectionOpen())
	                {
	                BoundProtocolLayer().ReceivedEventL(iMTPEvent);
	                }
            	}
            else
            	{

				#ifdef _DEBUG 
            	RDebug::Print(_L("cancel evnet received at idle state, stop data EPs, flush rx data, restart data eps,statusOK\n"));
				#endif

            	// stop data endpoint
            	DataEndpointsStop();
      
            	//flush rx data
            	TInt  nbytes = 0;
            	TInt err = iLdd.QueryReceiveBuffer(EndpointNumber(EMTPUsbEpBulkOut), nbytes);
				#ifdef _DEBUG
            	RDebug::Print(_L("QueryReceiveBuffer()-----err is %d , nbytes is %d"), err, nbytes);
				#endif	  

            	// has data, read it
            	if( (err == KErrNone) && (nbytes > 0) )
            		{
            		// create the read buff
            		RBuf8 readBuf;
            		readBuf.CreateL(nbytes);
            		// synchronously read the data
            		TRequestStatus status;
            		iLdd.ReadOneOrMore(status, EndpointNumber(EMTPUsbEpBulkOut), readBuf);
            		User::WaitForRequest(status);
            		if (KErrNone == status.Int())
            			{
	      		 		#ifdef _DEBUG
            			RDebug::Print(_L("%d bytes is flushed"), nbytes);
	      		  		#endif
            			}
            		readBuf.Close(); 
            		}
            	// initiate bulk request sequence.
            	InitiateBulkRequestSequenceL();
                       
            	SetDeviceStatus(EMTPUsbDeviceStatusOK);                   
            	}
            }

        // Initiate the next control request sequence.     
        InitiateControlRequestSequenceL();
        }
    __FLOG(_L8("ReceiveControlRequestDataCompleteL - Exit"));
    }
   
void CMTPUsbConnection::ReceiveControlRequestSetupCompleteL(TInt aError, MMTPType& aData)
    {
    __FLOG(_L8("ReceiveControlRequestSetupCompleteL - Entry"));
    if (!ControlRequestErrorHandled(aError))
        {
        TMTPUsbControlRequestSetup& data(static_cast<TMTPUsbControlRequestSetup&> (aData));
        __FLOG_VA((_L8("bRequest = 0x%X"), data.Uint8(TMTPUsbControlRequestSetup::EbRequest)));
        __FLOG_VA((_L8("wLength = %d bytes"), data.Uint16(TMTPUsbControlRequestSetup::EwLength)));
        
        switch (data.Uint8(TMTPUsbControlRequestSetup::EbRequest))
            {
        case EMTPUsbControlRequestCancel:
            ProcessControlRequestCancelL(data);
            break;
            
        case EMTPUsbControlRequestDeviceReset:
            ProcessControlRequestDeviceResetL(data);
            break;
            
        case EMTPUsbControlRequestDeviceStatus:
            ProcessControlRequestDeviceStatusL(data);
            break;
  
        default:
            __FLOG(_L8("Unrecognised class specific request received"));
            CloseConnection();
            break;
            }
        }
    __FLOG(_L8("ReceiveControlRequestSetupCompleteL - Exit"));
    }
    
void CMTPUsbConnection::SendBulkDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    __FLOG(_L8("SendBulkDataCompleteL - Entry")); 

#ifdef _DEBUG
    RDebug::Print(_L("\nCMTPUsbConnection::SendBulkDataCompleteL----entry"));
#endif
    if (!BulkRequestErrorHandled(aError))
        {
        TUint16 containerType(iUsbBulkContainer->Uint16L(CMTPUsbContainer::EContainerType));

#ifdef _DEBUG              
        TUint16 transactionID(iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
        RDebug::Print(_L("Time Stamp is :%d"), User::TickCount());
        RDebug::Print(_L("the container Type is 0x%x, the transaction ID is 0x%x\n"), containerType,transactionID);
#endif
        
        if (containerType == EMTPUsbContainerTypeResponseBlock)
            {
            // Response block sent.
            BoundProtocolLayer().SendResponseCompleteL(aError, *static_cast<TMTPTypeResponse*>(iUsbBulkContainer->Payload()), iMTPRequest);
    
            // Update the transaction state.
            if(ERequestPhase != iBulkTransactionState)
	            {
	            SetBulkTransactionState(ECompletingPhase);	
	            }
            }
        else if (containerType == EMTPUsbContainerTypeDataBlock)
            {
            // Data block sent.
            BoundProtocolLayer().SendDataCompleteL(aError, *iUsbBulkContainer->Payload(), iMTPRequest);
            }
        else
	        {
	        __FLOG(_L8("Invalid container type"));
            Panic(EMTPUsbBadState);
	        }
             
        // Reset the bulk container.
        if(ERequestPhase != iBulkTransactionState)
            {
    		iUsbBulkContainer->SetPayloadL(NULL);
            }		     
        }
    __FLOG(_L8("SendBulkDataCompleteL - Exit"));
    }
    
void CMTPUsbConnection::SendControlRequestDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    __FLOG(_L8("SendControlRequestDataCompleteL - Entry"));
    if (!ControlRequestErrorHandled(aError))
        {
        // Complete the control request sequence.
        if (iUsbControlRequestSetup.Uint8(TMTPUsbControlRequestSetup::EbRequest) == EMTPUsbControlRequestCancel)
            {
            // Cancel request processed, clear the device status.
            SetDeviceStatus(EMTPUsbDeviceStatusOK);
            __FLOG(_L8("Cancel Request processed"));
            }
        
        // Initiate the next control request sequence. 
        InitiateControlRequestSequenceL();            
        }
    __FLOG(_L8("SendControlRequestDataCompleteL - Exit"));
    }

void CMTPUsbConnection::SendInterruptDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    __FLOG(_L8("SendInterruptDataCompleteL - Entry"));
    iEventPending = EFalse;	
    BoundProtocolLayer().SendEventCompleteL(aError, iMTPEvent);
    __FLOG(_L8("SendInterruptDataCompleteL - Exit"));
    }    

/**
Provides the logical endpoint bit position bits of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@return The logical endpoint bit position bits.
*/
TUint CMTPUsbConnection::EndpointBitPosition(TUint aId) const
    {
    return iEndpointInfo[aId].iBitPosition;
    }    

/**
Provides the endpoint direction flag bits of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@return The endpoint direction flag bits.
*/
TUint CMTPUsbConnection::EndpointDirection(TUint aId) const
    {
    return iEndpointInfo[aId].iDirection;      
    }   

/**
Provides the capabilities of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@leave KErrOverflow, if the USB device does not support the minimum number of 
endpoints required by the USB MTP device class.
*/
const TUsbcEndpointCaps& CMTPUsbConnection::EndpointCapsL(TUint aId)
    {
    __FLOG(_L8("EndpointCapsL - Entry"));
    
    // Verify the the USB device supports the minimum number of endpoints.
    TInt totalEndpoints = iDeviceCaps().iTotalEndpoints;
    
    __FLOG_VA((_L8("% d endpoints available, %d required"), totalEndpoints, KMTPUsbRequiredNumEndpoints));
    if (totalEndpoints < KMTPUsbRequiredNumEndpoints)
        {
        User::Leave(KErrOverflow);            
        }      
        
    TUint   flags(EndpointDirectionAndType(aId));
    __FLOG_VA((_L8("Required EP%d iTypesAndDir = 0x%X"), aId, flags));
	
    TBool   found(EFalse);
    for (TUint i(0); ((!found) && (i < totalEndpoints)); i++)
        {
        TUsbcEndpointCaps&  caps(iEndpointCapSets[i].iCaps);
        
        if ((caps.iTypesAndDir & flags) == flags)
            {
            found           = ETrue;
            iEndpointCaps   = caps;
			
            __FLOG_VA((_L8("Matched EP%d iTypesAndDir = 0x%X"), i, caps.iTypesAndDir));
            __FLOG_VA((_L8("Matched EP%d MaxPacketSize = %d"), i, caps.MaxPacketSize()));
            __FLOG_VA((_L8("Matched EP%d MinPacketSize = %d"), i, caps.MinPacketSize()));
            }
        }
        
    if (!found)    
        {
        User::Leave(KErrHardwareNotAvailable);
        }
                
    __FLOG(_L8("EndpointCapsL - Exit"));
    return iEndpointCaps; 
    }   

/**
Provides the endpoint direction and type flag bits of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@return The logical endpoint number.
*/
TUint CMTPUsbConnection::EndpointDirectionAndType(TUint aId) const
    {
    return (EndpointDirection(aId) | EndpointType(aId));      
    } 

/**
Provides the logical endpoint number of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@return The logical endpoint number.
*/
TEndpointNumber CMTPUsbConnection::EndpointNumber(TUint aId) const
    {
    return iEndpointInfo[aId].iNumber;     
    }    

/**
Provides the endpoint type flag bits of the specified endpoint.
@param aId The internal endpoint identifier of the endpoint.
@return The endpoint type flag bits.
*/
TUint CMTPUsbConnection::EndpointType(TUint aId) const
    {
    return iEndpointInfo[aId].iType;      
    } 
    
/**
Provides the USB device client interface.
@return The USB device client interface.
*/
RDevUsbcClient& CMTPUsbConnection::Ldd()
    {
    return iLdd;
    }
    
void CMTPUsbConnection::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));    
    iLdd.AlternateDeviceStatusNotifyCancel();
    __FLOG(_L8("DoCancel - Exit"));  
    }
    
#ifdef __FLOG_ACTIVE
TInt CMTPUsbConnection::RunError(TInt aError)
#else
TInt CMTPUsbConnection::RunError(TInt /*aError*/)
#endif
    {
    __FLOG(_L8("RunError - Entry"));
    __FLOG_VA((_L8("Error = %d"), aError));
    
    // Cancel all the outstanding requests.
    Cancel();   
    
    // Stop the connection, if necessary.
    StopConnection(); 
    
    // Stop the control end point.
    ControlEndpointStop();
    
    // Issue the notify request again.
    IssueAlternateDeviceStatusNotifyRequest();
    
    __FLOG(_L8("RunError - Exit"));
    return KErrNone;
    }
    
void CMTPUsbConnection::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    
    if (!(iControllerStateCurrent & KUsbAlternateSetting))
        {
        // Alternative interface setting has not changed.
        __FLOG_VA((_L8("Alternate device state changed to %d"), iControllerStateCurrent));
        
        if ((SuspendState() & ESuspended) &&
        	(iControllerStateCurrent != EUsbcDeviceStateSuspended))
        	{
		    //  Update state.
		    SetSuspendState(ENotSuspended);	
        	}
        
        switch (iControllerStateCurrent)
            {
            case EUsbcDeviceStateUndefined:
            case EUsbcDeviceStateAttached:
            case EUsbcDeviceStatePowered:
            case EUsbcDeviceStateDefault:
                StopConnection(); 
                ControlEndpointStop();
                break;
            
            case EUsbcDeviceStateAddress:
            	// Set the Endpoint packet sizes.
            	SetTransportPacketSizeL();
            	
            	// Stop the control endpoint first, in case there is still 
            	// outstanding request.
            	ControlEndpointStop();
            	
                // Initiate control endpoint data transfer activity.
                ControlEndpointStartL();
                break;
            
            case EUsbcDeviceStateConfigured:
                {
				__FLOG(_L8("Device state : EUsbcDeviceStateConfigured"));

                if (iControllerStatePrevious == EUsbcDeviceStateSuspended)
                    {
                    // Resume connection data transfer activity.
                    ResumeConnectionL();
                    
                    // Process buffered events.
                    if (SuspendState() == ESuspendedEventsPending)
                        {
                        /* 
                        If remote wakeup is enabled then signal remote wakeup
                        before sending the buffered event data. Otherwise issue 
                        a PTP UnreportedStatus event.
                        */
                        
                        // Don't check for pending events since this
                        // is after a suspend
                        if (iRemoteWakeup)
                            {
                            // Send the event data.
                            __FLOG(_L8("Sending buffered event data"));
                            SendEventDataL();
                            }
                        else
                            {
                            // Send PTP UnreportedStatus event
                            __FLOG(_L8("Sending PTP UnreportedStatus event"));
                            SendUnreportedStatusEventL();
                            }
                        } 
                    }
                else
                    {                    
                    //restart the control endpoint    
                    ControlEndpointStop(); 
                    InitiateControlRequestSequenceL(); 

                    //restart the data endpoint
                    StartConnectionL();
                    }
                break;    
            
            case EUsbcDeviceStateSuspended:
                // Suspend connection activity.
	            SuspendConnectionL();   
                break;
                }
                
            default:
                __FLOG(_L8("Invalid alternate device state"));
                Panic(EMTPUsbBadState);
                break;
            }
        }
    else
        {
        // Alternate interface setting has changed.
        __FLOG_VA((_L8("Alternate interface setting changed from %d to %d"), iControllerStatePrevious, iControllerStateCurrent));
        }
        
        
    // Record the controller state and issue the next notification request.
    iControllerStatePrevious = iControllerStateCurrent;
    IssueAlternateDeviceStatusNotifyRequest();
    
    __FLOG(_L8("RunL - Exit"));
    }
    
/**
Constructor.
@param aConnectionMgr The MTP connection manager interface.
*/
CMTPUsbConnection::CMTPUsbConnection(MMTPConnectionMgr& aConnectionMgr) :
    CActive(EPriorityStandard),
    iEndpointInfo(KEndpointMetaData, EMTPUsbEpNumEndpoints),
    iIsCancelReceived(EFalse),
    iIsResetRequestSignaled(EFalse),
    iConnectionMgr(&aConnectionMgr)
    {
    CActiveScheduler::Add(this);
    }
    
/**
Second phase constructor.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    
    // Start the USB device.
    StartUsbL();
    
    // Create the device class endpoints.
    iEndpoints.InsertInOrderL(CMTPUsbEpControl::NewL(EMTPUsbEpControl, *this), CMTPUsbEpBase::LinearOrder);
    iEndpoints.InsertInOrderL(CMTPUsbEpBulkIn::NewL(EMTPUsbEpBulkIn, *this), CMTPUsbEpBase::LinearOrder);
    iEndpoints.InsertInOrderL(CMTPUsbEpBulkOut::NewL(EMTPUsbEpBulkOut, *this), CMTPUsbEpBase::LinearOrder);
    iEndpoints.InsertInOrderL(CMTPUsbEpInterruptIn::NewL(EMTPUsbEpInterrupt, *this), CMTPUsbEpBase::LinearOrder);
       
    // Create the generic data container buffers.
    iUsbBulkContainer   = CMTPUsbContainer::NewL();
    iUsbEventContainer  = CMTPUsbContainer::NewL();
    
    // Initialise the device status.
    SetDeviceStatus(EMTPUsbDeviceStatusOK);
    
    // Fetch the remote wakeup flag.
    TUsbDeviceCaps dCaps;
    User::LeaveIfError(iLdd.DeviceCaps(dCaps));
    iRemoteWakeup = dCaps().iRemoteWakeup;
    __FLOG_VA((_L8("iRemote = %d"), iRemoteWakeup));
    
    // Start monitoring the USB device controller state.
    IssueAlternateDeviceStatusNotifyRequest();
    
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
Issues a request for notification of USB device controller state and alternate
interface setting changes.
*/
void CMTPUsbConnection::IssueAlternateDeviceStatusNotifyRequest()
    {
    __FLOG(_L8("IssueAlternateDeviceStatusNotifyRequest - Entry"));
    
    if (!IsActive())
        {
        iLdd.AlternateDeviceStatusNotify(iStatus, iControllerStateCurrent);
        }

    SetActive();
    __FLOG(_L8("IssueAlternateDeviceStatusNotifyRequest - Exit"));
    }
    
/**
Populates the asynchronous event interrupt dataset buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::BufferEventDataL(const TMTPTypeEvent& aEvent)
    {
    __FLOG(_L8("BufferEventData - Entry")); 
    /* 
    Setup the parameter block payload dataset. Note that since this is a 
    variable length dataset, it must first be reset.
    */ 
    iUsbEventParameterBlock.Reset();
    iUsbEventParameterBlock.CopyIn(aEvent, TMTPTypeEvent::EEventParameter1, TMTPTypeEvent::EEventParameter3, EFalse, 0);
    
    // Setup the bulk container.
    iUsbEventContainer->SetPayloadL(const_cast<TMTPUsbParameterPayloadBlock*>(&iUsbEventParameterBlock));
    iUsbEventContainer->SetUint32L(CMTPUsbContainer::EContainerLength, iUsbEventContainer->Size());
    iUsbEventContainer->SetUint16L(CMTPUsbContainer::EContainerType, EMTPUsbContainerTypeEventBlock);
    iUsbEventContainer->SetUint16L(CMTPUsbContainer::ECode, aEvent.Uint16(TMTPTypeEvent::EEventCode));
    iUsbEventContainer->SetUint32L(CMTPUsbContainer::ETransactionID, aEvent.Uint32(TMTPTypeEvent::EEventTransactionID));
    __FLOG(_L8("BufferEventData - Exit"));
    }
    
/**
Initiates an interrupt data send sequence.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SendEventDataL()
    {
    __FLOG(_L8("SendEventData - Entry"));
    __FLOG_VA((_L8("Sending event 0x%4X (%d bytes)"), iUsbEventContainer->Uint16L(CMTPUsbContainer::ECode), iUsbEventContainer->Uint32L(CMTPUsbContainer::EContainerLength)));
    static_cast<CMTPUsbEpInterruptIn*>(iEndpoints[EMTPUsbEpInterrupt])->SendInterruptDataL(*iUsbEventContainer);
    iEventPending = ETrue;
    __FLOG(_L8("SendEventData - Exit"));
    }
    
/**
Issues a PTP UnreportedStatus event.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SendUnreportedStatusEventL()
    {
    __FLOG(_L8("SendUnreportedStatusEventL - Entry"));
        
    // Construct an UnreportedStatus event
    TMTPTypeEvent mtpEvent;
    mtpEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeUnreportedStatus);
    mtpEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionNone);
    mtpEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
    SendEventL(mtpEvent);
    
    __FLOG(_L8("SendUnreportedStatusEventL - Exit"));
    }
    
/**
Issues a request to the bulk-out endpoint to receive a command block dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::InitiateBulkRequestSequenceL()
    {
    __FLOG(_L8("InitiateBulkRequestSequenceL - Entry"));
    CMTPUsbEpBulkOut& bulkOut(*static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut]));

        // Update the transaction state.
        SetBulkTransactionState(ERequestPhase);
        
        // Setup the bulk container.
        iUsbBulkParameterBlock.Reset();
        iUsbBulkContainer->SetPayloadL(&iUsbBulkParameterBlock);
        
        // Initiate the next request phase bulk data receive sequence.
        bulkOut.ReceiveBulkDataL(*iUsbBulkContainer);
        
    __FLOG(_L8("InitiateBulkRequestSequenceL - Exit"));
    }

/**
Issues a request to the control endpoint to receive a request setup dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::InitiateControlRequestSequenceL()
    {
    __FLOG(_L8("InitiateControlRequestSequenceL - Entry"));
    CMTPUsbEpControl& ctrl(*static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl]));
    if (!ctrl.Stalled())
        {
        ctrl.ReceiveControlRequestSetupL(iUsbControlRequestSetup);
        }
    __FLOG(_L8("InitiateControlRequestSequenceL - Exit"));        
    }

/**
Processes received USB SIC bulk command block containers received from the 
connected host on the bulk out data pipe.
@param aError The error completion status of the bulk data receive request.
@leave One of the system wide error codes, if a processing failure occurs.
*/         
#ifdef __FLOG_ACTIVE
void CMTPUsbConnection::ProcessBulkCommandL(TInt aError)
#else
void CMTPUsbConnection::ProcessBulkCommandL(TInt /*aError*/)
#endif
    {
    __FLOG(_L8("ProcessBulkCommandL - Entry"));
    __FLOG_VA((_L8("aError = %d"), aError));
    if (BulkRequestTransactionStateValid(ERequestPhase))
        {
        // Request block received.
        TUint16 op(iUsbBulkContainer->Uint16L(CMTPUsbContainer::ECode));
   	    __FLOG_VA((_L8("Command block 0x%04X received"), op));
        
       	// Reset the iMTPRequest.
       	iMTPRequest.Reset();

       	// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
       	iMTPRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, op);
       	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestTransactionID, iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
        
       	// Set SessionID.
       	if (op == EMTPOpCodeOpenSession)
           	{
           	__FLOG(_L8("Processing OpenSession request"));
           	// Force OpenSession requests to be processed outside an active session.
           	// It is a known problem for MTP Protocol, it is a workaround here.
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, KMTPSessionNone);  
           	}
       	else if (op == EMTPOpCodeCloseSession || op == EMTPOpCodeResetDevice)
           	{
           	__FLOG(_L8("Processing CloseSession or the ResetDevice request"));
           	// Force CloseSession requests to be processed outside an active session. 
           	// ResetDevice currently behaves the same way as CloseSession. 
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, KMTPSessionNone); 
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestParameter1, iMTPSessionId); 
           	}       	
       	else
           	{
           	__FLOG_VA((_L8("Processing general request on session %d"), iMTPSessionId));
           	// Update the request dataset with the single active session's SessionID.
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iMTPSessionId);
           	}
       	
#ifdef _DEBUG
       	RDebug::Print(_L("The time stamp is: %d"), User::TickCount());
       	RDebug::Print(_L("New command comes, Operation code is 0x%x, transaction id is %d \n"), op, iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));  	
#endif
       	
        TUint  commandTransID(iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
        	   
        // process this command as usual
        // Update the device status.
        SetDeviceStatus(EMTPUsbDeviceStatusBusy);
                  
        // Set Parameter 1 .. Parameter 5.
        iUsbBulkParameterBlock.CopyOut(iMTPRequest, TMTPTypeRequest::ERequestParameter1, TMTPTypeRequest::ERequestParameter5);
        iUsbBulkParameterBlock.Reset();
       
        // Notify the protocol layer.
        BoundProtocolLayer().ReceivedRequestL(iMTPRequest);
        }
    __FLOG(_L8("ProcessBulkCommandL - Exit"));
    }

/**
Processes USB SIC bulk data block containers onto the connected host on the 
bulk in data pipe.
@param aRequest The MTP request dataset of the active MTP transaction.
@param aData The MTP data object source.
@leave One of the system wide error codes, if a processing failure occurs.
*/       
void CMTPUsbConnection::ProcessBulkDataInL(const TMTPTypeRequest& aRequest, const MMTPType& aData)
    {
    __FLOG(_L8("ProcessBulkDataInL - Entry"));
    
    // Update the transaction state.
    SetBulkTransactionState(EDataRToIPhase);
    
    // Setup the bulk container.
    iUsbBulkContainer->SetPayloadL(const_cast<MMTPType*>(&aData));
    
    TUint64 size(iUsbBulkContainer->Size());
    TUint32 containerLength((size > KMTPUsbContainerLengthMax) ? KMTPUsbContainerLengthMax : static_cast<TUint32>(size));
    iUsbBulkContainer->SetUint32L(CMTPUsbContainer::EContainerLength, containerLength);
    
    iUsbBulkContainer->SetUint16L(CMTPUsbContainer::EContainerType, EMTPUsbContainerTypeDataBlock);
    iUsbBulkContainer->SetUint16L(CMTPUsbContainer::ECode, aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
    iUsbBulkContainer->SetUint32L(CMTPUsbContainer::ETransactionID, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));

    // Initiate the bulk data send sequence.
    __FLOG_VA((_L8("Sending %d data bytes"), iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength)));
    
#ifdef _DEBUG
    RDebug::Print(_L("ProcessBulkDataInL:    iIsCancelReceived = %d, SuspendState() is  %d \n"),iIsCancelReceived,SuspendState());
#endif
    
    // if the cancel event is received before send data. That is, the phase is before DATA R2I, 
    // Device should not send the transaction  data to the host,just like what sendResponse does.
    if (SuspendState() != ESuspended && !iIsCancelReceived)
    	{
    	TPtr8 headerChunk(NULL, 0);
    	TBool hasTransportHeader = const_cast<MMTPType&>(aData).ReserveTransportHeader(KUSBHeaderSize, headerChunk);
        if(hasTransportHeader)
            {
            TUint16 containerType = EMTPUsbContainerTypeDataBlock;
            TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
            TUint32 transId = aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID);
            memcpy(const_cast<TUint8*>(headerChunk.Ptr()), &containerLength, sizeof(TUint32));
            memcpy(const_cast<TUint8*>(headerChunk.Ptr()) + 4, &containerType, sizeof(TUint16));
            memcpy(const_cast<TUint8*>(headerChunk.Ptr()) + 6, &operationCode, sizeof(TUint16)); 
            memcpy(const_cast<TUint8*>(headerChunk.Ptr()) + 8, &transId, sizeof(TUint32));
            static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->SendBulkDataL(aData);
            }
        else
            {
            static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->SendBulkDataL(*iUsbBulkContainer);
            }
        }
    else
    	{
    	
#ifdef _DEBUG    	
    	RDebug::Print(_L("the senddata is canceled!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
#endif    	
    	// by pass the calling to lower level send data
    	SendBulkDataCompleteL(KErrNone, *iUsbBulkContainer);
    	
    	}
    __FLOG(_L8("ProcessBulkDataInL - Exit"));
    }

/**
Processes received USB SIC bulk data block containers received from the 
connected host on the bulk out data pipe.
@param aError The error completion status of the bulk data receive request.
@leave One of the system wide error codes, if a processing failure occurs.
*/       
void CMTPUsbConnection::ProcessBulkDataOutL(TInt aError)
    {
    __FLOG(_L8("ProcessBulkDataOutL - Entry"));
    if ((BulkRequestTransactionStateValid(EDataIToRPhase)))
        {
        // Data block received, notify the protocol layer.
        __FLOG_VA((_L8("Data block received (%d bytes)"), iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength)));
        BoundProtocolLayer().ReceiveDataCompleteL(aError, *iUsbBulkContainer->Payload(), iMTPRequest);
        }
    __FLOG(_L8("ProcessBulkDataOutL - Exit"));
    }

/**
Processes received USB SIC class specific Cancel requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/   
void CMTPUsbConnection::ProcessControlRequestCancelL(const TMTPUsbControlRequestSetup& /*aRequest*/)
    {
    __FLOG(_L8("ProcessControlRequestCancelL - Entry"));
    static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->ReceiveControlRequestDataL(iUsbControlRequestCancelData);
    __FLOG(_L8("Waiting for Cancel Request Data"));
    __FLOG(_L8("ProcessControlRequestCancelL - Exit"));
    }
    
/**
Processes received USB SIC class specific Device Reset requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ProcessControlRequestDeviceResetL(const TMTPUsbControlRequestSetup& /*aRequest*/)
    {
    __FLOG(_L8("ProcessControlRequestDeviceResetL - Entry"));
    
    // Clear stalled endpoints and re-open connection
    BulkEndpointsStallClearL();
    StartConnectionL();
    
    /*
    The Device Reset Request is data-less. Complete the control request
    sequence and initiate the next control request sequence. 
    */
    static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->SendControlRequestStatus();
    TBool connIsStopped = StopConnection();
    InitiateControlRequestSequenceL();
    
    if (connIsStopped)
        {
        StartConnectionL();
        }
    else
        {
        iIsResetRequestSignaled = ETrue;
        }

    __FLOG(_L8("ProcessControlRequestDeviceResetL - Exit"));
    }
    
/**
Processes received USB SIC class specific Get Device Status requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ProcessControlRequestDeviceStatusL(const TMTPUsbControlRequestSetup& aRequest)
    {
    __FLOG(_L8("ProcessControlRequestDeviceStatusL - Entry"));
    iUsbControlRequestDeviceStatus.Reset();
    
    TUint offset = 0;
    for(TUint i(EMTPUsbEpControl); i<EMTPUsbEpNumEndpoints; ++i)
    	{        
        if ( IsEpStalled(i) )
            {
            TInt epSize(0);
            User::LeaveIfError( iLdd.GetEndpointDescriptorSize(KMTPUsbAlternateInterface, i, epSize) );
            
            RBuf8 epDesc; //endpoint descriptor, epDesc[2] is the address of endpoint. More info, pls refer to USB Sepc2.0 - 9.6.6
            epDesc.CreateL(epSize);
            CleanupClosePushL(epDesc); 
            User::LeaveIfError( iLdd.GetEndpointDescriptor(KMTPUsbAlternateInterface, i, epDesc) );

            //Maybe here is a little bit confused. Although an endpoint address is a 8-bit byte in Endpoint Descriptor,
            //but in practice, it's requested by host with a 32-bit value, so we plus offset with 4 to reflect this.
            TUint32 epAddress = epDesc[KEpAddressOffsetInEpDesc];
            iUsbControlRequestDeviceStatus.SetUint32((offset + TMTPUsbControlRequestDeviceStatus::EParameter1), epAddress);            
            CleanupStack::PopAndDestroy(); // calls epDesc.Close()            
            ++offset;
            }
        }

    // if the current status is OK and a cancel event has been received but the device has not respond 
    // transaction_cancelled yet, return transaction_cancelled firstly.
    TUint16 originalStatus = iDeviceStatusCode;
    if( (iDeviceStatusCode == EMTPUsbDeviceStatusOK) && isResponseTransactionCancelledNeeded )
    	{
    	SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
    	  // clear the transaction cancelled flag
        isResponseTransactionCancelledNeeded = false;
    	}

    // Set the Code and wLength fields and send the dataset.
    iUsbControlRequestDeviceStatus.SetUint16(TMTPUsbControlRequestDeviceStatus::ECode, iDeviceStatusCode);
    iUsbControlRequestDeviceStatus.SetUint16(TMTPUsbControlRequestDeviceStatus::EwLength, iUsbControlRequestDeviceStatus.Size());
    // send the response
    static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->SendControlRequestDataL(iUsbControlRequestDeviceStatus);
      
    // restore the original device status
    SetDeviceStatus(originalStatus);
    
    __FLOG(_L8("ProcessControlRequestDeviceStatusL - Exit"));
    }

/**
Processes bulk transfer request completion error checking. If the completion 
status is abnormal then the connection is shutdown.
@param aError bulk transfer request completion error. 
@return ETrue if the control request completion status was abnormal, otherwise
EFalse.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TBool CMTPUsbConnection::BulkRequestErrorHandled(TInt aError)
    {
    __FLOG(_L8("BulkRequestErrorHandled - Entry"));
    __FLOG_VA((_L8("Bulk transfer request completion status = %d"), aError));
    TBool ret(EFalse);
    
    // Only handle USB error codes
    if (aError <= KErrUsbDriverBase &&
    	aError >= KErrUsbEpNotReady)
        {   
        switch (aError)  
            {        
       	case KErrUsbDeviceNotConfigured:
        case KErrUsbInterfaceChange:
        case KErrUsbDeviceClosing:
        case KErrUsbCableDetached:
        case KErrUsbDeviceBusReset:
            // Interface state is changing (@see RunL).
            ret = ETrue;
            break;

        default:
            // Unknown error on a bulk endpoint, close the connection.
            CloseConnection();
            ret = ETrue;
            break;            
            }
        }
    __FLOG(_L8("BulkRequestErrorHandled - Exit"));
    return ret;
    }
    
/**
Processes bulk transfer request transaction state checking. If the transaction 
state is invalid, then the connection is shutdown.
@return ETrue if the control request completion status was abnormal, otherwise
EFalse.
*/
TBool CMTPUsbConnection::BulkRequestTransactionStateValid(TMTPTransactionPhase aExpectedTransactionState)
    {
    __FLOG(_L8("BulkRequestTransactionStateValid - Entry"));
    __FLOG_VA((_L8("Bulk transaction state = %d"), iBulkTransactionState));
    TBool valid(iBulkTransactionState == aExpectedTransactionState);
    if (!valid)
        {
        // Invalid bulk transaction state, close the connection.
        __FLOG_VA((_L8("Expected bulk transaction state = %d"), aExpectedTransactionState));
        CloseConnection();
        }
    __FLOG(_L8("BulkRequestTransactionStateValid - Exit"));
    return valid;
    }
    
/**
Processes control request completion. If the completion status is abnormal then
the connection is shutdown.
@param aError control request completion error code.
@return ETrue if the control request completion status was abnormal, otherwise
EFalse.
*/
TBool CMTPUsbConnection::ControlRequestErrorHandled(TInt aError)
    {
    __FLOG(_L8("ControlRequestErrorHandled - Entry"));
    __FLOG_VA((_L8("Control request completion status = %d"), aError));
    TBool ret(EFalse);
    
    if (aError != KErrNone)
        {
        switch (aError)  
            {
        case KErrCancel:
            // Control request sequence cancelled.
            break;
        
        case KErrUsbDeviceNotConfigured:    
        case KErrUsbInterfaceChange:
        case KErrUsbDeviceClosing:
        case KErrUsbCableDetached:
        case KErrUsbDeviceBusReset:
            // Interface state is changing (@see RunL).
            break;

        default:
            // Unknown error on the control endpoint, shutdown the connection.
            CloseConnection();
            break;            
            }
            
        // Signal abnormal completion.
        ret = ETrue;
        }
    
    __FLOG(_L8("ControlRequestErrorHandled - Exit"));
    return ret;
    }

/**
Clears the USB MTP device class configuration descriptor.
*/
void CMTPUsbConnection::ConfigurationDescriptorClear()
    {
    __FLOG(_L8("ConfigurationDescriptorClear - Entry"));
    const TInt KNumInterfacesOffset(4);
    
    TInt descriptorSize(0);
    iLdd.GetConfigurationDescriptorSize(descriptorSize);
    
    if (static_cast<TUint>(descriptorSize) == KUsbDescSize_Config)
        {
        TBuf8<KUsbDescSize_Config> descriptor;
        if (iLdd.GetConfigurationDescriptor(descriptor) == KErrNone)
            {
            --descriptor[KNumInterfacesOffset];
            iLdd.SetConfigurationDescriptor(descriptor);
            }
        }
    
    __FLOG(_L8("ConfigurationDescriptorClear - Exit"));
    }

/**
Sets the USB MTP device class configuration descriptor.
@leave KErrCorrupt, if the configuration descriptor size is invalid.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::ConfigurationDescriptorSetL()
    {
    __FLOG(_L8("SetupConfigurationDescriptorL - Entry"));
    const TInt KNumInterfacesOffset(4);
    
    TInt descriptorSize(0);
    iLdd.GetConfigurationDescriptorSize(descriptorSize);
    
    if (static_cast<TUint>(descriptorSize) != KUsbDescSize_Config)
        {
        User::Leave(KErrCorrupt);
        }
 
    TBuf8<KUsbDescSize_Config> descriptor;
    User::LeaveIfError(iLdd.GetConfigurationDescriptor(descriptor));
    ++descriptor[KNumInterfacesOffset];
    User::LeaveIfError(iLdd.SetConfigurationDescriptor(descriptor));
    
    __FLOG(_L8("SetupConfigurationDescriptorL - Exit"));
    }

/**
Indicates whether the connection state is closed.
*/    
TBool CMTPUsbConnection::ConnectionClosed() const
    {
    return (ConnectionState() < EOpen);        
    }

/**
Indicates whether the connection state is open.
*/
TBool CMTPUsbConnection::ConnectionOpen() const
    {
    return (!ConnectionClosed());        
    }
    
/**
Starts data transfer activity on the control endpoint.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::ControlEndpointStartL()
    {
    __FLOG(_L8("StartControlEndpoint - Entry"));
    InitiateControlRequestSequenceL();
    __FLOG(_L8("StartControlEndpoint - Exit"));  
    }

/**
Stops data transfer activity on the control endpoint.
*/    
void CMTPUsbConnection::ControlEndpointStop()
    {
    __FLOG(_L8("ControlEndpointStop - Entry"));
    iEndpoints[EMTPUsbEpControl]->Cancel();
    __FLOG(_L8("ControlEndpointStop - Exit"));    
    }

/**
Stalls all but the control endpoint.
*/    
void CMTPUsbConnection::BulkEndpointsStallL()
    {
    __FLOG(_L8("BulkEndpointsStallL - Entry"));
    EndpointStallL(EMTPUsbEpBulkIn);
    EndpointStallL(EMTPUsbEpBulkOut);
    __FLOG(_L8("BulkEndpointsStallL - Exit"));
    }

/**
Clears stall conditions all but the control endpoint.
*/
void CMTPUsbConnection::BulkEndpointsStallClearL()
    {
    __FLOG(_L8("BulkEndpointsStallClearL - Entry"));
    EndpointStallClearL(EMTPUsbEpBulkIn);
    EndpointStallClearL(EMTPUsbEpBulkOut);
    __FLOG(_L8("BulkEndpointsStallClearL - Exit"));  
    }

/**
Starts data transfer activity on the data endpoints.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPUsbConnection::DataEndpointsStartL()
    {
    __FLOG(_L8("DataEndpointsStartL - Entry"));
    InitiateBulkRequestSequenceL();
    __FLOG(_L8("DataEndpointsStartL - Exit"));
    }

/**
Stops data transfer activity on all but the control endpoint.
*/    
void CMTPUsbConnection::DataEndpointsStop()
    {
    __FLOG(_L8("DataEndpointsStop - Entry"));
    if (ConnectionOpen() && (!(SuspendState() & ESuspended))&& (iControllerStatePrevious != EUsbcDeviceStateSuspended))
        {
        __FLOG(_L8("Stopping active endpoints"));
        iEndpoints[EMTPUsbEpBulkIn]->Cancel();
        iEndpoints[EMTPUsbEpBulkOut]->Cancel();
        iEndpoints[EMTPUsbEpInterrupt]->Cancel();
        if ((iBulkTransactionState == EDataIToRPhase) && iUsbBulkContainer->Payload())
            {
            __FLOG(_L8("Aborting active I to R data phase"));
            TRAPD(err, BoundProtocolLayer().ReceiveDataCompleteL(KErrAbort, *iUsbBulkContainer->Payload(), iMTPRequest));
            UNUSED_VAR(err);
            }
        else if ((iBulkTransactionState == EDataRToIPhase) && iUsbBulkContainer->Payload())
            {
            __FLOG(_L8("Aborting active R to I data phase"));
            TRAPD(err, BoundProtocolLayer().SendDataCompleteL(KErrAbort, *iUsbBulkContainer->Payload(), iMTPRequest));
            UNUSED_VAR(err);
            }
		else if ((iBulkTransactionState == EResponsePhase) && iUsbBulkContainer->Payload())
            {
            __FLOG(_L8("Aborting active response phase"));
            TRAPD(err, BoundProtocolLayer().SendResponseCompleteL(KErrAbort, *static_cast<TMTPTypeResponse*>(iUsbBulkContainer->Payload()), iMTPRequest));
            UNUSED_VAR(err);
            }
        }
#ifdef __FLOG_ACTIVE
    else
        {
        __FLOG(_L8("Endpoints inactive, do nothing"));
        }
#endif // __FLOG_ACTIVE
    __FLOG(_L8("DataEndpointsStop - Exit"));
    }
    
void CMTPUsbConnection::EndpointStallL(TMTPUsbEndpointId aId)
    {
    __FLOG(_L8("EndpointStallL - Entry"));
    __FLOG_VA((_L8("Creating stall condition on endpoint %d"), aId));
    __ASSERT_DEBUG((aId < EMTPUsbEpNumEndpoints), Panic(EMTPUsbReserved));
    
    // Stall the endpoint.
    CMTPUsbEpBase& ep(*iEndpoints[aId]);
    ep.Stall();
    
    // Update the connection state.
    SetConnectionState(EStalled);
    
    __FLOG(_L8("EndpointStallL - Exit"));
    }
    
void CMTPUsbConnection::EndpointStallClearL(TMTPUsbEndpointId aId)
    {
    __FLOG(_L8("EndpointStallClearL - Entry"));
    __FLOG_VA((_L8("Clearing stall condition on endpoint %d"), aId));
    __ASSERT_DEBUG((aId < EMTPUsbEpNumEndpoints), Panic(EMTPUsbReserved));
    
    // Check the endoints current stall condition.
    CMTPUsbEpBase& ep(*iEndpoints[aId]);
    if ( IsEpStalled( aId ) )
        {
        // Clear the stalled endpoint.
        ep.StallClear();
        
        // Update the device status.
        if ((aId == EMTPUsbEpControl) &&
            ((iControllerStateCurrent == EUsbcDeviceStateAddress) ||
                (iControllerStateCurrent == EUsbcDeviceStateConfigured)))
            {
            // Control endpoint stall cleared on an active connection.    
            InitiateControlRequestSequenceL();
            }
        else if (!IsEpStalled( aId ) )
            {
            // All data endpoint stall conditions are clear.
          	SetConnectionState(EIdle);
            }
        }
    __FLOG(_L8("EndpointStallClearL - Exit"));
    }
       
/**
Resumes USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ResumeConnectionL()
    {
    __FLOG(_L8("ResumeConnectionL - Entry"));
    if (ConnectionOpen())
        {    
        // Restart data transfer activity.
        ControlEndpointStartL();
        DataEndpointsStartL();
        }
    __FLOG(_L8("ResumeConnectionL - Exit"));
    }
       
/**
Initiates USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::StartConnectionL()
    {
    __FLOG(_L8("StartConnectionL - Entry"));   
    
    // Notify the connection manager and update state, if necessary. 
    if (ConnectionClosed())
        {
        __FLOG(_L8("Notifying protocol layer connection opened"));
        iConnectionMgr->ConnectionOpenedL(*this);
        SetConnectionState(EOpen);
        SetDeviceStatus(EMTPUsbDeviceStatusOK);
        InitiateBulkRequestSequenceL();
        }
    __FLOG(_L8("StartConnectionL - Exit"));
    }
       
/**
Halts USB MTP device class processing.
*/ 
TBool CMTPUsbConnection::StopConnection()
    {
    __FLOG(_L8("StopConnection - Entry"));
    
    TBool ret = ETrue;
    // Stop all data transfer activity.
    DataEndpointsStop();    
    
    // Notify the connection manager and update state, if necessary.
    if (ConnectionOpen())
        {
        __FLOG(_L8("Notifying protocol layer connection closed"));
        ret = iConnectionMgr->ConnectionClosed(*this);
        SetBulkTransactionState(EUndefined);
        SetConnectionState(EIdle);
        SetSuspendState(ENotSuspended);
        SetDeviceStatus(EMTPUsbDeviceStatusBusy);
        }
    
    __FLOG(_L8("StopConnection - Exit"));
    
    return ret;
    }
       
/**
Suspends USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::SuspendConnectionL()
    {
    __FLOG(_L8("SuspendConnectionL - Entry"));
    if (ConnectionOpen())
        {    
        // Stop all data transfer activity.
        DataEndpointsStop();
        }
    ControlEndpointStop();

    //  Update state.
    SetSuspendState(ESuspended);
    __FLOG(_L8("SuspendConnectionL - Exit"));
    }

/**
Configures the USB MTP device class.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::StartUsbL()
    {
    __FLOG(_L8("StartUsbL - Entry"));
    
    // Open the USB device interface.
    User::LeaveIfError(iLdd.Open(KDefaultUsbClientController));
    
    // Configure the class descriptors.
    ConfigurationDescriptorSetL();
    SetInterfaceDescriptorL();
    
    __FLOG(_L8("StartUsbL - Exit"));
    }
    
/**
This method stops the end points transfer.
*/
void CMTPUsbConnection::StopUsb()
    {
    __FLOG(_L8("StopUsb - Entry"));
    // Stop monitoring the USB device controller state.
    iLdd.AlternateDeviceStatusNotifyCancel();
    
    // Stop monitoring the USB device controller state.
    Cancel();
    
    // Clear the configuration descriptor.
    ConfigurationDescriptorClear();
    
    // Close the USB device interface.
    iLdd.ReleaseInterface(KMTPUsbAlternateInterface);
    iLdd.Close();
    
    __FLOG(_L8("StopUsb - Exit"));
    }
    
/**
Provides the USB bulk container. 
@return The USB bulk container
*/
CMTPUsbContainer& CMTPUsbConnection::BulkContainer()
	{
	return *iUsbBulkContainer;
	}
	
/**
Provides the current state of the USB MTP device class connection.
@return The current USB MTP device class connection state.
@see TConnectionState
*/
TInt32 CMTPUsbConnection::ConnectionState() const
    {
    TInt32 state(iState & EConnectionStateMask);
    __FLOG_VA((_L8("Connection state = 0x%08X"), state));
    return (state); 
    }

/**
Provides the current USB device suspend state..
@return The current USB device suspend state.
@see TSuspendState
*/
TInt32 CMTPUsbConnection::SuspendState() const
    {
    TInt32 state(iState & ESuspendStateMask);
    __FLOG_VA((_L8("Suspend state = 0x%08X"), state));
    
    return (state);
    }

/**
Sets the bulk transfer transaction state.
@param aState The new connectio state bit flags.
@see TConnectionState
*/
void CMTPUsbConnection::SetBulkTransactionState(TMTPTransactionPhase aState)
    {
    __FLOG(_L8("SetBulkTransactionState - Entry"));
    iBulkTransactionState = aState;
    __FLOG_VA((_L8("SetBulkTransactionState state set to 0x%08X"), iBulkTransactionState));
    __FLOG(_L8("SetBulkTransactionState - Exit"));
    }

/**
Sets the MTP USB device class device status Code value.
@param aCode The PIMA 15740 Response Code or Vendor Code value.
*/
void CMTPUsbConnection::SetDeviceStatus(TUint16 aCode)
    {
    __FLOG(_L8("SetDeviceStatus - Entry"));
    iDeviceStatusCode = aCode;
    __FLOG_VA((_L8("Device status set to 0x%04X"), iDeviceStatusCode));
    __FLOG(_L8("SetDeviceStatus - Exit"));
    }

/**
Sets the USB MTP device class interface descriptor.
@leave KErrCorrupt, if the configuration descriptor size is invalid.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SetInterfaceDescriptorL()
    {
    __FLOG(_L8("SetInterfaceDescriptorL - Entry"));
    
    TUsbcInterfaceInfoBuf ifc;
    // Get device capabilities.
    User::LeaveIfError(iLdd.DeviceCaps(iDeviceCaps));
    
    // Fetch the endpoint capabilities set.
    TPtr8 capsPtr(reinterpret_cast<TUint8*>(iEndpointCapSets), sizeof(iEndpointCapSets), sizeof(iEndpointCapSets));
    User::LeaveIfError(iLdd.EndpointCaps(capsPtr));
    // Set the interface endpoint properties.
    for (TUint i(EMTPUsbEpBulkIn); (i < EMTPUsbEpNumEndpoints); i++)
        {
        // Fetch the endpoint capabilities and meta data.
        const TUsbcEndpointCaps&    caps(EndpointCapsL(i));
        const TEpInfo&              info(iEndpointInfo[i]);
        
        // Define the endpoint properties.
        const TUint idx(EndpointNumber(i) - 1);
        ifc().iEndpointData[idx].iType      	= info.iType;
        ifc().iEndpointData[idx].iFeatureWord1  = KUsbcEndpointInfoFeatureWord1_DMA|KUsbcEndpointInfoFeatureWord1_DoubleBuffering;
        ifc().iEndpointData[idx].iDir       	= info.iDirection;

		//set endpoint maxpacketsize.
        if (info.iType == KMTPUsbInterruptEpType)
            {
            //As default interface, interrupt endpoint maxpacketsize shall be up to 0x40
            ifc().iEndpointData[idx].iSize      = KMaxPacketTypeInterrupt;
            }
        else
            {
            ifc().iEndpointData[idx].iSize      = caps.MaxPacketSize();
            }
			
        ifc().iEndpointData[idx].iInterval  	= info.iInterval;
        ifc().iEndpointData[idx].iInterval_Hs 	= info.iInterval_Hs;
        }
        
    // Set the required interface descriptor values.
    ifc().iString               = const_cast<TDesC16*>(&KMTPUsbInterfaceString);
    ifc().iClass.iClassNum      = KMTPUsbInterfaceClassSIC;
	ifc().iClass.iSubClassNum   = KMTPUsbInterfaceSubClassSIC;
	ifc().iClass.iProtocolNum   = KMTPUsbInterfaceProtocolSIC;
    ifc().iTotalEndpointsUsed   = KMTPUsbRequiredNumEndpoints;
    
    // Allocate 512KB buffer for OUT EndPoint, and 64KB for IN EndPoint
    TUint32 bandwidthPriority = EUsbcBandwidthINPlus2 | EUsbcBandwidthOUTMaximum;
        
    // Write the active interface descriptor.
    User::LeaveIfError(iLdd.SetInterface(KMTPUsbAlternateInterface, ifc, bandwidthPriority));
        
    __FLOG(_L8("SetInterfaceDescriptorL - Exit"));
    }

/**
Sets the USB MTP device class connection state.
@param aState The new connection state bit flags.
@see TConnectionState
*/
void CMTPUsbConnection::SetConnectionState(TInt32 aState)
    {
    __FLOG(_L8("SetConnectionState - Entry"));
    iState = ((~EConnectionStateMask & iState) | aState);
    __FLOG_VA((_L8("Connection state set to 0x%08X"), iState));
    __FLOG(_L8("SetConnectionState - Exit"));
    }

/**
Sets the USB device suspend state.
@param aState The new suspend state bit flags.
@see TSuspendState
*/
void CMTPUsbConnection::SetSuspendState(TInt32 aState)
    {
    __FLOG(_L8("SetSuspendState - Entry"));
    iState = ((~ESuspendStateMask & iState) | aState);
    __FLOG_VA((_L8("Connection state set to 0x%08X"), iState));
    __FLOG(_L8("SetSuspendState - Exit"));
    }
    
/**
This method is called when EUsbcDeviceStateAddress state is reached. 
It sets the transport packet size based on the host type the device is
connected to.
*/
void CMTPUsbConnection::SetTransportPacketSizeL()
	{
	__FLOG(_L8("SetTransportPacketSizeL - Entry"));
	if(iLdd.CurrentlyUsingHighSpeed())
		{
		__FLOG(_L8("HS USB connection"));
		iEndpoints[EMTPUsbEpControl]->SetMaxPacketSizeL(KMaxPacketTypeControlHS);
		iEndpoints[EMTPUsbEpBulkIn]->SetMaxPacketSizeL(KMaxPacketTypeBulkHS);
		iEndpoints[EMTPUsbEpBulkOut]->SetMaxPacketSizeL(KMaxPacketTypeBulkHS);
		}
	else
		{
		__FLOG(_L8("FS USB connection"));
		iEndpoints[EMTPUsbEpControl]->SetMaxPacketSizeL(KMaxPacketTypeControlFS);
		iEndpoints[EMTPUsbEpBulkIn]->SetMaxPacketSizeL(KMaxPacketTypeBulkFS);
		iEndpoints[EMTPUsbEpBulkOut]->SetMaxPacketSizeL(KMaxPacketTypeBulkFS);
		}		
	iEndpoints[EMTPUsbEpInterrupt]->SetMaxPacketSizeL(KMaxPacketTypeInterrupt);
	
	__FLOG(_L8("SetTransportPacketSizeL - Exit"));
	}

TBool CMTPUsbConnection::IsEpStalled(const TUint& aEpNumber)
    {
    const TEndpointNumber number( static_cast<TEndpointNumber>(aEpNumber) );
    TEndpointState state;
    iLdd.EndpointStatus(number, state);
    return ( EEndpointStateStalled == state );
    }
