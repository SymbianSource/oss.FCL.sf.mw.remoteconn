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

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbconnectionTraces.h"
#endif

#define UNUSED_VAR(a) (a)=(a)


// File type constants.
const TInt KMTPNullChunkSize(0x00020000); // 100KB
const TUint KUSBHeaderSize = 12;

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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CMTPUSBCONNECTION_DES_ENTRY );
    
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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_CMTPUSBCONNECTION_DES_EXIT );
    }

void CMTPUsbConnection::BindL(MMTPConnectionProtocol& aProtocol)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BINDL_ENTRY );
    __ASSERT_DEBUG(!iProtocolLayer, Panic(EMTPUsbBadState));
    iProtocolLayer = &aProtocol;
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BINDL_EXIT );
    }
    
MMTPConnectionProtocol& CMTPUsbConnection::BoundProtocolLayer()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BOUNDPROTOCOLLAYER_ENTRY );
    __ASSERT_DEBUG(iProtocolLayer, Panic(EMTPUsbBadState));
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BOUNDPROTOCOLLAYER_EXIT );
    return *iProtocolLayer;
    }
    
void CMTPUsbConnection::CloseConnection()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CLOSECONNECTION_ENTRY );
    /* 
    Terminate all endpoint data transfer activity, stall all but the control 
    endpoints, and wait for the host to issue a Device Reset Request.
    */
    StopConnection();
    TRAPD(err, BulkEndpointsStallL());
    UNUSED_VAR(err);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_CLOSECONNECTION_EXIT );
    }
    
void CMTPUsbConnection::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RECEIVEDATAL_ENTRY );
    
    // Update the transaction state.
    SetBulkTransactionState(EDataIToRPhase);
    
    if (iIsCancelReceived) //cancel received
        {
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_RECEIVEDATAL, "Transaction has been cancelled, just flush trash data and complete" );
        static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->FlushRxDataL();
        BoundProtocolLayer().ReceiveDataCompleteL(KErrCancel, aData, iMTPRequest);
        }
    else
        {  
        // Setup the bulk container and initiate the bulk data receive sequence.
        iUsbBulkContainer->SetPayloadL(&aData);
        
        //Expected containerType pre-setup here in case we don't receive IToR dataphase at all so 
        //Cancel operation can trigger right call inside ReceiveBulkDataCompleteL(). 
        iUsbBulkContainer->SetUint16L(CMTPUsbContainer::EContainerType, EMTPUsbContainerTypeDataBlock);
        
        static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->ReceiveBulkDataL(*iUsbBulkContainer);
        }
         
    OstTraceFunctionExit0( CMTPUSBCONNECTION_RECEIVEDATAL_EXIT );
    }

void CMTPUsbConnection::ReceiveDataCancelL(const TMTPTypeRequest& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RECEIVEDATACANCELL_ENTRY );
    
    // Store the device status code.
    TUint16 deviceStatus = iDeviceStatusCode;
    
    SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
   	static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut])->CancelReceiveL(KErrCancel);
   	
   	// Restore it.
   	SetDeviceStatus(deviceStatus);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_RECEIVEDATACANCELL_EXIT );
    }

void CMTPUsbConnection::SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDDATAL_ENTRY ); 
    ProcessBulkDataInL(aRequest, aData);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDDATAL_EXIT );
    }

void CMTPUsbConnection::SendDataCancelL(const TMTPTypeRequest& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDDATACANCELL_ENTRY );
    // Store the device status code.
    TUint16 deviceStatus = iDeviceStatusCode;
    
    SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
 	static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->CancelSendL(KErrCancel);
 	// Restore it.
   	SetDeviceStatus(deviceStatus);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDDATACANCELL_EXIT );
    }
        
void CMTPUsbConnection::SendEventL(const TMTPTypeEvent& aEvent)
    {    
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDEVENTL_ENTRY );
    
    // Reset the event.
    iMTPEvent.Reset(); 
    MMTPType::CopyL(aEvent, iMTPEvent);
    
    switch (ConnectionState())
        {
    case EIdle:
    case EStalled:
        // Drop the event.    
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_SENDEVENTL, "Dropping the event" );
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
	            OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_SENDEVENTL, "Sending the event" );
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
                OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_SENDEVENTL, "Buffer event data and signal remote wakeup" );
                BufferEventDataL(aEvent);
                }
            else
                {
                // Remote wakeup is not enabled, drop the event.    
                OstTrace0( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_SENDEVENTL, "Dropping the event" );
                BoundProtocolLayer().SendEventCompleteL(KErrNone, aEvent);
                }
            
            /*
            Update state to trigger the correct processing when the USB connection is resumed.
            */
            SetSuspendState(ESuspendedEventsPending);
            break;
                
        case ESuspendedEventsPending:
            // Drop the event.    
            OstTrace0( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_SENDEVENTL, "Dropping the event" );
            BoundProtocolLayer().SendEventCompleteL(KErrNone, aEvent);
            break; 
              
        default:
            OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP5_CMTPUSBCONNECTION_SENDEVENTL, "Invalid suspend state %d", SuspendState() );
            Panic(EMTPUsbBadState);
            break;
            }
        break;
                
    default:
        OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP6_CMTPUSBCONNECTION_SENDEVENTL, "Invalid connection state %d", ConnectionState() );
        Panic(EMTPUsbBadState);
        break;
        }

    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDEVENTL_EXIT );
    }
    
