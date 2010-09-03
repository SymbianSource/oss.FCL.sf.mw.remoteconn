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

#ifndef CMTPUSBCONNECTION_H
#define CMTPUSBCONNECTION_H

#include <e32base.h>
#include <d32usbc.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/tmtptypenull.h>

#include "mtpusbtransportconstants.h"
#include "mmtptransportconnection.h"
#include "tmtpusbcontrolrequestsetup.h"
#include "tmtpusbcontrolrequestcanceldata.h"
#include "tmtpusbcontrolrequestdevicestatus.h"
#include "tmtpusbcontrolrequestmodfunction.h"
#include "tmtpusbcontrolrequestmodheader.h"
#include "tmtpusbparameterpayloadblock.h"

class CMTPUsbContainer;
class CMTPUsbEpBase;
class MMTPConnectionProtocol;
class MMTPConnectionMgr;
class MMTPType;

/** 
Implements the MTP USB device class connection protocol and transport layer 
interface.
@internalComponent
 
*/
class CMTPUsbConnection : 
    public CActive, 
    public MMTPTransportConnection 
    {
public:
    
    static CMTPUsbConnection* NewL(MMTPConnectionMgr& aConnectionMgr);
    ~CMTPUsbConnection();
    
public: // From MMTPTransportConnection

    void BindL(MMTPConnectionProtocol& aProtocol);
    MMTPConnectionProtocol& BoundProtocolLayer();
    void CloseConnection();
    void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest);
    void ReceiveDataCancelL(const TMTPTypeRequest& aRequest);   
    void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest);
    void SendDataCancelL(const TMTPTypeRequest& aRequest);    
    void SendEventL(const TMTPTypeEvent& aEvent);  
    void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest);  
    void TransactionCompleteL(const TMTPTypeRequest& aRequest); 
    void Unbind(MMTPConnectionProtocol& aProtocol);
    TAny* GetExtendedInterface(TUid aInterfaceUid); 
    TUint GetImplementationUid();
     
public:
  
    void ReceiveBulkDataCompleteL(TInt aError, MMTPType& aData);
    void ReceiveControlRequestDataCompleteL(TInt aError, MMTPType& aData);
    void ReceiveControlRequestSetupCompleteL(TInt aError, MMTPType& aData);
    void SendBulkDataCompleteL(TInt aError, const MMTPType& aData);
    void SendControlRequestDataCompleteL(TInt aError, const MMTPType& aData);
    void SendInterruptDataCompleteL(TInt aError, const MMTPType& aData);
    
    TUint EndpointBitPosition(TUint aId) const;
    const TUsbcEndpointCaps& EndpointCapsL(TUint aId);
    TUint EndpointDirection(TUint aId) const;
    TUint EndpointDirectionAndType(TUint aId) const;
    TEndpointNumber EndpointNumber(TUint aId) const;
    TUint EndpointType(TUint aId) const;
        
    RDevUsbcClient& Ldd();
    CMTPUsbContainer& BulkContainer();
    
    TMTPTypeRequest& GetCurrentRequest()
        {
        return iMTPRequest;
        }

private: // From CActive

    void DoCancel();
    TInt RunError(TInt aError);
    void RunL();

