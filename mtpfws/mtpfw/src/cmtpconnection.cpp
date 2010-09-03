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

#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptypeuint32.h>

#include "cmtpconnection.h"
#include "cmtpconnectionmgr.h"
#include "cmtpdataprovider.h"
#include "cmtpparserrouter.h"
#include "cmtpsession.h"
#include "mmtptransportconnection.h"

#ifdef MTP_CAPTURE_TEST_DATA
#include "cmtprequestlogger.h"
#endif

#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpconnectionTraces.h"
#endif

#define UNUSED_VAR(a) (a) = (a)

/**
CMTPConnection panics
*/
_LIT(KMTPPanicCategory, "CMTPConnection");
enum TMTPPanicReasons
    {
    EMTPPanicBusy = 0,
    EMTPPanicInvalidSession = 1,
    EMTPPanicInvalidState = 2,
    EMTPPanicPublishEvent = 3,
    };
    
LOCAL_C void Panic(TInt aReason)
    {
    User::Panic(KMTPPanicCategory, aReason);
    }

/**
CMTPConnection factory method. A pointer to the new CMTPConnection instance is
placed on the cleanup stack.
@param aConnectionId The unique identifier assigned to this connection by the 
MTP framework. 
@param aTransportConnection The MTP transport layer connection interface to 
which the CMTPConnection will bind.
@return Pointer to the new CMTPConnection instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPConnection* CMTPConnection::NewLC(TUint aConnectionId, MMTPTransportConnection& aTransportConnection)
    {
    CMTPConnection* self = new(ELeave) CMTPConnection(aConnectionId, aTransportConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/
CMTPConnection::~CMTPConnection()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CMTPCONNECTION_DES_ENTRY );
    CloseAllSessions();
    
	//remove all events
    DequeueAllEvents();
	
    iSessions.ResetAndDestroy();
    
    if (iTransportConnection != NULL)
        {
        iTransportConnection->Unbind(*this);
        }
    
    //close the property
    iProperty.Close();
    // delete the ‘name?property
    RProcess process;
    RProperty::Delete(process.SecureId(), EMTPConnStateKey);
    OstTraceFunctionExit0( CMTPCONNECTION_CMTPCONNECTION_DES_EXIT );
    }

/**
Initiates MTP transaction data phase processing for initiator-to-responder
data flows. This method should only be invoked when the MTP transaction phase 
state is ERequestPhase. This is an asynchronous method.
@param aData The MTP data object sink.
@param aRequest The MTP request dataset of the active MTP transaction.
@param aStatus The status used to return asynchronous completion 
information regarding the request.
@leave KErrNotFound If the MTP request dataset specifies an invalid SessionID.
@panic CMTPConnection 0 If an asynchronous request is already pending on the 
connection.
@panic CMTPConnection 2 If the MTP transaction phase is invalid.
*/
void CMTPConnection::ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus)
    {   
    OstTraceFunctionEntry0( CMTPCONNECTION_RECEIVEDATAL_ENTRY );
    
    iDataReceiveResult = KErrNone;
    const TUint KValidPhases(ERequestPhase);
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID));
    
    if (ValidFrameworkRequest(&session, KValidPhases, &aStatus))
        {
        session.SetTransactionPhase(EDataIToRPhase);
        session.SetRequestPending(aStatus);
        if(EMTPTypeFile == aData.Type())
            {
            ValidateAndPublishConnState(session, State());
            }
        
        iTransportConnection->ReceiveDataL(aData, aRequest);      
        }
    OstTraceFunctionExit0( CMTPCONNECTION_RECEIVEDATAL_EXIT );
    }

/**
Initiates MTP transaction data phase processing for responder-to-initiator
data flows. This method should only be invoked when the MTP transaction phase
state is ERequestPhase. This is an asynchronous method.
@param aData The MTP data object source.
@param aRequest The MTP request dataset of the active MTP transaction.
@param aStatus The status used to return asynchronous completion 
information regarding the request.
@leave KErrNotFound If the MTP request dataset specifies an invalid SessionID.
@panic CMTPConnection 0 If an asynchronous request is already pending on the 
connection.
@panic CMTPConnection 2 If the MTP transaction phase is invalid.
*/
void CMTPConnection::SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDDATAL_ENTRY );
#ifdef MTP_CAPTURE_TEST_DATA
    iRequestLogger->WriteDataPhaseL(aData, EDataRToIPhase);
#endif

    const TUint KValidPhases(ERequestPhase);
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID));
    
    if (ValidFrameworkRequest(&session, KValidPhases, &aStatus))
        {
        session.SetTransactionPhase(EDataRToIPhase);
        session.SetRequestPending(aStatus);
        if(EMTPTypeFile == aData.Type())
            {
            //In this case we should validate the state based on transaction phase then
            //publish the connection state info to the subscriber.
            ValidateAndPublishConnState(session, State());
            }
        iTransportConnection->SendDataL(aData, aRequest);
        }
    OstTraceFunctionExit0( CMTPCONNECTION_SENDDATAL_EXIT );
    }