void CMTPUsbConnection::SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDRESPONSEL_ENTRY );
    OstTraceExt3( TRACE_NORMAL, CMTPUSBCONNECTION_SENDRESPONSEL, 
            "DeviceState: 0x%x TransactionState: 0x%x Connection: 0x%x", (TUint32)iDeviceStatusCode, iBulkTransactionState, ConnectionState() );
    
    // Update the transaction state.
	SetBulkTransactionState(EResponsePhase);
    if (SuspendState() != ESuspended && !iIsCancelReceived)
    	{  	      
   		TUint16 opCode(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
   		TUint16 rspCode(aResponse.Uint16(TMTPTypeResponse::EResponseCode));
   		OstTraceExt2( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_SENDRESPONSEL, 
   		        "ResponseCode = 0x%04X, Operation Code = 0x%04X", rspCode, opCode );

   		if ((opCode == EMTPOpCodeOpenSession) && (rspCode == EMTPRespCodeOK))
        	{        
   	    	// An session has been opened. Record the active SessionID.
       		iMTPSessionId = aRequest.Uint32(TMTPTypeRequest::ERequestParameter1);
       		OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_SENDRESPONSEL, 
       		        "Processing OpenSession response, SessionID = %d", iMTPSessionId );
       		}
   		else if (((opCode == EMTPOpCodeCloseSession) || (opCode == EMTPOpCodeResetDevice))&& (rspCode == EMTPRespCodeOK))
        	{
   	    	// An session has been closed. Clear the active SessionID.        
            OstTrace1( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_SENDRESPONSEL, 
                    "Processing CloseSession or ResetDevice response, SessionID = %d", iMTPSessionId );
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
   		OstTraceExt2( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_SENDRESPONSEL, 
   		        "Sending response 0x%04X (%d bytes)", (TUint32)iUsbBulkContainer->Uint16L(CMTPUsbContainer::ECode), iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength));
   		static_cast<CMTPUsbEpBulkIn*>(iEndpoints[EMTPUsbEpBulkIn])->SendBulkDataL(*iUsbBulkContainer);
    	}
    else
    	{
    	BoundProtocolLayer().SendResponseCompleteL(KErrNone, aResponse, aRequest);
    	}

    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDRESPONSEL_EXIT );
    } 
    
void CMTPUsbConnection::TransactionCompleteL(const TMTPTypeRequest& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_TRANSACTIONCOMPLETEL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPUSBCONNECTION_TRANSACTIONCOMPLETEL, 
            "DeviceState: 0x%x TransactionState: 0x%x", iDeviceStatusCode, iBulkTransactionState );
   
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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_TRANSACTIONCOMPLETEL_EXIT );
    } 

