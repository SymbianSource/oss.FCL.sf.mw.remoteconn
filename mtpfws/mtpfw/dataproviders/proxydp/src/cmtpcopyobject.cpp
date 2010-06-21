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

__FLOG_STMT( _LIT8( KComponent,"PrxyCopyObj" ); )
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
    iSingletons.Close();
    iTargetDps.Close();
    iNewHandleParentStack.Close();
    iHandleDepths.Close();
    iHandles.Close();
    delete iObjBrowser;
    
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

/**
Constructor
*/    
CMTPCopyObject::CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy)
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }
    
/**
Second phase constructor.
*/
void CMTPCopyObject::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    iSingletons.OpenL();
    iOwnerDp = KInvalidDpId;
    __FLOG( _L8("-ConstructL") );
    }
    
/**
DeleteObject request handler
*/ 
void CMTPCopyObject::ServiceL()
    {
    __FLOG( _L8("+ServiceL") );
    iTargetDps.Reset();
    CMTPParserRouter& router(iSingletons.Router());
    CMTPParserRouter::TRoutingParameters params(Request(), iConnection);
    router.ParseOperationRequestL(params);
    router.RouteOperationRequestL(params, iTargetDps);
    
    BrowseHandlesL();
    
    __FLOG( _L8("-ServiceL") );
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
    __FLOG( _L8("+ProxySendResponseL") );
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
    __FLOG( _L8("-ProxySendResponseL") );
    }

#ifdef _DEBUG    
void CMTPCopyObject::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPCopyObject::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    __FLOG( _L8("+ProxyTransactionCompleteL") );
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
    __FLOG( _L8("-ProxyTransactionCompleteL") );
    }

void CMTPCopyObject::RunL()
    {
    __FLOG( _L8("+RunL") );
    
    __FLOG_1( _L8("iStatus == %d"), iStatus.Int() );

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
    __FLOG( _L8("-RunL") );
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
    __FLOG( _L8("+BrowseHandlesL") );
    
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
    __FLOG_1( _L8("iHandles.Count() = %d"), iHandles.Count() );
    
    if ( iHandles.Count() > 0 )
        {
        iCurrentHandle = iHandles.Count() - 1;
        Schedule( KErrNone );
        }
    else
        {
        SendResponseL( EMTPRespCodeInvalidObjectHandle );
        }
    
    __FLOG( _L8("-BrowseHandlesL") );
    }

void CMTPCopyObject::NextObjectHandleL()
    {
    __FLOG( _L8("+NextObjectHandleL") );
    __ASSERT_DEBUG( ( iNewHandleParentStack.Count() > 0 ), User::Invariant() );
    iOwnerDp = KInvalidDpId;
    if ( iCurrentHandle >=0 )
        {
        __FLOG_1( _L8("iCurrentHandle = %d"), iCurrentHandle );
        TUint32 handle = iHandles[iCurrentHandle];
        TUint32 depth = iHandleDepths[iCurrentHandle];
        __FLOG_1( _L8("depth = %d"), depth );
        if ( iCurrentHandle != ( iHandles.Count() - 1 ) )
            {
            TUint32 previousDepth = iHandleDepths[iCurrentHandle+1];
            __FLOG_1( _L8("previousDepth = %d"), previousDepth );
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
  
    __FLOG( _L8("-NextObjectHandleL") );
    }

void CMTPCopyObject::OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth )
    {
    CMTPCopyObject* self = reinterpret_cast< CMTPCopyObject* >( aSelf );
    if ( self->iTargetDps.Find(self->iSingletons.ObjectMgr().ObjectOwnerId(aHandle)) != KErrNotFound )
        {
        self->iHandles.AppendL( aHandle );
        self->iHandleDepths.AppendL( aCurDepth );        
        }    
    }