/**
Sends an MTP event dataset.
@param aEvent The MTP event dataset source.
@leave KErrNotFound If the MTP event dataset specifies an invalid SessionID.
*/
void CMTPConnection::SendEventL(const TMTPTypeEvent& aEvent)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDEVENTL_ENTRY );
    const TUint KValidPhases(EIdlePhase | ERequestPhase | EDataIToRPhase| EDataRToIPhase | EResponsePhase | ECompletingPhase);
    if (ValidFrameworkRequest(NULL, KValidPhases, NULL))
        {
        // Validate the SessionID
        TUint32 sessionId(aEvent.Uint32(TMTPTypeEvent::EEventSessionID));
        if (sessionId != KMTPSessionAll)
            {
            TInt ret = iSessions.FindInOrder(sessionId, SessionOrder);
            LEAVEIFERROR(ret, 
                    OstTrace1(TRACE_ERROR, CMTPCONNECTION_SENDEVENTL, "can't find according to session_id %d", sessionId));
            }
            

		EnqueueEvent(new (ELeave) CMTPEventLink(aEvent));
		if (iPendingEventCount == 1)
			{
			// Forward the event to the transport connection layer.
			iTransportConnection->SendEventL(iEventQ.First()->iEvent);			
			}
        }
    OstTraceFunctionExit0( CMTPCONNECTION_SENDEVENTL_EXIT );
    }

/**
Initiates MTP transaction response phase processing. This method should only 
be invoked when the MTP transaction phase state is either ERequestPhase, or 
EResponsePhase. This is an asynchronous method.
@param aResponse The MTP response dataset source.
@param aRequest The MTP request dataset of the active MTP transaction.
@param aStatus The status used to return asynchronous completion 
information regarding the request.
@leave KErrNotFound If the MTP response dataset specifies an invalid SessionID.
@leave KErrArgument If the MTP response dataset does not match the specified 
request dataset.
@panic CMTPConnection 0 If an asynchronous request is already pending on the 
connection.
@panic CMTPConnection 2 If the MTP transaction phase is invalid.
*/
void CMTPConnection::SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDRESPONSEL_ENTRY );
#ifdef MTP_CAPTURE_TEST_DATA
    // Running under debug capture mode save this request off to disk.
    iRequestLogger->LogResponseL(aResponse);
#endif
    
    const TUint KValidPhases(ERequestPhase | EResponsePhase );
    CMTPSession& session(SessionL(aResponse, TMTPTypeResponse::EResponseSessionID));
    
    if (ValidFrameworkRequest(&session, KValidPhases, &aStatus))
        {
        if ((aResponse.Uint32(TMTPTypeResponse::EResponseSessionID) != aRequest.Uint32(TMTPTypeRequest::ERequestSessionID)) ||
            (aResponse.Uint32(TMTPTypeResponse::EResponseTransactionID) != aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID)))
            {
            /* 
            Request/Response mismatch by the originating data provider plug-in. 
            Fail the transport connection to avoid leaving the current 
            transaction irrecoverably hanging.
            */
            UnrecoverableMTPError();
            OstTrace0(TRACE_ERROR, CMTPCONNECTION_SENDRESPONSEL, "Request/Response mismatch");
            User::Leave(KErrArgument);   
            }

        if (session.TransactionPhase() == ERequestPhase)
            {
            // Transaction has no data phase.
            session.SetTransactionPhase(EResponsePhase);
            }
            
        session.SetRequestPending(aStatus);
        
        iTransportConnection->SendResponseL(aResponse, aRequest);
        }
    OstTraceFunctionExit0( CMTPCONNECTION_SENDRESPONSEL_EXIT );
    }

/**
Deletes the session object assigned to the specified session and notifies all 
loaded data providers.
@param aMTPId The session identifier assigned by the MTP connection 
initiator.
@leave KErrNotFound, if a session with the specified SessionMTPId is not 
found.
@leave One of the system wide error codes, if a general processing failure 
occurs.
*/
EXPORT_C void CMTPConnection::SessionClosedL(TUint32 aMTPId)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SESSIONCLOSEDL_ENTRY );
    if(0x0FFFFFFF != aMTPId)
    	{
    	TInt idx(iSessions.FindInOrder(aMTPId, SessionOrder));
	    __ASSERT_DEBUG((idx != KErrNotFound), Panic(EMTPPanicInvalidSession));
	    CloseSession(idx);
	    }
    else
	    {
	    CMTPSession* session = NULL;
	    for(TInt i =0;i<iSessions.Count();i++)
		    {
		    session = iSessions[i];
		    TUint id(session->SessionMTPId());	
	    	if(0 != id)
		    	{
		    	SessionClosedL(id);	
		    	}
		    session = NULL;
		    }
	    }
    OstTraceFunctionExit0( CMTPCONNECTION_SESSIONCLOSEDL_EXIT );
    }

