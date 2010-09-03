// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalTechnology
*/

#include <f32file.h>
#include <bautils.h>
#include <e32const.h>

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypefile.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mmtpconnection.h>
#include <mtp/tmtptyperequest.h>

#include "mtpdebug.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "cmtpimagedpsendobjectinfo.h"
#include "mtpimagedppanic.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedpthumbnailcreator.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpsendobjectinfoTraces.h"
#endif


const TInt RollbackFuncCnt = 3;

const TMTPRequestElementInfo KMTPSendObjectPropListPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrWrite, 1, 0, 0}, 
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 2, KMTPHandleAll, KMTPHandleNone}
    };
/**
Two-phase construction method
@param aFramework  The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/ 
MMTPRequestProcessor* CMTPImageDpSendObjectInfo::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpSendObjectInfo* self = new (ELeave) CMTPImageDpSendObjectInfo(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPImageDpSendObjectInfo::~CMTPImageDpSendObjectInfo()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CMTPIMAGEDPSENDOBJECTINFO_DES_ENTRY );
        
    Rollback();
    iRollbackList.Close();

    delete iDateMod;
    delete iDateCreated;
    delete iFileReceived;
    delete iParentSuid;    
    delete iReceivedObject;
    delete iObjectInfo;
    delete iObjectPropList;

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CMTPIMAGEDPSENDOBJECTINFO_DES_EXIT );
    }

/**
Standard c++ constructor
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
*/    
CMTPImageDpSendObjectInfo::CMTPImageDpSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iDataProvider(aDataProvider),
    iHiddenStatus( EMTPVisible ),
    iObjectPropertyMgr(aDataProvider.PropertyMgr())
    {

    }

#define ADD_FSM_ENTRY(currentstate, event, nextstate, failedstate, action)   \
    iStateMachine[currentstate][event].iNextSuccessState = nextstate;        \
    iStateMachine[currentstate][event].iNextFailedState  = failedstate;      \
    iStateMachine[currentstate][event].iFsmAction        = action

/**
Second-phase construction
*/        
void CMTPImageDpSendObjectInfo::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CONSTRUCTL_ENTRY );
         
    iRollbackList.ReserveL(RollbackFuncCnt);
    iExpectedSendObjectRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, EMTPOpCodeSendObject);
    iReceivedObject = CMTPObjectMetaData::NewL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
    
    // build FSM matrix    
    // process sendobjectinfo/sendobjectproplist operation
    ADD_FSM_ENTRY(EStateIdle, EObjectInfoEvent,     EObjectInfoCheck, EStateIdle, FsmCheckObjectInfoParamsL);
    ADD_FSM_ENTRY(EStateIdle, EObjectPropListEvent, EObjectInfoCheck, EStateIdle, FsmCheckObjectPropListParamsL);
    ADD_FSM_ENTRY(EStateIdle, EObjectEvent,         EStateIdle,       EStateIdle, FsmCheckObjectParams);
    
    ADD_FSM_ENTRY(EObjectInfoCheck, EObjectInfoEvent,     EObjectInfoServ, EStateIdle, FsmServiceSendObjectInfoL);
    ADD_FSM_ENTRY(EObjectInfoCheck, EObjectPropListEvent, EObjectInfoServ, EStateIdle, FsmServiceSendObjectPropListL);
    ADD_FSM_ENTRY(EObjectInfoCheck, EObjectEvent,         EStateEnd,       EStateEnd,  NULL);
    
    ADD_FSM_ENTRY(EObjectInfoServ, EObjectInfoEvent,     EObjectInfoSucceed, EStateIdle, FsmDoHandleSendObjectInfoCompleteL);
    ADD_FSM_ENTRY(EObjectInfoServ, EObjectPropListEvent, EObjectInfoSucceed, EStateIdle, FsmDoHandleSendObjectPropListCompleteL);
    ADD_FSM_ENTRY(EObjectInfoServ, EObjectEvent,         EStateEnd,          EStateEnd,  NULL);
    
    // process sendobject operation
    ADD_FSM_ENTRY(EObjectInfoSucceed, EObjectInfoEvent,     EObjectInfoCheck, EStateIdle,          FsmCheckObjectInfoParamsL);
    ADD_FSM_ENTRY(EObjectInfoSucceed, EObjectPropListEvent, EObjectInfoCheck, EStateIdle,          FsmCheckObjectPropListParamsL);
    ADD_FSM_ENTRY(EObjectInfoSucceed, EObjectEvent,         EObjectCheck,     EObjectInfoSucceed,  FsmCheckObjectParams);
    
    ADD_FSM_ENTRY(EObjectCheck, EObjectInfoEvent,     EStateEnd,   EStateEnd,  NULL);
    ADD_FSM_ENTRY(EObjectCheck, EObjectPropListEvent, EStateEnd,   EStateEnd,  NULL);
    ADD_FSM_ENTRY(EObjectCheck, EObjectEvent,         EObjectServ, EObjectInfoSucceed, FsmServiceSendObjectL);
    
    ADD_FSM_ENTRY(EObjectServ, EObjectInfoEvent,     EStateEnd,   EStateEnd,  NULL);
    ADD_FSM_ENTRY(EObjectServ, EObjectPropListEvent, EStateEnd,   EStateEnd,  NULL);
    ADD_FSM_ENTRY(EObjectServ, EObjectEvent,         EStateIdle,  EObjectInfoSucceed, FsmDoHandleSendObjectCompleteL);

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CONSTRUCTL_EXIT );
    }

