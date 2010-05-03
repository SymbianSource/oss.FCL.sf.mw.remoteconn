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
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpobjectmgr.h"
#include "cmtpparserrouter.h"
#include "cmtpsetobjectproplist.h"
#include "mtpproxydppanic.h"
#include "rmtpframework.h"
#include "cmtpstoragemgr.h"

/**
Two-phase construction method
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPSetObjectPropList::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
    {
    CMTPSetObjectPropList* self = new (ELeave) CMTPSetObjectPropList(aFramework, aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPSetObjectPropList::~CMTPSetObjectPropList()
    {
    delete iObjectPropList;
    iDpList.Close();
    iSingletons.Close();
    }

/**
Standard c++ constructor
*/    
CMTPSetObjectPropList::CMTPSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL)
    {
    
    }
    
/**
Second phase constructor.
*/
void CMTPSetObjectPropList::ConstructL()
    {
    iSingletons.OpenL();
    }

/**
SetObjectPropList request handler
*/    
void CMTPSetObjectPropList::ServiceL()
    {
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    ReceiveDataL(*iObjectPropList);    
    }

/**
Override to handle the response phase of SetObjectPropList requests
@return EFalse
*/
TBool CMTPSetObjectPropList::DoHandleResponsePhaseL()
    {
    iDpIdx = 0;
    iPropertyIdx = 0;
    iDpListCreated = EFalse;
    iPropertyIdx = 0;
    iDpList.Reset();
    iObjectPropList->ResetCursor();             
    Reschedule(KErrNone);
    return EFalse;
    }

#ifdef _DEBUG
void CMTPSetObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSetObjectPropList::ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    __ASSERT_DEBUG(aData.Type() == EMTPTypeObjectPropListDataset, Panic(EMTPInvalidDataType));
    

	CMTPTypeObjectPropList* targetObjectPropList = static_cast<CMTPTypeObjectPropList*>(&aData); 
	targetObjectPropList->AppendObjectPropListL(*iObjectPropList);

    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }
    
void CMTPSetObjectPropList::ProxySendDataL(const MMTPType& /*aData*/, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& /*aStatus*/)
    {
    Panic(EMTPWrongRequestPhase);
    }
    
#ifdef _DEBUG    
void CMTPSetObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus)
#else
void CMTPSetObjectPropList::ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/, TRequestStatus& aStatus)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    iResponseCode = static_cast<TMTPResponseCode>(aResponse.Uint16(TMTPTypeResponse::EResponseCode));
    iPropertyIdx = aResponse.Uint32(TMTPTypeResponse::EResponseParameter1);
    TRequestStatus* status = &aStatus;
    User::RequestComplete(status, KErrNone);
    }

#ifdef _DEBUG    
void CMTPSetObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection)
#else
void CMTPSetObjectPropList::ProxyTransactionCompleteL(const TMTPTypeRequest& /*aRequest*/, MMTPConnection& /*aConnection*/)
#endif
    {
    __ASSERT_DEBUG(iRequest == &aRequest && &iConnection == &aConnection, Panic(EMTPNotSameRequestProxy));
    const TInt KErr(iResponseCode == EMTPRespCodeOK ? KErrNone : KErrGeneral);    
    if (KErr == KErrNone)
        {
        ++iDpIdx;
        }
    Reschedule(KErr);
    }

void CMTPSetObjectPropList::RunL()
    {    
    if (iStatus == KErrNone) // send request to the next dp
        {
        if (!iDpListCreated)
            {    
            // Data provider list under construction.
            iResponseCode = EMTPRespCodeOK;
            const TUint KElementCount(iObjectPropList->NumberOfElements());
            const TUint KRunLength(32);
		
            for (TUint i(0); ((i < KRunLength) && (iPropertyIdx < KElementCount) && (iResponseCode == EMTPRespCodeOK)); ++i)
                {
				iPropertyIdx++;
				CMTPTypeObjectPropListElement& element=iObjectPropList->GetNextElementL();			
                const TUint32 KHandle(element.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle));
                const TUint16 KPropCode(element.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode));
                CMTPParserRouter::TRoutingParameters params(*iRequest, iConnection);
                params.SetParam(CMTPParserRouter::TRoutingParameters::EParamObjectHandle, KHandle);
                params.SetParam(CMTPParserRouter::TRoutingParameters::EParamObjectPropCode, KPropCode);
                                
                CMTPObjectMetaData* meta(CMTPObjectMetaData::NewLC());
                if (!iSingletons.ObjectMgr().ObjectL(KHandle, *meta))
                    {
                    // Invalid object handle.
                    iResponseCode = EMTPRespCodeInvalidObjectHandle;
                    }
                else if(!iSingletons.StorageMgr().IsReadWriteStorage(meta->Uint(CMTPObjectMetaData::EStorageId)))
                	{
					iResponseCode = EMTPRespCodeAccessDenied;
                	}
                else
                    {
                    RArray<TUint> targets;
                    CleanupClosePushL(targets);
                    iSingletons.Router().RouteOperationRequestL(params, targets);
                    __ASSERT_DEBUG((targets.Count() <= 1), User::Invariant());
                    if (targets.Count() == 1)
                        {
                        iDpList.InsertInOrder(targets[0]);
                        }
                    else
                        {
                        // Unsupported object propertycode.
                        iResponseCode = EMTPRespCodeObjectPropNotSupported;
                        }
                    CleanupStack::PopAndDestroy(&targets);
                    }
                CleanupStack::PopAndDestroy(meta);
                
                if (iResponseCode != EMTPRespCodeOK)
                    {
                    // If an error occurs at this point then no properties have been updated.
                    iPropertyIdx = 0;
                    iDpList.Reset();
                    }
                }	                
            if ((iPropertyIdx < KElementCount) && (iResponseCode == EMTPRespCodeOK))
                {
                Reschedule(KErrNone);
                }
            else
                {
                iDpListCreated = ETrue;
                }
            }
            
        if (iDpListCreated)
            {
            if (iDpIdx < iDpList.Count())
                {
                iSingletons.DpController().DataProviderL(iDpList[iDpIdx]).ExecuteProxyRequestL(Request(), Connection(), *this);
                }
            else
                {
                SendResponseL(iResponseCode, 1, &iPropertyIdx);                
                }
            }
        }
    else
        {
        if (!iDpListCreated)
            {
            /* 
            If an error occurs prior to starting the proxy transaction cycles 
            then no properties have been updated.
            */
            iPropertyIdx = 0;
            }
            
        if (iResponseCode == EMTPRespCodeOK)
            {
            iResponseCode = EMTPRespCodeGeneralError;
            }
        SendResponseL(iResponseCode, 1, &iPropertyIdx);
        }    
    }

/**
Reschedules the active object with the specified completion code.
@param aError The asynchronous request completion code.
*/
void CMTPSetObjectPropList::Reschedule(TInt aError)
    {
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, aError);
    }
