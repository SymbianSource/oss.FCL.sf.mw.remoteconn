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

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtptypearray.h>

#include "rmtpframework.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpparserrouter.h"
#include "cmtpdataprovider.h"
#include "cmtpgetobjectpropssupported.h"
#include "mtpproxydppanic.h"
   
/**
Factory method.
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object.
*/ 
MMTPRequestProcessor* CMTPGetObjectPropsSupported::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetObjectPropsSupported* self = new (ELeave) CMTPGetObjectPropsSupported(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
	return self;
	}

/**
Destructor.
*/	
CMTPGetObjectPropsSupported::~CMTPGetObjectPropsSupported()
	{
	iObjectPropsSupported.Reset();
	delete iDataset;
    iTargetDps.Close();
	iSingletons.Close();
	}

/**
GetObjectPropsSupported request handler. Obtains a list of all the data providers
that support the request, then starts an active object to send it to them all.
*/	
void CMTPGetObjectPropsSupported::ServiceL()
	{
	iObjectPropsSupported.Reset();
	iTargetDps.Reset();
	
	CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
	iCurrentTarget = 0;	
	Schedule(KErrNone);
	}

void CMTPGetObjectPropsSupported::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
	{	
	Panic(EMTPWrongRequestPhase);
	}

#ifdef _DEBUG	
void CMTPGetObjectPropsSupported::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetObjectPropsSupported::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
	{
	__ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
	__ASSERT_DEBUG(aData.Type() == EMTPTypeAUINT16, Panic(EMTPInvalidDataType));
	const CMTPTypeArray& KProperties(static_cast<const CMTPTypeArray&>(aData));
	const TInt KCountProperties(KProperties.NumElements());
	for (TUint i(0); (i < KCountProperties); i++)
		{
		const TInt KErr(iObjectPropsSupported.InsertInOrder(KProperties.ElementUint(i)));
		if ((KErr != KErrNone) &&
		    (KErr != KErrAlreadyExists))
		    {
		    User::Leave(KErr);
		    }
		}
	TRequestStatus* status(&aStatus);
	User::RequestComplete(status, KErrNone);	
	}

#ifdef _DEBUG	
void CMTPGetObjectPropsSupported::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetObjectPropsSupported::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
	{
	__ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
	MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status(&aStatus);
	User::RequestComplete(status, KErrNone);
	}

#ifdef _DEBUG		
void CMTPGetObjectPropsSupported::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPGetObjectPropsSupported::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
	{
	__ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    TInt err((iResponse.Uint16(TMTPTypeResponse::EResponseCode) == EMTPRespCodeOK) ? KErrNone : KErrGeneral);    
    if (err == KErrNone)
        {
        iCurrentTarget++;
        }
    Schedule(err);
	}	

/**
Sends the request to all the data providers that expressed an interest,
one each iteration of the active object.
*/
void CMTPGetObjectPropsSupported::RunL()
	{
	// We cannot use assertion here, because it might be completed with KErrGeneral. See ProxyTransactionCompleteL()
    if (iStatus == KErrNone)
        {
        if (iCurrentTarget < iTargetDps.Count())   
            {
            const TUint KTarget(iTargetDps[iCurrentTarget]);
            iSingletons.DpController().DataProviderL(KTarget).ExecuteProxyRequestL(*iRequest, Connection(), *this);
            }
        else
    		{
    		delete iDataset;
    		iDataset = NULL;
    		iDataset = CMTPTypeArray::NewL(EMTPTypeAUINT16, iObjectPropsSupported);
			SendDataL(*iDataset);
			}
		}
    else
        {
        SendResponseL(iResponse.Uint16(TMTPTypeResponse::EResponseCode));
        } 	
	}

/**
Constructor.
*/	
CMTPGetObjectPropsSupported::CMTPGetObjectPropsSupported(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
	{
	
	}
    
/**
Second phase constructor.
*/
void CMTPGetObjectPropsSupported::ConstructL()
    {
    iSingletons.OpenL();
    }

/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPGetObjectPropsSupported::Schedule(TInt aError)
	{
	TRequestStatus* status(&iStatus);
	User::RequestComplete(status, aError);
	SetActive();
	}

/**
Sends a response to the initiator.
@param aCode MTP response code
*/
void CMTPGetObjectPropsSupported::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }
