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
 @internalTechnology
*/

#ifndef MMTPCONNECTIONPROTOCOL_H
#define MMTPCONNECTIONPROTOCOL_H

#include <e32cmn.h>
#include <e32def.h>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPType;
class TMTPTypeEvent;
class TMTPTypeRequest;
class TMTPTypeResponse;
class MMTPTransportConnection;

/** 
Defines the MTP connection protocol layer interface.

@internalTechnology
  
*/
class MMTPConnectionProtocol
    {
public:

    /**
    Provides the unique MTP protocol layer assigned identifier of the
    connection.
    @return The MTP protocol layer assigned connection identifier.
    */
    virtual TUint ConnectionId() const = 0;

    /**
    Signals the availability of an MTP event data object received from the MTP
    transport layer connection.
    @param aEvent The received MTP event data object.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void ReceivedEventL(const TMTPTypeEvent& aEvent) = 0;

    /**
    Signals the availability of an MTP request data object received from the MTP
    transport layer connection.
    @param aRequest The received MTP request data object.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void ReceivedRequestL(const TMTPTypeRequest& aRequest) = 0;

    /**
    Signals the completion of an MTP data object receive sequence in the MTP
    transport layer connection, that was previously initiated by a ReceiveData
    signal to the transport layer connection interface.
    @param aErr A system wide error code indicating the completion status of 
    the sequence.
    @param aData The MTP data object sink.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @leave One of the system wide error codes, if a processing failure occurs.
    @see MMTPCMMTPTransportConnection::ReceiveData
    @see MMTPCMMTPTransportConnection::ReceiveDataCancel
    */
    virtual void ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest) = 0;

    /**
    Signals the completion of an MTP data object send sequence in the MTP
    transport layer connection, that was previously initiated by a SendData
    signal to the transport layer connection interface.
    @param aErr A system wide error code indicating the completion status of 
    the sequence.
    @param aData The MTP data object source.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @leave One of the system wide error codes, if a processing failure occurs.
    @see MMTPCMMTPTransportConnection::SendData
    @see MMTPCMMTPTransportConnection::SendDataCancel
    */
    virtual void SendDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest) = 0;

    /**
    Signals the completion of an MTP event data object send sequence in the MTP
    transport layer connection, that was previously initiated by a SendEvent
    signal to the transport layer connection interface.
    @param aErr A system wide error code indicating the completion status of 
    the sequence.
    @param aData The MTP event data object source.
    @leave One of the system wide error codes, if a processing failure occurs.
    @see MMTPCMMTPTransportConnection::SendEvent
    */
    virtual void SendEventCompleteL(TInt aErr, const TMTPTypeEvent& aData) = 0;

    /**
    Signals the completion of an MTP response data object send sequence in the 
    MTP transport layer connection, that was previously initiated by a 
    SendResponse signal to the transport layer connection interface.
    @param aErr A system wide error code indicating the completion status of 
    the sequence.
    @param aResponse The MTP response data object source.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @leave One of the system wide error codes, if a processing failure occurs.
    @see MMTPCMMTPTransportConnection::SendResponse
    */
    virtual void SendResponseCompleteL(TInt aErr, const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest) = 0;

    /**
    Provides the current MTP transaction state for the specified session.
    @param aMtpSessionId The MTP connection assigned identifier of the 
    requested session.
    @leave KErrArgument, if the specified session identifier is not currently
    active on the connection.
    @return The MTP transaction state for the specified session.
    */
    virtual TMTPTransactionPhase TransactionPhaseL(TUint32 aMtpSessionId) const = 0;  
      
    virtual void Unbind(MMTPTransportConnection& aConnection) = 0;

    virtual void DisconnectionNotifyL()=0;
    };
#endif // MMTPCONNECTIONPROTOCOL_H
