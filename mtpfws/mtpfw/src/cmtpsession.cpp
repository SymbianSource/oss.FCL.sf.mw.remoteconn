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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"Session");)

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
    __FLOG(_L8("~CMTPSession - Entry"));
    iRoutingRegistrations.Close();
    __FLOG(_L8("~CMTPSession - Exit"));
    __FLOG_CLOSE;
    }
    
/**
Provides the next expected TransactionID. Transaction IDs are assigned 
in incremental sequence by the MTP initiator in the range 0x00000001 to
0xFFFFFFFE.
@return The next TransactionID expected on the session.
*/
TUint32 CMTPSession::ExpectedTransactionId() const
    {
    __FLOG(_L8("ExpectedTransactionId - Entry"));
    __FLOG_VA((_L8("iExpectedTransactionId = 0x%08X"), iExpectedTransactionId));
    __FLOG(_L8("ExpectedTransactionId - Exit")); 
    return iExpectedTransactionId; 
    }

/**
Increments the next expected TransactionID to the next value in the sequence.
TransactionIDs are assigned by the MTP initiator starting from 0x00000001. 
When the TransactionID increments to 0xFFFFFFFF it wraps back to 0x00000001.
*/
void CMTPSession::IncrementExpectedTransactionId()
    {
    __FLOG(_L8("IncrementExpectedTransactionId - Entry"));
    if (++iExpectedTransactionId == KMTPTransactionIdLast)
        {
        iExpectedTransactionId = KMTPTransactionIdFirst;
        }
    __FLOG_VA((_L8("iExpectedTransactionId = 0x%08X"), iExpectedTransactionId));
    __FLOG(_L8("IncrementExpectedTransactionId - Exit"));
    }

/**
Sets or resets the session's active transaction request dataset. The active 
transaction request dataset should only be set at the start of the transaction 
(ERequestPhase), and reset and the end of the transaction (ECompletingPhase).
@param aRequest The active transaction request dataset.
*/
void CMTPSession::SetActiveRequestL(const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("SetActiveRequestL - Entry"));
    MMTPType::CopyL(aRequest, iActiveRequest);    
    __FLOG(_L8("SetActiveRequestL - Exit"));
    }

/**
Sets the session's transaction phase state variable.
@param aPhase The new transaction phase state value.
*/
void CMTPSession::SetTransactionPhase(TMTPTransactionPhase aPhase)
    {
    __FLOG(_L8("SetTransactionPhase - Entry"));
    iTransactionPhase = aPhase;
    __FLOG_VA((_L8("iTransactionPhase = 0x%08X"), iTransactionPhase));
    __FLOG(_L8("SetTransactionPhase - Exit"));
    }

    
/**
Provides the current MTP transaction state for the session.
@return The MTP transaction state for the session.
*/
TMTPTransactionPhase CMTPSession::TransactionPhase() const
    {
    __FLOG(_L8("TransactionPhase - Entry"));
    __FLOG_VA((_L8("iTransactionPhase = 0x%08X"), iTransactionPhase));
    __FLOG(_L8("TransactionPhase - Exit"));
	return  iTransactionPhase;
    }
    
TInt CMTPSession::RouteRequest(const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("RouteRequest - Entry"));
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
            __FLOG_VA((_L8("Unregistering follow-on request 0x%08X"), op));
            iRoutingRegistrations.Remove(idx);
            }
        }
        
    __FLOG_VA((_L8("DP ID = %d"), ret));
    __FLOG(_L8("RouteRequest - Exit"));
    return ret;
    }
  
void CMTPSession::RouteRequestRegisterL(const TMTPTypeRequest& aRequest, TInt aDpId)
    {
    __FLOG(_L8("RouteRequestRegisterL - Entry"));
    // Locate any pre-existing registration (which if found, will be overwritten).
    TInt idx(iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
    if (idx == KErrNotFound)
        {
        iRoutingRegistrations.InsertInOrderL(aRequest, CMTPSession::RouteRequestOrder);
        User::LeaveIfError(idx = iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
        }
    
    /*
    For convenience the DP ID is saved in the registered request, in the 
    TransactionID element (which is unused for routing).
    */
    iRoutingRegistrations[idx].SetUint32(TMTPTypeRequest::ERequestTransactionID, aDpId);
    __FLOG(_L8("RouteRequestRegisterL - Exit"));
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
    __FLOG(_L8("RouteRequestPending - Entry"));
    __FLOG(_L8("RouteRequestPending - Entry"));
    return (iRoutingRegistrations.Find(aOpCode, CMTPSession::RouteRequestMatchOpCode) != KErrNotFound);
    }

void CMTPSession::RouteRequestUnregister(const TMTPTypeRequest& aRequest)
    {
    __FLOG(_L8("RouteRequestUnregister - Entry"));
    TInt idx(iRoutingRegistrations.FindInOrder(aRequest, CMTPSession::RouteRequestOrder));
    if (idx != KErrNotFound)
        {
        iRoutingRegistrations.Remove(idx);
        }
    __FLOG(_L8("RouteRequestUnregister - Exit"));
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
    __FLOG(_L8("CompletePendingRequest - Entry"));
    
    if (iRequestStatus != NULL)
        {
        __ASSERT_DEBUG(*iRequestStatus == KRequestPending, Panic(EMTPPanicStraySignal));
        User::RequestComplete(iRequestStatus, aErr);
        }
    
    __FLOG(_L8("CompletePendingRequest - Exit"));
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
    __FLOG(_L8("SetRequestPending - Entry"));
    __ASSERT_DEBUG(!iRequestStatus, Panic(EMTPPanicBusy));
    iRequestStatus = &aStatus;
    *iRequestStatus = KRequestPending;
    __FLOG(_L8("SetRequestPending - Exit"));
    }

const TMTPTypeRequest& CMTPSession::ActiveRequestL() const
    {
    __FLOG(_L8("ActiveRequestL - Entry"));
    
    if (iTransactionPhase == EIdlePhase)
        {
        User::Leave(KErrNotFound);            
        }
    
    __FLOG(_L8("ActiveRequestL - Exit"));
    return iActiveRequest;  
    }

TUint32 CMTPSession::SessionMTPId() const
    {
    __FLOG(_L8("SessionMTPId - Entry"));
    __FLOG_VA( (_L8("Session MTP ID = %d"), iIdMTP) );
    __FLOG(_L8("SessionMTPId - Exit"));
    return iIdMTP;        
    }

TUint CMTPSession::SessionUniqueId() const
    {
    __FLOG(_L8("SessionUniqueId - Entry"));
    __FLOG(_L8("SessionUniqueId - Exit"));
    return iIdUnique;        
    }
    
TAny* CMTPSession::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    __FLOG(_L8("GetExtendedInterface - Entry"));
    __FLOG(_L8("GetExtendedInterface - Exit"));
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    __FLOG(_L8("ConstructL - Exit"));
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
