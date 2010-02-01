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
#include <mtp/cmtptypedeleteobjectproplist.h>
#include <mtp/tmtptypedatapair.h>
#include <mtp/mmtpdataproviderframework.h>

#include "cmtpdeleteobjectproplist.h"
#include "mtpproxydppanic.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpobjectmgr.h"
#include "cmtpdataprovider.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeleteObjectPropList");)


/**
Verification data for the DeleteObjectPropList request
*/
const TMTPRequestElementInfo KMTPDeleteObjectPropListPolicy[] = 
    {
        { TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, (EMTPElementAttrDir | EMTPElementAttrWrite), 1, KMTPHandleAll, 0 }
    };

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPDeleteObjectPropList::NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection )
    {
    CMTPDeleteObjectPropList* self = new (ELeave) CMTPDeleteObjectPropList( aFramework, aConnection );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/ 
CMTPDeleteObjectPropList::~CMTPDeleteObjectPropList()
    {
    __FLOG(_L8("~CMTPDeleteObjectPropList - Entry"));
    
    delete iDeleteObjectPropList;
    iSingletons.Close();
    iTargetDps.Close();
    iSubDatasets.ResetAndDestroy();
    
    __FLOG(_L8("~CMTPDeleteObjectPropList - Exit"));
    __FLOG_CLOSE;
    }

CMTPDeleteObjectPropList::CMTPDeleteObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection):
CMTPRequestProcessor( aFramework, aConnection, (sizeof(KMTPDeleteObjectPropListPolicy) / sizeof(TMTPRequestElementInfo)), KMTPDeleteObjectPropListPolicy )
    {
    
    }

void CMTPDeleteObjectPropList::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    
    iSingletons.OpenL();
    
    __FLOG(_L8("ConstructL - Exit"));
    }

void CMTPDeleteObjectPropList::ServiceL()
    {
    __FLOG(_L8("ServiceL - Entry"));
    
    delete iDeleteObjectPropList;
    iTargetDps.Reset();
    iSubDatasets.Reset();
    

    iDeleteObjectPropList = CMTPTypeDeleteObjectPropList::NewL();
    ReceiveDataL(*iDeleteObjectPropList);

    __FLOG(_L8("ServiceL - Exit"));
    }

TBool CMTPDeleteObjectPropList::DoHandleResponsePhaseL()
    {
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));
      
    //if the ObjectHandle is 0x00000000, 
    //discard the received data and return Invalid_ObjectHandle error code
    if ( Request().Uint32(TMTPTypeRequest::ERequestParameter1) == KMTPHandleNone )
     {  
     TRAP_IGNORE(SendResponseL(EMTPRespCodeInvalidObjectHandle));
     }
    else
     {
     BuildSubRequestsL();
     
     if(iTargetDps.Count() == 0)
    	 {
    	 TRAP_IGNORE(SendResponseL(EMTPRespCodeInvalidDataset));
    	 }
     else
    	 {
	     iCurrentTarget = 0;
	     
	     Schedule(KErrNone);
    	 }
     }
    
    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));
    
    return EFalse;
    }

void CMTPDeleteObjectPropList::BuildSubRequestsL()
    {
    __FLOG(_L8("BuildSubRequests - Entry"));

    TInt count(iDeleteObjectPropList->NumberOfElements());
    TInt dpid(0);
    TInt index(-1);
    TInt tmpDpid(-1);
    for( TUint i(0); i < count; i++)
        {
        TMTPTypeDataPair& temp(iDeleteObjectPropList->ElementL(i));
        TMTPTypeDataPair* element = new (ELeave) TMTPTypeDataPair();
        CleanupStack::PushL(element);
        MMTPType::CopyL( temp, *element );
        dpid = iSingletons.ObjectMgr().ObjectOwnerId( temp.Uint32(TMTPTypeDataPair::EOwnerHandle) );
        if ( tmpDpid != dpid )
            {
            iTargetDps.Append(dpid);
            CMTPTypeDeleteObjectPropList* dataset = CMTPTypeDeleteObjectPropList::NewLC();
            iSubDatasets.AppendL( dataset );
            dataset->AppendL( element);
            CleanupStack::Pop(dataset);
            
            tmpDpid = dpid;
            ++index;
            }
        else
            {          
            iSubDatasets[index]->AppendL( element );
            }
        
        CleanupStack::Pop(element);
        }
    
    __FLOG(_L8("BuildSubRequests - Exit"));
    }

