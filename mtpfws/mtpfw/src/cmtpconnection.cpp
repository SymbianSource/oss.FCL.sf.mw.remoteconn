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

#define UNUSED_VAR(a) (a) = (a)

__FLOG_STMT(_LIT8(KComponent,"MTPConnection");)

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
    __FLOG(_L8("~CMTPConnection - Entry"));
    CloseAllSessions();
    
    // Remove any events not associated 
    // with a session
    TSglQueIter<CMTPEventLink> iter(iEventQ);
    iter.SetToFirst();
    CMTPEventLink* link = NULL;
    
    while ((link = iter++) != NULL)
    	{
    	delete link;
    	}
    
    if (iTransportConnection != NULL)
	    {
	    iTransportConnection->Unbind(*this);
	    }
    iSessions.ResetAndDestroy();
    //close the property
    iProperty.Close();
    // delete the ‘name?property
    RProcess process;
    RProperty::Delete(process.SecureId(), EMTPConnStateKey);
    __FLOG(_L8("~CMTPConnection - Exit"));
	__FLOG_CLOSE;
    }

/**
Unbinds the Transport Connection
*/    
void CMTPConnection::Unbind(MMTPTransportConnection& /*aConnection*/)
	{
	iTransportConnection = NULL;
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
    __FLOG(_L8("ReceiveDataL - Entry"));
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
    __FLOG(_L8("ReceiveDataL - Exit"));
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
    __FLOG(_L8("SendDataL - Entry"));
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
    __FLOG(_L8("SendDataL - Exit"));
    }

/**
Sends an MTP event dataset.
@param aEvent The MTP event dataset source.
@leave KErrNotFound If the MTP event dataset specifies an invalid SessionID.
*/
void CMTPConnection::SendEventL(const TMTPTypeEvent& aEvent)
    {
    __FLOG(_L8("SendEventL - Entry"));
    const TUint KValidPhases(EIdlePhase | ERequestPhase | EDataIToRPhase| EDataRToIPhase | EResponsePhase | ECompletingPhase);
    if (ValidFrameworkRequest(NULL, KValidPhases, NULL))
        {
        // Validate the SessionID
        TUint32 sessionId(aEvent.Uint32(TMTPTypeEvent::EEventSessionID));
        if (sessionId != KMTPSessionAll)
            {
            User::LeaveIfError(iSessions.FindInOrder(sessionId, SessionOrder));
            }
            

		EnqueueEvent(new (ELeave) CMTPEventLink(aEvent));
		if (iPendingEventCount == 1)
			{
			// Forward the event to the transport connection layer.
			iTransportConnection->SendEventL(iEventQ.First()->iEvent);			
			}
        }
    __FLOG(_L8("SendEventL - Exit"));
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
    __FLOG(_L8("SendResponseL - Entry"));
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
    __FLOG(_L8("SendResponseL - Exit"));
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
    __FLOG(_L8("SessionClosedL - Entry"));
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
    __FLOG(_L8("SessionClosedL - Exit"));
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
    __FLOG(_L8("SessionOpenedL - Entry"));
    // Validate the SessionID
    if (SessionWithMTPIdExists(aMTPId))
        {
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
        TMTPNotificationParamsSessionChange params = {aMTPId, *this};
        iSingletons.DpController().NotifyDataProvidersL(EMTPSessionOpened, &params);
        }
    __FLOG(_L8("SessionOpenedL - Exit"));
    }

/*
 * Signals the connection is suspended, the connection state is set to EStateShutdown which 
 * means that all the current transaction will not be able to send/receive any data via the
 * connection
 */