/**
Creates a new session object for the specified session and notifies all loaded
data providers. The session is known by two identifiers:
    1. SessionMTPId - Assigned by the MTP connection initiator and unique only 
        to the MTP connection on which the session was opened. In a multiple-
        connection configuration this identifier may not uniquely identify the 
        session.
    2. SessionUniqueId - Assigned by the MTP daemon and guaranteed to uniquely
        identify the session in a multiple-connection configuration.
Currently the MTP daemon does not support multiple-connection configuration 
and both identifiers are assigned the same value.
@param aMTPId The session identifier assigned by the MTP connection 
initiator.
@leave KErrAlreadyExists, if a session with the specified SessionMTPId is 
already open.
@leave One of the system wide error codes, if a general processing failure 
occurs.
*/
EXPORT_C void CMTPConnection::SessionOpenedL(TUint32 aMTPId)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SESSIONOPENEDL_ENTRY );
    // Validate the SessionID
    if (SessionWithMTPIdExists(aMTPId))
        {
        OstTrace1(TRACE_ERROR, CMTPCONNECTION_SESSIONOPENEDL, "session %d alreay exist", aMTPId);
        User::Leave(KErrAlreadyExists);   
        }
    
    // Create a new session object
    CMTPSession* session = CMTPSession::NewLC(aMTPId, aMTPId);
    session->SetTransactionPhase(EIdlePhase);
    iSessions.InsertInOrder(session, CMTPConnection::SessionOrder);
    CleanupStack::Pop(session); 
    
    if (aMTPId != KMTPSessionNone)
        {
        // Notify the data providers if other than the null session is closing.
        // OpenSession Command coming and now it is really connected to host.
        PublishConnState(EConnectedToHost);
        TMTPNotificationParamsSessionChange params = {aMTPId, *this};
        iSingletons.DpController().NotifyDataProvidersL(EMTPSessionOpened, &params);
        }
    OstTraceFunctionExit0( CMTPCONNECTION_SESSIONOPENEDL_EXIT );
    }

/*
 * Signals the connection is suspended, the connection state is set to EStateShutdown which 
 * means that all the current transaction will not be able to send/receive any data via the
 * connection
 * @return ETrue - there is an active transaction currently, and Connection will suspend when it finishes
 * EFalse - No active transaction, connection suspends immediately.
 */
TBool CMTPConnection::ConnectionSuspended()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CONNECTIONSUSPENDED_ENTRY );
    
    TBool ret = EFalse;
    TUint currentState = State();
    if (currentState != EStateShutdown)
        {
        if (ActiveSessions() == 0 || currentState == EStateErrorShutdown)
            {
            CompleteCloseConnection();
            ret = ETrue;
            }
          
        SetState(EStateShutdown);
        PublishConnState(EDisconnectedFromHost);   
        }
    
    OstTraceFunctionExit0( CMTPCONNECTION_CONNECTIONSUSPENDED_EXIT );
    return ret;
    }

void CMTPConnection::CompleteCloseConnection()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_COMPLETECLOSECONNECTION_ENTRY );
    
    CloseAllSessions();
    iSessions.Reset();
    
    //notify ConnectionMgr and corresponding transports of completion of connection close
    iSingletons.ConnectionMgr().ConnectionCloseComplete(iConnectionId);    
    iSingletons.Close();

    OstTraceFunctionExit0( CMTPCONNECTION_COMPLETECLOSECONNECTION_EXIT );
    }

/*
 * Signals that the connection is resumed to EStateOpen state which means that data providers
 * can receive requests from host again.
 * @aTransportConnection The new transport connection object
 * @leave One of the system wide error codes, if a processing failure occurs.
 */
void CMTPConnection::ConnectionResumedL(MMTPTransportConnection& aTransportConnection)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CONNECTIONRESUMEDL_ENTRY );
    
    TUint currentState = State();
    if (currentState != EStateOpen && currentState != EStateErrorRecovery)
        {
        iSingletons.OpenL();
        
        /* 
        Create the null session object which owns the transaction state for 
        transaction occuring outside a session (i.e. with SessionID == 0x00000000)
        */
        SessionOpenedL(KMTPSessionNone);            
        
        if (iTransportConnection != NULL)
            {
            iTransportConnection->Unbind(*this);
            }
        iTransportConnection = &aTransportConnection;
        iTransportConnection->BindL(*this);
        SetState(EStateOpen); 
         
        }
    
    OstTraceFunctionExit0( CMTPCONNECTION_CONNECTIONRESUMEDL_EXIT );
    }

/**
Signals the completion of the current transaction processing sequence. This
method should only be invoked when the MTP transaction phase state is 
ECompletingPhase.
@param aRequest The MTP request dataset of the completed MTP transaction.
@leave KErrNotFound If the MTP request dataset specifies an invalid SessionID.
@panic CMTPConnection 2 If the MTP transaction phase is invalid.
*/
void CMTPConnection::TransactionCompleteL(const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_TRANSACTIONCOMPLETEL_ENTRY );

    const TUint KValidPhases(ECompletingPhase);
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID));    

    if (ValidFrameworkRequest(&session, KValidPhases, NULL))
        {
        session.SetTransactionPhase(EIdlePhase);
        
		if (iTransportConnection != NULL)
            {
            iTransportConnection->TransactionCompleteL(aRequest);
            }
			
        if (State() == EStateShutdown && ActiveSessions() == 0)
            {     
            CompleteCloseConnection();
            }
        }

    OstTraceFunctionExit0( CMTPCONNECTION_TRANSACTIONCOMPLETEL_EXIT );
    }
    
TUint CMTPConnection::ConnectionId() const
    {
    return iConnectionId;
    }
    
TUint CMTPConnection::SessionCount() const
    {
    return iSessions.Count();
    }

TBool CMTPConnection::SessionWithMTPIdExists(TUint32 aMTPId) const
    {
    return (iSessions.FindInOrder(aMTPId, SessionOrder) != KErrNotFound);
    }

