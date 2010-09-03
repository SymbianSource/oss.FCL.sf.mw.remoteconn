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

#include <mtp/mtpprotocolconstants.h>

#include "cmtpsession.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsessionTraces.h"
#endif


// Class constants.

#ifdef _DEBUG

/**
CMTPSession panics
*/
_LIT(KMTPPanicCategory, "CMTPSession");
enum TMTPPanicReasons
    {
    EMTPPanicBusy = 0,
    EMTPPanicStraySignal = 1,
    };
    
LOCAL_C void Panic(TInt aReason)
    {
    User::Panic(KMTPPanicCategory, aReason);
    }

#endif //_DEBUG

/**
CMTPSession factory method. A pointer to the new CMTPSession instance is placed
on the cleanup stack.
@param aMTPId The session identifier assigned by the MTP connection on which 
the session resides. 
@param aUniqueId The session identifier assigned by the MTP data provider framework that 
is unique across all active connections.
@return Pointer to the new CMTPSession instance. Ownership IS transfered.
@leave One of the system wide error codes.
*/
CMTPSession* CMTPSession::NewLC(TUint32 aMTPId, TUint aUniqueId)
    {
    CMTPSession* self = new(ELeave) CMTPSession(aMTPId, aUniqueId);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Destructor.
*/ 
CMTPSession::~CMTPSession()
    {
    OstTraceFunctionEntry0( CMTPSESSION_CMTPSESSION_DES_ENTRY );
    iRoutingRegistrations.Close();
    OstTraceFunctionExit0( CMTPSESSION_CMTPSESSION_DES_EXIT );
    }
    
/**
Provides the next expected TransactionID. Transaction IDs are assigned 
in incremental sequence by the MTP initiator in the range 0x00000001 to
0xFFFFFFFE.
@return The next TransactionID expected on the session.
*/
TUint32 CMTPSession::ExpectedTransactionId() const
    {
    OstTraceFunctionEntry0( CMTPSESSION_EXPECTEDTRANSACTIONID_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPSESSION_EXPECTEDTRANSACTIONID, 
            "iExpectedTransactionId = 0x%08X", iExpectedTransactionId);
    OstTraceFunctionExit0( CMTPSESSION_EXPECTEDTRANSACTIONID_EXIT );
    return iExpectedTransactionId; 
    }

/**
Increments the next expected TransactionID to the next value in the sequence.
TransactionIDs are assigned by the MTP initiator starting from 0x00000001. 
When the TransactionID increments to 0xFFFFFFFF it wraps back to 0x00000001.
*/
void CMTPSession::IncrementExpectedTransactionId()
    {
    OstTraceFunctionEntry0( CMTPSESSION_INCREMENTEXPECTEDTRANSACTIONID_ENTRY );
    if (++iExpectedTransactionId == KMTPTransactionIdLast)
        {
        iExpectedTransactionId = KMTPTransactionIdFirst;
        }
    OstTrace1(TRACE_NORMAL, CMTPSESSION_INCREMENTEXPECTEDTRANSACTIONID, 
            "iExpectedTransactionId = 0x%08X", iExpectedTransactionId);    
    OstTraceFunctionExit0( CMTPSESSION_INCREMENTEXPECTEDTRANSACTIONID_EXIT );
    }

/**
Sets or resets the session's active transaction request dataset. The active 
transaction request dataset should only be set at the start of the transaction 
(ERequestPhase), and reset and the end of the transaction (ECompletingPhase).
@param aRequest The active transaction request dataset.
*/
void CMTPSession::SetActiveRequestL(const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPSESSION_SETACTIVEREQUESTL_ENTRY );
    MMTPType::CopyL(aRequest, iActiveRequest);    
    OstTraceFunctionExit0( CMTPSESSION_SETACTIVEREQUESTL_EXIT );
    }

/**
Sets the session's transaction phase state variable.
@param aPhase The new transaction phase state value.
*/
void CMTPSession::SetTransactionPhase(TMTPTransactionPhase aPhase)
    {
    OstTraceFunctionEntry0( CMTPSESSION_SETTRANSACTIONPHASE_ENTRY );
    iTransactionPhase = aPhase;
    OstTrace1(TRACE_NORMAL, CMTPSESSION_SETTRANSACTIONPHASE, 
            "iTransactionPhase = 0x%08X", iTransactionPhase);        
    OstTraceFunctionExit0( CMTPSESSION_SETTRANSACTIONPHASE_EXIT );
    }

    
