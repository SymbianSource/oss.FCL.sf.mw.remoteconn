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

#ifndef MMTPTRANSACTIONPROXY_H
#define MMTPTRANSACTIONPROXY_H

#include <e32cmn.h>
#include <e32def.h>

class MMTPConnection;
class MMTPObjectMgr;
class MMTPPuidMgr;
class MMTPReferenceMgr;
class MMTPType;
class TMTPTypeEvent;
class TMTPTypeResponse;
class TMTPTypeRequest;
class RFs;

/** 
Defines the MTP framework transaction proxy handler application interface.
This is an internal interface that allows a data provider to complete the 
processing of a delegated (proxy) request.
@internalComponent
 
*/
class MMTPTransactionProxy
    {
public:
    
    /**
    Initiates a data object receive sequence in the MTP transaction proxy 
    handler. This is an asynchronous method. This method should only be invoked
    when processing the ERequestPhase of an MTP transaction. 
    @param aData The MTP data object sink buffer.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed. 
    @param aStatus The status used to return asynchronous completion 
    information regarding the request.
    @leave KErrNotReady, if invoked when the current MTP transaction phase is 
    not ERequestPhase.
    */
    virtual void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus) = 0;
    
    /**
    Initiates a data object send sequence in the MTP transaction proxy 
    handler. This is an asynchronous method. This method should only be invoked
    when processing the ERequestPhase of an MTP transaction. 
    @param aData The MTP data object source buffer.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @param aStatus The status used to return asynchronous completion 
    information regarding the request.
    @leave KErrNotReady, if invoked when the current MTP transaction phase is 
    not ERequestPhase.
    */
    virtual void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus) = 0;
    
    /**
    Initiates a data object an MTP response dataset send sequence in the MTP 
    transaction proxy handler. This is an asynchronous method. This method 
    should only be invoked when processing either the ERequestPhase or 
    EResponsePhase of an MTP transaction.
    @param aData The MTP aResponse dataset source buffer.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @param aStatus The status used to return asynchronous completion 
    information regarding the request.
    @leave KErrNotReady, if invoked when the current MTP transaction phase is 
    not ERequestPhase or EResponsePhase.
    */
    virtual void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus) = 0;
    
    /**
    Signals to the MTP transaction proxy handler that all processing related 
    to the current transaction has been completed. This method should only be 
    invoked when processing the ECompletingPhase of the MTP transaction 
    @param aRequest The MTP request dataset that initiated the transaction.
    @param aConnection The handle of the MTP connection on which the transaction
    is being processed.
    @param aStatus The status used to return asynchronous completion 
    information regarding the request.
    @leave KErrNotReady If invoked when the current MTP transaction phase is 
    invalid.
    */
    virtual void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;
    };
    
#endif // MMTPTRANSACTIONPROXY_H