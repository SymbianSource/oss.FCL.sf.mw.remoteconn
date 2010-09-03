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

#ifndef CMTPUSBEPBASE_H
#define CMTPUSBEPBASE_H

#include <e32std.h>
#include <e32base.h>
#include <d32usbc.h>

class CMTPUsbConnection;

/**
Defines the USB MTP device class endpoint data transfer controller base class.
@internalComponent
 
*/
class CMTPUsbEpBase : public CActive
    {
public:
  
    virtual ~CMTPUsbEpBase();

    TEndpointNumber EndpointNumber() const;
    TUint Id() const;
    void SetMaxPacketSizeL(TUint aSize);
    void Stall();
    void StallClear();
    TBool Stalled() const;
    
    static TInt LinearOrder(const CMTPUsbEpBase& aL, const CMTPUsbEpBase& aR);
    
    void ResumeReceiveDataL(MMTPType& aSink);    
    void CancelReceiveL(TInt aReason);
    void CancelSendL(TInt aReason);
    
    void FlushRxDataL();
    void FlushBufferedRxDataL();
    void FlushOnePacketL();
    
protected:

    CMTPUsbEpBase(TUint aId, TPriority aPriority, CMTPUsbConnection& aConnection);
    
    virtual void ConstructL();

    CMTPUsbConnection& Connection() const;
    
    void ReceiveDataL(MMTPType& aSink);
    virtual void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);
    
    void SendDataL(const MMTPType& aSource);
    virtual void SendDataCompleteL(TInt aError, const MMTPType& aSource);
    
protected: // From CActive

   	void DoCancel();

private: // From CActive

    TInt RunError(TInt aError);
    void RunL();
    
private:

    TInt32 DataStreamDirection() const;
    void ResetReceiveDataStream();
    void ResetSendDataStream();
    void ResumeReceiveDataStreamL();
    void ProcessSendDataStreamL();
    
    void SetStreamState(TInt aState);
    TBool ValidateUSBHeaderL();
    void InitiateFirstChunkReceiveL();
    void ProcessFirstReceivedChunkL();

private: // Owned

    /**
    The data stream states.
    */
    enum TState
        {
        /**
        Both send and receive data streams are inactive.
        */
        EIdle                   = 0x00000000,
        
        /**
        Both send and receive data streams are stalled.
        */
        EStalled                = 0x0F000000,
        
        /**
        The receive data stream is active.
        */
        EReceivingState         = 0x40000000,
        EReceiveInitialising    = 0x40000001,
        EReceiveInProgress      = 0x40000002,
        EReceiveCompleting      = 0x40000003,
        EReceiveComplete        = 0x40000004,
        
        /**
        The send data stream is active.
        */
        ESendingState           = 0x80000000,
        ESendInitialising       = 0x80000001,
        ESendInProgress         = 0x80000002,
        ESendCompleting         = 0x80000003,
        ESendComplete           = 0x80000004,
        
        EStateDirection         = 0xF0000000
        }; 
    
    /**
    The active data stream chunk status.
    */
    TInt                iChunkStatus;

    /**
    The internal endpoint identifier.
    */
    TUint               iId;
    
    /**
    The residual packet data buffer.
    */
    RBuf8               iPacketBuffer;
    
    /**
    The maximum bulk-in endpoint size.
    */
    TUint               iPacketSizeMax;

    /** 
    The receive data stream commit flag.
    */
    TBool               iReceiveDataCommit;
    
    /**
    The receive data stream chunk data buffer.
    */
    TPtr8               iReceiveChunkData;
    
    /**
    The receive data stream data pointer.
    */
    TPtr8               iReceiveData;

    /**
    The send data stream chunk data.
    */
    TPtrC8              iSendChunkData;
    
    /**
    The send data stream data pointer.
    */
    TPtrC8              iSendData;
    
    /**
    The data stream state variable.
    */
    TInt32              iState;   
    
    /**
    A flag to tell if the receive stream is the first chunk.
    */			
    TBool 				iIsFirstChunk;
    
    /**
    The amount of data received.
    */
    TUint64 			iDataCounter;
    
    /**
    The amount of data expected.
    */    
    TUint64 			iDataLength;
    
private: // Not owned

    /**
    The MTP USB device class transport connection which controls the endpoint.
    */
    CMTPUsbConnection&  iConnection;

    /**
    The receive data stream sink buffer
    */
    MMTPType*           iReceiveDataSink;

    /**
    The send data stream source buffer
    */
    const MMTPType*     iSendDataSource;
    };
    
#endif // CMTPUSBEPBASE_H