/**
Provides the current MTP transaction state for the session.
@return The MTP transaction state for the session.
*/
TMTPTransactionPhase CMTPSession::TransactionPhase() const
    {
    OstTraceFunctionEntry0( CMTPSESSION_TRANSACTIONPHASE_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPSESSION_TRANSACTIONPHASE, 
            "iTransactionPhase = 0x%08X", iTransactionPhase);        
	OstTraceFunctionExit0( CMTPSESSION_TRANSACTIONPHASE_EXIT );
	return  iTransactionPhase;
    }
    
TInt CMTPSession::RouteRequest(const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPSESSION_ROUTEREQUEST_ENTRY );
    TInt ret(KErrNotFound);
    
    // Attempt to match the request to existing registrations.
    TInt idx(iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
    if (idx != KErrNotFound)
        {
        // Retrieve the request registration.
        const TMTPTypeRequest& registration(iRoutingRegistrations[idx]);
            
        /*
        Extract the registered DP ID. For convenience the DP ID is saved in 
        the registered request, in the TransactionID element (which is unused 
        for routing).
        */  
        ret = registration.Uint32(TMTPTypeRequest::ERequestTransactionID);
        
        /* 
        Recognised follow-on request types match one request occurence 
        and are then deleted.
        */
        TUint16 op(aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode));
        if ((op == EMTPOpCodeSendObject) ||
            (op == EMTPOpCodeTerminateOpenCapture))
            {
            OstTrace1(TRACE_NORMAL, CMTPSESSION_ROUTEREQUEST, 
                    "Unregistering follow-on request 0x%08X", op);
            iRoutingRegistrations.Remove(idx);
            }
        }
        
    OstTrace1(TRACE_NORMAL, DUP1_CMTPSESSION_ROUTEREQUEST, 
            "DP ID = %d", ret);
    OstTraceFunctionExit0( CMTPSESSION_ROUTEREQUEST_EXIT );
    return ret;
    }
  
void CMTPSession::RouteRequestRegisterL(const TMTPTypeRequest& aRequest, TInt aDpId)
    {
    OstTraceFunctionEntry0( CMTPSESSION_ROUTEREQUESTREGISTERL_ENTRY );
    // Locate any pre-existing registration (which if found, will be overwritten).
    TInt idx(iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
    if (idx == KErrNotFound)
        {
        iRoutingRegistrations.InsertInOrderL(aRequest, CMTPSession::RouteRequestOrder);
        LEAVEIFERROR(idx = iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder),
                OstTrace1( TRACE_ERROR, CMTPSESSION_ROUTEREQUESTREGISTERL, "can't find in routing registrations for request, dpId %d",  aDpId));
        }
    
    /*
    For convenience the DP ID is saved in the registered request, in the 
    TransactionID element (which is unused for routing).
    */
    iRoutingRegistrations[idx].SetUint32(TMTPTypeRequest::ERequestTransactionID, aDpId);
    OstTraceFunctionExit0( CMTPSESSION_ROUTEREQUESTREGISTERL_EXIT );
    }

/**
Indicates if a routing request is registered on the session with the 
specified MTP operation code.
@param aOpCode The MTP operation code.
@return ETrue if a routing request with the specified MTP operation code is 
registered on the session, otherwise EFalse.
*/
TBool CMTPSession::RouteRequestRegistered(TUint16 aOpCode) const
    {
    OstTraceFunctionEntry0( CMTPSESSION_ROUTEREQUESTREGISTERED_ENTRY );
    OstTraceFunctionExit0( CMTPSESSION_ROUTEREQUESTREGISTERED_EXIT );
    return (iRoutingRegistrations.Find(aOpCode, CMTPSession::RouteRequestMatchOpCode) != KErrNotFound);
    }

