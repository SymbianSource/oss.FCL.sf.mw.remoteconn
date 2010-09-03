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
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/cmtptypearray.h>

#include "rmtpframework.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpparserrouter.h"
#include "cmtpdataprovider.h"
#include "cmtpgetobjectproplist.h"
#include "mtpproxydppanic.h"
#include "cmtpobjectbrowser.h"
#include "mtpdppanic.h"
#include "cmtpobjectmgr.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpgetobjectproplistTraces.h"
#endif


const TUint KInvalidDpId = 0xFF;

/**
Verification data for the GetObjectPropList request
*/
const TMTPRequestElementInfo KMTPGetObjectPropListPolicy[] = 
    {
		{ TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrNone, 2, KMTPHandleAll, KMTPHandleNone}
    };

/**
Factory method.
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object.
*/ 
MMTPRequestProcessor* CMTPGetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPGetObjectPropList* self = new (ELeave) CMTPGetObjectPropList(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
	return self;
	}

/**
Destructor.
*/	
CMTPGetObjectPropList::~CMTPGetObjectPropList()
	{
	OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_ENTRY );
	delete iObjectPropList;
	iSingletons.Close();
	iTargetDps.Close();
    iHandles.Close();
    delete iObjBrowser;
	OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_EXIT );
	}

/**
GetObjectPropList request handler. Builds a list of all the data providers
that support the request, then starts an active object to send it to them all.
*/	
void CMTPGetObjectPropList::ServiceL()
    {
    if(iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted)
        {
        TUint32 handle = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
        TUint32 depth = Request().Uint32( TMTPTypeRequest::ERequestParameter5 );  
        if (depth > 0)
            {
            if (handle != KMTPHandleNone || depth != 1)
                {
                RegisterPendingRequest();
                return;
                }
            }
        }
    iTargetDps.Reset();
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(Request(), iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    
    GetObjectHandlesL();
    }

void CMTPGetObjectPropList::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
	{	
	Panic(EMTPWrongRequestPhase);
	}

#ifdef _DEBUG	
void CMTPGetObjectPropList::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetObjectPropList::ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
	{
	__ASSERT_DEBUG((( (iRequest == &aRequest) || ( &iCurrentRequest == &aRequest ) ) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
	__ASSERT_DEBUG(aData.Type() == EMTPTypeObjectPropListDataset, Panic(EMTPInvalidDataType));
	
	const CMTPTypeObjectPropList& sourceObjectPropList(static_cast<const CMTPTypeObjectPropList&>(aData));

	iObjectPropList->AppendObjectPropListL(sourceObjectPropList);
	
    TRequestStatus* status(&aStatus);
	User::RequestComplete(status, KErrNone);	
	}

#ifdef _DEBUG	
void CMTPGetObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPGetObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
	{
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_PROXYSENDRESPONSEL_ENTRY );
    
	__ASSERT_DEBUG((( (iRequest == &aRequest) || ( &iCurrentRequest == &aRequest ) ) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
	MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status(&aStatus);
	User::RequestComplete(status, KErrNone);

	OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_PROXYSENDRESPONSEL_EXIT );
	}

#ifdef _DEBUG		
void CMTPGetObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPGetObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
	{
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_PROXYTRANSACTIONCOMPLETEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPGETOBJECTPROPLIST_PROXYTRANSACTIONCOMPLETEL, 
            "Response code is 0x%08X",  iResponse.Uint16(TMTPTypeResponse::EResponseCode));
    
	__ASSERT_DEBUG((( (iRequest == &aRequest) || ( &iCurrentRequest == &aRequest ) ) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
	TUint16 response = iResponse.Uint16(TMTPTypeResponse::EResponseCode);
	if(( EMTPRespCodeGroupNotSupported == response) || (EMTPRespCodeSpecificationByGroupUnsupported == response) || ( EMTPRespCodeObjectPropNotSupported == response))
		{
		response = EMTPRespCodeOK;
		iResponse.SetUint16(TMTPTypeResponse::EResponseCode, response);
		}
	
    TInt err((EMTPRespCodeOK == response) ? KErrNone : KErrGeneral);      
    if (err == KErrNone)
        {
        ++iCurrentHandle;
        Schedule(err);
        }
    else
        {
        SendResponseL( response );    
        }
    
    
    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_PROXYTRANSACTIONCOMPLETEL_EXIT );
	}	

/**
Sends the request to all the data providers that expressed an interest,
one each iteration of the active object.
*/
void CMTPGetObjectPropList::RunL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_RUNL_ENTRY );
    
    // We cannot use assertion here, because it might be completed with KErrGeneral. See ProxyTransactionCompleteL()
    
    if ( iStatus == KErrNone )
        {
        GetNextObjectPropL();
        if ( iOwnerDp != KInvalidDpId )
            {
            CMTPDataProvider& dp = iSingletons.DpController().DataProviderL( iOwnerDp );
            dp.ExecuteProxyRequestL( iCurrentRequest, Connection(), *this );
            }
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }

    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_RUNL_EXIT );
    }