private:
 
    CMTPUsbConnection(MMTPConnectionMgr& aConnectionMgr);
    void ConstructL();
    
    void IssueAlternateDeviceStatusNotifyRequest();
    
    void BufferEventDataL(const TMTPTypeEvent& aEvent);
    void SendEventDataL();
    void SendUnreportedStatusEventL();
    
    void InitiateBulkRequestSequenceL();
    void InitiateControlRequestSequenceL();
    
    void ProcessBulkCommandL(TInt aError);
    void ProcessBulkDataInL(const TMTPTypeRequest& aRequest, const MMTPType& aData);
    void ProcessBulkDataOutL(TInt aError);
    void ProcessControlRequestCancelL(const TMTPUsbControlRequestSetup& aRequest);
    void ProcessControlRequestDeviceResetL(const TMTPUsbControlRequestSetup& aRequest);
    void ProcessControlRequestDeviceStatusL(const TMTPUsbControlRequestSetup& aRequest);
  
    TBool BulkRequestErrorHandled(TInt aError);
    TBool BulkRequestTransactionStateValid(TMTPTransactionPhase aExpectedTransactionState);
    TBool ControlRequestErrorHandled(TInt aError);
    
    void ConfigurationDescriptorSetL();
    void ConfigurationDescriptorClear();
    
    TBool ConnectionClosed() const;
    TBool ConnectionOpen() const;
    
    void ControlEndpointStartL();
    void ControlEndpointStop();
    
    void BulkEndpointsStallL();
    void BulkEndpointsStallClearL();

    void DataEndpointsStartL();
    void DataEndpointsStop();
    
    void EndpointStallL(TMTPUsbEndpointId aId);
    void EndpointStallClearL(TMTPUsbEndpointId aId);
    
    void ResumeConnectionL();
    void StartConnectionL();
    TBool StopConnection();
    void SuspendConnectionL();
    
    void StartUsbL();
    void StopUsb();
    
    TInt32 ConnectionState() const;
    TInt32 SuspendState() const;
    
    void SetBulkTransactionState(TMTPTransactionPhase aState);
    void SetConnectionState(TInt32 aState);
    void SetDeviceStatus(TUint16 aCode);
    void SetInterfaceDescriptorL();
    void SetSuspendState(TInt32 aState);
    void SetTransportPacketSizeL();

    TBool IsEpStalled(const TUint& aEpNumber);
    