MMTPSession& CMTPConnection::SessionWithMTPIdL(TUint32 aMTPId) const
    {
    TInt idx(iSessions.FindInOrder(aMTPId, SessionOrder));
    LEAVEIFERROR(idx, 
            OstTrace1(TRACE_ERROR, CMTPCONNECTION_SESSIONWITHMTPIDL, "can't find according to session_id %d", aMTPId));
    return *iSessions[idx];
    }
    
TBool CMTPConnection::SessionWithUniqueIdExists(TUint32 aUniqueId) const
    {
    return SessionWithMTPIdExists(aUniqueId);
    }

MMTPSession& CMTPConnection::SessionWithUniqueIdL(TUint32 aUniqueId) const
    {
    return SessionWithMTPIdL(aUniqueId);
    }
    
void CMTPConnection::ReceivedEventL(const TMTPTypeEvent& aEvent)
    {  
    OstTraceFunctionEntry0( CMTPCONNECTION_RECEIVEDEVENTL_ENTRY );

    TInt idx(KErrNotFound);
    
    // Validate the SessionID.
    TUint32 sessionId(aEvent.Uint32(TMTPTypeEvent::EEventSessionID));
    if (sessionId != KMTPSessionAll)
        {
        idx = iSessions.FindInOrder(sessionId, SessionOrder);
        LEAVEIFERROR(idx, 
                OstTrace1(TRACE_ERROR, CMTPCONNECTION_RECEIVEDEVENTL, "can't find according to session_id %d", sessionId));
        }
       
    // Check that this event is valid.
    CMTPSession& session(*iSessions[idx]);

    TMTPTypeRequest request; 
    TRAPD( err, MMTPType::CopyL(session.ActiveRequestL(), request) );


    if( err == KErrNotFound  )
    	{
    	session.StorePendingEventL(aEvent);
    	}
    else
    	{
        TUint32 requestTransactionId = request.Uint32(TMTPTypeRequest::ERequestTransactionID);
        TUint32 eventTransactionId = aEvent.Uint32(TMTPTypeEvent::EEventTransactionID); 
	   	if (eventTransactionId > requestTransactionId)
	        {
	        // Event to be queued for future use, we can only queue one event at a time
	        session.StorePendingEventL(aEvent);
	        }
	   	else if (eventTransactionId == requestTransactionId)
	        {     
	        // Event is valid	     
	        // Perform transport layer processing.
	        if (aEvent.Uint16(TMTPTypeEvent::EEventCode) == EMTPEventCodeCancelTransaction)
	            {
	            if (sessionId == KMTPSessionAll)
	                {
	                const TUint noSessions = iSessions.Count();
	                for (TUint i(0); (i < noSessions); i++)
	                    {
	                    InitiateTransactionCancelL(i);
	                    } 
	                }
	            else
	                {
	                InitiateTransactionCancelL(idx);
	                }
	            }
	        
	        // Forward the event to the DP framework layer.
	        iSingletons.Router().ProcessEventL(aEvent, *this); 
	        }
	   	
	   	//discard the event if the event transaction id < requestion transaction id
	   	
    	}	

    OstTraceFunctionExit0( CMTPCONNECTION_RECEIVEDEVENTL_EXIT );
    }

void CMTPConnection::ReceivedRequestL(const TMTPTypeRequest& aRequest)
    {  
    OstTraceFunctionEntry0( CMTPCONNECTION_RECEIVEDREQUESTL_ENTRY );
    
#ifdef MTP_CAPTURE_TEST_DATA
    // Running under debug capture mode save this request off to disk.
    iRequestLogger->LogRequestL(aRequest);
#endif

    // Resolve the session    
    TInt idx(iSessions.FindInOrder(aRequest.Uint32(TMTPTypeRequest::ERequestSessionID), SessionOrder));
	
    // Process the request.
    if (idx == KErrNotFound)
        {
        // Invalid SessionID
        InitiateMTPErrorRecoveryL(aRequest, EMTPRespCodeSessionNotOpen);
        }
    else
        {           
        CMTPSession& session(*iSessions[idx]);
        
        if (session.TransactionPhase() != EIdlePhase)
            {
            // Initiator violation of the MTP transaction protocol.
            UnrecoverableMTPError();
            }
        else
            {
            // Set the session state
            session.IncrementExpectedTransactionId();
            session.SetTransactionPhase(ERequestPhase);
            session.SetActiveRequestL(aRequest);

            // Forward the request to the DP framework layer.
            TRAPD(err,iSingletons.Router().ProcessRequestL(session.ActiveRequestL(), *this));   
            if(err!=KErrNone)
                {
                session.SetTransactionPhase(EIdlePhase);
                OstTrace1(TRACE_ERROR, CMTPCONNECTION_RECEIVEDREQUESTL, "router process request error! error code %d", err);
                User::Leave(err);
                }
            }
        }
    OstTraceFunctionExit0( CMTPCONNECTION_RECEIVEDREQUESTL_EXIT );
    }

#ifdef MTP_CAPTURE_TEST_DATA
void CMTPConnection::ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
#else
void CMTPConnection::ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
#endif
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_RECEIVEDATACOMPLETEL_ENTRY );
    
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID)); 
    __ASSERT_DEBUG((session.TransactionPhase() == EDataIToRPhase), Panic(EMTPPanicInvalidState));
    
	if(EMTPTypeFile == aData.Type())
	{
		//All data transfer is over now we can publish that
		//state is connected to host or idle
		PublishConnState(EConnectedToHost);
	}
#ifdef MTP_CAPTURE_TEST_DATA
    iRequestLogger->WriteDataPhaseL(aData, EDataIToRPhase);
#endif
    
	session.SetTransactionPhase(EResponsePhase);
	iDataReceiveResult = aErr;
	session.CompletePendingRequest(aErr);
	OstTraceFunctionExit0( CMTPCONNECTION_RECEIVEDATACOMPLETEL_EXIT );
    }

void CMTPConnection::SendDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDDATACOMPLETEL_ENTRY );   
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID)); 
    __ASSERT_DEBUG((session.TransactionPhase() == EDataRToIPhase), Panic(EMTPPanicInvalidState));

    session.SetTransactionPhase(EResponsePhase);
   
    if(EMTPTypeFile == aData.Type())
        {
        //All data transfer is over now we can publish that
        //state is connected to host or idle
        PublishConnState(EConnectedToHost);
        }
    
    session.CompletePendingRequest(aErr);
    
    OstTraceFunctionExit0( CMTPCONNECTION_SENDDATACOMPLETEL_EXIT );
    }

void CMTPConnection::SendEventCompleteL(TInt aErr, const TMTPTypeEvent& aEvent)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDEVENTCOMPLETEL_ENTRY );

    
    if (aErr != KErrNone)
        {
        UnrecoverableMTPError();            
        }    
    else if (aEvent.Uint16(TMTPTypeEvent::EEventCode) == EMTPEventCodeCancelTransaction)
        {
        TUint32 sessionId(aEvent.Uint32(TMTPTypeEvent::EEventSessionID));
        TInt idx(KErrNotFound);
        if (sessionId == KMTPSessionAll)
            {
            const TUint noSessions = iSessions.Count();
            for (TUint i(0); (i < noSessions); i++)
                {
                InitiateTransactionCancelL(i);
                }    
            }
        else
            {
            idx = iSessions.FindInOrder(sessionId, SessionOrder);
            InitiateTransactionCancelL(idx);
            }   
        }
    // Dequeue first since the code below might leave.
    DequeueEvent(iEventQ.First());       
   	if (iPendingEventCount > 0)
   		{
		if (NULL != iTransportConnection)
			{
			// Forward the event to the transport connection layer.
			OstTrace0( TRACE_NORMAL, CMTPCONNECTION_SENDEVENTCOMPLETEL, "Sending queued event");
			iTransportConnection->SendEventL(iEventQ.First()->iEvent);
			}
		else
			{
			DequeueAllEvents();
			}
   		}
    
    OstTraceFunctionExit0( CMTPCONNECTION_SENDEVENTCOMPLETEL_EXIT );
    }

void CMTPConnection::SendResponseCompleteL(TInt aErr, const TMTPTypeResponse& /*aResponse*/, const TMTPTypeRequest& aRequest)
    {   
    OstTraceFunctionEntry0( CMTPCONNECTION_SENDRESPONSECOMPLETEL_ENTRY );
    
	if(iState == EStateErrorRecovery)
		{
		MTPErrorRecoveryComplete();	    
		iTransportConnection->TransactionCompleteL(aRequest);
		}
	else{
        CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID)); 
	    __ASSERT_DEBUG((session.TransactionPhase() == EResponsePhase), Panic(EMTPPanicInvalidState));
    	session.SetTransactionPhase(ECompletingPhase);
    	session.CompletePendingRequest(aErr);
		}
    OstTraceFunctionExit0( CMTPCONNECTION_SENDRESPONSECOMPLETEL_EXIT );
    }

void CMTPConnection::Unbind(MMTPTransportConnection& /*aConnection*/)
    {
    iTransportConnection = NULL;
    }

   
TMTPTransactionPhase CMTPConnection::TransactionPhaseL(TUint32 aMTPId) const
    {
    TInt idx(iSessions.FindInOrder(aMTPId, SessionOrder));
    LEAVEIFERROR(idx, 
            OstTrace1(TRACE_ERROR, CMTPCONNECTION_TRANSACTIONPHASEL, "can't find according to session_id %d", aMTPId));
    return iSessions[idx]->TransactionPhase();
    }

/**
Constructor.
*/
CMTPConnection::CMTPConnection(TUint aConnectionId, MMTPTransportConnection& aTransportConnection) :
    iConnectionId(aConnectionId),
    iEventQ(_FOFF(CMTPEventLink, iLink)),
    iTransportConnection(&aTransportConnection)
    {
    iTransportConnection->BindL(*this);
    }
    