void CMTPUsbConnection::Unbind(MMTPConnectionProtocol& /*aProtocol*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_UNBIND_ENTRY );
    __ASSERT_DEBUG(iProtocolLayer, Panic(EMTPUsbBadState));
    iProtocolLayer = NULL;
    OstTraceFunctionExit0( CMTPUSBCONNECTION_UNBIND_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RECEIVEBULKDATACOMPLETEL_ENTRY );
    if (!BulkRequestErrorHandled(aError))
        { 
        TUint type(iUsbBulkContainer->Uint16L(CMTPUsbContainer::EContainerType));        
	    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_RECEIVEBULKDATACOMPLETEL, 
	            "Received container type 0x%04X", type );

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
       		OstTrace1( TRACE_ERROR, DUP1_CMTPUSBCONNECTION_RECEIVEBULKDATACOMPLETEL, 
       		        "ReceiveBulkDataCompleteL - error: %d", aError );
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
            	OstTrace1( TRACE_ERROR, DUP2_CMTPUSBCONNECTION_RECEIVEBULKDATACOMPLETEL, 
            	        "Invalid container type = 0x%04X", type );
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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_RECEIVEBULKDATACOMPLETEL_EXIT );
    }
    
void CMTPUsbConnection::ReceiveControlRequestDataCompleteL(TInt aError, MMTPType& /*aData*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL_ENTRY );
    if (!ControlRequestErrorHandled(aError))
        {
        // Complete the control request sequence.
        static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->SendControlRequestStatus();
          
        if (iUsbControlRequestSetup.Uint8(TMTPUsbControlRequestSetup::EbRequest) == EMTPUsbControlRequestCancel)
            {
            // Cancel data received.
            OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, "Cancel request data received." );

            // Setup the event dataset.
                        OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                    "Cancellation Code = 0x%04X", iUsbControlRequestCancelData.Uint16(TMTPUsbControlRequestCancelData::ECancellationCode));
            OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                    "Transaction ID = 0x%08X", iUsbControlRequestCancelData.Uint32(TMTPUsbControlRequestCancelData::ETransactionID));
  
			#ifdef _DEBUG            
            // print log about the cacel event
            OstTrace1( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                    "cancel event received!!!!!!!!!!!!!!!!!Transaction phase is %d ----------------", 
                    BoundProtocolLayer().TransactionPhaseL(iMTPSessionId));
            OstTrace1( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                    "The Transaction ID want to canceled is %d -------------", 
                    iUsbControlRequestCancelData.Uint32(TMTPUsbControlRequestCancelData::ETransactionID));
            OstTrace1( TRACE_NORMAL, DUP5_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                    "Current Transaction ID is %d ----------------", iMTPRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));
			#endif
            
            isResponseTransactionCancelledNeeded = true;
            TMTPTransactionPhase transPhase = BoundProtocolLayer().TransactionPhaseL(iMTPSessionId);
            if( transPhase > EIdlePhase && transPhase < ECompletingPhase ) 
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
            else if (transPhase == ECompletingPhase)
                {
                OstTrace0( TRACE_NORMAL, DUP6_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                        "cancel event received at completing phase, flush rx data" );

                //flush rx data.
                iEndpoints[EMTPUsbEpBulkOut]->FlushRxDataL();
                }
            else
            	{
                OstTrace0( TRACE_NORMAL, DUP7_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL, 
                        "cancel event received at idle phase, stop data EPs, flush rx data, restart data eps" );

            	// stop data endpoint
            	DataEndpointsStop();
      
                //flush rx data.
                iEndpoints[EMTPUsbEpBulkOut]->FlushRxDataL();
                
            	// initiate bulk request sequence.
            	InitiateBulkRequestSequenceL();   
            	
                SetDeviceStatus(EMTPUsbDeviceStatusOK);
            	}
            }

        // Initiate the next control request sequence.     
        InitiateControlRequestSequenceL();
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_RECEIVECONTROLREQUESTDATACOMPLETEL_EXIT );
    }
   
void CMTPUsbConnection::ReceiveControlRequestSetupCompleteL(TInt aError, MMTPType& aData)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RECEIVECONTROLREQUESTSETUPCOMPLETEL_ENTRY );
    if (!ControlRequestErrorHandled(aError))
        {
        TMTPUsbControlRequestSetup& data(static_cast<TMTPUsbControlRequestSetup&> (aData));
        OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_RECEIVECONTROLREQUESTSETUPCOMPLETEL, 
                "bRequest = 0x%X", data.Uint8(TMTPUsbControlRequestSetup::EbRequest) );
        OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTSETUPCOMPLETEL, 
                "wLength = %d bytes", data.Uint16(TMTPUsbControlRequestSetup::EwLength));
        
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
            OstTrace0( TRACE_ERROR, DUP2_CMTPUSBCONNECTION_RECEIVECONTROLREQUESTSETUPCOMPLETEL, 
                    "Unrecognised class specific request received" );
            CloseConnection();
            break;
            }
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_RECEIVECONTROLREQUESTSETUPCOMPLETEL_EXIT );
    }
    
void CMTPUsbConnection::SendBulkDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDBULKDATACOMPLETEL_ENTRY );

    if (!BulkRequestErrorHandled(aError))
        {
        TUint16 containerType(iUsbBulkContainer->Uint16L(CMTPUsbContainer::EContainerType));

#ifdef _DEBUG              
        TUint16 transactionID(iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
        OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SENDBULKDATACOMPLETEL, "Time Stamp is :%d", User::TickCount() );
        OstTraceExt2( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_SENDBULKDATACOMPLETEL, 
                "the container Type is 0x%x, the transaction ID is 0x%x", containerType,transactionID );
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
	        OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP2_CMTPUSBCONNECTION_SENDBULKDATACOMPLETEL, 
	                "Invalid container type %d", containerType );
            Panic(EMTPUsbBadState);
	        }
             
        // Reset the bulk container.
        if(ERequestPhase != iBulkTransactionState)
            {
    		iUsbBulkContainer->SetPayloadL(NULL);
            }		     
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDBULKDATACOMPLETEL_EXIT );
    }
    
void CMTPUsbConnection::SendControlRequestDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDCONTROLREQUESTDATACOMPLETEL_ENTRY );
    if (!ControlRequestErrorHandled(aError))
        {
        // Complete the control request sequence.
        if (iUsbControlRequestSetup.Uint8(TMTPUsbControlRequestSetup::EbRequest) == EMTPUsbControlRequestCancel)
            {
            // Cancel request processed, clear the device status.
            SetDeviceStatus(EMTPUsbDeviceStatusOK);
            OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_SENDCONTROLREQUESTDATACOMPLETEL, "Cancel Request processed" );
            }
        
        // Initiate the next control request sequence. 
        InitiateControlRequestSequenceL();            
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDCONTROLREQUESTDATACOMPLETEL_EXIT );
    }

void CMTPUsbConnection::SendInterruptDataCompleteL(TInt aError, const MMTPType& /*aData*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDINTERRUPTDATACOMPLETEL_ENTRY );
    iEventPending = EFalse;	
    
    if ( NULL != iProtocolLayer)
        {
    BoundProtocolLayer().SendEventCompleteL(aError, iMTPEvent);
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDINTERRUPTDATACOMPLETEL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_ENDPOINTCAPSL_ENTRY );
    
    // Verify the the USB device supports the minimum number of endpoints.
    TInt totalEndpoints = iDeviceCaps().iTotalEndpoints;
    
    OstTraceExt2( TRACE_NORMAL, CMTPUSBCONNECTION_ENDPOINTCAPSL, 
            "% d endpoints available, %d required", totalEndpoints, KMTPUsbRequiredNumEndpoints );
    if (totalEndpoints < KMTPUsbRequiredNumEndpoints)
        {
        OstTrace0( TRACE_ERROR, DUP1_CMTPUSBCONNECTION_ENDPOINTCAPSL, 
                "Count of endpoints smaller than the minimum number of endpoints required to support the USM MTP device" );
        User::Leave(KErrOverflow);            
        }      
        
    TUint   flags(EndpointDirectionAndType(aId));
    OstTraceExt2( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_ENDPOINTCAPSL, "Required EP%d iTypesAndDir = 0x%X", aId, flags );

    TBool   found(EFalse);
    for (TUint i(0); ((!found) && (i < totalEndpoints)); i++)
        {
        TUsbcEndpointCaps&  caps(iEndpointCapSets[i].iCaps);
        
        if ((caps.iTypesAndDir & flags) == flags)
            {
            found           = ETrue;
            iEndpointCaps   = caps;

            OstTraceExt2( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_ENDPOINTCAPSL, 
                    "Matched EP%d iTypesAndDir = 0x%X", i, caps.iTypesAndDir );
            OstTraceExt2( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_ENDPOINTCAPSL, 
                    "Matched EP%d MaxPacketSize = %d", i, caps.MaxPacketSize() );
            OstTraceExt2( TRACE_NORMAL, DUP5_CMTPUSBCONNECTION_ENDPOINTCAPSL, 
                    "Matched EP%d MinPacketSize = %d", i, caps.MinPacketSize() );
            }
        }
        
    if (!found)    
        {
        OstTrace1( TRACE_ERROR, DUP6_CMTPUSBCONNECTION_ENDPOINTCAPSL, "Can't find endpoint capabilities for flags %d", flags );
        User::Leave(KErrHardwareNotAvailable);
        }

    OstTraceFunctionExit0( CMTPUSBCONNECTION_ENDPOINTCAPSL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_DOCANCEL_ENTRY );
    iLdd.AlternateDeviceStatusNotifyCancel();
    OstTraceFunctionExit0( CMTPUSBCONNECTION_DOCANCEL_EXIT );
    }
    
#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPUsbConnection::RunError(TInt aError)
#else
TInt CMTPUsbConnection::RunError(TInt /*aError*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RUNERROR_ENTRY );
    OstTraceDef1( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, 
            CMTPUSBCONNECTION_RUNERROR, "error code %d", aError);
    
    // Cancel all the outstanding requests.
    Cancel();   
    
    // Stop the connection, if necessary.
    StopConnection(); 
    
    // Stop the control end point.
    ControlEndpointStop();
    
    // Issue the notify request again.
    IssueAlternateDeviceStatusNotifyRequest();

    OstTraceFunctionExit0( CMTPUSBCONNECTION_RUNERROR_EXIT );
    return KErrNone;
    }
    
void CMTPUsbConnection::RunL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RUNL_ENTRY );
    
    if (!(iControllerStateCurrent & KUsbAlternateSetting))
        {
        // Alternative interface setting has not changed.
        OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_RUNL, 
                "Alternate device state changed to %d", iControllerStateCurrent );
        
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
				OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_RUNL, "Device state : EUsbcDeviceStateConfigured" );

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
                            OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_RUNL, "Sending buffered event data" );
                            SendEventDataL();
                            }
                        else
                            {
                            // Send PTP UnreportedStatus event
                            OstTrace0( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_RUNL, "Sending PTP UnreportedStatus event" );
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
                OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_FATAL, DUP4_CMTPUSBCONNECTION_RUNL, 
                        "Invalid alternate device state %d", iControllerStateCurrent );
                Panic(EMTPUsbBadState);
                break;
            }
        }
    else
        {
        // Alternate interface setting has changed.
        OstTraceExt2( TRACE_NORMAL, DUP5_CMTPUSBCONNECTION_RUNL, 
                "Alternate interface setting changed from %d to %d", iControllerStatePrevious, iControllerStateCurrent );
        }
        
        
    // Record the controller state and issue the next notification request.
    iControllerStatePrevious = iControllerStateCurrent;
    IssueAlternateDeviceStatusNotifyRequest();

    OstTraceFunctionExit0( CMTPUSBCONNECTION_RUNL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONSTRUCTL_ENTRY );
    
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
    LEAVEIFERROR(iLdd.DeviceCaps(dCaps),
            OstTrace0( TRACE_ERROR, DUP1_CMTPUSBCONNECTION_CONSTRUCTL, "Retrieves the capabilities of the USB device failed!" ));
            
    iRemoteWakeup = dCaps().iRemoteWakeup;
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_CONSTRUCTL, "iRemote = %d", iRemoteWakeup );

    // Start monitoring the USB device controller state.
    IssueAlternateDeviceStatusNotifyRequest();

    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONSTRUCTL_EXIT );
    }

/**
Issues a request for notification of USB device controller state and alternate
interface setting changes.
*/
void CMTPUsbConnection::IssueAlternateDeviceStatusNotifyRequest()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_ISSUEALTERNATEDEVICESTATUSNOTIFYREQUEST_ENTRY );
    
    if (!IsActive())
        {
        iLdd.AlternateDeviceStatusNotify(iStatus, iControllerStateCurrent);
        }

    SetActive();
    OstTraceFunctionExit0( CMTPUSBCONNECTION_ISSUEALTERNATEDEVICESTATUSNOTIFYREQUEST_EXIT );
    }
    
