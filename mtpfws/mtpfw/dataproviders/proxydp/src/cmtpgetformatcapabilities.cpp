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
#include <mtp/mmtpdataproviderframework.h>


#include "cmtpgetformatcapabilities.h"
#include "mtpproxydppanic.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpobjectmgr.h"
#include "cmtpdataprovider.h"
#include "cmtpparserrouter.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetformatcapabilitiesTraces.h"
#endif



// Class constants.


/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPGetFormatCapabilities::NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection )
    {
    CMTPGetFormatCapabilities* self = new (ELeave) CMTPGetFormatCapabilities( aFramework, aConnection );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/ 
CMTPGetFormatCapabilities::~CMTPGetFormatCapabilities()
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_CMTPGETFORMATCAPABILITIES_DES_ENTRY );
    
    iSingletons.Close();
    delete iCapabilityList;
    iTargetDps.Close();

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_CMTPGETFORMATCAPABILITIES_DES_EXIT );
    }

CMTPGetFormatCapabilities::CMTPGetFormatCapabilities(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection):
CMTPRequestProcessor( aFramework, aConnection, 0, NULL )
    {
    
    }

void CMTPGetFormatCapabilities::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_CONSTRUCTL_ENTRY );
    
    iSingletons.OpenL();

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_CONSTRUCTL_EXIT );
    }


void CMTPGetFormatCapabilities::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_SERVICEL_ENTRY );
    
    delete iCapabilityList;
    iCapabilityList = CMTPTypeFormatCapabilityList::NewL();
    iTargetDps.Reset();
    
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    iCurrentTarget = 0;
    Schedule(KErrNone);
    
    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_SERVICEL_EXIT );
    }

void CMTPGetFormatCapabilities::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_PROXYRECEIVEDATAL_ENTRY );

    Panic(EMTPWrongRequestPhase);

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_PROXYRECEIVEDATAL_EXIT );
    }


#ifdef _DEBUG   
void CMTPGetFormatCapabilities::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetFormatCapabilities::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_PROXYSENDDATAL_ENTRY );

    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    __ASSERT_DEBUG(aData.Type() == EMTPTypeFormatCapabilityListDataset, Panic(EMTPInvalidDataType));
    
    
    const CMTPTypeFormatCapabilityList& sourceList(static_cast<const CMTPTypeFormatCapabilityList&>(aData));
    const TInt count(sourceList.NumberOfElements());
    for(TInt i(0); (i < count); i++)
        {
        CMTPTypeFormatCapability& element(sourceList.ElementL(i)); 
        CMTPTypeFormatCapability* newElement = CMTPTypeFormatCapability::NewLC();
        MMTPType::CopyL(element, *newElement);
        iCapabilityList->AppendL(newElement);
        CleanupStack::Pop(newElement);
        }

    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    
    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_PROXYSENDDATAL_EXIT );
    }

/**
  Send the corresponding response for the request is routed by Proxy DP
  ProxySendResponseL will complete the request of framework's AO, then switch the transaction phase to TranscationComplete
  so ProxyTransactionCompleteL will be called later.
  @param aResponse      The response which is sent by some DP
  @param aRequest       The corresponding request 
  @param aConnection    The corresponding connection
  @param aStatus        The status of framework AO 
 */
#ifdef _DEBUG
void CMTPGetFormatCapabilities::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetFormatCapabilities::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_PROXYSENDRESPONSEL_ENTRY );
    
    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
    TRequestStatus* status(&aStatus);
    User::RequestComplete(status, KErrNone);
    
    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_PROXYSENDRESPONSEL_EXIT );
    }


/**
  Complete the transaction phase for the request is routed by Proxy DP
  Drive its AO to route other requests or send the response.
  @param aRequest       The corresponding request 
  @param aConnection    The corresponding connection
 */
#ifdef _DEBUG
void CMTPGetFormatCapabilities::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPGetFormatCapabilities::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_PROXYTRANSACTIONCOMPLETEL_ENTRY );
     
    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    TInt err((iResponse.Uint16(TMTPTypeResponse::EResponseCode) == EMTPRespCodeOK) ? KErrNone : KErrGeneral);    
    if (err == KErrNone)
        {
        ++iCurrentTarget;
        }
    Schedule(err);
        
    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_PROXYTRANSACTIONCOMPLETEL_EXIT );
    }


void CMTPGetFormatCapabilities::SendResponseL(TUint16 aCode)
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_SENDRESPONSEL_ENTRY );
        
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_SENDRESPONSEL_EXIT );
    }

void CMTPGetFormatCapabilities::RunL()
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_RUNL_ENTRY );

    if (iStatus == KErrNone)
       {
       if (iCurrentTarget < iTargetDps.Count())   
           {
           TUint id(iTargetDps[iCurrentTarget]);
           CMTPDataProvider& dp(iSingletons.DpController().DataProviderL(id));
           dp.ExecuteProxyRequestL(*iRequest, Connection(), *this);
           }
       else
           {
           SendDataL(*iCapabilityList);
           }
       }
   else
       {
       SendResponseL(iResponse.Uint16(TMTPTypeResponse::EResponseCode));
       }   

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_RUNL_EXIT );
    }
        
TInt CMTPGetFormatCapabilities::RunError(TInt /*aError*/)
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_RUNERROR_ENTRY );

    TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_RUNERROR_EXIT );
    return KErrNone;
    }
            
/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPGetFormatCapabilities::Schedule(TInt aError)
    {
    OstTraceFunctionEntry0( CMTPGETFORMATCAPABILITIES_SCHEDULE_ENTRY );

    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    SetActive();

    OstTraceFunctionExit0( CMTPGETFORMATCAPABILITIES_SCHEDULE_EXIT );
    }

