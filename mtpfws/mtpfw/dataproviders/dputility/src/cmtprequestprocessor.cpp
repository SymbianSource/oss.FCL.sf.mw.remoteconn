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

#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/mmtpconnection.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtprequestprocessor.h"
#include "cmtprequestchecker.h"

const static TInt KNullBufferSize = 4096;
__FLOG_STMT(_LIT8(KComponent,"MTPRequestProcessor");)

/**
Standard c++ constructor
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@param aElementCount Number of element in the request.
@param aElements	The element info data.
@return a pointer to the created request processor object
*/   
EXPORT_C CMTPRequestProcessor::CMTPRequestProcessor(
											MMTPDataProviderFramework& aFramework,											
											MMTPConnection& aConnection,
											TInt aElementCount,
											const TMTPRequestElementInfo* aElements)
	:CActive(EPriorityStandard),
	iFramework(aFramework),
	iConnection(aConnection),
	iElementCount(aElementCount),
	iElements(aElements)
	{
	CActiveScheduler::Add(this);
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	}

/**
Destructor
*/	
EXPORT_C CMTPRequestProcessor::~CMTPRequestProcessor()
	{
	Cancel();
	iNullBuffer.Close();
	delete iRequestChecker;
	__FLOG_CLOSE;
	}

/**
Relese (delete) this request processor
*/	
EXPORT_C void CMTPRequestProcessor::Release()
	{
	delete this;
	}

/**
Send a response to the initiator
@param aResponseCode The response code to send
@param aParamCount	The number of parameters
@param aParmas	The pointer to array of parameters
*/	
EXPORT_C void CMTPRequestProcessor::SendResponseL(TMTPResponseCode aResponseCode, TInt aParameterCount, TUint32* aParams)
	{
	__ASSERT_DEBUG(aParameterCount < TMTPTypeRequest::ENumElements, User::Invariant());
    
	iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aResponseCode);
		    
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, iSessionId);
	
	iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, iTransactionCode);
   
    TInt i = 0;	
	for(i = 0; i < aParameterCount; i++)
		{
		iResponse.SetUint32(TMTPTypeResponse::EResponseParameter1 + i, aParams[i]);
		}

	i += TMTPTypeResponse::EResponseParameter1;
	while(i <= TMTPTypeResponse::EResponseParameter5)
	    {
	    iResponse.SetUint32(i, KMTPNotSpecified32);
	    i++;
	    }

	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	iFramework.SendResponseL(iResponse, *iRequest, iConnection);
	}

/**
The current active request
@return A reference to the current request
*/	
EXPORT_C const TMTPTypeRequest& CMTPRequestProcessor::Request() const
	{
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	return *iRequest;
	}
	
/**
The connection from which the current request comes
@return A reference to the current connection
*/	
EXPORT_C MMTPConnection& CMTPRequestProcessor::Connection() const
	{
	return iConnection;
	}
/**
The Session ID from the current request
@return the value of the session ID
 */
EXPORT_C TUint32 CMTPRequestProcessor::SessionId()
    {
    return iSessionId;
    }
/**
Signal to the framework that the current request transaction has completed
*/
EXPORT_C void CMTPRequestProcessor::CompleteRequestL()
	{
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	iFramework.TransactionCompleteL(*iRequest, iConnection);
	}

/**
Send data to the initiator
@param aData	The data to send
*/	
EXPORT_C void CMTPRequestProcessor::SendDataL(const MMTPType& aData)
	{
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	iFramework.SendDataL(aData, *iRequest, iConnection);
	}

/**
Receive data from the initiator
@param aData 	The data to receive
*/
EXPORT_C void CMTPRequestProcessor::ReceiveDataL(MMTPType& aData)
	{
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	iFramework.ReceiveDataL(aData, *iRequest, iConnection);
	}
	
/**
Handle the request
@param aRequest	The request to be processed
@param aPhase	The current transaction phase of the request
@return ETrue if the transaction has completed, otherwise, EFalse
*/	
EXPORT_C TBool CMTPRequestProcessor::HandleRequestL(const TMTPTypeRequest& aRequest, TMTPTransactionPhase aPhase)
	{
	iRequest = &aRequest;
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	TBool result = EFalse;
	switch(aPhase)
		{
		case ERequestPhase:		
			ExtractSessionTransactionId();
			result = DoHandleRequestPhaseL();
			break;
		case EDataIToRPhase:
			result = DoHandleDataIToRPhaseL();
			break;
		case EDataRToIPhase:
			result = DoHandleRToIPhaseL();
			break;
		case EResponsePhase:
			if (iResponseCode != EMTPRespCodeOK && HasDataphase())
				{
				SendResponseL(TMTPResponseCode(iResponseCode));
				iNullBuffer.Close();
				}
			else
				{
				result = DoHandleResponsePhaseL();	
				}
			break;
			
		case ECompletingPhase:
			result = DoHandleCompletingPhaseL();
			break;
		}
	return result;	
	}

/**
Handle the event
@param aEvent The event to be processed
*/	
EXPORT_C void CMTPRequestProcessor::HandleEventL(const TMTPTypeEvent& aEvent)
	{
	TUint16 eventCode = aEvent.Uint16(TMTPTypeEvent::EEventCode);
	iCancelled = (eventCode == EMTPEventCodeCancelTransaction);
	}
	