/**
Second phase constructor.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPConnection::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CONSTRUCTL_ENTRY );   
    //define the property for publishing connection state.
    DefineConnStatePropertyL();
    PublishConnState(EDisconnectedFromHost);  
#ifdef MTP_CAPTURE_TEST_DATA
    // Running under debug capture mode save this request off to disk.
    iRequestLogger = CMTPRequestLogger::NewL();
#endif
    OstTraceFunctionExit0( CMTPCONNECTION_CONSTRUCTL_EXIT );
    }
    
/**
Initiates an MTP connection level protocol error recovery sequence. This 
sequence is invoked when a recoverable protocol error is detected that cannot 
be processed above the connection layer, e.g. when a request is made on a 
non-existant SessionID, or an out-of-sequence TransactionID is detected. An
appropriate MTP response dataset is formed and sent to the MTP initiator. The
error recovery sequence is concluded by MTPErrorRecoveryComplete when the 
connection transport layer signals SendResponseComplete to the MTP connection 
protocol layer.
@param aRequest The MTP request dataset of the erroneous MTP transaction.
@param aResponseCode The MTP response datacode to be returned to the MTP
initiator.
@leave One of the system wide error codes, if a processing failure occurs.
@see MTPErrorRecoveryComplete
*/
void CMTPConnection::InitiateMTPErrorRecoveryL(const TMTPTypeRequest& aRequest, TUint16 aResponseCode)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_INITIATEMTPERRORRECOVERYL_ENTRY );   
    // Populate error response.
    iResponse.Reset();
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aResponseCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, aRequest.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));
    
    // Set the connection state pending completion, and send the response.
    SetState(EStateErrorRecovery);
    iTransportConnection->SendResponseL(iResponse, aRequest);
    OstTraceFunctionExit0( CMTPCONNECTION_INITIATEMTPERRORRECOVERYL_EXIT );
    }
    
/**
Concludes an MTP connection level protocol error recovery sequence.
@see InitiateMTPErrorRecoveryL
*/
void CMTPConnection::MTPErrorRecoveryComplete()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_MTPERRORRECOVERYCOMPLETE_ENTRY ); 
    SetState(EStateOpen);
    PublishConnState(EConnectedToHost);	
    OstTraceFunctionExit0( CMTPCONNECTION_MTPERRORRECOVERYCOMPLETE_EXIT );
    }
    
/**
Forces the immediate shutdown of the MTP connection. This is invoked when a 
protocol error is detected that cannot be recovered from, e.g. if an attempt
is detected to initiate an MTP transaction before a previous transaction has 
concluded.
*/
void CMTPConnection::UnrecoverableMTPError()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_UNRECOVERABLEMTPERROR_ENTRY ); 
    SetState(EStateErrorShutdown);
    PublishConnState(EDisconnectedFromHost);		
    iTransportConnection->CloseConnection();
    OstTraceFunctionExit0( CMTPCONNECTION_UNRECOVERABLEMTPERROR_EXIT );
    }

/**
Signals the MTP connection transport to terminate any in-progress data phase 
processing on the specified session.
@param aidx The sessions table index of the required session.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPConnection::InitiateTransactionCancelL(TInt aIdx)
    {    
    OstTraceFunctionEntry0( CMTPCONNECTION_INITIATETRANSACTIONCANCELL_ENTRY );   
    // Initiate transport connection level termination of the active data phase.
    CMTPSession& session(*iSessions[aIdx]);
    
    switch (session.TransactionPhase())
        {
    case EIdlePhase:
    case ECompletingPhase:
    case ERequestPhase:
        break;

    case EDataIToRPhase:
        iTransportConnection->ReceiveDataCancelL(session.ActiveRequestL());
        break;
        
    case EResponsePhase:
    case EDataRToIPhase:
        iTransportConnection->SendDataCancelL(session.ActiveRequestL());
        break;
        }
    OstTraceFunctionExit0( CMTPCONNECTION_INITIATETRANSACTIONCANCELL_EXIT );
    }

/**
Provides a count of the number of sessions with transactions in-progress.
*/
TUint CMTPConnection::ActiveSessions() const
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_ACTIVESESSIONS_ENTRY );   
    TUint active(0);
    const TUint count(iSessions.Count());
    for (TUint i(0); (i < count); i++)
        {
        if (iSessions[i]->TransactionPhase() > EIdlePhase)
            {
            active++;
            }
        }
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_ACTIVESESSIONS, "Active sessions = %d", active );  
    OstTraceFunctionExit0( CMTPCONNECTION_ACTIVESESSIONS_EXIT );
    return active;
    }

/**
Closes all sessions which have been opened on the connection.
*/
void CMTPConnection::CloseAllSessions()
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CLOSEALLSESSIONS_ENTRY );
	
    TInt count = iSessions.Count();
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_CLOSEALLSESSIONS, "Sessions number to be closed = %d", count );
	for (TInt i(count - 1); i>=0; i--)
		{
		CloseSession(i);
		}
	
    OstTraceFunctionExit0( CMTPCONNECTION_CLOSEALLSESSIONS_EXIT );
    }

/**
Closes the sessions with the specified session index.
@param aIdx The session index.
*/
void CMTPConnection::CloseSession(TUint aIdx)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_CLOSESESSION_ENTRY );
    
    OstTrace1(TRACE_NORMAL, CMTPCONNECTION_CLOSESESSION, "Session index to be closed = %d", aIdx);
    CMTPSession* session(iSessions[aIdx]);
        
    TUint id(session->SessionMTPId());
    if (id != KMTPSessionNone)
        {
        // Notify the data providers if other than the null session is closing.
        TMTPNotificationParamsSessionChange params = {id, *this};
        TRAPD(err, iSingletons.DpController().NotifyDataProvidersL(EMTPSessionClosed, &params));
        UNUSED_VAR(err);
        }
    
    // Remove any queued events for session
    RemoveEventsForSession(id);
    
    // Delete the session object.
    iSessions.Remove(aIdx);
    delete session;
    
    OstTraceFunctionExit0( CMTPCONNECTION_CLOSESESSION_EXIT );
    }
    