TBool CMTPImageDpSendObjectInfo::FsmCheckObjectInfoParamsL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->CheckObjectInfoParamsL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmCheckObjectPropListParamsL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->CheckObjectPropListParamsL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmCheckObjectParams(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->CheckObjectParams(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmServiceSendObjectInfoL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->ServiceSendObjectInfoL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmServiceSendObjectPropListL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->ServiceSendObjectPropListL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmServiceSendObjectL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->ServiceSendObjectL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmDoHandleSendObjectInfoCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->DoHandleSendObjectInfoCompleteL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmDoHandleSendObjectPropListCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->DoHandleSendObjectPropListCompleteL(aPtr);
    }

TBool CMTPImageDpSendObjectInfo::FsmDoHandleSendObjectCompleteL(CMTPImageDpSendObjectInfo* aObject, TAny *aPtr)
    {
    return aObject->DoHandleSendObjectCompleteL(aPtr);
    }

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/    
TMTPResponseCode CMTPImageDpSendObjectInfo::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CHECKREQUESTL_ENTRY );
    
    iOperationCode = Request().Uint16(TMTPTypeRequest::ERequestOperationCode);
    
    switch (iOperationCode)
        {      
    case EMTPOpCodeSendObjectInfo:
        iEvent = EObjectInfoEvent;
        iElementCount = sizeof(KMTPSendObjectPropListPolicy) / sizeof(TMTPRequestElementInfo); //for the checker
        iElements = KMTPSendObjectPropListPolicy;            
        break;
        
    case EMTPOpCodeSendObjectPropList:
        iEvent = EObjectPropListEvent;
        iElementCount = sizeof(KMTPSendObjectPropListPolicy) / sizeof(TMTPRequestElementInfo); //for the checker
        iElements = KMTPSendObjectPropListPolicy;
        break;
        
    case EMTPOpCodeSendObject:
    	  //In ParseRouter everytime SendObject gets resolved then will be removed from Registry
    	  //Right away therefore we need reRegister it here again in case possible cancelRequest
    	  //Against this SendObject being raised.
    	  iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
    	  iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
        iEvent = EObjectEvent;
        iElementCount = 0; //for the checker
        iElements = NULL;
        break;
        
    default:
        // Nothing to do
        break;
        }
    
    FsmAction pCheck = iStateMachine[iCurrentState][iEvent].iFsmAction;
    __ASSERT_ALWAYS((pCheck != NULL), Panic(EMTPImageDpNoMatchingProcessor));

    //coverity[var_deref_model]
    TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
    if(EMTPRespCodeOK == result)
        {
        TBool ret = EFalse;
        TRAPD(err, ret = (*pCheck)(this, &result));
        if (ret)
            {
            iCurrentState = iStateMachine[iCurrentState][iEvent].iNextSuccessState;
            }
        else
            {
            iCurrentState = iStateMachine[iCurrentState][iEvent].iNextFailedState;
            }
        LEAVEIFERROR(err,
                OstTrace1( TRACE_ERROR, DUP1_CMTPIMAGEDPSENDOBJECTINFO_CHECKREQUESTL,
                        "the request check failed! error code %d", err ));
        }

    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_CHECKREQUESTL, "Result: 0x%04x", result );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CHECKREQUESTL_EXIT );
    return result;    
    }
    
TBool CMTPImageDpSendObjectInfo::HasDataphase() const
    {
    return ETrue;
    }

TBool CMTPImageDpSendObjectInfo::CheckObjectInfoParamsL(TAny *aPtr)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTINFOPARAMSL_ENTRY );
    
    TMTPResponseCode* ret = static_cast<TMTPResponseCode*>(aPtr);
    *ret = EMTPRespCodeOK;
    
    const TUint32 storeId(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    const TUint32 parentHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    
    // this checking is only valid when the second parameter is not a special value.
    if (parentHandle != KMTPHandleAll && parentHandle != KMTPHandleNone)
        {
        //does not take owernship
        CMTPObjectMetaData* parentObjInfo = CMTPObjectMetaData::NewLC();        
        if (iFramework.ObjectMgr().ObjectL(parentHandle, *parentObjInfo))
            {
            TUint32 storageId = parentObjInfo->Uint(CMTPObjectMetaData::EStorageId);   
            if (storeId != storageId)        
                {
                *ret = EMTPRespCodeInvalidObjectHandle;
                }
            }
        else
            {
            *ret = EMTPRespCodeInvalidObjectHandle;            
            }        
        CleanupStack::PopAndDestroy(parentObjInfo);
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTINFOPARAMSL_EXIT );
    return (*ret == EMTPRespCodeOK) ? ETrue : EFalse;
    }