TBool CMTPDeleteObjectPropList::HasDataphase() const
    {
    __FLOG(_L8("HasDataphase - Entry"));
        
    
        
    __FLOG(_L8("HasDataphase - Exit"));
    
    return ETrue;
    }


#ifdef _DEBUG
void CMTPDeleteObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPDeleteObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));

    MMTPType::CopyL( *iSubDatasets[iCurrentTarget], aData);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }


void CMTPDeleteObjectPropList::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest&  /*aRequest*/, MMTPConnection& /* aConnection */, TRequestStatus& /*aStatus*/)
    {
    __FLOG(_L8("ProxySendDataL - Entry"));
        
    Panic(EMTPWrongRequestPhase);
        
    __FLOG(_L8("ProxySendDataL - Exit"));
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
void CMTPDeleteObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPDeleteObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __FLOG(_L8("ProxySendResponseL - Entry"));
    
    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    
    __FLOG(_L8("ProxySendResponseL - Exit"));
    }


/**
  Complete the transaction phase for the request is routed by Proxy DP
  Drive its AO to route other requests or send the response.
  @param aRequest       The corresponding request 
  @param aConnection    The corresponding connection
 */
#ifdef _DEBUG
void CMTPDeleteObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPDeleteObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    __FLOG(_L8("ProxyTransactionCompleteL - Entry"));
        
    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    const TUint16 KResponseCode(iResponse.Uint16(TMTPTypeResponse::EResponseCode)); 
    TInt err((KResponseCode == EMTPRespCodeOK) ? KErrNone : KErrGeneral);    
    if (err == KErrNone)
       {
       ++iCurrentTarget;
       }
    else
        {
        //when error, calculate the zero-based index of the first failed property
        TUint index = iResponse.Uint32(TMTPTypeResponse::EResponseParameter1);
        for(TInt i = 0 ; i < iCurrentTarget; i++ )
            {
            index  += iSubDatasets[i]->NumberOfElements();
            }
        iResponse.SetUint32(TMTPTypeResponse::EResponseParameter1, index );
        
        if(KResponseCode == EMTPRespCodeOperationNotSupported)
            {
            iResponse.SetUint16(TMTPTypeResponse::EResponseCode, EMTPRespCodeInvalidObjectHandle );
            }
        }

    Schedule(err);
        
    __FLOG(_L8("ProxyTransactionCompleteL - Exit"));
    }


    
void CMTPDeleteObjectPropList::SendResponseL(TUint16 aCode)
    {
    __FLOG(_L8("SendResponseL - Entry"));
        
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
        
    __FLOG(_L8("SendResponseL - Exit"));
    }

void CMTPDeleteObjectPropList::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    
    if ( (iStatus == KErrNone) && (iCurrentTarget < iTargetDps.Count()) )    
        {
        TUint id(iTargetDps[iCurrentTarget]);
        
        if(iSingletons.DpController().IsDataProviderLoaded(id))
        	{
        	iSingletons.DpController().DataProviderL(id).ExecuteProxyRequestL( *iRequest, Connection(), *this );
        	}
        else
        	{
        	 TUint index(0);
	         for(TInt i = 0 ; i < iCurrentTarget; i++ )
	            {
	            index  += iSubDatasets[i]->NumberOfElements();
	            }
	         iResponse.SetUint32(TMTPTypeResponse::EResponseParameter1, index );
        	 TRAP_IGNORE(SendResponseL(EMTPRespCodeInvalidObjectHandle));
        	}
        }
    else        
        {
        //Any error will stop the process, and send the corresponding response.
        SendResponseL(iResponse.Uint16(TMTPTypeResponse::EResponseCode));
        }  
    
    __FLOG(_L8("RunL - Exit"));
    }
        
TInt CMTPDeleteObjectPropList::RunError(TInt /*aError*/)
    {
    __FLOG(_L8("RunError - Entry"));
    
    TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
    
    __FLOG(_L8("RunError - Exit"));
    return KErrNone;
    }
            
/**
Completes the current asynchronous request with the specified 
completion code.
@param aError The asynchronous request completion request.
*/
void CMTPDeleteObjectPropList::Schedule(TInt aError)
    {
    __FLOG(_L8("Schedule - Entry"));
    
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    SetActive();
    
    __FLOG(_L8("Schedule - Exit"));
    }