void CMTPSession::RouteRequestUnregister(const TMTPTypeRequest& aRequest)
    {
    OstTraceFunctionEntry0( CMTPSESSION_ROUTEREQUESTUNREGISTER_ENTRY );
    TInt idx(iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
    if (idx != KErrNotFound)
        {
        iRoutingRegistrations.Remove(idx);
        }
    OstTraceFunctionExit0( CMTPSESSION_ROUTEREQUESTUNREGISTER_EXIT );
    }
    
void CMTPSession::StorePendingEventL(const TMTPTypeEvent& aEvent)
    {
    MMTPType::CopyL(aEvent, iPendingEvent);
    }
    
TBool CMTPSession::CheckPendingEvent(const TMTPTypeRequest& aRequest) const
    {
    TBool ret = EFalse;
    
    // Compare transaction ID in the request and any pending event
    if ( aRequest.Uint32(TMTPTypeRequest::ERequestTransactionID)  
         == iPendingEvent.Uint32(TMTPTypeEvent::EEventTransactionID) )
        {
        ret = ETrue;
        }
    
    return ret;
    }
    
const TMTPTypeEvent& CMTPSession::PendingEvent() const
    {
    return iPendingEvent;
    }
    
/**
Completes the currently pending asynchronous request status with the specified
completion code.
@param aErr The asynchronous request completion request.
*/
void CMTPSession::CompletePendingRequest(TInt aErr)
    {
    OstTraceFunctionEntry0( CMTPSESSION_COMPLETEPENDINGREQUEST_ENTRY );
    
    if (iRequestStatus != NULL)
        {
        __ASSERT_DEBUG(*iRequestStatus == KRequestPending, Panic(EMTPPanicStraySignal));
        User::RequestComplete(iRequestStatus, aErr);
        }

    OstTraceFunctionExit0( CMTPSESSION_COMPLETEPENDINGREQUEST_EXIT );
    }
    

/**
Indicates if an asynchronous request is currently pending.
@return ETrue if an asynchronous request is currently pending, otherwise 
EFalse.
*/
TBool CMTPSession::RequestPending() const
    {
    return (iRequestStatus != NULL);        
    }

/**
Set the status to complete for the currently pending asynchronous request.
@param aStatus The asynchronous request status to complete.
*/
void CMTPSession::SetRequestPending(TRequestStatus& aStatus)
    {
    OstTraceFunctionEntry0( CMTPSESSION_SETREQUESTPENDING_ENTRY );
    __ASSERT_DEBUG(!iRequestStatus, Panic(EMTPPanicBusy));
    iRequestStatus = &aStatus;
    *iRequestStatus = KRequestPending;
    OstTraceFunctionExit0( CMTPSESSION_SETREQUESTPENDING_EXIT );
    }

const TMTPTypeRequest& CMTPSession::ActiveRequestL() const
    {
    OstTraceFunctionEntry0( CMTPSESSION_ACTIVEREQUESTL_ENTRY );
    
    if (iTransactionPhase == EIdlePhase)
        {
        OstTrace0( TRACE_ERROR, CMTPSESSION_ACTIVEREQUESTL, "transaction in EIdlePhase!" );
        User::Leave(KErrNotFound);            
        }
    
    OstTraceFunctionExit0( CMTPSESSION_ACTIVEREQUESTL_EXIT );
    return iActiveRequest;  
    }

TUint32 CMTPSession::SessionMTPId() const
    {
    OstTraceFunctionEntry0( CMTPSESSION_SESSIONMTPID_ENTRY );
    OstTrace1(TRACE_NORMAL, CMTPSESSION_SESSIONMTPID, "Session MTP ID = %d", iIdMTP);
    OstTraceFunctionExit0( CMTPSESSION_SESSIONMTPID_EXIT );
    return iIdMTP;        
    }

TUint CMTPSession::SessionUniqueId() const
    {
    OstTraceFunctionEntry0( CMTPSESSION_SESSIONUNIQUEID_ENTRY );
    OstTraceFunctionExit0( CMTPSESSION_SESSIONUNIQUEID_EXIT );
    return iIdUnique;        
    }
    
TAny* CMTPSession::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    OstTraceFunctionEntry0( CMTPSESSION_GETEXTENDEDINTERFACE_ENTRY );
    OstTraceFunctionExit0( CMTPSESSION_GETEXTENDEDINTERFACE_EXIT );
    return NULL;        
    }
    
/**
Constructor.
*/
CMTPSession::CMTPSession(TUint32 aMTPId, TUint aUniqueId) :
    iExpectedTransactionId(KMTPTransactionIdFirst),
    iIdMTP(aMTPId),
    iIdUnique(aUniqueId),
    iRequestStatus(NULL)
    {
    
    }

void CMTPSession::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSESSION_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPSESSION_CONSTRUCTL_EXIT );
    }
    
TBool CMTPSession::RouteRequestMatchOpCode(const TUint16* aOpCode, const TMTPTypeRequest& aRequest)
    {
    return (aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode) == *aOpCode);
    }

TInt CMTPSession::RouteRequestOrder(const TMTPTypeRequest& aLeft, const TMTPTypeRequest& aRight)
    {
    TInt unequal(aLeft.Uint16(TMTPTypeRequest::ERequestOperationCode) - aRight.Uint16(TMTPTypeRequest::ERequestOperationCode));
    if (!unequal)
        {
        for (TUint i(TMTPTypeRequest::ERequestParameter1); ((i <= TMTPTypeRequest::ERequestParameter5) && (!unequal)); i++)
            {
            unequal = aLeft.Uint32(i) - aRight.Uint32(i);
            }
        }
    return unequal;
    }