/**
Populates the asynchronous event interrupt dataset buffer.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::BufferEventDataL(const TMTPTypeEvent& aEvent)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BUFFEREVENTDATAL_ENTRY );
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
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BUFFEREVENTDATAL_EXIT );
    }
    
/**
Initiates an interrupt data send sequence.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SendEventDataL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDEVENTDATAL_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPUSBCONNECTION_SENDEVENTDATAL, 
            "Sending event 0x%4X (%d bytes)", (TUint32)iUsbEventContainer->Uint16L(CMTPUsbContainer::ECode), 
            iUsbEventContainer->Uint32L(CMTPUsbContainer::EContainerLength) );
    
    static_cast<CMTPUsbEpInterruptIn*>(iEndpoints[EMTPUsbEpInterrupt])->SendInterruptDataL(*iUsbEventContainer);
    iEventPending = ETrue;
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDEVENTDATAL_EXIT );
    }
    
/**
Issues a PTP UnreportedStatus event.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SendUnreportedStatusEventL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SENDUNREPORTEDSTATUSEVENTL_ENTRY );
        
    // Construct an UnreportedStatus event
    TMTPTypeEvent mtpEvent;
    mtpEvent.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeUnreportedStatus);
    mtpEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionNone);
    mtpEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
    SendEventL(mtpEvent);

    OstTraceFunctionExit0( CMTPUSBCONNECTION_SENDUNREPORTEDSTATUSEVENTL_EXIT );
    }
    
/**
Issues a request to the bulk-out endpoint to receive a command block dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::InitiateBulkRequestSequenceL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_INITIATEBULKREQUESTSEQUENCEL_ENTRY );
    CMTPUsbEpBulkOut& bulkOut(*static_cast<CMTPUsbEpBulkOut*>(iEndpoints[EMTPUsbEpBulkOut]));

        // Update the transaction state.
        SetBulkTransactionState(ERequestPhase);
        
        // Setup the bulk container.
        iUsbBulkParameterBlock.Reset();
        iUsbBulkContainer->SetPayloadL(&iUsbBulkParameterBlock);
        
        // Initiate the next request phase bulk data receive sequence.
        bulkOut.ReceiveBulkDataL(*iUsbBulkContainer);

    OstTraceFunctionExit0( CMTPUSBCONNECTION_INITIATEBULKREQUESTSEQUENCEL_EXIT );
    }

/**
Issues a request to the control endpoint to receive a request setup dataset.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::InitiateControlRequestSequenceL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_INITIATECONTROLREQUESTSEQUENCEL_ENTRY );
    CMTPUsbEpControl& ctrl(*static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl]));
    if (!ctrl.Stalled())
        {

        ctrl.ReceiveControlRequestSetupL(iUsbControlRequestSetup);
        }     
    OstTraceFunctionExit0( CMTPUSBCONNECTION_INITIATECONTROLREQUESTSEQUENCEL_EXIT );
    }

/**
Processes received USB SIC bulk command block containers received from the 
connected host on the bulk out data pipe.
@param aError The error completion status of the bulk data receive request.
@leave One of the system wide error codes, if a processing failure occurs.
*/         
#ifdef OST_TRACE_COMPILER_IN_USE
void CMTPUsbConnection::ProcessBulkCommandL(TInt aError)
#else
void CMTPUsbConnection::ProcessBulkCommandL(TInt /*aError*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSBULKCOMMANDL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, "aError = %d", aError );

    if (BulkRequestTransactionStateValid(ERequestPhase))
        {
        // Request block received.
        TUint16 op(iUsbBulkContainer->Uint16L(CMTPUsbContainer::ECode));
   	    OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, 
   	            "Command block 0x%04X received", op );

       	// Reset the iMTPRequest.
       	iMTPRequest.Reset();

       	// Setup the MTP request dataset buffer. Set Operation Code and TransactionID
       	iMTPRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, op);
       	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestTransactionID, iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
        
       	// Set SessionID.
       	if (op == EMTPOpCodeOpenSession)
           	{
           	OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, "Processing OpenSession request" );
           	// Force OpenSession requests to be processed outside an active session.
           	// It is a known problem for MTP Protocol, it is a workaround here.
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, KMTPSessionNone);  
           	}
       	else if (op == EMTPOpCodeCloseSession || op == EMTPOpCodeResetDevice)
           	{
           	OstTrace0( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, "Processing CloseSession or the ResetDevice request" );
           	// Force CloseSession requests to be processed outside an active session. 
           	// ResetDevice currently behaves the same way as CloseSession. 
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, KMTPSessionNone); 
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestParameter1, iMTPSessionId); 
           	}       	
       	else
           	{
           	OstTrace1( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, "Processing general request on session %d", iMTPSessionId );
           	// Update the request dataset with the single active session's SessionID.
           	iMTPRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iMTPSessionId);
           	}
       	
#ifdef _DEBUG
       	OstTrace1( TRACE_NORMAL, DUP5_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, "The time stamp is: %d", User::TickCount() );
       	OstTraceExt2( TRACE_NORMAL, DUP6_CMTPUSBCONNECTION_PROCESSBULKCOMMANDL, 
       	        "New command comes, Operation code is 0x%x, transaction id is %d", (TUint32)op, iUsbBulkContainer->Uint32L(CMTPUsbContainer::ETransactionID));
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
    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSBULKCOMMANDL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSBULKDATAINL_ENTRY );
    
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
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_PROCESSBULKDATAINL, 
            "Sending %d data bytes", iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength) );
    
#ifdef _DEBUG
    OstTraceExt2( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_PROCESSBULKDATAINL, 
            "ProcessBulkDataInL: iIsCancelReceived = %d, SuspendState() is  %d", (TInt32)iIsCancelReceived,SuspendState() );
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
    	OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_PROCESSBULKDATAINL, 
    	        "the senddata is canceled!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
#endif    	
    	// by pass the calling to lower level send data
    	SendBulkDataCompleteL(KErrNone, *iUsbBulkContainer);
    	
    	}
    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSBULKDATAINL_EXIT );
    }

/**
Processes received USB SIC bulk data block containers received from the 
connected host on the bulk out data pipe.
@param aError The error completion status of the bulk data receive request.
@leave One of the system wide error codes, if a processing failure occurs.
*/       
void CMTPUsbConnection::ProcessBulkDataOutL(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSBULKDATAOUTL_ENTRY );
    if ((BulkRequestTransactionStateValid(EDataIToRPhase)))
        {
        // Data block received, notify the protocol layer.
        OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_PROCESSBULKDATAOUTL, 
                "Data block received (%d bytes)", iUsbBulkContainer->Uint32L(CMTPUsbContainer::EContainerLength) );
        BoundProtocolLayer().ReceiveDataCompleteL(aError, *iUsbBulkContainer->Payload(), iMTPRequest);
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSBULKDATAOUTL_EXIT );
    }