TBool CMTPImageDpSendObjectInfo::CheckObjectPropListParamsL(TAny *aPtr)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTPROPLISTPARAMSL_ENTRY );
    TMTPResponseCode* ret = static_cast<TMTPResponseCode*>(aPtr);
    *ret = EMTPRespCodeOK;
    
    TMTPFormatCode formatCode = static_cast<TMTPFormatCode>(Request().Uint32(TMTPTypeRequest::ERequestParameter3));
    if (!IsFormatValid(formatCode))
        {
        *ret = EMTPRespCodeInvalidObjectFormatCode;
        }
    else
        {
        iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        TUint32 objectSizeHigh = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
        TUint32 objectSizeLow = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
        iObjectSize = MAKE_TUINT64(objectSizeHigh, objectSizeLow);
        
        if (iStorageId == KMTPStorageDefault)
            {
            iStorageId = iFramework.StorageMgr().DefaultStorageId();
            }
        
        if(IsTooLarge(iObjectSize))
            {
            *ret = EMTPRespCodeObjectTooLarge;
            }
        }

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTPROPLISTPARAMSL_EXIT );
    return (*ret == EMTPRespCodeOK) ? ETrue : EFalse;
    }

TBool CMTPImageDpSendObjectInfo::CheckObjectParams(TAny *aPtr)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTPARAMS_ENTRY );
    TMTPResponseCode* ret = static_cast<TMTPResponseCode*>(aPtr);
    *ret = EMTPRespCodeOK;
    
    /**
    * If the previous request is not the SendObjectInfo/SendObjectPropList/UpdateObjectPropList operation,
    * the SendObject operation should failed.
    */
    if ( (iPreviousTransactionID + 1) != Request().Uint32(TMTPTypeRequest::ERequestTransactionID))
        {
        *ret = EMTPRespCodeNoValidObjectInfo;
        }

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CHECKOBJECTPARAMS_EXIT );
    return (*ret == EMTPRespCodeOK) ? ETrue : EFalse;    
    }

/**
SendObjectInfo/SendObject request handler
NOTE: SendObjectInfo has to be comes before SendObject requests.  To maintain the state information
between the two requests, the two requests are combined together in one request processor.
*/    
void CMTPImageDpSendObjectInfo::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_SERVICEL_ENTRY );

    FsmAction pService = iStateMachine[iCurrentState][iEvent].iFsmAction;
    __ASSERT_DEBUG(pService, Panic(EMTPImageDpNoMatchingProcessor));
    
    TBool ret = EFalse;
    TRAPD(err, ret = (*pService)(this, NULL));
    if (ret)
        {
        iCurrentState = iStateMachine[iCurrentState][iEvent].iNextSuccessState;
        }
    else
        {
        iCurrentState = iStateMachine[iCurrentState][iEvent].iNextFailedState;
        }    
    
    if (err != KErrNone)
        {
        Rollback();
        }
    LEAVEIFERROR(err,
            OstTrace1( TRACE_ERROR, CMTPIMAGEDPSENDOBJECTINFO_SERVICEL, "Action failed! error code %d", err));

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_SERVICEL_EXIT );
    }

/**
Override to match both the SendObjectInfo and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPImageDpSendObjectInfo::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_MATCH_ENTRY );
    TBool result = EFalse;
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
    if ((operationCode == EMTPOpCodeSendObjectInfo || 
        operationCode == EMTPOpCodeSendObject ||
        operationCode == EMTPOpCodeSendObjectPropList) &&
        &iConnection == &aConnection)
        {
        result = ETrue;
        }
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_MATCH_EXIT );
    return result;    
    }

/**
Override to handle the response phase of SendObjectInfo and SendObject requests
@return EFalse
*/
TBool CMTPImageDpSendObjectInfo::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLERESPONSEPHASEL_ENTRY );

    //to check if the sending/receiving data is successful
    iSuccessful = !iCancelled;
 
    FsmAction pResponse = iStateMachine[iCurrentState][iEvent].iFsmAction;
    __ASSERT_DEBUG(pResponse, Panic(EMTPImageDpNoMatchingProcessor));
    
    TBool ret = EFalse;
    TRAPD(err, ret = (*pResponse)(this, &iSuccessful))
    if (ret)
        {
        iCurrentState = iStateMachine[iCurrentState][iEvent].iNextSuccessState;
        }
    else
        {
        iCurrentState = iStateMachine[iCurrentState][iEvent].iNextFailedState;
        }          
    
    if (err != KErrNone)
        {
        Rollback();
        }
    LEAVEIFERROR(err,
            OstTrace1( TRACE_ERROR, CMTPIMAGEDPSENDOBJECTINFO_DOHANDLERESPONSEPHASEL, "Response error! error code %d", err));

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;
    }

/**
Override to handle the completing phase of SendObjectInfo and SendObject requests
@return ETrue if succesfully received the file, otherwise EFalse
*/    
TBool CMTPImageDpSendObjectInfo::DoHandleCompletingPhaseL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLECOMPLETINGPHASEL_ENTRY );
    TBool result = ETrue;
    CMTPRequestProcessor::DoHandleCompletingPhaseL();

    if (iSuccessful)
        {
        if (iOperationCode == EMTPOpCodeSendObjectInfo || iOperationCode == EMTPOpCodeSendObjectPropList)
            {
            iPreviousTransactionID = Request().Uint32(TMTPTypeRequest::ERequestTransactionID);
            result = EFalse;
            }
        }
    else
        {
        if (iOperationCode == EMTPOpCodeSendObject)
            {
            iPreviousTransactionID++;
            }
        result = EFalse;
        }

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLECOMPLETINGPHASEL_EXIT );
    return result;    
    }

