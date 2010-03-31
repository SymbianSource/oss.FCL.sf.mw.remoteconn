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

#ifndef __CMTPREQUESTPROCESSOR_H__
#define __CMTPREQUESTPROCESSOR_H__

#include <e32base.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperesponse.h>
#include <mtp/tmtptypenull.h>
#include "cmtprequestchecker.h"
#include "mtpdebug.h"

class MMTPConnection;
class TMTPTypeRequest;
class CMTPDataProviderPlugin;
class MMTPDataProviderFramework;
class TMTPTypeEvent;
class CMTPObjectInfo;

/** 
Defines a request processor interface

@internalTechnology
*/	
class MMTPRequestProcessor
	{
public:	
	/**
	Process a request from the initiator
	@param aRequest	The request to be processed
	@param aPhase	The transaction phase of the request
	@return ETrue to signal that the processor object can be deleted, EFalse to keep the processor object
	*/
	virtual TBool HandleRequestL(const TMTPTypeRequest& aRequest, TMTPTransactionPhase aPhase) = 0;
	
	/**
	Process an event from the initiator
	@param aEvent	The event to be processed
	*/
	virtual void HandleEventL(const TMTPTypeEvent& aEvent) = 0;
	
	/**
	delete the request processor object
	*/
	virtual void Release() = 0;
	
	/**
	Check if the processor matches the request on the connection
	@param aRequest	The request to be checked
	@param aConnection The connection from which the request comes
	@return ETrue to indicate the processor can handle the request, otherwise, EFalse
	*/
	virtual TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const = 0;

	/**
	Check if the processor matches the event on the connection
	@param aEvent The event to be checked
	@param aConnection The connection from which the event comes
	@return ETrue to indicate the processor can handle the event, otherwise, EFalse
	*/	
	virtual TBool Match(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection) const = 0;
	
	/**
	Get the request object which the processor is currently handling
	@return the request object which the processor is currently handling
	*/
	virtual const TMTPTypeRequest& Request() const = 0;
	
	/**
	Get the connection object associated with the current request object
	@return the connection object associated with the current request object
	*/
	virtual MMTPConnection& Connection() const = 0;
    /**
    Get the Session ID associated with the current request object
    @return Session ID associated with the current request object
    */	
	virtual TUint32 SessionId() = 0;
	
	};

/** 
Defines a processor factory function pointer

@internalTechnology
*/
typedef MMTPRequestProcessor* (*TMTPRequestProcessorCreateFunc)(
																MMTPDataProviderFramework& aFramework, 
																MMTPConnection& aConnection);

/** 
Defines an entry which maps from operation code to the request processor

@internalTechnology
*/																
typedef struct 
	{
	TUint16							iOperationCode;
	TMTPRequestProcessorCreateFunc	iCreateFunc;
	}TMTPRequestProcessorEntry;


/** 
Defines a generic request processor interface from which all processors derive

@internalTechnology
*/		
class CMTPRequestProcessor : public CActive, public MMTPRequestProcessor
	{
protected:	
	IMPORT_C CMTPRequestProcessor(
						MMTPDataProviderFramework& aFramework,
						MMTPConnection& aConnection,
						TInt aElementCount,
						const TMTPRequestElementInfo* aElements);
	IMPORT_C virtual ~CMTPRequestProcessor();	

protected:	//utility methods	
	IMPORT_C void SendResponseL(TMTPResponseCode aResponseCode, TInt aParameterCount = 0, TUint32* aParams = NULL);
	IMPORT_C void CompleteRequestL();
	IMPORT_C void SendDataL(const MMTPType& aData);
	IMPORT_C void ReceiveDataL(MMTPType& aData);
    IMPORT_C void RegisterPendingRequest(TUint aTimeOut = 0);
		
protected:	//	from MMTPRequestProcessor	
	IMPORT_C virtual TBool HandleRequestL(const TMTPTypeRequest& aRequest, TMTPTransactionPhase aPhase);
	IMPORT_C virtual void HandleEventL(const TMTPTypeEvent& aEvent);
	
	IMPORT_C virtual void Release();

	IMPORT_C virtual TBool Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const;
	IMPORT_C virtual TBool Match(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection) const;
		
	IMPORT_C virtual const TMTPTypeRequest& Request() const;
	IMPORT_C virtual MMTPConnection& Connection() const;
	   
    IMPORT_C virtual TUint32 SessionId();

protected:	// from CActive
	IMPORT_C virtual void RunL();
	IMPORT_C virtual void DoCancel();
	IMPORT_C virtual TInt RunError(TInt aError);
	
protected:	//new virtuals 	
	IMPORT_C virtual TBool DoHandleRequestPhaseL();
	IMPORT_C virtual TBool DoHandleDataIToRPhaseL();
	IMPORT_C virtual TBool DoHandleRToIPhaseL();
	IMPORT_C virtual TBool DoHandleResponsePhaseL();
	IMPORT_C virtual TBool DoHandleCompletingPhaseL();
	IMPORT_C virtual TMTPResponseCode CheckRequestL();
	IMPORT_C virtual TBool HasDataphase() const;
	
	/**
	service a request at request phase
	*/
	IMPORT_C virtual void ServiceL() = 0;


private:
	void ExtractSessionTransactionId();					
	
protected:
	MMTPDataProviderFramework&	    iFramework;
	const TMTPTypeRequest*		    iRequest;			//the pending requst object
	MMTPConnection&				    iConnection;		//the connection from which the request comes
	TMTPTypeResponse			    iResponse;			//the response object to send to the initiator
	TBool						    iCancelled;			//indicates whether this request has been cancelled
	CMTPRequestChecker*		    	iRequestChecker;	//a utility class providing generic request verification service
	TInt					    	iElementCount;		//number of verification elements used for request checker
	const TMTPRequestElementInfo*	iElements;	 		//pointer to an array of verification elements
	TUint32						    iSessionId;			//session id for the pending request
	TUint32						    iTransactionCode;	//transaction code for the pending request

private:
	TMTPResponseCode 				iResponseCode;		// contains response from CheckRequestL call
	RBuf8 							iNullBuffer; 		// buffer to receive data from discarded data phase
	TMTPTypeNull 					iNull;
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
	};
	
#endif // __CMTPREQUESTPROCESSOR_H__

