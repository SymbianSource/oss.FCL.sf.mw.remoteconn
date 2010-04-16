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

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/cmtptypearray.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdeleteobject.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"
#include "cmtpstoragemgr.h"
#include "mtpproxydppanic.h"
#include "rmtpframework.h"
#include "cmtpobjectbrowser.h"
#include "mtpdppanic.h"

__FLOG_STMT( _LIT8( KComponent,"PrxyDelObj" ); )
const TUint KInvalidDpId = 0xFF;

/**
Verification data for the DeleteObject request
*/
const TMTPRequestElementInfo KMTPDeleteObjectPolicy[] = 
    {
        { TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, (EMTPElementAttrDir | EMTPElementAttrWrite), 1, KMTPHandleAll, 0 }
    };

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPDeleteObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPDeleteObject* self = new (ELeave) CMTPDeleteObject(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPDeleteObject::~CMTPDeleteObject()
    {
    iSingletons.Close();
    iTargetDps.Close();
    iHandles.Close();
    delete iObjBrowser;
    
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

/**
Constructor
*/    
CMTPDeleteObject::CMTPDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPDeleteObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPDeleteObjectPolicy),
    iDeletedObjectsNumber(0)
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }
    
/**
Second phase constructor.
*/
void CMTPDeleteObject::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    
    iSingletons.OpenL();
    iOwnerDp = KInvalidDpId;
    __FLOG( _L8("-ConstructL") );
    }
    
TMTPResponseCode CMTPDeleteObject::CheckRequestL()
	{
    __FLOG(_L8("CheckRequestL - Entry"));
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();   
    if ((EMTPRespCodeOK == responseCode) && (iSingletons.DpController().EnumerateState() == CMTPDataProviderController::EEnumeratingSubDirFiles))
        {
		responseCode = EMTPRespCodeDeviceBusy;
        }
    
	__FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
    return responseCode;
	}

/**
DeleteObject request handler
*/ 
void CMTPDeleteObject::ServiceL()
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

void CMTPDeleteObject::ProxyReceiveDataL(MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);    
    }
    
void CMTPDeleteObject::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }
    
#ifdef _DEBUG    
void CMTPDeleteObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPDeleteObject::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(( ( (iRequest == &aRequest) || ( &iCurrentRequest == &aRequest ) ) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
	TRequestStatus* status = &aStatus;
	User::RequestComplete(status, KErrNone);
    }

#ifdef _DEBUG    
void CMTPDeleteObject::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPDeleteObject::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    __ASSERT_DEBUG(( ( (iRequest == &aRequest) || ( &iCurrentRequest == &aRequest ) ) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    TInt err(KErrNone);      
    TUint16 responseCode = iResponse.Uint16(TMTPTypeResponse::EResponseCode);
    
    //Delete one object successfully
    if (responseCode == EMTPRespCodeOK)
        {
        ++iDeletedObjectsNumber;
        }
    //if object is write-protected or assocation object is not empty, we continue to schedule the AO to delete the 
    //following objects. If it is not the 2 cases, just schedule with KErrGeneral so that we can sendresponse directly 
    //in RunL
    else if (responseCode != EMTPRespCodeObjectWriteProtected && responseCode != EMTPRespCodeAccessDenied && responseCode != EMTPRespCodeStoreReadOnly)
        {
        err = KErrGeneral;
        }
    
    ++iCurrentHandle;
    Schedule(err);
    }

void CMTPDeleteObject::RunL()
    {
    __FLOG( _L8("+RunL") );
    
    if ( iStatus == KErrNone )
        {
        //First check if the operation has been cancelled or not
        if(iCancelled)
            {
            __FLOG(_L8("Initiator cancell delete, send response with cancelled code "));
            SendResponseL(EMTPRespCodeTransactionCancelled);
            iCancelled = EFalse;
            }
        else
            {
            NextObjectHandleL();
            if ( iOwnerDp != KInvalidDpId )
                {
                CMTPDataProvider& dp = iSingletons.DpController().DataProviderL( iOwnerDp );
                dp.ExecuteProxyRequestL( iCurrentRequest, Connection(), *this );
                }
            }
        }
    else
        {
        SendResponseL( iResponse.Uint16( TMTPTypeResponse::EResponseCode ) );
        }
   
    __FLOG( _L8("-RunL") );
    }
    
/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPDeleteObject::Schedule(TInt aError)
    {
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    }

/**
Sends a response to the initiator.
@param aCode MTP response code
*/
void CMTPDeleteObject::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }

void CMTPDeleteObject::BrowseHandlesL()
    {
    __FLOG( _L8("+BrowseHandlesL") );
    
    delete iObjBrowser;
    iObjBrowser = NULL;
    iObjBrowser = CMTPObjectBrowser::NewL( iFramework );
    
    iHandles.Reset();
    iCurrentHandle = 0;
    iDeletedObjectsNumber = 0;
    
    MMTPType::CopyL( Request(), iCurrentRequest );
    
    CMTPObjectBrowser::TBrowseCallback callback = { CMTPDeleteObject::OnBrowseObjectL, this };
    TUint32 handle = Request().Uint32( TMTPTypeRequest::ERequestParameter1 );
    TUint32 fmtCode = KMTPFormatsAll;
    if ( KMTPHandleAll == handle )
    	{
    	fmtCode= Request().Uint32( TMTPTypeRequest::ERequestParameter2 );
    	}
    iObjBrowser->GoL( fmtCode, handle, KMaxTUint32, callback );
    
    if ( 0 == iHandles.Count() )
        {
        SendResponseL( EMTPRespCodeOK );        
        }
    else
        {
        Schedule( KErrNone );
        }

    __FLOG( _L8("-BrowseHandlesL") );
    }

void CMTPDeleteObject::NextObjectHandleL()
    {
    __FLOG( _L8("+NextObjectHandleL") );

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
        if (iDeletedObjectsNumber == iHandles.Count())
            {
            SendResponseL(EMTPRespCodeOK);
            }
        else if (iDeletedObjectsNumber == 0)
            {
            SendResponseL(EMTPRespCodeObjectWriteProtected);
            }
        else
            {
            SendResponseL(EMTPRespCodePartialDeletion);
            }
        }

    __FLOG( _L8("-NextObjectHandleL") );
    }

void CMTPDeleteObject::OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 /*aCurDepth*/ )
    {
    CMTPDeleteObject* self = reinterpret_cast< CMTPDeleteObject* >( aSelf );
    if ( self->iTargetDps.Find(self->iSingletons.ObjectMgr().ObjectOwnerId(aHandle)) != KErrNotFound )
        {
        self->iHandles.AppendL( aHandle );
        }
    }

