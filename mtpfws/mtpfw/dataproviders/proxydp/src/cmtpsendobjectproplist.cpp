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
#include <mtp/cmtptypeobjectproplist.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpparserrouter.h"
#include "cmtpsendobjectproplist.h"
#include "mtpproxydppanic.h"
#include "rmtpframework.h"
#include "cmtpstoragemgr.h"

/**
Verification data for the SendObjectPropList request
*/
const TMTPRequestElementInfo KMTPSendObjectPropListPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrWrite, 1, 0, 0}, 
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeObjectHandle, (EMTPElementAttrDir | EMTPElementAttrWrite), 2, KMTPHandleAll, KMTPHandleNone}
    };

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPSendObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPSendObjectPropList* self = new (ELeave) CMTPSendObjectPropList(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPSendObjectPropList::~CMTPSendObjectPropList()
    {
    delete iObjectPropList;
    iSingletons.Close();
    }

/**
Standard c++ constructor
*/    
CMTPSendObjectPropList::CMTPSendObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, (sizeof(KMTPSendObjectPropListPolicy) / sizeof(TMTPRequestElementInfo)), KMTPSendObjectPropListPolicy)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPSendObjectPropList::ConstructL()
    {
    iSingletons.OpenL();
    }
    
void CMTPSendObjectPropList::ServiceL()
    {
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    ReceiveDataL(*iObjectPropList);
    }

TBool CMTPSendObjectPropList::HasDataphase() const
    {
    return ETrue;
    }
    
TBool CMTPSendObjectPropList::DoHandleResponsePhaseL()
    {
    TMTPResponseCode rsp(EMTPRespCodeOK);
    
    TUint formatcode = Request().Uint32(TMTPTypeResponse::EResponseParameter3);
    
    if( EMTPFormatCodeAssociation != formatcode )
        {
        //Here, it must be that FormatCode is supported by multi DPs and the first Parameter (StorageID) should be used.
        //Because if only one target DP, Parser&Router will directly dispatch the request to it
        //if no target DP, the request will be dispatch to Device DP.
        
        RArray<TUint> targets;
        CleanupClosePushL(targets);
        CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
        iSingletons.Router().ParseOperationRequestL(params);
        iSingletons.Router().RouteOperationRequestL(params, targets);
        CMTPStorageMgr& storages(iSingletons.StorageMgr());
        const TUint KStorageId = Request().Uint32(TMTPTypeResponse::EResponseParameter1);
        if( KMTPNotSpecified32 == KStorageId)
            {
            rsp = EMTPRespCodeGeneralError;
            }
        else if( storages.ValidStorageId(KStorageId) )
            {
            TInt dpID(KErrNotFound);
            if (storages.LogicalStorageId(KStorageId))
                {
                dpID = storages.LogicalStorageOwner(KStorageId);
                }
            else
                {
                dpID = storages.PhysicalStorageOwner(KStorageId);
                }
            
            if( targets.Find( dpID ) == KErrNotFound )
                {
                rsp = EMTPRespCodeGeneralError;
                }
            else
                {
                iSingletons.DpController().DataProviderL(dpID).ExecuteProxyRequestL(Request(), Connection(), *this);
                }
            }
        else
            {
            rsp = EMTPRespCodeInvalidStorageID;
            }
        
        CleanupStack::PopAndDestroy(&targets);
        
        }
    else
        {
        /* 
        Only requests with an ObjectFormatCode of EMTPFormatCodeAssociation 
        are routed to the proxy DP. Locate and extract the association type from 
        the received ObjectPropList dataset.
        */
        TUint32 count(iObjectPropList->NumberOfElements());
		iObjectPropList->ResetCursor();		
        TInt    type(KErrNotFound);
        TUint16 tmpAssocValue(EMTPAssociationTypeGenericFolder);
        for (TUint i(0); ((rsp == EMTPRespCodeOK) && (type == KErrNotFound) && (i < count)); i++)
            {
			CMTPTypeObjectPropListElement& prop=iObjectPropList->GetNextElementL();			
            if (prop.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode) == EMTPObjectPropCodeAssociationType)
                {
                if (prop.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16)
                    {
                    rsp = EMTPRespCodeInvalidObjectPropFormat;
                    }
                else
                    {
                    type = i;
					tmpAssocValue = prop.Uint16L(CMTPTypeObjectPropListElement::EValue);
                    }
                }
            }	        
        TUint16 assocValue;
        if ((type == KErrNotFound) &&
            (rsp == EMTPRespCodeOK))
            {
            // If no property type is specified, create a generic folder
            assocValue = EMTPAssociationTypeGenericFolder;
            }
        else
            {
            assocValue = tmpAssocValue;
            }
        
        if (rsp == EMTPRespCodeOK)
            {
            CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
            iSingletons.Router().ParseOperationRequestL(params);
            params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatCode, EMTPFormatCodeAssociation);
            params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, assocValue);
            RArray<TUint> targets;
            CleanupClosePushL(targets);
            iSingletons.Router().RouteOperationRequestL(params, targets);
            __ASSERT_DEBUG((targets.Count() == 1), User::Invariant());
            iSingletons.DpController().DataProviderL(targets[0]).ExecuteProxyRequestL(Request(), Connection(), *this);
            CleanupStack::PopAndDestroy(&targets);
            }
        }
    if (rsp != EMTPRespCodeOK)
        {
        SendResponseL(rsp);
        }
    return EFalse;
    }

#ifdef _DEBUG
void CMTPSendObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSendObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));

	CMTPTypeObjectPropList* targetObjectPropList = static_cast<CMTPTypeObjectPropList*>(&aData); 
	
	targetObjectPropList->AppendObjectPropListL(*iObjectPropList);

    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }
    
void CMTPSendObjectPropList::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }
    
#ifdef _DEBUG    
void CMTPSendObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSendObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);    
    }

void CMTPSendObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __ASSERT_DEBUG(((iRequest == &aRequest) && (&iConnection == &aConnection)), Panic(EMTPNotSameRequestProxy));
    iFramework.SendResponseL(iResponse, aRequest, aConnection);
    }
    
void CMTPSendObjectPropList::SendResponseL(TUint16 aCode)
    {
    const TMTPTypeRequest& req(Request());
    iResponse.SetUint16(TMTPTypeResponse::EResponseCode, aCode);
    iResponse.SetUint32(TMTPTypeResponse::EResponseSessionID, req.Uint32(TMTPTypeRequest::ERequestSessionID));
    iResponse.SetUint32(TMTPTypeResponse::EResponseTransactionID, req.Uint32(TMTPTypeRequest::ERequestTransactionID));
    iFramework.SendResponseL(iResponse, req, Connection());
    }