/**
Processes received USB SIC class specific Cancel requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/   
void CMTPUsbConnection::ProcessControlRequestCancelL(const TMTPUsbControlRequestSetup& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTCANCELL_ENTRY );
    static_cast<CMTPUsbEpControl*>(iEndpoints[EMTPUsbEpControl])->ReceiveControlRequestDataL(iUsbControlRequestCancelData);
    OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_PROCESSCONTROLREQUESTCANCELL, "Waiting for Cancel Request Data" );
    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTCANCELL_EXIT );
    }
    
/**
Processes received USB SIC class specific Device Reset requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ProcessControlRequestDeviceResetL(const TMTPUsbControlRequestSetup& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICERESETL_ENTRY );
    
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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICERESETL_EXIT );
    }
    
/**
Processes received USB SIC class specific Get Device Status requests
@param aRequest The USB SIC class specific request setup data.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ProcessControlRequestDeviceStatusL(const TMTPUsbControlRequestSetup& /*aRequest*/)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICESTATUSL_ENTRY );
    iUsbControlRequestDeviceStatus.Reset();
    
    TUint offset = 0;
    for(TUint i(EMTPUsbEpControl); i<EMTPUsbEpNumEndpoints; ++i)
    	{        
        if ( IsEpStalled(i) )
            {
            TInt epSize(0);
            LEAVEIFERROR( iLdd.GetEndpointDescriptorSize(KMTPUsbAlternateInterface, i, epSize),
                    OstTrace1( TRACE_ERROR, CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICESTATUSL, 
                            "Copies the size of the endpoint descriptor failed! error code %d", munged_err));
                    
            
            RBuf8 epDesc; //endpoint descriptor, epDesc[2] is the address of endpoint. More info, pls refer to USB Sepc2.0 - 9.6.6
            epDesc.CreateL(epSize);
            CleanupClosePushL(epDesc); 
            LEAVEIFERROR( iLdd.GetEndpointDescriptor(KMTPUsbAlternateInterface, i, epDesc),
                    OstTrace1( TRACE_ERROR, DUP1_CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICESTATUSL, 
                            "Copies Copies the endpoint descriptor failed! error code %d", munged_err));

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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_PROCESSCONTROLREQUESTDEVICESTATUSL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BULKREQUESTERRORHANDLED_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_BULKREQUESTERRORHANDLED, "Bulk transfer request completion status = %d", aError );
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
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BULKREQUESTERRORHANDLED_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BULKREQUESTTRANSACTIONSTATEVALID_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_BULKREQUESTTRANSACTIONSTATEVALID, 
            "Bulk transaction state = %d", iBulkTransactionState);
    
    TBool valid(iBulkTransactionState == aExpectedTransactionState);
    if (!valid)
        {
        // Invalid bulk transaction state, close the connection.
        OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_BULKREQUESTTRANSACTIONSTATEVALID, 
                "Expected bulk transaction state = %d", aExpectedTransactionState );

        //if transaction is in request phase, while the container type of data we received is other transaction phase,
        //just ignore the data and initiate another request receiving. 
        if (ERequestPhase == iBulkTransactionState)
            {
            InitiateBulkRequestSequenceL();
            }
        else
            {
        CloseConnection();
            }
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BULKREQUESTTRANSACTIONSTATEVALID_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONTROLREQUESTERRORHANDLED_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_CONTROLREQUESTERRORHANDLED, 
            "Control request completion status = %d", aError );
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
        case KErrUsbEpNotReady:
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
    
    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONTROLREQUESTERRORHANDLED_EXIT );
    return ret;
    }