/**
SendObjectInfo request handler
*/
TBool CMTPImageDpSendObjectInfo::ServiceSendObjectInfoL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTINFOL_ENTRY );
    
    delete iObjectInfo;
    iObjectInfo = NULL;
    iObjectInfo = CMTPTypeObjectInfo::NewL();
    ReceiveDataL(*iObjectInfo);

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTINFOL_EXIT );
    return ETrue;
    }

/**
SendObjectPropList request handler
*/
TBool CMTPImageDpSendObjectInfo::ServiceSendObjectPropListL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTPROPLISTL_ENTRY );
    
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode, iRequest->Uint32(TMTPTypeRequest::ERequestParameter3));
    ReceiveDataL(*iObjectPropList);

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTPROPLISTL_EXIT );
    return ETrue;
    }
    
/**
SendObject request handler
*/    
TBool CMTPImageDpSendObjectInfo::ServiceSendObjectL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTL_ENTRY );
         
    iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObject);
    //prepare for rollback
    iRollbackList.AppendL(RemoveObjectFromDb);        
    
    ReceiveDataL(*iFileReceived);

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_SERVICESENDOBJECTL_EXIT );
    return ETrue;
    }

/**
Get a default parent object, if the request does not specify a parent object.
*/
void CMTPImageDpSendObjectInfo::GetDefaultParentObjectL()
    {    
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL_ENTRY );

    if (iStorageId == KMTPStorageDefault)
        {
        iStorageId = iFramework.StorageMgr().DefaultStorageId();
        }
    TInt drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    LEAVEIFERROR(drive,
            OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL, 
                    "Can't get drive number for storage %d! error code %d", iStorageId, munged_err ));

    delete iParentSuid;
    iParentSuid = NULL;
    iParentSuid = (iFramework.StorageMgr().StorageL(iStorageId).DesC(CMTPStorageMetaData::EStorageSuid)).AllocL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EParentHandle, KMTPHandleNoParent);
              
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL_EXIT );
    }

/**
Get parent object and storage id
@return EMTPRespCodeOK if successful, otherwise, EMTPRespCodeInvalidParentObject
*/
TMTPResponseCode CMTPImageDpSendObjectInfo::GetParentObjectAndStorageIdL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL_ENTRY );  
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPImageDpRequestCheckNull));

    iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    iParentHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    //does not take ownership
    CMTPObjectMetaData* parentObjectInfo = iRequestChecker->GetObjectInfo(iParentHandle);

    if (!parentObjectInfo)
        {
        GetDefaultParentObjectL();    
        }
    else
        {        
        delete iParentSuid;
        iParentSuid = NULL;
        iParentSuid = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();
        iReceivedObject->SetUint(CMTPObjectMetaData::EParentHandle, iParentHandle);
        }

    OstTraceExt1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL, 
            "ParentSuid = %S", *iParentSuid );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL_EXIT );
    return EMTPRespCodeOK;
    }

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPImageDpSendObjectInfo::DoHandleSendObjectInfoCompleteL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_ENTRY );  
  
    TBool result(ETrue);
    TUint16 format(iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    result = IsFormatValid(TMTPFormatCode(format));
    
    if (result)
        {
        delete iDateMod;
        iDateMod = NULL;
        iDateMod = iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EDateModified).AllocL();
        delete iDateCreated;
        iDateCreated = NULL;
        iDateCreated = iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EDateCreated).AllocL();

        TMTPResponseCode responseCode(GetParentObjectAndStorageIdL());
        if (responseCode != EMTPRespCodeOK)
            {
            SendResponseL(responseCode);
            result = EFalse;
            }
        }
    else
        {
        SendResponseL(EMTPRespCodeInvalidObjectFormatCode);
        }
        
    if (result)
        {
        iObjectSize = iObjectInfo->Uint32L(CMTPTypeObjectInfo::EObjectCompressedSize);
        if(IsTooLarge(iObjectSize))
            {
            SendResponseL(EMTPRespCodeObjectTooLarge);
            result = EFalse;
            }
        }

    if (result)
        {
        iProtectionStatus = iObjectInfo->Uint16L(CMTPTypeObjectInfo::EProtectionStatus);
        if (iProtectionStatus !=  EMTPProtectionNoProtection &&
            iProtectionStatus != EMTPProtectionReadOnly)
            {
            SendResponseL(EMTPRespCodeParameterNotSupported);
            result = EFalse;
            }
        }

    if (result)
        {
        result = GetFullPathName(iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EFilename));
        if (!result)
            {        
            // File and/or parent pathname invalid.
            SendResponseL(EMTPRespCodeInvalidDataset);
            }
        }

    if (result)
        {    
        result = !Exists(iFullPath);
        if (!result)
            {
            SendResponseL(EMTPRespCodeAccessDenied);
            }
        else
            {            
            TRAPD(err,CreateFsObjectL());
            if (err != KErrNone)
                {
                OstTrace1( TRACE_ERROR, CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
                        "Fail to create fs object %d", err );
                SendResponseL(ErrorToMTPError(err));
                Rollback();
                result = EFalse;
                }
            else
                {
                ReserveObjectL();            
                imageWidth = iObjectInfo->Uint32L(CMTPTypeObjectInfo::EImagePixWidth);
                imageHeight = iObjectInfo->Uint32L(CMTPTypeObjectInfo::EImagePixHeight);
                imageBitDepth = iObjectInfo->Uint32L(CMTPTypeObjectInfo::EImageBitDepth);            
                iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode, format);
                SetPropertiesL();                            
                ReturnResponseL();
                }
            }
        }
    
    iSuccessful = result;    
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_EXIT );
    return result;    
    }