/**
Check whether the processor can process the request
@param aRequest The request to be processed
@param aConnection The connection from which the request comes
@return ETrue if the processor can process the request, otherwise EFalse
*/	
EXPORT_C TBool CMTPRequestProcessor::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
	{
	__ASSERT_DEBUG(iRequest, User::Invariant()); 
	TBool result = ((&aRequest == iRequest) && (&iConnection == &aConnection));
	return result;		
	}

/**
Check whether the processor can process the event
@param aEvent The event to be processed
@param aConnection The connection from which the request comes
@return ETrue if the processor can process the request, otherwise EFalse
*/	
EXPORT_C TBool CMTPRequestProcessor::Match(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection) const
	{

	TUint32 eventSessionId = aEvent.Uint32(TMTPTypeEvent::EEventSessionID);
	TUint32 eventTransactionCode = aEvent.Uint32(TMTPTypeEvent::EEventTransactionID);
	
	TBool result = EFalse;
	if(iSessionId == eventSessionId && 
		iTransactionCode == eventTransactionCode &&
		&iConnection == &aConnection)
		{
		result = ETrue;
		}
	return result;	
	}

/**
Handle the request phase of the current request
@return EFalse
*/
EXPORT_C TBool CMTPRequestProcessor::DoHandleRequestPhaseL()
	{
	__FLOG(_L8("DoHandleRequestPhaseL - Entry."));
	TRAPD(err, iResponseCode = CheckRequestL());
	if ((err != KErrNone) || (iResponseCode != EMTPRespCodeOK))
		{
		if (HasDataphase()) 
			{
			// If we have a dataphase
			// we need to read in the data and discard it
			__FLOG(_L8("Response code is not OK, there is data phase."));			
			iNullBuffer.Close();
			iNullBuffer.CreateMaxL(KNullBufferSize);
			iNull.SetBuffer(iNullBuffer);
			ReceiveDataL(iNull);
			}
		else
			{
			if(err != KErrNone)
				{
				User::Leave(err);
				}
			SendResponseL(TMTPResponseCode(iResponseCode));
			}
		}
	else
		{
		TRAP(err, ServiceL());		
		if (err != KErrNone)
			{
			iResponseCode = EMTPRespCodeGeneralError;
			if (HasDataphase())
				{				
				// If we have a dataphase
				// we need to read in the data and discard it		
				iNullBuffer.Close();
				iNullBuffer.CreateMaxL(KNullBufferSize);					
				iNull.SetBuffer(iNullBuffer);			
				ReceiveDataL(iNull);		
				}
			else
				{
				User::Leave(err);
				}			
			}
		}
	__FLOG(_L8("DoHandleRequestPhaseL - Exit."));	
	return EFalse;	
	}
	
/**
Handle the receiving data phase of the current request
@return EFalse
*/	
EXPORT_C TBool CMTPRequestProcessor::DoHandleDataIToRPhaseL()
	{
	User::Invariant();
	return EFalse;
	}

/**
Handle the sending data phase of the current request
@return EFalse
*/		
EXPORT_C TBool CMTPRequestProcessor::DoHandleRToIPhaseL()
 	{
	User::Invariant();
	return EFalse;
	}

/**
Handle the response phase of the current request
@return EFalse
*/		
EXPORT_C TBool CMTPRequestProcessor::DoHandleResponsePhaseL()
	{
	TMTPResponseCode responseCode = (iCancelled ? EMTPRespCodeIncompleteTransfer : EMTPRespCodeOK);
	SendResponseL(responseCode);
	return EFalse;
	}

/**
Handle the completing phase of the current request
@return ETrue
*/		
EXPORT_C TBool CMTPRequestProcessor::DoHandleCompletingPhaseL()
	{
	CompleteRequestL();
	return ETrue;	
	}

/**
Check the current request
@return EMTPRespCodeOK if the reqeust is good, otherwise, one of the error response codes
*/
EXPORT_C TMTPResponseCode CMTPRequestProcessor::CheckRequestL()
	{
	if(!iRequestChecker)
		{
		iRequestChecker = CMTPRequestChecker::NewL(iFramework, iConnection);	
		}	
	__ASSERT_DEBUG(iRequest, User::Invariant()); 		
	return iRequestChecker->VerifyRequestL(*iRequest, iElementCount, iElements);
	}
	
/**
part of active object framework, provide default implementation
*/
EXPORT_C void CMTPRequestProcessor::RunL()
	{
	}

/**
part of active object framework, provide default implementation
*/
EXPORT_C void CMTPRequestProcessor::DoCancel()
	{
	}
	
/**
part of active object framework, provide default implementation
*/
EXPORT_C TInt CMTPRequestProcessor::RunError(TInt /*aError*/)
	{
	TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
	return KErrNone;
	}

EXPORT_C TBool CMTPRequestProcessor::HasDataphase() const
	{
	return EFalse;
	}

/**
retrieve the session id and transaction code from the current request
*/
void CMTPRequestProcessor::ExtractSessionTransactionId()
	{    
	iSessionId = iRequest->Uint32(TMTPTypeRequest::ERequestSessionID);    
	iTransactionCode = iRequest->Uint32(TMTPTypeRequest::ERequestTransactionID);		
	}