/**
Clears the USB MTP device class configuration descriptor.
*/
void CMTPUsbConnection::ConfigurationDescriptorClear()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORCLEAR_ENTRY );
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

    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORCLEAR_EXIT );
    }

/**
Sets the USB MTP device class configuration descriptor.
@leave KErrCorrupt, if the configuration descriptor size is invalid.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::ConfigurationDescriptorSetL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORSETL_ENTRY );
    const TInt KNumInterfacesOffset(4);
    
    TInt descriptorSize(0);
    iLdd.GetConfigurationDescriptorSize(descriptorSize);
    
    if (static_cast<TUint>(descriptorSize) != KUsbDescSize_Config)
        {
        OstTrace0( TRACE_ERROR, CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORSETL, "Not configration descriptor!");
        User::Leave(KErrCorrupt);
        }
 
    TBuf8<KUsbDescSize_Config> descriptor;
    LEAVEIFERROR(iLdd.GetConfigurationDescriptor(descriptor),
            OstTrace1( TRACE_ERROR, DUP1_CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORSETL, 
                    "Copies the current configuration descriptor failed! error code %d", munged_err));
    ++descriptor[KNumInterfacesOffset];
    LEAVEIFERROR(iLdd.SetConfigurationDescriptor(descriptor),
            OstTrace1( TRACE_ERROR, DUP2_CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORSETL, 
                    "Sets the current configuration descriptor failed! error code %d", munged_err));

    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONFIGURATIONDESCRIPTORSETL_EXIT );
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
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONTROLENDPOINTSTARTL_ENTRY );
    InitiateControlRequestSequenceL();
    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONTROLENDPOINTSTARTL_EXIT );
    }

/**
Stops data transfer activity on the control endpoint.
*/    
void CMTPUsbConnection::ControlEndpointStop()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_CONTROLENDPOINTSTOP_ENTRY );
    iEndpoints[EMTPUsbEpControl]->Cancel();
    OstTraceFunctionExit0( CMTPUSBCONNECTION_CONTROLENDPOINTSTOP_EXIT );
    }

/**
Stalls all but the control endpoint.
*/    
void CMTPUsbConnection::BulkEndpointsStallL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BULKENDPOINTSSTALLL_ENTRY );
    EndpointStallL(EMTPUsbEpBulkIn);
    EndpointStallL(EMTPUsbEpBulkOut);
    SetDeviceStatus(EMTPUsbDeviceStatusTransactionCancelled);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BULKENDPOINTSSTALLL_EXIT );
    }

/**
Clears stall conditions all but the control endpoint.
*/
void CMTPUsbConnection::BulkEndpointsStallClearL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_BULKENDPOINTSSTALLCLEARL_ENTRY );
    EndpointStallClearL(EMTPUsbEpBulkIn);
    EndpointStallClearL(EMTPUsbEpBulkOut);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_BULKENDPOINTSSTALLCLEARL_EXIT );
    }

/**
Starts data transfer activity on the data endpoints.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPUsbConnection::DataEndpointsStartL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_DATAENDPOINTSSTARTL_ENTRY );
    InitiateBulkRequestSequenceL();
    OstTraceFunctionExit0( CMTPUSBCONNECTION_DATAENDPOINTSSTARTL_EXIT );
    }

/**
Stops data transfer activity on all but the control endpoint.
*/    
void CMTPUsbConnection::DataEndpointsStop()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_DATAENDPOINTSSTOP_ENTRY );
    if (ConnectionOpen() && (!(SuspendState() & ESuspended))&& (iControllerStatePrevious != EUsbcDeviceStateSuspended))
        {
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_DATAENDPOINTSSTOP, "Stopping active endpoints" );
        iEndpoints[EMTPUsbEpBulkIn]->Cancel();
        iEndpoints[EMTPUsbEpBulkOut]->Cancel();
        iEndpoints[EMTPUsbEpInterrupt]->Cancel();
        if ((iBulkTransactionState == EDataIToRPhase) && iUsbBulkContainer->Payload())
            {
            OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_DATAENDPOINTSSTOP, "Aborting active I to R data phase" );
            TRAPD(err, BoundProtocolLayer().ReceiveDataCompleteL(KErrAbort, *iUsbBulkContainer->Payload(), iMTPRequest));
            UNUSED_VAR(err);
            }
        else if ((iBulkTransactionState == EDataRToIPhase) && iUsbBulkContainer->Payload())
            {
            OstTrace0( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_DATAENDPOINTSSTOP, "Aborting active R to I data phase" );
            TRAPD(err, BoundProtocolLayer().SendDataCompleteL(KErrAbort, *iUsbBulkContainer->Payload(), iMTPRequest));
            UNUSED_VAR(err);
            }
		else if ((iBulkTransactionState == EResponsePhase) && iUsbBulkContainer->Payload())
            {
            OstTrace0( TRACE_NORMAL, DUP3_CMTPUSBCONNECTION_DATAENDPOINTSSTOP, "Aborting active response phase" );
            TRAPD(err, BoundProtocolLayer().SendResponseCompleteL(KErrAbort, *static_cast<TMTPTypeResponse*>(iUsbBulkContainer->Payload()), iMTPRequest));
            UNUSED_VAR(err);
            }
        }