void CMTPConnection::ConnectionSuspended()
    {
    __FLOG(_L8("ConnectionSuspended - Entry"));
    
    TUint currentState = State();
    if (currentState!=EStateShutdown && currentState!=EStateErrorShutdown)
        {
        SetState(EStateShutdown);
        
        if (iTransportConnection != NULL)
            {
            iTransportConnection->Unbind(*this);
            iTransportConnection = NULL;
            }
        PublishConnState(EDisconnectedFromHost);   
    
        if (ActiveSessions() == 0)
            {
            CloseAllSessions();
            iSessions.Reset();
            iSingletons.Close();
            }
        else 
            {
            //some session may be in data or response phase, complete them and set transaction phase to ECompletingPhase.
            const TUint count(iSessions.Count());
            for (TUint i(0); (i < count); i++)
                {
                if (iSessions[i]->TransactionPhase() & (EDataIToRPhase|EDataRToIPhase|EResponsePhase))
                    {
                    iSessions[i]->SetTransactionPhase(ECompletingPhase);
                    iSessions[i]->CompletePendingRequest(KErrCancel);
                    }
                }
            }
        }
    
    __FLOG(_L8("ConnectionSuspended - Exit"));
    }

/*
 * Signals that the connection is resumed to EStateOpen state which means that data providers
 * can receive requests from host again.
 * @aTransportConnection The new transport connection object
 * @leave One of the system wide error codes, if a processing failure occurs.
 */
void CMTPConnection::ConnectionResumedL(MMTPTransportConnection& aTransportConnection)
    {
    __FLOG(_L8("ConnectionResumed - Entry"));
    
    TUint currentState = State();
    if (currentState != EStateOpen && currentState != EStateErrorRecovery)
        {
        iSingletons.OpenL();
        
        /* 
        Create the null session object which owns the transaction state for 
        transaction occuring outside a session (i.e. with SessionID == 0x00000000)
        */
        SessionOpenedL(KMTPSessionNone);            
        
        iTransportConnection = &aTransportConnection;
        iTransportConnection->BindL(*this); 
        SetState(EStateOpen); 
        PublishConnState(EConnectedToHost); 
        }
    
    __FLOG(_L8("ConnectionResumed - Exit"));
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
    __FLOG(_L8("TransactionCompleteL - Entry"));
    const TUint KValidPhases(ECompletingPhase);
    CMTPSession& session(SessionL(aRequest, TMTPTypeRequest::ERequestSessionID));    

    if (ValidFrameworkRequest(&session, KValidPhases, NULL))
        {
        session.SetTransactionPhase(EIdlePhase);
        if (State() == EStateShutdown)
            {
            if (ActiveSessions() == 0)
                {        
                CloseAllSessions();
                iSessions.Reset();
                iSingletons.Close();
                
                // Move the log here because ShutdownComplete will delete this object.
                __FLOG(_L8("TransactionCompleteL - Exit"));
                }
            }
        else
            {
            iTransportConnection->TransactionCompleteL(aRequest);
            __FLOG(_L8("TransactionCompleteL - Exit"));
            }
        }
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
    User::LeaveIfError(idx);
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
    __FLOG(_L8("ReceivedEventL - Entry"));
    TInt idx(KErrNotFound);
    
    // Validate the SessionID.
    TUint32 sessionId(aEvent.Uint32(TMTPTypeEvent::EEventSessionID));
    if (sessionId != KMTPSessionAll)
        {
        idx = iSessions.FindInOrder(sessionId, SessionOrder);
        User::LeaveIfError(idx);
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
	   	if (request.Uint32(TMTPTypeRequest::ERequestTransactionID) > 
	    	aEvent.Uint32(TMTPTypeEvent::EEventTransactionID) )
	        {
	        // Event to be queued for future use, we can only queue one event at a time
	        session.StorePendingEventL(aEvent);
	        }
	        
	    if (request.Uint32(TMTPTypeRequest::ERequestTransactionID) == 
	         aEvent.Uint32(TMTPTypeEvent::EEventTransactionID) )
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
    	}	
    __FLOG(_L8("ReceivedEventL - Exit"));
    }

void CMTPConnection::ReceivedRequestL(const TMTPTypeRequest& aRequest)
    {  
    __FLOG(_L8("ReceivedRequestL - Entry"));
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
                User::Leave(err);
                }
            }
        }
    __FLOG(_L8("ReceivedRequestL - Exit"));
    }

#ifdef MTP_CAPTURE_TEST_DATA
void CMTPConnection::ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
#else
void CMTPConnection::ReceiveDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
#endif
    {
    __FLOG(_L8("ReceiveDataCompleteL - Entry"));    
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
    
    __FLOG(_L8("ReceiveDataCompleteL - Exit"));
    }