private: // Owned.

    /**
    The MTP USB device class connection state bit flags.
    */
    enum TConnectionState
        {
        /**
        The Connection is closed.
        */
        EIdle                   = 0x00000000,
        
        /**
        The Connection is open and in an error state pending a device reset control
        request from the host.
        */
        EStalled                = 0x00000001,
        
        /**
        The Connection is open and inactive.
        */
        EOpen                   = 0x00000002,
        
        /**
        The Connection is open and actively processing a bulk data transfer.
        */
        EBusy                   = 0x00000004,
        
        /**
        The connection state bit flags mask.
        */
        EConnectionStateMask    = 0x0000000F,
        };

    /**
    The MTP USB suspend state bit flags.
    */
    enum TSuspendState
        {
        /**
        The USB device is not suspended.
        */
        ENotSuspended           = 0x00000000,
        
        /**
        The USB device is suspended.
        */
        ESuspended              = 0x80000000,
        
        /**
        The USB device is suspended and asynchronous MTP event data is awaiting
        transmission to the host.
        */
        ESuspendedEventsPending = 0xC0000000,
        
        /**
        The suspend state bit flags mask.
        */
        ESuspendStateMask       = 0xF0000000,
        };
      
    /**
    Defines the MTP USB device class endpoint meta data.
    */
    struct TEpInfo
        {        
        /**
        The logical endpoint bit position.
        */
        TUint           iBitPosition;   
        
        /**
        The endpoint direction flag bits.
        */
        TUint           iDirection; 
        
        /**
        The endpoint polling interval.
        */
        TUint           iInterval;
        
        /**
        The high speed NAK rate
        */
        TUint			iInterval_Hs;    
        
        /**
        The logical endpoint number.
        */
        TEndpointNumber iNumber; 
        
        /**
        The endpoint type flag bits.
        */
        TUint           iType;         
        };
    
    /**
    The bulk transfer transaction state.
    */
    TMTPTransactionPhase                                iBulkTransactionState;
    
    /** 
    The current USB controller/interface alternate setting state.
    */
    TUint                                               iControllerStateCurrent;
    
    /** 
    The previous USB controller/interface alternate setting state.
    */
    TUint                                               iControllerStatePrevious;
    
    /**
    The USB device capabilities buffer.
    */
    TUsbDeviceCaps                                      iDeviceCaps;
    
    /**
    The current device status.
    */
    TUint16                                             iDeviceStatusCode;
    
    /**
    The endpoint capabilities buffer.
    */
    TUsbcEndpointCaps                                   iEndpointCaps;
    
    /**
    The USB device endpoints capability sets.
    */
    TUsbcEndpointData                                   iEndpointCapSets[KUsbcMaxEndpoints];
    
    /** 
    The USB MTP device class endpoints.
    */
    RPointerArray<CMTPUsbEpBase>                        iEndpoints;
    
    /** 
    The USB device.
    */
    RDevUsbcClient                                      iLdd;
    
    /**
    The MTP event dataset buffer.
    */
    TMTPTypeEvent                                       iMTPEvent;
    
    /**
    Event pending flag.
    */
    TBool												iEventPending;
    
    /**
    The active MTP operation request dataset buffer.
    */
    TMTPTypeRequest                                     iMTPRequest;
    
    /** 
    The active MTP SessionID.
    */
    TUint32                                             iMTPSessionId;
    
    /**
    The remote wakeup enabled flag.
    */
    TBool                                               iRemoteWakeup;
    
    /**
    The MTP USB device class state variable.
    */
    TInt32                                              iState;
    
    /** 
    The USB MTP device class generic bulk pipe container buffer.
    */
    CMTPUsbContainer*                                   iUsbBulkContainer;
    
    /**
    The USB MTP device class variable length Command/Response Block payload 
    dataset. 
    */
    TMTPUsbParameterPayloadBlock                        iUsbBulkParameterBlock;
    
    /** 
    The USB MTP device class event dataset buffer.
    */
    CMTPUsbContainer*                                   iUsbEventContainer;
    
    /**
    The USB MTP device class variable length Event Block payload dataset. 
    */
    TMTPUsbParameterPayloadBlock                        iUsbEventParameterBlock;
    
    /** 
    MTP USB device class cancel control request dataset buffer.
    */
    TMTPUsbControlRequestCancelData                     iUsbControlRequestCancelData;
    
    
    /** 
    MTP USB device class device status control request dataset buffer.
    */
    TMTPUsbControlRequestDeviceStatus                   iUsbControlRequestDeviceStatus;
    
    /** 
    The MTP USB device class control request setup dataset buffer.
    */
    TMTPUsbControlRequestSetup                          iUsbControlRequestSetup;
    
    /** 
    The MTP USB device class Microsoft OS Feature Descriptor (MOD) header 
    section dataset buffer.
    */
    TMTPUsbControlRequestMODHeader                      iUsbMODHeader;
    
    /** 
    The MTP USB device class Microsoft OS Feature Descriptor (MOD) function 
    section dataset buffer.
    */
    TMTPUsbControlRequestMODFunction                    iUsbMODFunction;
    
    /**
    The MTP USB device class endpoint meta data table.
    */
    static const TEpInfo                                KEndpointMetaData[];
    const TFixedArray<TEpInfo, EMTPUsbEpNumEndpoints>   iEndpointInfo;

	/**
	Cached error code in case of a ItoR transaction error.
	*/
	TInt												iXferError;

	/**
	Data sink for consuming data during error recovery from
	a failed ItoR transaction.
	*/
	TMTPTypeNull										iNull;

	/**
	Buffer for reading discarded data into.
	*/
	RBuf8												iNullBuffer;
	
	/**
	A flag to reflect if a Host initiated cancel transaction
	is triggered.
	*/
	TBool 												iIsCancelReceived;
	
    /**
    A flag to reflect if a Host initiated cancel transaction
    is triggered.
    */
    TBool                                               iIsResetRequestSignaled;

private: // Not owned.
    
    /** 
    The MTP connection manager.
    */
    MMTPConnectionMgr*                                  iConnectionMgr;

    /** 
    The MTP connection protocol layer binding.
    */
    MMTPConnectionProtocol*                             iProtocolLayer;
 
    /**
     * indicate if a transaction_cannled needs to be return to PC
     */
    TBool                                               isResponseTransactionCancelledNeeded;
	/**
	 * indicate if a transaction has been ignored
	 */
	TBool												isCommandIgnored;

    };
    
#endif // CMTPUSBCONNECTION_H
