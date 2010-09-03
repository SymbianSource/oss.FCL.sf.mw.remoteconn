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

#ifndef CMTPCONNECTION_H
#define CMTPCONNECTION_H

#include <e32property.h>
#include <e32base.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/tmtptyperesponse.h>
#include <mtp/rmtpclient.h>

#include "mmtpconnectionprotocol.h"
#include "rmtpframework.h"
#include "mtpdebug.h"
#include "mtpbuildoptions.hrh"
#include "mtpdebug.h"

class CMTPSession;
class MMTPTransportConnection;
class TRequestStatus;
class TMTPTypeRequest;
class TMTPTypeFlatBase;

#ifdef MTP_CAPTURE_TEST_DATA
class CMTPRequestLogger;
#endif

/** 
Implements the connection level portion of the MTP communication model and 
associated transport layer protocol binding. This class Implements the API 
portion (@see MMTPConnectionProtocol) of the transport layer API/SPI interface 
pair by which MTP framework and transport protocol connections 
(@see MMTPTransportConnection) interact. A unique @see CMTPConnection instance 
is created and bound to each @see MMTPTransportConnection instance that is 
loaded. 

It also implements the data provider layer MTP connection interface 
(@see MMTPConnection) which represents the virtual circuit between data provider 
and the transport layer.
@internalComponent
 
*/
class CMTPConnection : 
    public CBase,
    public MMTPConnection, 
    public MMTPConnectionProtocol
    {
private:

	class CMTPEventLink :
		public CBase
		{
	public:
		CMTPEventLink(const TMTPTypeEvent& aEvent);
		
	public:
		TSglQueLink		iLink;
		TMTPTypeEvent	iEvent;		
		};
    
public:

    static CMTPConnection* NewLC(TUint aConnectionId, MMTPTransportConnection& aTransportConnection);
    ~CMTPConnection();

    void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus);
    void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus);
    void SendEventL(const TMTPTypeEvent& aEvent);
    void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus);
    IMPORT_C void SessionClosedL(TUint32 aSessionMTPId);
    IMPORT_C void SessionOpenedL(TUint32 aSessionMTPId);
    IMPORT_C TInt GetDataReceiveResult() const;
    TBool ConnectionSuspended();
    void ConnectionResumedL(MMTPTransportConnection& aTransportConnection);
    void TransactionCompleteL(const TMTPTypeRequest& aRequest);
    
public: // From MMTPConnection & MMTPConnectionProtocol

    TUint ConnectionId() const;
    
public: // From MMTPConnection

    TUint SessionCount() const;
    TBool SessionWithMTPIdExists(TUint32 aMTPId) const;
    MMTPSession& SessionWithMTPIdL(TUint32 aMTPId) const;
    TBool SessionWithUniqueIdExists(TUint32 aUniqueId) const;
    MMTPSession& SessionWithUniqueIdL(TUint32 aUniqueId) const;
    TMTPTransactionPhase TransactionPhaseL(TUint32 aMTPSessionId) const;
	void DisconnectionNotifyL();

private: // From MMTPConnectionProtocol

    void ReceivedEventL(const TMTPTypeEvent& aEvent);
    void ReceivedRequestL(const TMTPTypeRequest& aRequest);
    void ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest);
    void SendDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest);
    void SendEventCompleteL(TInt aErr, const TMTPTypeEvent& aEvent);
    void SendResponseCompleteL(TInt aErr, const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest);
    void Unbind(MMTPTransportConnection& aConnection);

private:

    CMTPConnection(TUint aConnectionId, MMTPTransportConnection& aTransportConnection);
    void ConstructL();
  
    void InitiateMTPErrorRecoveryL(const TMTPTypeRequest& aRequest, TUint16 aResponseCode);
    void MTPErrorRecoveryComplete();
    void UnrecoverableMTPError();
    
    void InitiateTransactionCancelL(TInt aSessionIdx);
    
    TUint ActiveSessions() const;
    void CloseAllSessions();
    void CloseSession(TUint aIdx);
    void CompleteCloseConnection();
    CMTPSession& SessionL(const TMTPTypeFlatBase& aDataset, TInt aSessionIdElementNo) const;
    
    static TInt SessionOrder(const TUint32* aL, const CMTPSession& aR);
    static TInt SessionOrder(const CMTPSession& aL, const CMTPSession& aR);
    
    void SetState(TUint aState);
    TUint State() const;
    
    TBool ValidFrameworkRequest(CMTPSession* aSession, TUint aValidStates, TRequestStatus* aStatus);
    
    void RemoveEventsForSession(TUint32 aMTPId);
    
    void EnqueueEvent(CMTPEventLink* aLink);
    void DequeueEvent(CMTPEventLink* aLink);
	void DequeueAllEvents();

/**
  * This method define and attach the property for publishing connection state 
  *  events.
  */
    void DefineConnStatePropertyL(); 

/**
  * This method is to publish various connection state.
  */
    void PublishConnState(TMTPConnStateType aConnState);	

    void ValidateAndPublishConnState(CMTPSession& aSession, TInt aState);
	
    
private: // Owned
  
    enum TStates
        {
        EStateUnknown,
        EStateOpen,
        EStateErrorRecovery,
        EStateErrorShutdown,
        EStateShutdown
        };

    TUint                       iConnectionId;
    RPointerArray<CMTPSession>  iSessions;
    TUint                       iState;
    RMTPFramework               iSingletons;
    TAny*                       iExtendedInterface;
    TMTPTypeResponse 			iResponse;
    TSglQue<CMTPEventLink>		iEventQ;
    TInt 						iPendingEventCount;   
    TInt 						iDataReceiveResult;
    
private: // Not owned

    MMTPTransportConnection*    iTransportConnection;

    //For publishing the connection state.
    RProperty iProperty; 
	
    /**
    Special build, capture request, data phase and responses to disk.
    */
#ifdef MTP_CAPTURE_TEST_DATA 
    CMTPRequestLogger*          iRequestLogger;
#endif
    };
    
#endif // CMTPCONNECTION_H