/**
Handling the completing phase of SendObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPImageDpSendObjectInfo::DoHandleSendObjectPropListCompleteL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL_ENTRY );
    TBool result(ETrue);
    
    TMTPResponseCode responseCode(GetParentObjectAndStorageIdL());
    if (responseCode != EMTPRespCodeOK)
        {
        SendResponseL(responseCode);
        result = EFalse;
        }    

    if (result)
        {
        // Any kind of association is treated as a folder
        const TUint32 formatCode(Request().Uint32(TMTPTypeRequest::ERequestParameter3));

        TInt invalidParameterIndex = KErrNotFound;
        responseCode = VerifyObjectPropListL(invalidParameterIndex);
        result = (responseCode == EMTPRespCodeOK);    
        if (!result)
            {
            TUint32 parameters[4];
            parameters[0] = 0;
            parameters[1] = 0;
            parameters[2] = 0;
            parameters[3] = invalidParameterIndex;
            SendResponseL(responseCode, 4, parameters);
            }
        }
        
    if (result) 
        {
        result = !Exists(iFullPath);
        if (!result)
            {
            // Object with the same name already exists.
            SendResponseL(EMTPRespCodeAccessDenied);
            }
        }    
    
    if (result)
        {
        TRAPD(err,CreateFsObjectL());
        if (err != KErrNone)
            {
            OstTrace1( TRACE_ERROR, CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL, 
                    "Fail to create fs object %d", err );
            SendResponseL(ErrorToMTPError(err));
            Rollback();
            result = EFalse;
            }
        else
            {
            //the EFormatCode property has been set in ServiceSendObjectPropListL() function
            ReserveObjectL();
            SetPropertiesL();
            ReturnResponseL();
            }
        }
        
    iSuccessful = result;
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL_EXIT );
    return result;    
    }
    
/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/    
TBool CMTPImageDpSendObjectInfo::DoHandleSendObjectCompleteL(TAny* /*aPtr*/)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL_ENTRY );  
    TBool result(ETrue);
    
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    TInt64 objectsize = 0;
#else
    TInt objectsize = 0;
#endif
    
    iFileReceived->File().Size(objectsize);    
    
    if (objectsize != iObjectSize)
        {
        OstTraceExt2( TRACE_NORMAL, DUP1_CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL, 
                "object sizes differ %Lu != %Lu", objectsize, iObjectSize );
        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);         
        Rollback();
        
        TMTPResponseCode responseCode = EMTPRespCodeObjectTooLarge;
        if (objectsize < iObjectSize)
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        SendResponseL(responseCode);
        result = EFalse;
        }
        
    // SendObject is cancelled or connection is dropped.
    if(result && iCancelled)
        {
        OstTrace0( TRACE_NORMAL, DUP2_CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL, "It is a cancel for sendObject." );
        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        Rollback();
        SendResponseL(EMTPRespCodeTransactionCancelled);    
        }
    else if (result && !iCancelled)
	    {
        TUint attValue = 0;
        User::LeaveIfError(iFileReceived->File().Att(attValue));
        if (iProtectionStatus ==  EMTPProtectionNoProtection ||
            iProtectionStatus == EMTPProtectionReadOnly)
            {
            attValue &= ~(KEntryAttNormal | KEntryAttReadOnly);
            
            if (iProtectionStatus == EMTPProtectionNoProtection)
                {                        
                attValue |= KEntryAttNormal;
                }
            else
                {
                attValue |= KEntryAttReadOnly;
                }
            User::LeaveIfError(iFileReceived->File().SetAtt(attValue, ~attValue));
            }
        if ( iHiddenStatus == EMTPHidden )
            {
            attValue &= ~KEntryAttHidden;
            attValue |= KEntryAttHidden;
            User::LeaveIfError(iFileReceived->File().SetAtt(attValue, ~attValue));
            }
        TTime modifiedTime;
        //update datemodified property.
        if(iDateMod != NULL && iDateMod->Length())
           {           
           iObjectPropertyMgr.ConvertMTPTimeStr2TTimeL(*iDateMod, modifiedTime);
           }
        else if(iDateCreated != NULL && iDateCreated->Length())
           {
           iObjectPropertyMgr.ConvertMTPTimeStr2TTimeL(*iDateCreated, modifiedTime);
           }
        User::LeaveIfError(iFileReceived->File().SetModified(modifiedTime));

        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        
        //The MTP spec states that it is not mandatory for SendObjectInfo/SendObjectPropList
        //to be followed by a SendObject.  An object is reserved in the ObjectStore on 
        //receiving a SendObjectInfo/SendObjectPropList request, but we only commit it 
        //on receiving the corresponding SendObject request.  With Associations however 
        //we commit the object straight away as the SendObject phase is often absent 
        //with folder creation.
		
        CleanUndoList();
        SendResponseL(EMTPRespCodeOK);
	    }
    
    delete iFileReceived;
    iFileReceived = NULL;  
    
    iSuccessful = result;
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL_EXIT );
    return result;
    }