#ifdef OST_TRACE_COMPILER_IN_USE
    else
        {
        OstTrace0( TRACE_NORMAL, DUP4_CMTPUSBCONNECTION_DATAENDPOINTSSTOP, "Endpoints inactive, do nothing" );
        }
#endif // OST_TRACE_COMPILER_IN_USE
    OstTraceFunctionExit0( CMTPUSBCONNECTION_DATAENDPOINTSSTOP_EXIT );
    }
    
void CMTPUsbConnection::EndpointStallL(TMTPUsbEndpointId aId)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_ENDPOINTSTALLL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_ENDPOINTSTALLL, "Creating stall condition on endpoint %d", aId );
    
    __ASSERT_DEBUG((aId < EMTPUsbEpNumEndpoints), Panic(EMTPUsbReserved));
    
    // Stall the endpoint.
    CMTPUsbEpBase& ep(*iEndpoints[aId]);
    ep.Stall();
    
    // Update the connection state.
    SetConnectionState(EStalled);
    
    OstTraceFunctionExit0( CMTPUSBCONNECTION_ENDPOINTSTALLL_EXIT );
    }
    
void CMTPUsbConnection::EndpointStallClearL(TMTPUsbEndpointId aId)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_ENDPOINTSTALLCLEARL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_ENDPOINTSTALLCLEARL, "Clearing stall condition on endpoint %d", aId );
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
    OstTraceFunctionExit0( CMTPUSBCONNECTION_ENDPOINTSTALLCLEARL_EXIT );
    }
       
/**
Resumes USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::ResumeConnectionL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_RESUMECONNECTIONL_ENTRY );
    if (ConnectionOpen())
        {    
        // Restart data transfer activity.
        ControlEndpointStartL();
        DataEndpointsStartL();
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_RESUMECONNECTIONL_EXIT );
    }
       
/**
Initiates USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::StartConnectionL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_STARTCONNECTIONL_ENTRY );  
    
    // Notify the connection manager and update state, if necessary. 
    if (ConnectionClosed())
        {
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_STARTCONNECTIONL, "Notifying protocol layer connection opened" );
        iConnectionMgr->ConnectionOpenedL(*this);
        SetConnectionState(EOpen);
        SetDeviceStatus(EMTPUsbDeviceStatusOK);
        InitiateBulkRequestSequenceL();
        }
    OstTraceFunctionExit0( CMTPUSBCONNECTION_STARTCONNECTIONL_EXIT );
    }
       
/**
Halts USB MTP device class processing.
*/ 
TBool CMTPUsbConnection::StopConnection()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_STOPCONNECTION_ENTRY );
    
    TBool ret = ETrue;
    // Stop all data transfer activity.
    DataEndpointsStop();    
    
    // Notify the connection manager and update state, if necessary.
    if (ConnectionOpen())
        {
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_STOPCONNECTION, "Notifying protocol layer connection closed" );
        ret = iConnectionMgr->ConnectionClosed(*this);
        SetBulkTransactionState(EUndefined);
        SetConnectionState(EIdle);
        SetSuspendState(ENotSuspended);
		iMTPSessionId = KMTPSessionNone;
        }
    
    OstTraceFunctionExit0( CMTPUSBCONNECTION_STOPCONNECTION_EXIT );
    return ret;
    }
       
/**
Suspends USB MTP device class processing.
@leave One of the system wide error codes, if a processing failure occurs.
*/ 
void CMTPUsbConnection::SuspendConnectionL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SUSPENDCONNECTIONL_ENTRY );
    if (ConnectionOpen())
        {    
        // Stop all data transfer activity.
        DataEndpointsStop();
        
        //flush buffered rx data.
        iEndpoints[EMTPUsbEpBulkOut]->FlushBufferedRxDataL();
        }
    ControlEndpointStop();

    //  Update state.
    SetSuspendState(ESuspended);
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SUSPENDCONNECTIONL_EXIT );
    }

/**
Configures the USB MTP device class.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::StartUsbL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_STARTUSBL_ENTRY );
    
    // Open the USB device interface.
    LEAVEIFERROR(iLdd.Open(KDefaultUsbClientController),
            OstTrace1( TRACE_ERROR, CMTPUSBCONNECTION_STARTUSBL, 
                    "Open the USB device interface failed! error code %d", munged_err));

    // Configure the class descriptors.
    ConfigurationDescriptorSetL();
    SetInterfaceDescriptorL();

    OstTraceFunctionExit0( CMTPUSBCONNECTION_STARTUSBL_EXIT );
    }
    
/**
This method stops the end points transfer.
*/
void CMTPUsbConnection::StopUsb()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_STOPUSB_ENTRY );
    // Stop monitoring the USB device controller state.
    iLdd.AlternateDeviceStatusNotifyCancel();
    
    // Stop monitoring the USB device controller state.
    Cancel();
    
    // Clear the configuration descriptor.
    ConfigurationDescriptorClear();
    
    // Close the USB device interface.
    iLdd.ReleaseInterface(KMTPUsbAlternateInterface);
    iLdd.Close();

    OstTraceFunctionExit0( CMTPUSBCONNECTION_STOPUSB_EXIT );
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
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_CONNECTIONSTATE, "Connection state = 0x%08X", state );
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
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SUSPENDSTATE, "Suspend state = 0x%08X", state );

    return (state);
    }

/**
Sets the bulk transfer transaction state.
@param aState The new connectio state bit flags.
@see TConnectionState
*/
void CMTPUsbConnection::SetBulkTransactionState(TMTPTransactionPhase aState)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETBULKTRANSACTIONSTATE_ENTRY );
    iBulkTransactionState = aState;
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SETBULKTRANSACTIONSTATE,
            "SetBulkTransactionState state set to 0x%08X", iBulkTransactionState );
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SETBULKTRANSACTIONSTATE_EXIT );
    }

