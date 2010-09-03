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
//

/**
 @internalComponent
*/

#ifndef CSOCKETHANDLER_H_
#define CSOCKETHANDLER_H_

#include "es_sock.h"
#include "in_sock.h"
#include "e32base.h"
#include "cptpipconnection.h"

#include "ptpipprotocolconstants.h"


//The maximum number of bytes buffered locally before sending over socket
// If the data chunk is larger than this, it is sent directly, if its smaller its buffered.
const TInt KBufferSize = 64;

/**
This class encapsulates the functions for sending and receving data over
the @see RSocket over TCP/IP

It has the common functionality for handling and communicating with the socket, 
and for communicating with the ptpip connection class. 

The command and Event handler classes derived from this class.
 
*/
class CPTPIPSocketHandlerBase : public CActive
	{
public :

	enum TSocketState
		{
		
		EIdle                     = 0x00000000,
		EInitSendInProgress       = 0x80000005,
		
        // The receive data stream is active.
        EReceivingState           = 0x40000000,
		EReceiveInProgress        = 0x40000001,
		EReceiveFastInProgress    = 0x40000002,
		EReceiveComplete          = 0x40000003,

        // The send data stream is active.
        ESendingState             = 0x80000000,
		ESendInitialising         = 0x80000001,
		ESendInProgress           = 0x80000002,
		ESendCompleting           = 0x80000003,
		ESendComplete             = 0x80000004,
		
		// Send stream is active and is sending the data in packets during R to I data phase
		ESendDataState            = 0x60000000,
		ESendDataInitialising     = 0x60000001,
		ESendingDataHeader        = 0x60000002,
		ESendingDataPacket        = 0x60000003,
		ESendDataPacketCompleting = 0x60000004,
		ESendDataComplete         = 0x60000005,
		ESendDataCancelled        = 0x60000006,

		EStateDirection           = 0xF0000000
		};
	
	// Data Sending functions
	void SendDataL(const MMTPType& aData, TUint32 aTransactionId);
    virtual void SendDataCompleteL(TInt aError, const MMTPType& aSource);
	void CancelSendL(TInt aReason);

	// Data Receiving functions
	void ReceiveDataL(MMTPType& aData);
    virtual void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);
	void CancelReceiveL(TInt aReason);
    
    // Other helper functions
	virtual ~CPTPIPSocketHandlerBase();
	CPTPIPConnection& Connection();
	RSocket& Socket();
	void SetSocket(RSocket& aSocket);
	void SetState(TSocketState aState);
	void SetSocketOptions();
	void SetCancel()
		{
		iCancelReceived = ETrue;
		}
	
	// From CActive
	void DoCancel();

public:

	/*Flag to inidicate whether to populate the chunk given by MTP framework, 
	 from the last saved location or beginning if its a new chunk
	 * */
	TBool iUseOffset ;			
		
protected:

	CPTPIPSocketHandlerBase(CPTPIPConnection& aConnection, TPriority priority );
    virtual void ConstructL();
	virtual TInt ParsePTPIPHeaderL() = 0;
	virtual TBool HandleInitAck() = 0;
	
	
private:

	// From CActive
	void RunL();
	TInt RunError(TInt aError);

	// other helper functions.
	void ProcessSendDataL();
	void ProcessFirstReceivedChunkL();
	void ResumeReceiveDataStreamL();
	void InitiateFirstChunkReceiveL();
	void ResetSendDataStream();
	void ResetReceiveDataStream();
	void CompleteSelf(TInt aCompletionCode);
	TInt32 DataStreamDirection() const;
	void SendDataPacketL();
	void SendHeaderPacketL();
	void CreateAndSendDataPacketsL();	

protected :
	/**
	This is the socket used for transferring data , 
	it is controlled by the socketHandler
	*/
	RSocket 			iSocket;
	
    /**
    The active data stream chunk status.
    */
    TInt                iChunkStatus;

    /**
    The pointer to the current chunk in the send data stream.
    */
    TPtrC8              iSendChunkData;
    
    /**
    The send data stream data pointer.
    */
    TPtrC8              iSendData;
    
    /**
    The pointer to the current chunk in the receive data stream
    */
    TPtr8               iReceiveChunkData;
    
       
    
    /**
    The amount of data expected in one ptpip packet, this is filled from the size from ptp header
    */
    TUint32				iPTPPacketLength;
	
    /**
    Keeps track of whether socket is sending, receiving, idle etc.
    */
    TSocketState		iState;

private :
    /**
    The packet data buffer used to copy and append the chunk locally
    if the individual chunks are too small to send as one tcp packet. 
    */
    RBuf8               iPacketBuffer;
    
    /**
    The maximum size of the packet buffer
    */
    TUint               iPacketSizeMax;
    
    /**
    The receive data stream data pointer.
    */
    TPtr8               iReceiveData;

    /**
    A flag to tell if the receive stream is the first chunk.
    */			
    TBool 				iIsFirstChunk;
    
    /**
    The amount of data received so far, keeps incrementing with each tcp packet.
    */
    TUint32 			iPTPPacketLengthReceived;
     
    /** 
    The receive data stream commit flag.
    */
    TBool               iReceiveDataCommit;
	
    /**
    The send data stream data pointer.
    */
    TPtrC8              iSendHeaderData;
    
	/**
    The header for the ptp ip data packet.
    */
    CPTPIPDataContainer* iPTPIPDataHeader;
    
    /**
    The flag to keep track whether cancel has been received. 
    */
    TBool               iCancelReceived;
    
    
private : // Not owned
	CPTPIPConnection& iConnection;

	/**
	This is the buffer used to receive data from the socket. This will be a pointer 
	to the PTPIP data type. 
	*/
	MMTPType* iReceiveDataSink;
	
	/**
	This is the buffer used to send data to the socket. This will be a pointer 
	to the PTPIP data type. 
	*/
	const MMTPType* iSendDataSource;	
	
	/**
    The pointer to the current chunk in the receive data stream
    */
    TPtr8               iCurrentChunkData; 
    
    /*Type of the current PTP/IP Packet */
    TUint32 iType;
	
	};

#endif /*CSOCKETHANDLER_H_*/
