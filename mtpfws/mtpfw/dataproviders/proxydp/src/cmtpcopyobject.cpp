// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/cmtptypearray.h>

#include "cmtpdataprovider.h"
#include "cmtpcopyobject.h"
#include "cmtpparserrouter.h"
#include "mtpproxydppanic.h"
#include "cmtpobjectbrowser.h"
#include "mtpdppanic.h"
#include "cmtpobjectmgr.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpcopyobjectTraces.h"
#endif


const TUint KInvalidDpId = 0xFF;

/**
Verification data for the CopyObject request
*/    
const TMTPRequestElementInfo KMTPCopyObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir, 0, 0, 0},   	
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeStorageId, EMTPElementAttrWrite, 0, 0, 0},                
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 1, 0, 0}
    };


/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPCopyObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPCopyObject* self = new (ELeave) CMTPCopyObject(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPCopyObject::~CMTPCopyObject()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_CMTPCOPYOBJECT_DES_ENTRY );
    iSingletons.Close();
    iTargetDps.Close();
    iNewHandleParentStack.Close();
    iHandleDepths.Close();
    iHandles.Close();
    delete iObjBrowser;
    OstTraceFunctionExit0( CMTPCOPYOBJECT_CMTPCOPYOBJECT_DES_EXIT );
    }

/**
Constructor
*/    
CMTPCopyObject::CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy)
    {

    }
    
/**
Second phase constructor.
*/
void CMTPCopyObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
    iOwnerDp = KInvalidDpId;
    OstTraceFunctionExit0( CMTPCOPYOBJECT_CONSTRUCTL_EXIT );
    }
    
/**
DeleteObject request handler
*/ 
void CMTPCopyObject::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_SERVICEL_ENTRY );
    iTargetDps.Reset();
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(Request(), iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    
    BrowseHandlesL();

    OstTraceFunctionExit0( CMTPCOPYOBJECT_SERVICEL_EXIT );
    }

void CMTPCopyObject::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);    
    }
    
void CMTPCopyObject::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }
    
#ifdef _DEBUG    
void CMTPCopyObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPCopyObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_PROXYSENDRESPONSEL_ENTRY );
    __ASSERT_DEBUG(((&iCurrentRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    
    if ( aStatus == KErrNone )
        {
        if ( iIsCopyingFolder )
            {
            iNewHandleParentStack.AppendL( aResponse.Uint32( TMTPTypeResponse::EResponseParameter1 ) );
            }
        if ( KMTPHandleNone == iRespHandle )
            {
            iRespHandle = aResponse.Uint32( TMTPTypeResponse::EResponseParameter1 );
            }
        }
    
    MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status = &aStatus;
	User::RequestComplete(status, KErrNone);
	OstTraceFunctionExit0( CMTPCOPYOBJECT_PROXYSENDRESPONSEL_EXIT );
    }

#ifdef _DEBUG    
void CMTPCopyObject::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPCopyObject::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_PROXYTRANSACTIONCOMPLETEL_ENTRY );
    __ASSERT_DEBUG(((&iCurrentRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    TInt err((iResponse.Uint16(TMTPTypeResponse::EResponseCode) == EMTPRespCodeOK) ? KErrNone : KErrGeneral);    
    if (err == KErrNone)
        {
        iCurrentHandle--;
        Schedule(err);
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );        
        }
    OstTraceFunctionExit0( CMTPCOPYOBJECT_PROXYTRANSACTIONCOMPLETEL_EXIT );
    }

void CMTPCopyObject::RunL()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_RUNL_ENTRY ); 
    OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_RUNL, "iStatus == %d", iStatus.Int() );
    
    if ( iStatus == KErrNone )
        {
        NextObjectHandleL();
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
    OstTraceFunctionExit0( CMTPCOPYOBJECT_RUNL_EXIT );
    }
    
TInt CMTPCopyObject::RunError(TInt /*aError*/)
	{
	TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
	return KErrNone;
	}
	
/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPCopyObject::Schedule(TInt aError)
    {
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    }

/**
Sends a response to the initiator.
@param aCode MTP response code
*/
void CMTPCopyObject::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }

void CMTPCopyObject::BrowseHandlesL()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_BROWSEHANDLESL_ENTRY );
    
    delete iObjBrowser;
    iObjBrowser = NULL;
    iObjBrowser = CMTPObjectBrowser::NewL( iFramework );
    
    iHandles.Reset();
    iHandleDepths.Reset();
    iNewHandleParentStack.Reset();
    
    iRespHandle = KMTPHandleNone;
    
    MMTPType::CopyL( Request(), iCurrentRequest );
    
    CMTPObjectBrowser::TBrowseCallback callback = { CMTPCopyObject::OnBrowseObjectL, this };
    TUint32 handle = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
    TUint32 newHandleParent = Request().Uint32( TMTPTypeRequest::ERequestParameter3 );
    iNewHandleParentStack.AppendL( newHandleParent );
    iObjBrowser->GoL( KMTPFormatsAll, handle, KMaxTUint32, callback );
    OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_BROWSEHANDLESL, "iHandles.Count() = %d", iHandles.Count() );
    
    
    if ( iHandles.Count() > 0 )
        {
        iCurrentHandle = iHandles.Count() - 1;
        Schedule( KErrNone );
        }
    else
        {
        SendResponseL( EMTPRespCodeInvalidObjectHandle );
        }
    
    OstTraceFunctionExit0( CMTPCOPYOBJECT_BROWSEHANDLESL_EXIT );
    }

void CMTPCopyObject::NextObjectHandleL()
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_NEXTOBJECTHANDLEL_ENTRY );
    __ASSERT_DEBUG( ( iNewHandleParentStack.Count() > 0 ), User::Invariant() );
    iOwnerDp = KInvalidDpId;
    if ( iCurrentHandle >=0 )
        {
        OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_NEXTOBJECTHANDLEL, "iCurrentHandle = %d", iCurrentHandle );
        TUint32 handle = iHandles[iCurrentHandle];
        TUint32 depth = iHandleDepths[iCurrentHandle];
        OstTrace1( TRACE_NORMAL, DUP1_CMTPCOPYOBJECT_NEXTOBJECTHANDLEL, "depth = %d", depth );
        if ( iCurrentHandle != ( iHandles.Count() - 1 ) )
            {
            TUint32 previousDepth = iHandleDepths[iCurrentHandle+1];
            OstTrace1( TRACE_NORMAL, DUP2_CMTPCOPYOBJECT_NEXTOBJECTHANDLEL, "previousDepth = %d", previousDepth );
            if ( depth < previousDepth )
                {
                // Completed copying folder and all its sub-folder and files, pop all copied folders' handle which are not shallower than the current one.
                
                // Step 1: pop the previous handle itself if it is am empty folder
                if ( iIsCopyingFolder )
                    {
                    iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                    }
                // Step 2: pop the other folders' handle which are not shallower than the current one
                TUint32 loopCount = previousDepth - depth;
                for ( TUint i = 0; i < loopCount; i++ )
                    {
                    iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                    }
                }
            else if ( ( depth == previousDepth ) && iIsCopyingFolder )
                {
                // Completed copying empty folder, pop its handle
                iNewHandleParentStack.Remove( iNewHandleParentStack.Count() - 1 );
                }
            }
        
        iIsCopyingFolder = EFalse;
        iOwnerDp = iSingletons.ObjectMgr().ObjectOwnerId( handle );
        if ( iOwnerDp == KInvalidDpId )
            {
            SendResponseL( EMTPRespCodeInvalidObjectHandle );
            }
        else
            {
            iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter1, handle );
            iCurrentRequest.SetUint32( TMTPTypeRequest::ERequestParameter3, iNewHandleParentStack[iNewHandleParentStack.Count()-1] );
            }
        if ( iOwnerDp==iSingletons.DpController().DeviceDpId() )
            {
            iIsCopyingFolder = ETrue;
            }
        }
    else
        {
        iResponse.SetUint32( TMTPTypeResponse::EResponseParameter1, iRespHandle );
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }
  
    OstTraceFunctionExit0( CMTPCOPYOBJECT_NEXTOBJECTHANDLEL_EXIT );
    }

void CMTPCopyObject::OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth )
    {
    OstTraceFunctionEntry0( CMTPCOPYOBJECT_ONBROWSEOBJECTL_ENTRY );
    CMTPCopyObject* self = reinterpret_cast< CMTPCopyObject* >( aSelf );
    if ( self->iTargetDps.Find(self->iSingletons.ObjectMgr().ObjectOwnerId(aHandle)) != KErrNotFound )
        {
        self->iHandles.AppendL( aHandle );
        self->iHandleDepths.AppendL( aCurDepth );        
        }    
    OstTraceFunctionExit0( CMTPCOPYOBJECT_ONBROWSEOBJECTL_EXIT );
    }