/**
Sets the MTP USB device class device status Code value.
@param aCode The PIMA 15740 Response Code or Vendor Code value.
*/
void CMTPUsbConnection::SetDeviceStatus(TUint16 aCode)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETDEVICESTATUS_ENTRY );
    iDeviceStatusCode = aCode;
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SETDEVICESTATUS, "Device status set to 0x%04X", iDeviceStatusCode );
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SETDEVICESTATUS_EXIT );
    }

/**
Sets the USB MTP device class interface descriptor.
@leave KErrCorrupt, if the configuration descriptor size is invalid.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPUsbConnection::SetInterfaceDescriptorL()
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL_ENTRY );
    
    TUsbcInterfaceInfoBuf ifc;
    // Get device capabilities.
    LEAVEIFERROR(iLdd.DeviceCaps(iDeviceCaps),
            OstTrace1( TRACE_ERROR, DUP3_CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, 
                    "Get device capabilities failed! error code %d", munged_err));

    // Fetch the endpoint capabilities set.
    TPtr8 capsPtr(reinterpret_cast<TUint8*>(iEndpointCapSets), sizeof(iEndpointCapSets), sizeof(iEndpointCapSets));
    LEAVEIFERROR(iLdd.EndpointCaps(capsPtr),
            OstTrace1( TRACE_ERROR, DUP4_CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, 
                    "Get end point capabilities failed! error code %d", munged_err));
            
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
    
    // Allocate 512KB*2 buffer for OUT EndPoint, and 64KB for IN EndPoint
    TUint32 bandwidthPriority = EUsbcBandwidthINPlus2 | EUsbcBandwidthOUTMaximum;
        
    // Write the active interface descriptor.
    TInt err = iLdd.SetInterface(KMTPUsbAlternateInterface, ifc, bandwidthPriority);
    
    if (err == KErrNoMemory)
        {
        OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, "NoMem when setinterface, try with lower priority" );
        // Allocate 64KB*2 buffer for OUT EndPoint, and 64KB for IN EndPoint
        bandwidthPriority = EUsbcBandwidthINPlus2 | EUsbcBandwidthOUTPlus2;
        err = iLdd.SetInterface(KMTPUsbAlternateInterface, ifc, bandwidthPriority);
        OstTrace1( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, "setinterface return for lower priority:%d", err);
        }
    OstTrace1( TRACE_NORMAL, DUP2_CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, "setinterface error code:%d",err );
    
    LEAVEIFERROR(err,  OstTrace1( TRACE_NORMAL, DUP5_CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL, 
                    "Set interface failed! error code %d", munged_err ));

    OstTraceFunctionExit0( CMTPUSBCONNECTION_SETINTERFACEDESCRIPTORL_EXIT );
    }

/**
Sets the USB MTP device class connection state.
@param aState The new connection state bit flags.
@see TConnectionState
*/
void CMTPUsbConnection::SetConnectionState(TInt32 aState)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETCONNECTIONSTATE_ENTRY );
    iState = ((~EConnectionStateMask & iState) | aState);
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SETCONNECTIONSTATE, "Connection state set to 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SETCONNECTIONSTATE_EXIT );
    }

/**
Sets the USB device suspend state.
@param aState The new suspend state bit flags.
@see TSuspendState
*/
void CMTPUsbConnection::SetSuspendState(TInt32 aState)
    {
    OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETSUSPENDSTATE_ENTRY );
    iState = ((~ESuspendStateMask & iState) | aState);
    OstTrace1( TRACE_NORMAL, CMTPUSBCONNECTION_SETSUSPENDSTATE, "Connection state set to 0x%08X", iState );
    OstTraceFunctionExit0( CMTPUSBCONNECTION_SETSUSPENDSTATE_EXIT );
    }
    
/**
This method is called when EUsbcDeviceStateAddress state is reached. 
It sets the transport packet size based on the host type the device is
connected to.
*/
void CMTPUsbConnection::SetTransportPacketSizeL()
	{
	OstTraceFunctionEntry0( CMTPUSBCONNECTION_SETTRANSPORTPACKETSIZEL_ENTRY );
	if(iLdd.CurrentlyUsingHighSpeed())
		{
		OstTrace0( TRACE_NORMAL, CMTPUSBCONNECTION_SETTRANSPORTPACKETSIZEL, "HS USB connection" );
		iEndpoints[EMTPUsbEpControl]->SetMaxPacketSizeL(KMaxPacketTypeControlHS);
		iEndpoints[EMTPUsbEpBulkIn]->SetMaxPacketSizeL(KMaxPacketTypeBulkHS);
		iEndpoints[EMTPUsbEpBulkOut]->SetMaxPacketSizeL(KMaxPacketTypeBulkHS);
		}
	else
		{
		OstTrace0( TRACE_NORMAL, DUP1_CMTPUSBCONNECTION_SETTRANSPORTPACKETSIZEL, "FS USB connection" );
		iEndpoints[EMTPUsbEpControl]->SetMaxPacketSizeL(KMaxPacketTypeControlFS);
		iEndpoints[EMTPUsbEpBulkIn]->SetMaxPacketSizeL(KMaxPacketTypeBulkFS);
		iEndpoints[EMTPUsbEpBulkOut]->SetMaxPacketSizeL(KMaxPacketTypeBulkFS);
		}		
	iEndpoints[EMTPUsbEpInterrupt]->SetMaxPacketSizeL(KMaxPacketTypeInterrupt);

	OstTraceFunctionExit0( CMTPUSBCONNECTION_SETTRANSPORTPACKETSIZEL_EXIT );
	}

TBool CMTPUsbConnection::IsEpStalled(const TUint& aEpNumber)
    {
    const TEndpointNumber number( static_cast<TEndpointNumber>(aEpNumber) );
    TEndpointState state;
    iLdd.EndpointStatus(number, state);
    return ( EEndpointStateStalled == state );
    }