void CMTPImageDpSendObjectInfo::UnreserveObject(CMTPImageDpSendObjectInfo* aObject)
    {
    aObject->UnreserveObject();
    }

void CMTPImageDpSendObjectInfo::RemoveObjectFromFs(CMTPImageDpSendObjectInfo* aObject)
    {
    aObject->RemoveObjectFromFs();
    }

void CMTPImageDpSendObjectInfo::RemoveObjectFromDb(CMTPImageDpSendObjectInfo* aObject)
    {
    aObject->RemoveObjectFromDb();
    }

void CMTPImageDpSendObjectInfo::UnreserveObject()
    {    
    __ASSERT_DEBUG(iReceivedObject, Panic(EMTPImageDpObjectNull));
    TRAP_IGNORE(iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObject));
    }

void CMTPImageDpSendObjectInfo::RemoveObjectFromFs()
    {  
    OstTraceFunctionEntry0( DUP1_CMTPIMAGEDPSENDOBJECTINFO_REMOVEOBJECTFROMFS_ENTRY );
    delete iFileReceived;
    iFileReceived = NULL;
    TInt err = iFramework.Fs().Delete(iFullPath);
    if (err != KErrNone)
        {
        //add Suid to deleteobjectlist
        TRAP_IGNORE(iDataProvider.AppendDeleteObjectsArrayL(iFullPath));
        }
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPSENDOBJECTINFO_REMOVEOBJECTFROMFS_EXIT );
    }

void CMTPImageDpSendObjectInfo::RemoveObjectFromDb()
    {    
    /**
     * remove all cached properties if rollback occured.
     */
    TRAP_IGNORE(
            iFramework.ObjectMgr().RemoveObjectL(iReceivedObject->Uint(CMTPObjectMetaData::EHandle));
            iObjectPropertyMgr.ClearCache(iReceivedObject->Uint(CMTPObjectMetaData::EHandle));            
            );
    }

void CMTPImageDpSendObjectInfo::ReturnResponseL()
    {
    iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
    iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
    
    TUint32 parameters[3];
    parameters[0] = iStorageId;
    parameters[1] = iParentHandle;
    parameters[2] = iReceivedObject->Uint(CMTPObjectMetaData::EHandle);
    SendResponseL(EMTPRespCodeOK, (sizeof(parameters) / sizeof(parameters[0])), parameters);       
    }

/**
*/
TBool CMTPImageDpSendObjectInfo::IsFormatValid(TMTPFormatCode aFormat) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_ISFORMATVALID_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_ISFORMATVALID, "Format: 0x%04x", aFormat );
    TInt count(sizeof(KMTPValidCodeExtensionMappings) / sizeof(KMTPValidCodeExtensionMappings[0]));        
    for(TInt i=0; i < count; i++)
        {
        if (KMTPValidCodeExtensionMappings[i].iFormatCode == aFormat)
            {
            OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_ISFORMATVALID_EXIT );
            return ETrue;
            }
        }
    OstTraceFunctionExit0( DUP1_CMTPIMAGEDPSENDOBJECTINFO_ISFORMATVALID_EXIT );
    return EFalse;
    }

/**
Get the full path name of the object to be saved
@param aFileName, on entry, contains the file name of the object,
on return, contains the full path name of the object to be saved
@return ETrue if the name is valid, EFalse otherwise
*/
TBool CMTPImageDpSendObjectInfo::GetFullPathName(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_GETFULLPATHNAME_ENTRY );
    OstTraceExt1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_GETFULLPATHNAME, "FileName: %S", aFileName );
    TBool result(EFalse);
    if (aFileName.Length() > 0)
        {
        iFullPath = *iParentSuid;
        if (iFullPath.Length() + aFileName.Length() < iFullPath.MaxLength())
            {
            iFullPath.Append(aFileName);
            result = iFramework.Fs().IsValidName(iFullPath);
            }
        OstTraceExt1( TRACE_NORMAL, DUP1_CMTPIMAGEDPSENDOBJECTINFO_GETFULLPATHNAME, "FullPath: %S", iFullPath );
        }

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_GETFULLPATHNAME_EXIT );
    return result;
    }

/**
Check if the file already exists on the storage.
@return ETrue if file is exists, otherwise EFalse
*/
TBool CMTPImageDpSendObjectInfo::Exists(const TDesC& aName) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_EXISTS_ENTRY );
    // This detects both files and folders
    TBool ret(EFalse); 
    ret = BaflUtils::FileExists(iFramework.Fs(), aName);
    OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_EXISTS, "Exists: %S (%d)", aName, ret);
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_EXISTS_EXIT );
    return ret;
    }

/**
Check if the property list is valid and extract properties (file name)
@param aInvalidParameterIndex if invalid, contains the index of the property.  Undefined, if it is valid.
@return if error, one of the error response code; otherwise EMTPRespCodeOK
*/
TMTPResponseCode CMTPImageDpSendObjectInfo::VerifyObjectPropListL(TInt& aInvalidParameterIndex)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL_ENTRY );
    
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    const TUint KCount(iObjectPropList->NumberOfElements());
    iObjectPropList->ResetCursor();
    for (TUint i(0); (i < KCount); i++)
        {
        CMTPTypeObjectPropListElement& KElement=iObjectPropList->GetNextElementL();
        const TUint32 KHandle(KElement.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle));
        aInvalidParameterIndex = i;
        if (KHandle != KMTPHandleNone)
            {
            responseCode = EMTPRespCodeInvalidObjectHandle;            
            break;
            }
            
        responseCode = CheckPropCodeL(KElement);
        if (responseCode != EMTPRespCodeOK)
            {
            break;
            }
        responseCode = ExtractPropertyL(KElement);
        if (responseCode != EMTPRespCodeOK)
            {
            break;
            }        
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL, "Result = 0x%04X", responseCode );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL_EXIT );
    return responseCode;        
    }

