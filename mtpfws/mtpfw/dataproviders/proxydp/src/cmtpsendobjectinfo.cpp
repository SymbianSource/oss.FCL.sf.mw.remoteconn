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

#include "rmtpframework.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpparserrouter.h"
#include "cmtpdataprovider.h"
#include "cmtpsendobjectinfo.h"
#include "mtpproxydppanic.h"
#include "cmtpproxydpconfigmgr.h"
#include "cmtpstoragemgr.h"

__FLOG_STMT( _LIT8( KComponent,"PrxySendObjectInfo" ); )

/**
Verification data for the SendObjectInfo request
*/
const TMTPRequestElementInfo KMTPSendObjectInfoPolicy[] =
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrWrite, 1, 0, 0},
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeObjectHandle, EMTPElementAttrDir | EMTPElementAttrWrite, 2, KMTPHandleAll, KMTPHandleNone}
    };

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/
MMTPRequestProcessor* CMTPSendObjectInfo::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPSendObjectInfo* self = new (ELeave) CMTPSendObjectInfo(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/
CMTPSendObjectInfo::~CMTPSendObjectInfo()
    {
    __FLOG(_L8("~CMTPSendObjectInfo - Entry"));
    
    delete iObjectInfo;
    iSingletons.Close();
    iProxyDpSingletons.Close();
    
    __FLOG(_L8("~CMTPSendObjectInfo - Exit"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/
CMTPSendObjectInfo::CMTPSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPSendObjectInfoPolicy)/sizeof(TMTPRequestElementInfo), KMTPSendObjectInfoPolicy)
    {

    }

/**
Second phase constructor.
*/
void CMTPSendObjectInfo::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    
    iSingletons.OpenL();
    
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
SendObjectInfo/SendObject request handler
NOTE: SendObjectInfo has to be comes before SendObject requests.  To maintain the state information
between the two requests, the two requests are combined together in one signal processor.
*/
void CMTPSendObjectInfo::ServiceL()
    {
    delete iObjectInfo;
    iObjectInfo = NULL;
    iObjectInfo = CMTPTypeObjectInfo::NewL();
    ReceiveDataL(*iObjectInfo);
    }

/**
Override to handle the response phase of SendObjectInfo requests
@return EFalse
*/
TBool CMTPSendObjectInfo::DoHandleResponsePhaseL()
    {
    DoHandleSendObjectInfoCompleteL();
    return EFalse;
    }

TBool CMTPSendObjectInfo::HasDataphase() const
    {
    return ETrue;
    }

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/
void CMTPSendObjectInfo::DoHandleSendObjectInfoCompleteL()
    {
    __FLOG(_L8("DoHandleSendObjectInfoCompleteL - Entry"));
    
    CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
    iSingletons.Router().ParseOperationRequestL(params);
    TBool fileFlag=EFalse;
    RArray<TUint> targets;
    CleanupClosePushL(targets);
    params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatCode, iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    iProxyDpSingletons.OpenL(iFramework);
    TInt index(KErrNotFound);
	const TUint16 formatCode=iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat);
	__FLOG_1( _L8("formatCode = %d"), formatCode );
	switch(formatCode)
		{
	case EMTPFormatCodeAssociation:
        params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType));
        break;

    case EMTPFormatCodeScript:
    	{
    	__FLOG_1( _L8("formatCode = %d"), EMTPFormatCodeScript );
    	const TDesC& filename = iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EFilename);
    	HBufC* lowFileName = HBufC::NewLC(filename.Length());
    	TPtr16 prt(lowFileName->Des());
    	prt.Append(filename);
    	prt.LowerCase();   	
    	__FLOG_1( _L8("lowFileName = %s"), &prt );
    	if (iProxyDpSingletons.FrameworkConfig().GetFileName(prt,index) )
    		{
    		fileFlag=ETrue;
    		}
    	CleanupStack::PopAndDestroy(lowFileName);
    	
    	params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, EMTPAssociationTypeUndefined);	
        break;
    	}
    default:
    	params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, EMTPAssociationTypeUndefined);
        break;
		}
	
    __FLOG_1( _L8("fileFlag = %d"), fileFlag );
    if(fileFlag)
    	{
    	TInt  syncdpid =  iSingletons.DpController().DpId(iProxyDpSingletons.FrameworkConfig().GetDPId(index));
       	iSingletons.DpController().DataProviderL(syncdpid).ExecuteProxyRequestL(Request(), Connection(), *this);
    	}
    else
    	{
    	iSingletons.Router().RouteOperationRequestL(params, targets);
        CMTPStorageMgr& storages(iSingletons.StorageMgr());
    	const TUint KStorageId = Request().Uint32(TMTPTypeResponse::EResponseParameter1);
        __FLOG_1( _L8("KStorageId = %d"), KStorageId );
        __FLOG_1( _L8("targets.Count() = %d"), targets.Count() );
        if( KMTPNotSpecified32 == KStorageId)
            {
            iSingletons.DpController().DataProviderL(targets[0]).ExecuteProxyRequestL(Request(), Connection(), *this);
            }
        else if( storages.ValidStorageId(KStorageId) )
            {
        	if(targets.Count() == 1)
        		{
        		__FLOG_1( _L8("targets[0] = %d"), targets[0] );
        		iSingletons.DpController().DataProviderL(targets[0]).ExecuteProxyRequestL(Request(), Connection(), *this);
        		}
        	else
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
	            __FLOG_1( _L8("dpID = %d"), dpID );
	            if( targets.Find( dpID ) == KErrNotFound )
	                {
	                __FLOG(_L8("No target dp is found, so send one GeneralError response."));
	                SendResponseL( EMTPRespCodeGeneralError );
	                }
	            else
	                {
	                iSingletons.DpController().DataProviderL(dpID).ExecuteProxyRequestL(Request(), Connection(), *this);
	                }
        		}
            }
        else
            {
            __FLOG(_L8("StorageID is invalid."));
            SendResponseL( EMTPRespCodeInvalidStorageID );
            }
    	}	
    CleanupStack::PopAndDestroy(&targets);
    
    __FLOG(_L8("DoHandleSendObjectInfoCompleteL - Exit"));
    }

#ifdef _DEBUG
void CMTPSendObjectInfo::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSendObjectInfo::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(*iObjectInfo, aData);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }

void CMTPSendObjectInfo::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }

#ifdef _DEBUG
void CMTPSendObjectInfo::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSendObjectInfo::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    MMTPType::CopyL(aResponse, iResponse);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }

void CMTPSendObjectInfo::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    iFramework.SendResponseL(iResponse, aRequest, aConnection);
    }
    