void CMTPConnection::SendDataCompleteL(TInt aErr, const MMTPType& aData, const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("SendDataCompleteL - Entry"));  
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
    
    __FLOG(_L8("SendDataCompleteL - Exit"));
    }

void CMTPConnection::SendEventCompleteL(TInt aErr, const TMTPTypeEvent& aEvent)
    {
    __FLOG(_L8("SendEventCompleteL - Entry"));

    
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
   		// Forward the event to the transport connection layer.
   		__FLOG(_L8("Sending queued event"));
 	    iTransportConnection->SendEventL(iEventQ.First()->iEvent);
   		}
    
    __FLOG(_L8("SendEventCompleteL - Exit"));
    }

void CMTPConnection::SendResponseCompleteL(TInt aErr, const TMTPTypeResponse& /*aResponse*/, const TMTPTypeRequest& aRequest)
    {   
    __FLOG(_L8("SendResponseCompleteL - Entry"));
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
    __FLOG(_L8("SendResponseCompleteL - Exit"));
    }

   
TMTPTransactionPhase CMTPConnection::TransactionPhaseL(TUint32 aMTPId) const
    {
    TInt idx(iSessions.FindInOrder(aMTPId, SessionOrder));
    User::LeaveIfError(idx);
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
    
    }
    
/**
Second phase constructor.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPConnection::ConstructL()
    {
	__FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    //define the property for publishing connection state.
    DefineConnStatePropertyL();
    PublishConnState(EDisconnectedFromHost);  
#ifdef MTP_CAPTURE_TEST_DATA
    // Running under debug capture mode save this request off to disk.
    iRequestLogger = CMTPRequestLogger::NewL();
#endif
    __FLOG(_L8("ConstructL - Exit"));
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
    __FLOG(_L8("InitiateMTPErrorRecoveryL - Entry"));
    // Populate error response.
    iResponse.Reset();
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aResponseCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, aRequest.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID));
    
    // Set the connection state pending completion, and send the response.
    SetState(EStateErrorRecovery);
    iTransportConnection->SendResponseL(iResponse, aRequest);
    __FLOG(_L8("InitiateMTPErrorRecoveryL - Exit"));
    }
    
/**
Concludes an MTP connection level protocol error recovery sequence.
@see InitiateMTPErrorRecoveryL
*/
void CMTPConnection::MTPErrorRecoveryComplete()
    {
    __FLOG(_L8("MTPErrorRecoveryComplete - Entry"));
    SetState(EStateOpen);
    PublishConnState(EConnectedToHost);	
    __FLOG(_L8("MTPErrorRecoveryComplete - Exit"));
    }
    
/**
Forces the immediate shutdown of the MTP connection. This is invoked when a 
protocol error is detected that cannot be recovered from, e.g. if an attempt
is detected to initiate an MTP transaction before a previous transaction has 
concluded.
*/
void CMTPConnection::UnrecoverableMTPError()
    {
    __FLOG(_L8("UnrecoverableMTPError - Entry"));
    SetState(EStateErrorShutdown);
    PublishConnState(EDisconnectedFromHost);		
    iTransportConnection->CloseConnection();
    __FLOG(_L8("UnrecoverableMTPError - Entry"));
    }

/**
Signals the MTP connection transport to terminate any in-progress data phase 
processing on the specified session.
@param aidx The sessions table index of the required session.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPConnection::InitiateTransactionCancelL(TInt aIdx)
    {    
    __FLOG(_L8("InitiateTransactionCancelL - Entry"));
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
    __FLOG(_L8("InitiateTransactionCancelL - Exit"));
    }

/**
Provides a count of the number of sessions with transactions in-progress.
*/
TUint CMTPConnection::ActiveSessions() const
    {
    __FLOG(_L8("ActiveSessions - Entry"));
    TUint active(0);
    const TUint count(iSessions.Count());
    for (TUint i(0); (i < count); i++)
        {
        if (iSessions[i]->TransactionPhase() > EIdlePhase)
            {
            active++;
            }
        }
    __FLOG_VA((_L8("Active sessions = %d"), active));
    __FLOG(_L8("ActiveSessions - Exit"));
    return active;
    }