/**
Extracts the file information from the object property list element
@param aElement an object property list element
@param aPropertyCode MTP property code for the element
@return MTP response code
*/
TMTPResponseCode CMTPImageDpSendObjectInfo::ExtractPropertyL(const CMTPTypeObjectPropListElement& aElement)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_EXTRACTPROPERTYL_ENTRY );
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    switch (aElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode))
        {       
    case EMTPObjectPropCodeObjectFileName:
        {
        const TDesC& KFileName = aElement.StringL(CMTPTypeObjectPropListElement::EValue);
        if (!GetFullPathName(KFileName))
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        }
        break;

    case EMTPObjectPropCodeProtectionStatus:
        {
        iProtectionStatus = aElement.Uint16L(CMTPTypeObjectPropListElement::EValue);
        if (iProtectionStatus !=  EMTPProtectionNoProtection &&
            iProtectionStatus != EMTPProtectionReadOnly)
            {
            responseCode = EMTPRespCodeParameterNotSupported;
            }
        }
        break;

    case EMTPObjectPropCodeDateModified:
        delete iDateMod;
        iDateMod = NULL;
        iDateMod = aElement.StringL(CMTPTypeObjectPropListElement::EValue).AllocL();
        break;
        
    case EMTPObjectPropCodeDateCreated:
        delete iDateCreated;
        iDateCreated = NULL;
        iDateCreated = aElement.StringL(CMTPTypeObjectPropListElement::EValue).AllocL();
        break;
        
	case EMTPObjectPropCodeName:
    	iName = aElement.StringL(CMTPTypeObjectPropListElement::EValue);
    	break;
        
    case EMTPObjectPropCodeObjectFormat:
        iFormatCode = aElement.Uint16L(CMTPTypeObjectPropListElement::EValue);
        if (iFormatCode != EMTPFormatCodeEXIFJPEG)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        break;
        
    case EMTPObjectPropCodeWidth:
        imageWidth = aElement.Uint32L(CMTPTypeObjectPropListElement::EValue);
        break;
        
    case EMTPObjectPropCodeHeight:
        imageHeight = aElement.Uint32L(CMTPTypeObjectPropListElement::EValue);
        break;
        
    case EMTPObjectPropCodeImageBitDepth:
        imageBitDepth = aElement.Uint32L(CMTPTypeObjectPropListElement::EValue);
        break;
        
    case EMTPObjectPropCodeNonConsumable:
        iNonConsumable = aElement.Uint8L(CMTPTypeObjectPropListElement::EValue);       
        break;
    case EMTPObjectPropCodeHidden:
        iHiddenStatus = aElement.Uint16L(CMTPTypeObjectPropListElement::EValue);
        break;    
    default:
        break;
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_EXTRACTPROPERTYL, "Result = 0x%04X", responseCode );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_EXTRACTPROPERTYL_EXIT );
    return responseCode;    
    }

/**
Validates the data type for a given property code.
@param aElement an object property list element
@param aPropertyCode MTP property code for the element
@return EMTPRespCodeOK if the combination is valid, or another MTP response code if not
*/
TMTPResponseCode CMTPImageDpSendObjectInfo::CheckPropCodeL(const CMTPTypeObjectPropListElement& aElement) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CHECKPROPCODEL_ENTRY );
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    switch(aElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode))
        {
    case EMTPObjectPropCodeStorageID:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        else if (iStorageId != aElement.Uint32L(CMTPTypeObjectPropListElement::EValue))
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        break;
    
    case EMTPObjectPropCodeObjectFormat:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        else if (Request().Uint32(TMTPTypeRequest::ERequestParameter3) != aElement.Uint16L(CMTPTypeObjectPropListElement::EValue))
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        break;
       
    case EMTPObjectPropCodeObjectSize:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT64)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        else if (iObjectSize != aElement.Uint64L(CMTPTypeObjectPropListElement::EValue))
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        break;
         
    case EMTPObjectPropCodeParentObject:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        else if (Request().Uint32(TMTPTypeRequest::ERequestParameter2) != aElement.Uint32L(CMTPTypeObjectPropListElement::EValue))
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        break;

    case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
        responseCode =     EMTPRespCodeAccessDenied;
        break;

    case EMTPObjectPropCodeRepresentativeSampleFormat:
    case EMTPObjectPropCodeProtectionStatus:
    case EMTPObjectPropCodeHidden:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }                        
        break;
        
    case EMTPObjectPropCodeDateCreated:
    case EMTPObjectPropCodeDateModified:                    
    case EMTPObjectPropCodeObjectFileName:    
    case EMTPObjectPropCodeName:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeString)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        break;
        
    case EMTPObjectPropCodeWidth:
    case EMTPObjectPropCodeHeight:
    case EMTPObjectPropCodeImageBitDepth:
    case EMTPObjectPropCodeRepresentativeSampleSize:
    case EMTPObjectPropCodeRepresentativeSampleHeight:
    case EMTPObjectPropCodeRepresentativeSampleWidth:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        break;
    case EMTPObjectPropCodeRepresentativeSampleData:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeAUINT8)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }        
        break;
    case EMTPObjectPropCodeNonConsumable:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT8)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }
        break;        
    default:
        responseCode = EMTPRespCodeInvalidObjectPropCode;
        break;
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_CHECKPROPCODEL, "Result = 0x%04X", responseCode );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CHECKPROPCODEL_EXIT );
    return responseCode;    
    }

