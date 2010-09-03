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

#ifndef CMTPSESSION_H
#define CMTPSESSION_H

#include <e32base.h>
#include <mtp/mmtpsession.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeevent.h>

/** 
Implements the session level portion of the MTP communication model. It also 
implements the data provider layer MTP session interface (@see MMTPSession).
@internalComponent
 
*/
class CMTPSession : 
    public CBase,
    public MMTPSession
    {
public:

    static CMTPSession* NewLC(TUint32 aMTPId, TUint aUniqueId);
    ~CMTPSession();    
    
    TUint32 ExpectedTransactionId() const;
    void IncrementExpectedTransactionId();
    
    void SetActiveRequestL(const TMTPTypeRequest& aRequest);
    void SetTransactionPhase(TMTPTransactionPhase aPhase);
    TMTPTransactionPhase TransactionPhase() const;
    TInt RouteRequest(const TMTPTypeRequest& aRequest);
    void RouteRequestRegisterL(const TMTPTypeRequest& aRequest, TInt aDpId);
    TBool RouteRequestRegistered(TUint16 aOpCode) const;
    void RouteRequestUnregister(const TMTPTypeRequest& aRequest);
    TBool CheckPendingEvent(const TMTPTypeRequest& aRequest) const;
    void StorePendingEventL(const TMTPTypeEvent& aEvent);
    const TMTPTypeEvent& PendingEvent() const;
    
    void CompletePendingRequest(TInt aErr);
    TBool RequestPending() const;
    void SetRequestPending(TRequestStatus& aStatus);

public: // From MMTPSession

    const TMTPTypeRequest& ActiveRequestL() const;
    TUint32 SessionMTPId() const;
    TUint SessionUniqueId() const;
    TAny* GetExtendedInterface(TUid aInterfaceUid);
    
private:

    CMTPSession(TUint32 aMTPId, TUint aUniqueId);
    void ConstructL();
    
    static TBool RouteRequestMatchOpCode(const TUint16* aOpCode, const TMTPTypeRequest& aRequest);
    static TInt RouteRequestOrder(const TMTPTypeRequest& aLeft, const TMTPTypeRequest& aRight);
    
private: // Owned

    TUint32                 iExpectedTransactionId;
    TUint32                 iIdMTP;
    TUint                   iIdUnique;
    TMTPTransactionPhase    iTransactionPhase;
    RArray<TMTPTypeRequest> iRoutingRegistrations;
    TAny*                   iExtendedInterface;
    TMTPTypeEvent           iPendingEvent;
    TMTPTypeRequest         iActiveRequest;
    
private: // Not owned

    TRequestStatus*         iRequestStatus;
    };

#endif // CMTPSESSION_H