/**
Provides a reference to the session with the MTP connection assigned identifier
specified in the supplied MTP dataset. 
@param aDataset The MTP dataset.
@param aSessionIdElementNo The element number in the MTP dataset of the MTP 
connection assigned identifier.
@leave KErrNotFound If the specified session identifier is not currently
active on the connection.
@return The reference of the session with the specified MTP connection 
assigned identifier.
*/
CMTPSession& CMTPConnection::SessionL(const TMTPTypeFlatBase& aDataset, TInt aSessionIdElementNo) const
    {
    return static_cast<CMTPSession&>(SessionWithMTPIdL(aDataset.Uint32(aSessionIdElementNo)));
    }

/**
Implements an order relation for CMTPSession objects based on relative MTP
connection assigned session IDs.
@param aL The first MTP connection assigned session ID.
@param aR The second object.
@return Zero, if the two objects are equal; a negative value, if the aFirst 
is less than aSecond, or; a positive value, if the aFirst is greater than 
aSecond.
*/
TInt CMTPConnection::SessionOrder(const TUint32* aL, const CMTPSession& aR)
    {
    return *aL - aR.SessionMTPId();
    }

/**
Implements a TLinearOrder relation for CMTPSession objects based on relative 
MTP connection assigned session IDs.
@param aL The first object.
@param aR The second object.
@return Zero, if the two objects are equal; a negative value, if the first 
object is less than second, or; a positive value, if the first object is 
greater than the second.
*/
TInt CMTPConnection::SessionOrder(const CMTPSession& aL, const CMTPSession& aR)
    {
    return aL.SessionMTPId() - aR.SessionMTPId();
    }
    
/**
Get the data receive result.
@return the data recevice result.
*/
EXPORT_C TInt CMTPConnection::GetDataReceiveResult() const
	{
    OstTraceFunctionEntry0( CMTPCONNECTION_GETDATARECEIVERESULT_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_GETDATARECEIVERESULT, "Data receive result = %d", iDataReceiveResult);
    OstTraceFunctionExit0( CMTPCONNECTION_GETDATARECEIVERESULT_EXIT );
    return iDataReceiveResult;
	}
    
/**
Sets the MTP connection state variable.
@param aState The new MTP connection state value.
*/
void CMTPConnection::SetState(TUint aState)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_SETSTATE_ENTRY );   
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_SETSTATE, "CMTPConnection::SetState;aState=%d", aState );
    iState = aState;
    OstTraceFunctionExit0( CMTPCONNECTION_SETSTATE_EXIT );
    }
  
    
/**
Provide the current MTP connection state.
@return The current MTP connection state.
*/
TUint CMTPConnection::State() const
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_STATE_ENTRY );    
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_STATE, "State = %d", iState);
    OstTraceFunctionExit0( CMTPCONNECTION_STATE_EXIT );
    return iState;        
    }
    
/**
Performs common validation processing for requests initiated from the data 
provider framework layer. The following validation checks are performed.
    1.  Attempt to initiate concurrent asynchronous requests to the same 
        connection. This will result in a panic.
    2.  Attempt to initiate a request that is invalid for the current
        transaction phase. This will result in a panic.
    3.  Attempt to initiate a request when the connection is in an 
        unrecoverable error shutdown mode. This will result in the immediate
        cancellation of the request.
@return ETrue if the request is valid to proceed, otherwise EFalse.
@panic CMTPConnection 0 If an asynchronous request is already pending on the 
connection.
@panic CMTPConnection 2 If the MTP transaction phase is invalid.
*/
TBool CMTPConnection::ValidFrameworkRequest(CMTPSession* aSession, TUint aValidPhases, TRequestStatus* aStatus)
    {
    OstTraceFunctionEntry0( CMTPCONNECTION_VALIDFRAMEWORKREQUEST_ENTRY );
    __ASSERT_ALWAYS((!aSession || (aSession->TransactionPhase() & aValidPhases)), Panic(EMTPPanicInvalidState));
    __ASSERT_ALWAYS((!aStatus || (!aSession->RequestPending())), Panic(EMTPPanicBusy));
    
    TBool ret(ETrue);
    switch (State())
        {
    case EStateUnknown:
    case EStateErrorRecovery:
    default:
        Panic(EMTPPanicInvalidState);
        break;
        
    case EStateOpen:
        break;

    case EStateShutdown:    
    case EStateErrorShutdown:
        // Shutdown in progress.
        if (aSession != NULL) //Transaction is still alive during shutdown
            {
            ret = (aSession->TransactionPhase() == ECompletingPhase);
            aSession->SetTransactionPhase(ECompletingPhase);
            PublishConnState(EDisconnectedFromHost);		
            if (aStatus)
                {
                User::RequestComplete(aStatus, KErrCancel); 
                }
            }
        else //SendEventL happens during shutdown
            {
            ret = EFalse;
            }
        break;
        }
        
    OstTraceFunctionExit0( CMTPCONNECTION_VALIDFRAMEWORKREQUEST_EXIT );
    return ret;
    }

