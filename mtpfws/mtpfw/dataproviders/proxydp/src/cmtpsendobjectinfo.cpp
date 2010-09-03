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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpsendobjectinfoTraces.h"
#endif


/**
Verification data for the SendObjectInfo request
*/
const TMTPRequestElementInfo KMTPSendObjectInfoPolicy[] =
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrWrite, 1, 0, 0},
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 2, KMTPHandleAll, KMTPHandleNone}
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
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CMTPSENDOBJECTINFO_DES_ENTRY );

    delete iObjectInfo;
    iSingletons.Close();
    iProxyDpSingletons.Close();
    
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CMTPSENDOBJECTINFO_DES_EXIT );
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
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CONSTRUCTL_ENTRY );
    
    iSingletons.OpenL();

    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CONSTRUCTL_EXIT );
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
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_ENTRY );

    CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
    iSingletons.Router().ParseOperationRequestL(params);
    TBool fileFlag=EFalse;
    RArray<TUint> targets;
    CleanupClosePushL(targets);
    params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatCode, iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    iProxyDpSingletons.OpenL(iFramework);
    TInt index(KErrNotFound);
	const TUint16 formatCode=iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat);
	OstTrace1(TRACE_NORMAL, CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, "formatCode = %d", formatCode);
	switch(formatCode)
		{
	case EMTPFormatCodeAssociation:
        params.SetParam(CMTPParserRouter::TRoutingParameters::EParamFormatSubCode, iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType));
        break;

    case EMTPFormatCodeScript:
    	{
    	const TDesC& filename = iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EFilename);
    	HBufC* lowFileName = HBufC::NewLC(filename.Length());
    	TPtr16 prt(lowFileName->Des());
    	prt.Append(filename);
    	prt.LowerCase();   	
    	OstTraceExt1(TRACE_NORMAL, DUP1_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
    	        "lowFileName = %S", prt);
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
	
    OstTrace1( TRACE_NORMAL, DUP2_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, "fileFlag = %d", fileFlag);
    
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
        OstTrace1(TRACE_NORMAL, DUP3_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
                "KStorageId = %d", KStorageId);
        OstTrace1(TRACE_NORMAL, DUP4_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
                "targets.Count() = %d", targets.Count());
        if( KMTPNotSpecified32 == KStorageId)
            {
            iSingletons.DpController().DataProviderL(targets[0]).ExecuteProxyRequestL(Request(), Connection(), *this);
            }
        else if( storages.ValidStorageId(KStorageId) )
            {
        	if(targets.Count() == 1)
        		{
                OstTrace1(TRACE_NORMAL, DUP5_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
                        "targets[0] = %d", targets[0]);
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
                OstTrace1(TRACE_NORMAL, DUP6_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
                        "dpID = %d", dpID);
	            if( targets.Find( dpID ) == KErrNotFound )
	                {
	                OstTrace0( TRACE_NORMAL, DUP7_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
	                        "No target dp is found, so send one GeneralError response." );	                
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
            OstTrace0( TRACE_NORMAL, DUP8_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL,"StorageID is invalid." );
            SendResponseL( EMTPRespCodeInvalidStorageID );
            }
    	}	
    CleanupStack::PopAndDestroy(&targets);

    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_EXIT );
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
    