/**
Reserves space for and assigns an object handle to the received object, then
sends a success response.
*/
void CMTPImageDpSendObjectInfo::ReserveObjectL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_RESERVEOBJECTL_ENTRY );
    const TInt objectStatusBitmask = 0x8000;//the most significant bit represents importing flag
    
    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatSubCode, objectStatusBitmask);//mark object imported due to it sent by PC
    iReceivedObject->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);    
    iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObject, iObjectSize);    
    
    // prepare for rollback
    iRollbackList.AppendL(UnreserveObject);    
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_RESERVEOBJECTL_EXIT );
    }

/**
Sets the read only status on the current file to match the sent object.
*/
void CMTPImageDpSendObjectInfo::SetPropertiesL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_SETPROPERTIESL_ENTRY );
    
    iObjectPropertyMgr.SetCurrentObjectL(*iReceivedObject, ETrue, ETrue);
    iReceivedObject->SetDesCL(CMTPObjectMetaData::ESuid, iFullPath);
    if (iName.Length() == 0)
        {
        TParsePtrC pathParser(iFullPath);
        iName = pathParser.Name();
        }
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeName, iName);
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeProtectionStatus, iProtectionStatus);
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeWidth, imageWidth);
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeHeight, imageHeight);
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeImageBitDepth, imageBitDepth);    
    iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeNonConsumable, iNonConsumable);
  
    if(iDateCreated != NULL && iDateCreated->Length())
        {//currently image dp can not support this property
        iObjectPropertyMgr.SetPropertyL(EMTPObjectPropCodeDateCreated, *iDateCreated);
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_SETPROPERTIESL_EXIT );
    }
    
void CMTPImageDpSendObjectInfo::Rollback()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_ROLLBACK_ENTRY ); 
    
    TInt count = iRollbackList.Count();
    while(--count >= 0)
        {
        TRAP_IGNORE((*iRollbackList[count])(this));
        }
    iRollbackList.Reset();

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_ROLLBACK_EXIT );
    }
    
void CMTPImageDpSendObjectInfo::CleanUndoList()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_CLEANUNDOLIST_ENTRY );
    
    iRollbackList.Reset();

    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_CLEANUNDOLIST_EXIT );
    }

void CMTPImageDpSendObjectInfo::CreateFsObjectL()
    {
    delete iFileReceived;
    iFileReceived = NULL;
    //prepare for rollback
    iRollbackList.AppendL(RemoveObjectFromFs);
        
    iFileReceived = CMTPTypeFile::NewL(iFramework.Fs(), iFullPath, EFileWrite);
    iFileReceived->SetSizeL(iObjectSize);
    }

TMTPResponseCode CMTPImageDpSendObjectInfo::ErrorToMTPError(TInt aError) const
    {
    TMTPResponseCode resp = EMTPRespCodeGeneralError;
    
    switch (aError)
        {
    case KErrNone:
        resp = EMTPRespCodeOK;
        break;
        
    case KErrAccessDenied:
        resp = EMTPRespCodeAccessDenied;
        break;
        
    case KErrDiskFull:
        resp = EMTPRespCodeStoreFull;
        break;
        
    default:
        break;
        }

    return resp;
    }

/**
Check if the object is too large
@return ETrue if yes, otherwise EFalse
*/
TBool CMTPImageDpSendObjectInfo::IsTooLarge(TUint64 aObjectSize) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPSENDOBJECTINFO_ISTOOLARGE_ENTRY );
    TBool ret(aObjectSize > KMaxTInt64);
    
    if(!ret)
        {
        TBuf<255> fsname;
        TUint32 storageId = iStorageId;
        if (storageId == KMTPStorageDefault)
            {
            storageId = iFramework.StorageMgr().DefaultStorageId();
            }
        TInt drive( iFramework.StorageMgr().DriveNumber(storageId) );
        if(drive != KErrNotFound)
            {
            iFramework.Fs().FileSystemSubType(drive, fsname);        
        
            const TUint64 KMaxFatFileSize = 0xFFFFFFFF; //Maximal file size supported by all FAT filesystems (4GB-1)
            _LIT(KFsFAT16, "FAT16");
            _LIT(KFsFAT32, "FAT32");
        
            if((fsname.CompareF(KFsFAT16) == 0 || fsname.CompareF(KFsFAT32) == 0) && aObjectSize > KMaxFatFileSize)
                {
                ret = ETrue;
                }
            }
        }
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPSENDOBJECTINFO_ISTOOLARGE, "Result = %d", ret );
    OstTraceFunctionExit0( CMTPIMAGEDPSENDOBJECTINFO_ISTOOLARGE_EXIT );
    return ret;
    }