void CMTPConnection::RemoveEventsForSession(TUint32 aMTPId)
	{
    OstTraceFunctionEntry0( CMTPCONNECTION_REMOVEEVENTSFORSESSION_ENTRY );
    
    TSglQueIter<CMTPEventLink> iter(iEventQ);
    iter.SetToFirst();
    CMTPEventLink* link = NULL;
    
    while ((link = iter++) != NULL)
    	{
		if (link->iEvent.Uint32(TMTPTypeEvent::EEventSessionID) == aMTPId)
			{
			DequeueEvent(link);
			}
		}
    
	OstTraceFunctionExit0( CMTPCONNECTION_REMOVEEVENTSFORSESSION_EXIT );
	}

void CMTPConnection::DequeueAllEvents()
	{
	TSglQueIter<CMTPEventLink> iter(iEventQ);
    iter.SetToFirst();
    CMTPEventLink* link = NULL;
    
    while ((link = iter++) != NULL)
    	{
    	delete link;
    	}
	iPendingEventCount = 0;
	}
	
void CMTPConnection::DequeueEvent(CMTPEventLink* aLink)
	{
	iEventQ.Remove(*aLink);
	delete aLink;
	--iPendingEventCount;	
	}

void CMTPConnection::EnqueueEvent(CMTPEventLink* aLink)
	{
   	iEventQ.AddLast(*aLink);
   	++iPendingEventCount;	
	}
	
CMTPConnection::CMTPEventLink::CMTPEventLink(const TMTPTypeEvent& aEvent) :
	iEvent(aEvent)
	{
	}

/**
  * This method define and attach the property for publishing connection state 
  *  events.
  */
void CMTPConnection::DefineConnStatePropertyL()
	{
	OstTraceFunctionEntry0( CMTPCONNECTION_DEFINECONNSTATEPROPERTYL_ENTRY );	
	 RProcess process;
	 TUid tSid = process.SecureId();	
	//Property can read by anyone who subscribe for it.
	_LIT_SECURITY_POLICY_PASS(KAllowReadAll);
	_LIT_SECURITY_POLICY_S0(KAllowWrite, (TSecureId )KMTPPublishConnStateCat);
	
	TInt error = RProperty::Define(tSid, EMTPConnStateKey, RProperty::EInt, KAllowReadAll, KAllowReadAll);	
	if (KErrAlreadyExists != error)
		{
        LEAVEIFERROR(error, OstTrace1(TRACE_ERROR, CMTPCONNECTION_DEFINECONNSTATEPROPERTYL, "property define error! error code %d", error));
		}
	LEAVEIFERROR(iProperty.Attach(tSid, EMTPConnStateKey, EOwnerThread), 
	        OstTrace0(TRACE_ERROR, DUP1_CMTPCONNECTION_DEFINECONNSTATEPROPERTYL, "property attach error"));
	OstTraceFunctionExit0( CMTPCONNECTION_DEFINECONNSTATEPROPERTYL_EXIT );
	}

/**
  * This method is to publish various connection state. 
  */
void CMTPConnection::PublishConnState(TMTPConnStateType aConnState)	
	{  
    OstTraceFunctionEntry0( CMTPCONNECTION_PUBLISHCONNSTATE_ENTRY);
	OstTrace1( TRACE_NORMAL, CMTPCONNECTION_PUBLISHCONNSTATE, "publishing state = %d", (TInt)aConnState );
	
	RProcess process;    
	TInt error = iProperty.Set(process.SecureId(), EMTPConnStateKey, (TInt)aConnState);		
	 __ASSERT_DEBUG((error == KErrNone), Panic(EMTPPanicPublishEvent));;
	 OstTraceFunctionExit0( CMTPCONNECTION_PUBLISHCONNSTATE_EXIT);
	}

/**
  * This method is used to publish the events based on the TransactionPhase.
  * 
  */
void CMTPConnection::ValidateAndPublishConnState(CMTPSession& aSession, TInt aState)
	{	
    OstTraceFunctionEntry0(CMTPCONNECTION_VALIDATEANDPUBLISHCONNSTATE_ENTRY);
    OstTrace1( TRACE_NORMAL, CMTPCONNECTION_VALIDATEANDPUBLISHCONNSTATE, "publishing state = %d", aState );
    	
	TMTPConnStateType conState = EConnectedToHost;
	switch((TStates)aState)
		{
		  case EStateOpen:
		  	{
		  	TMTPTransactionPhase tPhase = aSession.TransactionPhase();
			switch(tPhase)
				{
				case EDataRToIPhase:
					conState = ESendingDataToHost;
				break;

				case EDataIToRPhase:
					conState = EReceiveDataFromHost;	
				break;

				case EIdlePhase:
				case EUndefined:
				case ERequestPhase:
				case EResponsePhase:
				case ECompletingPhase:
				default:
					conState = EConnectedToHost;
				break;
				}
		  	}
		   break;
		  case EStateShutdown:    
		  case EStateErrorShutdown:
		  	conState = EDisconnectedFromHost;
		  break;

		  case EStateErrorRecovery:
		  case EStateUnknown:
		  default:
		  	conState = EConnectedToHost;
		  break;
		}
	PublishConnState(conState);
	OstTraceFunctionExit0(CMTPCONNECTION_VALIDATEANDPUBLISHCONNSTATE_EXIT);
	}

void CMTPConnection::DisconnectionNotifyL()
	{
	iSingletons.DpController().NotifyDataProvidersL(EMTPDisconnected,this);	
	}