/**
Closes all sessions which have been opened on the connection.
*/
void CMTPConnection::CloseAllSessions()
    {
    __FLOG(_L8("CloseAllSessions - Entry"));
	
    TInt count = iSessions.Count();
    __FLOG_VA((_L8("Sessions number to be closed = %d"), count));   
	for (TInt i(count - 1); i>=0; i--)
		{
		CloseSession(i);
		}
	
    __FLOG(_L8("CloseAllSessions - Exit"));
    }

/**
Closes the sessions with the specified session index.
@param aIdx The session index.
*/
void CMTPConnection::CloseSession(TUint aIdx)
    {
    __FLOG(_L8("CloseSession - Entry"));
    
    __FLOG_VA((_L8("Session index to be closed = %d"), aIdx));    
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
    
    __FLOG(_L8("CloseSession - Exit"));
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
	__FLOG(_L8("GetDataReceiveResult - Entry"));
    __FLOG_VA((_L8("Data receive result = %d"), iDataReceiveResult));
    __FLOG(_L8("GetDataReceiveResult - Exit"));
    return iDataReceiveResult;
	}
    
/**
Sets the MTP connection state variable.
@param aState The new MTP connection state value.
*/
void CMTPConnection::SetState(TUint aState)
    {
    __FLOG(_L8("SetState - Entry"));
    __FLOG_VA((_L8("Setting state = %d"), aState));
    iState = aState;
    __FLOG(_L8("SetState - Exit"));
    }
  
    
/**
Provide the current MTP connection state.
@return The current MTP connection state.
*/
TUint CMTPConnection::State() const
    {
    __FLOG(_L8("State - Entry"));
    __FLOG_VA((_L8("State = %d"), iState));
    __FLOG(_L8("State - Exit"));
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
    __FLOG(_L8("ValidFrameworkRequest - Entry"));
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
        
    __FLOG(_L8("ValidFrameworkRequest - Exit"));
    return ret;
    }

void CMTPConnection::RemoveEventsForSession(TUint32 aMTPId)
	{
    __FLOG(_L8("RemoveEventsForSession - Entry"));
    
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
    
    __FLOG(_L8("RemoveEventsForSession - Exit"));
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
	
	 __FLOG(_L8("DefineConnStatePropertyL - Entry"));
	 RProcess process;
	 TUid tSid = process.SecureId();	
	//Property can read by anyone who subscribe for it.
	_LIT_SECURITY_POLICY_PASS(KAllowReadAll);
	_LIT_SECURITY_POLICY_S0(KAllowWrite, (TSecureId )KMTPPublishConnStateCat);
	
	TInt error = RProperty::Define(tSid, EMTPConnStateKey, RProperty::EInt, KAllowReadAll, KAllowReadAll);	
	if (KErrAlreadyExists != error)
		{
		User::LeaveIfError(error);
		}
	User::LeaveIfError(iProperty.Attach(tSid, EMTPConnStateKey, EOwnerThread));
	__FLOG(_L8("DefineConnStatePropertyL - Exit"));
	}

/**
  * This method is to publish various connection state. 
  */
void CMTPConnection::PublishConnState(TMTPConnStateType aConnState)	
	{
	__FLOG_VA((_L8("PublishConnState - Entry \n publishing state = %d"), (TInt)aConnState));
	RProcess process;    
	TInt error = iProperty.Set(process.SecureId(), EMTPConnStateKey, (TInt)aConnState);		
	 __ASSERT_DEBUG((error == KErrNone), Panic(EMTPPanicPublishEvent));;
	__FLOG(_L8("PublishConnState - Exit"));
	}

/**
  * This method is used to publish the events based on the TransactionPhase.
  * 
  */
void CMTPConnection::ValidateAndPublishConnState(CMTPSession& aSession, TInt aState)
	{	
    	__FLOG_VA((_L8("ValidateAndPublishConnState - Entry \n publishing state = %d"), aState));

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
	__FLOG(_L8("ValidateAndPublishConnStateL - Exit"));
	}

void CMTPConnection::DisconnectionNotifyL()
	{
	iSingletons.DpController().NotifyDataProvidersL(EMTPDisconnected,this);	
	}
