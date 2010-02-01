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

#ifndef MMTPTRANSPORTCONNECTION_H
#define MMTPTRANSPORTCONNECTION_H

#include <e32cmn.h>

class MMTPConnectionProtocol;
class MMTPType;
class TMTPTypeEvent;
class TMTPTypeRequest;
class TMTPTypeResponse;

/** 
Defines the MTP connection transport layer interface.
@internalTechnology
  
*/
class MMTPTransportConnection
    {
public:

    /**
    Binds the MTP protocol layer notification interface to the connection transport 
    layer.
    @param aProtocol The MTP protocol layer notification interface.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void BindL(MMTPConnectionProtocol& aProtocol) = 0;

    /**
    Provides the MTP protocol layer notification interface which is bound to
    the connection transport layer.
    @return The MTP protocol layer notification interface binding.
    */
    virtual MMTPConnectionProtocol& BoundProtocolLayer() = 0;
    
    /**
    Initiates a connection shutdown sequence, usually as a result of an 
    unrecoverable MTP protocol error. This sequence is completed when the 
    connection transport layer signals ConnectionClosed to the MTP connection 
    manager interface.
    @see MMTPConnectionMgr::ConnectionClosed.
    */
    virtual void CloseConnection() = 0;

    /**
    Initiates an MTP data object receive sequence in the connection transport 
    layer. This sequence is completed when the transport layer connection signals
    ReceiveDataComplete to the MTP protocol layer notification interface.
    @param aData The MTP data object sink.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @see MMTPConnectionProtocol::ReceiveDataComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest) = 0;

    /**
    Aborts an MTP data object receive sequence in-progress in the connection 
    transport layer. This sequence is completed when the connection transport 
    layer signals ReceiveDataComplete to the MTP connection protocol layer 
    interface.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @see ReceiveData
    @see MMTPConnectionProtocol::ReceiveDataComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void ReceiveDataCancelL(const TMTPTypeRequest& aRequest) = 0;
    
    /**
    Initiates an MTP data object send sequence in the connection transport 
    layer. This sequence is completed when the transport layer connection 
    signals SendDataComplete to the MTP protocol layer notification interface.
    @param aData The MTP data object source.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @see MMTPConnectionProtocol::SendDataComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest) = 0;

    /**
    Aborts an MTP data object send sequence in-progress in the connection 
    transport layer. This sequence is completed when the connection transport 
    layer signals SendDataComplete to the MTP connection protocol layer 
    interface.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @see SendData
    @see MMTPConnectionProtocol::SendDataComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SendDataCancelL(const TMTPTypeRequest& aRequest) = 0;
    
    /**
    Initiates an MTP event dataset send sequence in the connection 
    transport layer. This sequence is completed when the connection transport 
    layer signals SendEventComplete to the MTP connection protocol layer 
    interface.
    @param aEvent The MTP event data object source.
    @see MMTPConnectionProtocol::SendEventComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SendEventL(const TMTPTypeEvent& aEvent) = 0;
    
    /**
    Initiates an MTP response dataset send sequence in the connection 
    transport layer. This sequence is completed when the connection transport 
    layer signals SendResponseComplete to the MTP connection protocol layer 
    interface.
    @param aResponse The MTP response data object source.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @see MMTPConnectionProtocol::SendResponseComplete
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest) = 0;
    
    /**
    Signals the completion of the active MTP operation request transaction 
    sequence initiated by a preceding ReceivedRequest signal made to the
    MTP protocol layer notification interface.
    @param aResponse The completed MTP request data object.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void TransactionCompleteL(const TMTPTypeRequest& aRequest) = 0;

    /**
    Unbinds the MTP protocol layer notification interface from the connection transport 
    layer.
    @param aProtocol The MTP protocol layer notification interface.
    */
    virtual void Unbind(MMTPConnectionProtocol& aProtocol) = 0;
    
    /**
    Provides an MTP connection transport layer extension interface implementation 
    for the specified interface Uid. 
    @param aInterfaceUid Unique identifier for the extension interface being 
    requested.
    @return Pointer to an interface instance or 0 if the interface is not 
    supported. Ownership is NOT transfered.
    */
    virtual TAny* GetExtendedInterface(TUid aInterfaceUid) = 0;
    
    /*
     * Provide a method to get implementation UID of concrete transport connection bearer
     * @return Implementation UID of concrete transport connection bearer.
     */
    virtual TUint GetImplementationUid() = 0;
};
    
#endif // MMTPTRANSPORTCONNECTION_H