/**
Constructor.
*/	
CMTPGetObjectPropList::CMTPGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPGetObjectPropListPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetObjectPropListPolicy)
    {
    OstTraceFunctionEntry0( DUP1_CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPGETOBJECTPROPLIST_CMTPGETOBJECTPROPLIST_EXIT );
    }

/**
Second phase constructor.
*/
void CMTPGetObjectPropList::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
    iOwnerDp = KInvalidDpId;
    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_CONSTRUCTL_EXIT );
    }

/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPGetObjectPropList::Schedule(TInt aError)
	{
	TRequestStatus* status(&iStatus);
	User::RequestComplete(status, aError);
	SetActive();
	}

/**
Sends a response to the initiator.
@param aCode MTP response code
*/
void CMTPGetObjectPropList::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }

void CMTPGetObjectPropList::GetObjectHandlesL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_GETOBJECTHANDLESL_ENTRY );
    
    delete iObjBrowser;
    iObjBrowser = NULL;
    iObjBrowser = CMTPObjectBrowser::NewL( iFramework );
    
    iHandles.Reset();
    
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    
    iCurrentHandle = 0;
    MMTPType::CopyL( Request(), iCurrentRequest );
    // Set depth to be zero to avoid the other DPs enumerating objects repeatedly.
    iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter5, 0 );
    
    CMTPObjectBrowser::TBrowseCallback callback = { CMTPGetObjectPropList::OnBrowseObjectL, this };
    TUint32 handle = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
    TUint32 fmtCode = Request().Uint32( TMTPTypeRequest::ERequestParameter2 );
    TUint32 depth = Request().Uint32( TMTPTypeRequest::ERequestParameter5 );
    iObjBrowser->GoL( fmtCode, handle, depth, callback );
    OstTrace1( TRACE_NORMAL, CMTPGETOBJECTPROPLIST_GETOBJECTHANDLESL, 
            "The total of object handles is %d", iHandles.Count() );

    if ( 0 == iHandles.Count() )
        {
          SendDataL( *iObjectPropList );
        }
    else
        {
        Schedule( KErrNone );
        }

    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_GETOBJECTHANDLESL_EXIT );
    }

void CMTPGetObjectPropList::GetNextObjectPropL()
    {
    OstTraceFunctionEntry0( CMTPGETOBJECTPROPLIST_GETNEXTOBJECTPROPL_ENTRY );

    iOwnerDp = KInvalidDpId;
    if ( iCurrentHandle < iHandles.Count() )
        {
        TUint32 handle = iHandles[iCurrentHandle];
        iOwnerDp = iSingletons.ObjectMgr().ObjectOwnerId( handle );
        if ( iOwnerDp == KInvalidDpId )
            {
            SendResponseL(EMTPRespCodeInvalidObjectHandle);
            }
        else
            {
            iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter1, handle );
            }
        }
    else
        {
        SendDataL( *iObjectPropList );
        }

    OstTraceFunctionExit0( CMTPGETOBJECTPROPLIST_GETNEXTOBJECTPROPL_EXIT );
    }

void CMTPGetObjectPropList::OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 /*aCurDepth*/ )
    {
    CMTPGetObjectPropList* self = reinterpret_cast< CMTPGetObjectPropList* >( aSelf );
    if ( self->iTargetDps.Find(self->iSingletons.ObjectMgr().ObjectOwnerId(aHandle)) != KErrNotFound )
        {
        self->iHandles.AppendL( aHandle );
        }
    }
