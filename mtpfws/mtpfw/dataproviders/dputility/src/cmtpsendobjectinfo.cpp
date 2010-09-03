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

#include <f32file.h>
#include <bautils.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mmtpdataproviderframework.h>

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypefile.h>
#include <mtp/cmtptypeobjectinfo.h>
#include <mtp/cmtptypeobjectproplist.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/mtpprotocolconstants.h>


#include <mtp/tmtptyperequest.h>
#include "cmtpconnection.h"
#include "cmtpconnectionmgr.h"
#include "cmtpsendobjectinfo.h"
#include "mtpdppanic.h"
#include "cmtpfsexclusionmgr.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "cmtpstoragemgr.h"
#include "mtpdebug.h"
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
@param aFramework  The data provider framework
@param aConnection The connection from which the request comes
@return a pointer to the created request processor object
*/ 
EXPORT_C MMTPRequestProcessor* CMTPSendObjectInfo::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
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
EXPORT_C CMTPSendObjectInfo::~CMTPSendObjectInfo()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CMTPSENDOBJECTINFO_DES_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPSENDOBJECTINFO_CMTPSENDOBJECTINFO, 
            "iProgress:%d NoRollback:%d", iProgress,iNoRollback );
    if ((iProgress == EObjectInfoSucceed ||
        iProgress == EObjectInfoFail || 
        iProgress == EObjectInfoInProgress) && !iNoRollback)
        {
        // Not finished SendObjectInfo/PropList SendObject pair detected.
        Rollback();
        }
    
    iDpSingletons.Close();
    delete iDateMod;
    delete iFileReceived;
    delete iParentSuid;    
    delete iReceivedObject;
    delete iObjectInfo;
    delete iObjectPropList;
    iSingletons.Close();
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CMTPSENDOBJECTINFO_DES_EXIT );
    }

/**
Standard c++ constructor
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
*/    
CMTPSendObjectInfo::CMTPSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iHiddenStatus( EMTPVisible )
    {
    }

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/    
TMTPResponseCode CMTPSendObjectInfo::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CHECKREQUESTL_ENTRY );
    TMTPResponseCode result = CheckSendingStateL();
    
    if (result != EMTPRespCodeOK) 
        {
        OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CHECKREQUESTL_EXIT );
        return result;
        }
    
    if (iProgress == EObjectNone)    //this is the SendObjectInfo phase
        {
        iElementCount = sizeof(KMTPSendObjectInfoPolicy) / sizeof(TMTPRequestElementInfo);
        iElements = KMTPSendObjectInfoPolicy;            
        }
    else if (iProgress == EObjectInfoSucceed)
        {
        iElementCount = 0;
        iElements = NULL;
        }
    //coverity[var_deref_model]
	result = CMTPRequestProcessor::CheckRequestL();     	
 
    if (EMTPRespCodeOK == result)
        {
        result = MatchStoreAndParentL();
        }
        
    if (result == EMTPRespCodeOK && iOperationCode == EMTPOpCodeSendObjectPropList)
        {
        TMTPFormatCode formatCode = static_cast<TMTPFormatCode>(Request().Uint32(TMTPTypeRequest::ERequestParameter3));
        if (!iDpSingletons.ExclusionMgrL().IsFormatValid(formatCode))
            {
            result = EMTPRespCodeInvalidObjectFormatCode;
            }
        else
            {
            iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
            TUint32 objectSizeHigh = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
            TUint32 objectSizeLow = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
            if (iStorageId == KMTPStorageDefault)
                {
                iStorageId = iFramework.StorageMgr().DefaultStorageId();
                }
            iObjectSize = MAKE_TUINT64(objectSizeHigh, objectSizeLow);
            if (IsTooLarge(iObjectSize))
                {
                result = EMTPRespCodeObjectTooLarge;            
                }
         	   
            }
        }
        
    // If the previous request is not SendObjectInfo or SendObjectPropList, SendObject fails
    if (result == EMTPRespCodeOK && iOperationCode == EMTPOpCodeSendObject)
        {
        if (iPreviousTransactionID + 1 != Request().Uint32(TMTPTypeRequest::ERequestTransactionID))
            {
            result = EMTPRespCodeNoValidObjectInfo;
            }
        }
        
    OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_CHECKREQUESTL, "Result = 0x%04X", result );
    OstTraceFunctionExit0( DUP1_CMTPSENDOBJECTINFO_CHECKREQUESTL_EXIT );
    return result;    
    }
    
TBool CMTPSendObjectInfo::HasDataphase() const
    {
    return ETrue;
    }

/**
SendObjectInfo/SendObject request handler
NOTE: SendObjectInfo has to be comes before SendObject requests.  To maintain the state information
between the two requests, the two requests are combined together in one request processor.
*/    
void CMTPSendObjectInfo::ServiceL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_SERVICEL_ENTRY );
    if (iProgress == EObjectNone)
        {
        iIsFolder = EFalse;
        if (iOperationCode == EMTPOpCodeSendObjectInfo)
            {
            ServiceSendObjectInfoL();
            }
        else
            {
            ServiceSendObjectPropListL();
            }
        }
    else
        {
        ServiceSendObjectL();
        }    
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_SERVICEL_EXIT );
    }

/**
Second-phase construction
*/        
void CMTPSendObjectInfo::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CONSTRUCTL_ENTRY );
    iExpectedSendObjectRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, EMTPOpCodeSendObject);
    iReceivedObject = CMTPObjectMetaData::NewL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
    iDpSingletons.OpenL(iFramework);
    iNoRollback = EFalse;
    iSingletons.OpenL();
    _LIT(KM4A, ".m4a");
    _LIT(KODF, ".odf");
    iExceptionList.AppendL(KM4A());
    iExceptionList.AppendL(KODF());
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CONSTRUCTL_EXIT );
    }

/**
Override to match both the SendObjectInfo and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPSendObjectInfo::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_MATCH_ENTRY );
    TBool result = EFalse;
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
    if ((operationCode == EMTPOpCodeSendObjectInfo || 
        operationCode == EMTPOpCodeSendObject ||
        operationCode == EMTPOpCodeSendObjectPropList) &&
        &iConnection == &aConnection)
        {
        result = ETrue;
        }
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_MATCH_EXIT );
    return result;    
    }

/**
Override to handle the response phase of SendObjectInfo and SendObject requests
@return EFalse
*/
TBool CMTPSendObjectInfo::DoHandleResponsePhaseL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLERESPONSEPHASEL_ENTRY );
    //to check if the sending/receiving data is successful
    TBool successful = !iCancelled;
    if (iProgress == EObjectInfoInProgress)
        {
        if (iOperationCode == EMTPOpCodeSendObjectInfo)
            {            
            successful = DoHandleSendObjectInfoCompleteL();
            }
        else
            {
            successful = DoHandleSendObjectPropListCompleteL();
            }
        iProgress = (successful ? EObjectInfoSucceed : EObjectInfoFail);
        if(iIsFolder && iProgress == EObjectInfoSucceed)
			{
			iProgress = EObjectNone;
			}
        }
    else if (iProgress == ESendObjectInProgress)
        {
        successful = DoHandleSendObjectCompleteL();
        iProgress = (successful ? ESendObjectSucceed : ESendObjectFail);
        }
        
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLERESPONSEPHASEL_EXIT );
    return EFalse;
    }

/**
Override to handle the completing phase of SendObjectInfo and SendObject requests
@return ETrue if succesfully received the file, otherwise EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleCompletingPhaseL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLECOMPLETINGPHASEL_ENTRY );
    TBool result = ETrue;
    CMTPRequestProcessor::DoHandleCompletingPhaseL();
    if (iProgress == EObjectInfoSucceed)
        {
        if (iOperationCode == EMTPOpCodeSendObjectInfo || iOperationCode == EMTPOpCodeSendObjectPropList)
            {
            iPreviousTransactionID = Request().Uint32(TMTPTypeRequest::ERequestTransactionID);
            }
        result = EFalse;
        }
    else if (iProgress == ESendObjectFail)
        {
        if (iOperationCode == EMTPOpCodeSendObject)
            {
            iPreviousTransactionID++;
            }
        iProgress = EObjectInfoSucceed;
        result = EFalse;
        }

    OstTraceExt2( TRACE_NORMAL, CMTPSENDOBJECTINFO_DOHANDLECOMPLETINGPHASEL, 
            "Exit result:%d progress:%d",result,iProgress );
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLECOMPLETINGPHASEL_EXIT );
    return result;    
    }


/**
Verify if the SendObject request comes after SendObjectInfo request
@return EMTPRespCodeOK if SendObject request comes after a valid SendObjectInfo request, otherwise
EMTPRespCodeNoValidObjectInfo
*/
TMTPResponseCode CMTPSendObjectInfo::CheckSendingStateL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CHECKSENDINGSTATEL_ENTRY );
    TMTPResponseCode result = EMTPRespCodeOK;
    iOperationCode = Request().Uint16(TMTPTypeRequest::ERequestOperationCode);

    if (iOperationCode == EMTPOpCodeSendObject)
    	{
    	//In ParseRouter everytime SendObject gets resolved then will be removed from Registry
    	//Right away therefore we need reRegister it here again in case possible cancelRequest
    	//Against this SendObject being raised.
    	iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
    	iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
       	}        
    
    if (iProgress == EObjectNone)
        {
        if (iOperationCode == EMTPOpCodeSendObject)
            {
            result = EMTPRespCodeNoValidObjectInfo;
            }        
        }
    else if (iProgress == EObjectInfoSucceed) 
        {
        if (iOperationCode == EMTPOpCodeSendObjectInfo || iOperationCode == EMTPOpCodeSendObjectPropList)
            {
            //SendObjectInfo/SendObjectPropList sending the folder over which not necessarily
            //being followed by SendObject as per MTP Specification in which case we need unregister RouteRequest of 
            //SendObject made by previous SendObjectInfo/SendObjectPropList transaction without SendObject being followed.
            if( iIsFolder )
            	{
            	iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        		}
        
            delete iObjectInfo;
            iObjectInfo = NULL;
            delete iObjectPropList;
            iObjectPropList = NULL;
            iProgress = EObjectNone;
            }
        }
    else 
        {
        OstTrace1( TRACE_ERROR, CMTPSENDOBJECTINFO_CHECKSENDINGSTATEL, 
                "The progress state is not right.  Current state is %d", iProgress );
        User::Leave( KErrGeneral );
        }
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CHECKSENDINGSTATEL_EXIT );
    return result;    
    }

/**
SendObjectInfo request handler
*/
void CMTPSendObjectInfo::ServiceSendObjectInfoL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTINFOL_ENTRY );
    delete iObjectInfo;
    iObjectInfo = NULL;
    iObjectInfo = CMTPTypeObjectInfo::NewL();
    iCancelled = EFalse;
    ReceiveDataL(*iObjectInfo);
    iProgress = EObjectInfoInProgress;
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTINFOL_EXIT );
    }

/**
SendObjectPropList request handler
*/
void CMTPSendObjectInfo::ServiceSendObjectPropListL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTPROPLISTL_ENTRY );
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    iCancelled = EFalse;
    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode, iRequest->Uint32(TMTPTypeRequest::ERequestParameter3));
    ReceiveDataL(*iObjectPropList);
    iProgress = EObjectInfoInProgress;
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTPROPLISTL_EXIT );
    }
    
/**
SendObject request handler
*/    
void CMTPSendObjectInfo::ServiceSendObjectL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTL_ENTRY );
    if (iIsFolder)
        {
        // A generic folder doesn't have anything interesting during its data phase
        ReceiveDataL(iNullObject);
        }
    else    
        {        
        ReceiveDataL(*iFileReceived);
        }
    
    iProgress = ESendObjectInProgress;
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_SERVICESENDOBJECTL_EXIT );
    }

/**
Get a default parent object, if the request does not specify a parent object.
*/
void CMTPSendObjectInfo::GetDefaultParentObjectL()
    {    
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL_ENTRY );
    if (iStorageId == KMTPStorageDefault)
        {
        iStorageId = iFramework.StorageMgr().DefaultStorageId();
        }
    TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
    LEAVEIFERROR(drive,
            OstTrace1( TRACE_ERROR, CMTPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL, "can't get driver number for storage %d", iStorageId));

    // Obtain the root of the drive.  Logical storages can sometimes have a filesystem root
    // other than <drive>:\ .  For example an MP3 DP might have a root of c:\media\music\
    // The DevDP needs to be aware of this when handling associations (folders) so they are
    // created in the correct location on the filesystem.
    delete iParentSuid;
    iParentSuid = NULL;
    iParentSuid=(iFramework.StorageMgr().StorageL(iStorageId).DesC(CMTPStorageMetaData::EStorageSuid)).AllocL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EParentHandle, KMTPHandleNoParent);     
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_GETDEFAULTPARENTOBJECTL_EXIT );
    }

/**
Get parent object and storage id
@return EMTPRespCodeOK if successful, otherwise, EMTPRespCodeInvalidParentObject
*/
TMTPResponseCode CMTPSendObjectInfo::GetParentObjectAndStorageIdL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL_ENTRY );    
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));

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

    OstTraceExt1( TRACE_NORMAL, CMTPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL, "iParentSuid = %S", *iParentSuid );
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_GETPARENTOBJECTANDSTORAGEIDL_EXIT );
    return EMTPRespCodeOK;
    }

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectInfoCompleteL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_ENTRY ); 
    TBool result(ETrue);
    TUint16 format(iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    result = iDpSingletons.ExclusionMgrL().IsFormatValid(TMTPFormatCode(format));
    
    if (result)
        {     
		OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, 
		        "ASSOCIATION TYPE IS: %X", iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType));
		
        if(format == EMTPFormatCodeAssociation)
			{
			if((iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType) == EMTPAssociationTypeGenericFolder) ||
        		      (iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType) == EMTPAssociationTypeUndefined))
				iIsFolder = ETrue;
			else{
				SendResponseL(EMTPRespCodeInvalidDataset);
	            result = EFalse;	
				}
			}
        delete iDateMod;
        iDateMod = NULL;
    
        iDateMod = iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EDateModified).AllocL();
    
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
        
        if (IsTooLarge(iObjectSize))
            {
            SendResponseL(EMTPRespCodeObjectTooLarge);
            result = EFalse;            
            }
        }

    if (result)
        {
        iProtectionStatus = iObjectInfo->Uint16L(CMTPTypeObjectInfo::EProtectionStatus);
        result = GetFullPathNameL(iObjectInfo->StringCharsL(CMTPTypeObjectInfo::EFilename));
        if (!result)
            {        
            // File and/or parent pathname invalid.
            SendResponseL(EMTPRespCodeInvalidDataset);
            }
        }

    if (result && !iIsFolder)
        {    
        result &= !Exists(iFullPath);
        if (!result)
            {        
            // Object with the same name already exists.
            iNoRollback = ETrue;
            SendResponseL(EMTPRespCodeAccessDenied);
            }
        }
    
    if (result)
        {
        iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode, format);
        
        if (iIsFolder)
        	{
        	iReceivedObject->SetUint(CMTPObjectMetaData::EFormatSubCode, EMTPAssociationTypeGenericFolder);
        	}
        	
        TRAPD(err, CreateFsObjectL());
        
        if (err != KErrNone)
            {
            OstTrace1( TRACE_ERROR, DUP1_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL, "Fail to create fs object %d", err );
            SendResponseL(ErrorToMTPError(err));
            result = EFalse;
            }
        else
            {
            ReserveObjectL();
            }
        }
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTINFOCOMPLETEL_EXIT );
    return result;    
    }

/**
Handling the completing phase of SendObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectPropListCompleteL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL_ENTRY );
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
        iIsFolder = (formatCode == EMTPFormatCodeAssociation);

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
        
    if (result && !iIsFolder)
        {
        result = !Exists(iFullPath);
        if (!result)
            {
            // Object with the same name already exists.
            iNoRollback = ETrue;
            SendResponseL(EMTPRespCodeAccessDenied);
            }
        }    
    
    if (result)
        {
        if (iIsFolder)
        	{
        	iReceivedObject->SetUint(CMTPObjectMetaData::EFormatSubCode, EMTPAssociationTypeGenericFolder);
        	}
        
        TRAPD(err, CreateFsObjectL());
        
        if (err != KErrNone)
            {
            OstTrace1( TRACE_ERROR, CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL, "Fail to create fs object %d", err );
            SendResponseL(ErrorToMTPError(err));
            result = EFalse;
            }
        else
            {
            ReserveObjectL();
            }
        }

    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTPROPLISTCOMPLETEL_EXIT );
    return result;    
    }
    
/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectCompleteL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL_ENTRY );
    TBool result(ETrue);
        
    if (!iIsFolder)
        {        
        delete iFileReceived;
        iFileReceived = NULL;
        
        TEntry fileEntry;
        LEAVEIFERROR(iFramework.Fs().Entry(iFullPath, fileEntry),
                OstTraceExt1( TRACE_ERROR, CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL, "Gets the entry details for %S failed!", iFullPath));
        if (fileEntry.FileSize() != iObjectSize)
            {
			delete iFileReceived;
        	iFileReceived = NULL;
            iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
            
            iFramework.Fs().Delete(iFullPath);
            iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObject);
            TMTPResponseCode responseCode = EMTPRespCodeObjectTooLarge;
            if (fileEntry.FileSize() < iObjectSize)
                {
                responseCode = EMTPRespCodeInvalidDataset;
                }
            SendResponseL(responseCode);
            result = EFalse;
            }
        }
    
    
    // Get the result of the SendObject operation. 
    RMTPFramework frameworkSingletons;   
    frameworkSingletons.OpenL();
    TUint connectionId = iConnection.ConnectionId();
    CMTPConnectionMgr& connectionMgr = frameworkSingletons.ConnectionMgr();
    CMTPConnection& connection = connectionMgr.ConnectionL(connectionId);
    TInt ret = connection.GetDataReceiveResult(); 
    frameworkSingletons.Close();
     // SendObject is cancelled or connection is dropped.
    if(result && (iCancelled || (ret == KErrAbort)))
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL, "It is a cancel for sendObject." );
        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        Rollback();
        SendResponseL(EMTPRespCodeTransactionCancelled);        
        }
    else if (result && !iCancelled)
	    {
	     iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        
        //The MTP spec states that it is not mandatory for SendObjectInfo/SendObjectPropList
        //to be followed by a SendObject.  An object is reserved in the ObjectStore on 
        //receiving a SendObjectInfo/SendObjectPropList request, but we only commit it 
        //on receiving the corresponding SendObject request.  With Associations however 
        //we commit the object straight away as the SendObject phase is often absent 
        //with folder creation.

        if(!iIsFolder)
            {
            SetPropertiesL();    
            delete iFileReceived;
            iFileReceived = NULL;
            iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObject);
            iFullPath.LowerCase();
            OstTraceExt1( TRACE_NORMAL, DUP2_CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL, "File Name %S", iFullPath);
            TParsePtrC file( iFullPath );
            if ( file.ExtPresent() && file.Ext().Length()<=KExtensionLength && iExceptionList.Find(file.Ext()) != KErrNotFound)
                {
                TUint32 DpId = iFramework.DataProviderId();
                HBufC* mime = iDpSingletons.MTPUtility().ContainerMimeType(iFullPath);
                CleanupStack::PushL(mime);
                if ( mime != NULL )
                    {
                    DpId = iDpSingletons.MTPUtility().GetDpIdL(file.Ext().Mid(1),*mime);
                    }
                else
                    {
                    DpId = iDpSingletons.MTPUtility().GetDpIdL(file.Ext().Mid(1), KNullDesC);
                    }
                if ( DpId!=iFramework.DataProviderId())
                    {
                    iReceivedObject->SetUint(CMTPObjectMetaData::EDataProviderId,DpId);
                    TUint32 format = EMTPFormatCodeUndefined;
                    TUint16 subFormat = 0;
                    if(mime != NULL)
                        {
                        format = iDpSingletons.MTPUtility().GetFormatCodeByMimeTypeL(file.Ext().Mid(1),*mime);
                        subFormat = iDpSingletons.MTPUtility().GetSubFormatCodeL(file.Ext().Mid(1),*mime);
                        }
                    else
                        {
                        format = iDpSingletons.MTPUtility().GetFormatByExtension(file.Ext().Mid(1));
                        }
                    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode,format);
                    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatSubCode,subFormat);
                    iFramework.ObjectMgr().ModifyObjectL(*iReceivedObject);
                    TUint32 handle = iReceivedObject->Uint(CMTPObjectMetaData::EHandle);
                    iSingletons.DpController().NotifyDataProvidersL(DpId,EMTPObjectAdded,(TAny*)&handle);
                    }
                CleanupStack::PopAndDestroy(mime);
                }
            }
        
        SendResponseL(EMTPRespCodeOK);
	    }
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_DOHANDLESENDOBJECTCOMPLETEL_EXIT );
    return result;
    }


/**
Get the full path name of the object to be saved
@param aFileName, on entry, contains the file name of the object,
on return, contains the full path name of the object to be saved
@return ETrue if the name is valid, EFalse otherwise
*/
TBool CMTPSendObjectInfo::GetFullPathNameL(const TDesC& aFileName)
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_GETFULLPATHNAMEL_ENTRY );
    TBool result(EFalse);
    if (aFileName.Length() > 0)
        {
        iFullPath = *iParentSuid;
        if (iFullPath.Length() + aFileName.Length() < iFullPath.MaxLength())
            {
            iFullPath.Append(aFileName);
            if (iIsFolder)
                {
                iFullPath.Append(KPathDelimiter);
                
                   TBool valid(EFalse);
                if (BaflUtils::CheckWhetherFullNameRefersToFolder(iFullPath, valid) == KErrNone)
                    {
                    result = valid;
                    }
                }
            else
                {
                result = iFramework.Fs().IsValidName(iFullPath);
                }
            }
        }

#ifdef OST_TRACE_COMPILER_IN_USE
    TFileName tempName;
    tempName.Copy(iFullPath);
    tempName.Collapse();
    OstTraceExt2( TRACE_NORMAL, CMTPSENDOBJECTINFO_GETFULLPATHNAMEL, "iFullPath = %S, Result = %d", tempName, result);
#endif
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_GETFULLPATHNAMEL_EXIT );
    return result;
    }

/**
Check if the object is too large
@return ETrue if yes, otherwise EFalse
*/
TBool CMTPSendObjectInfo::IsTooLarge(TUint64 aObjectSize) const
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_ISTOOLARGE_ENTRY );
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
        LEAVEIFERROR(drive,
                OstTrace1( TRACE_ERROR, CMTPSENDOBJECTINFO_ISTOOLARGE, "can't get driver number for storage %d", storageId));
        iFramework.Fs().FileSystemSubType(drive, fsname);        
        
        const TUint64 KMaxFatFileSize = 0xFFFFFFFF; //Maximal file size supported by all FAT filesystems (4GB-1)
        _LIT(KFsFAT16, "FAT16");
        _LIT(KFsFAT32, "FAT32");
        
        if((fsname.CompareF(KFsFAT16) == 0 || fsname.CompareF(KFsFAT32) == 0) && aObjectSize > KMaxFatFileSize)
            {
            ret = ETrue;
            }
        }
    OstTrace1( TRACE_NORMAL, DUP1_CMTPSENDOBJECTINFO_ISTOOLARGE, "Result = %d", ret );
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_ISTOOLARGE_EXIT );
    return ret;
    }
    
/**
Check if the file already exists on the storage.
@return ETrue if file is exists, otherwise EFalse
*/
TBool CMTPSendObjectInfo::Exists(const TDesC& aName) const
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_EXISTS_ENTRY );
    // This detects both files and folders
    TBool ret(EFalse); 
    ret = BaflUtils::FileExists(iFramework.Fs(), aName);
    OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_EXISTS, "Result = %d", ret );
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_EXISTS_EXIT );
    return ret;
    }

/**
Check if the property list is valid and extract properties (file name)
@param aInvalidParameterIndex if invalid, contains the index of the property.  Undefined, if it is valid.
@return if error, one of the error response code; otherwise EMTPRespCodeOK
*/
TMTPResponseCode CMTPSendObjectInfo::VerifyObjectPropListL(TInt& aInvalidParameterIndex)
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL_ENTRY );
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
    OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL, "Result = 0x%04X", responseCode);
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_VERIFYOBJECTPROPLISTL_EXIT );
    return responseCode;        
    }

/**
Extracts the file information from the object property list element
@param aElement an object property list element
@param aPropertyCode MTP property code for the element
@return MTP response code
*/
TMTPResponseCode CMTPSendObjectInfo::ExtractPropertyL(const CMTPTypeObjectPropListElement& aElement)
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_EXTRACTPROPERTYL_ENTRY );
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    switch (aElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode))
        {
    case EMTPObjectPropCodeAssociationDesc:
        // Actually, any association is treated as a folder, and iIsFolder should already be set
        iIsFolder = ((aElement.Uint32L(CMTPTypeObjectPropListElement::EValue) == EMTPAssociationTypeGenericFolder)||
					(aElement.Uint32L(CMTPTypeObjectPropListElement::EValue) == EMTPAssociationTypeUndefined));
        break;
        
    case EMTPObjectPropCodeObjectFileName:
        {
        const TDesC& KFileName = aElement.StringL(CMTPTypeObjectPropListElement::EValue);
        if (!GetFullPathNameL(KFileName))
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
    case EMTPObjectPropCodeName:
    	iName = aElement.StringL(CMTPTypeObjectPropListElement::EValue);
    	break;
    case EMTPObjectPropCodeHidden:
        iHiddenStatus = aElement.Uint16L(CMTPTypeObjectPropListElement::EValue);
        break;
    default:
        break;
        }
    OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_EXTRACTPROPERTYL, "Result = 0x%04X", responseCode);
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_EXTRACTPROPERTYL_EXIT );
    return responseCode;    
    }

/**
Validates the data type for a given property code.
@param aElement an object property list element
@param aPropertyCode MTP property code for the element
@return EMTPRespCodeOK if the combination is valid, or another MTP response code if not
*/
TMTPResponseCode CMTPSendObjectInfo::CheckPropCodeL(const CMTPTypeObjectPropListElement& aElement) const
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_CHECKPROPCODEL_ENTRY );
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

    case EMTPObjectPropCodeProtectionStatus:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16)
            {
            responseCode = EMTPRespCodeInvalidObjectPropFormat;
            }                        
        break;
        
    case EMTPObjectPropCodeDateModified:                    
    case EMTPObjectPropCodeObjectFileName:    
    case EMTPObjectPropCodeName:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeString)
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
        
    case EMTPObjectPropCodeAssociationType:
    case EMTPObjectPropCodeHidden:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16)
             {
             responseCode = EMTPRespCodeInvalidObjectPropFormat;
             }
    	break;
    	
    case EMTPObjectPropCodeAssociationDesc:
        if (aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32)
             {
             responseCode = EMTPRespCodeInvalidObjectPropFormat;
             }
    	break;
                
    default:
        responseCode = EMTPRespCodeInvalidObjectPropCode;
        break;
        }
    OstTrace1( TRACE_NORMAL, CMTPSENDOBJECTINFO_CHECKPROPCODEL, "Result = 0x%04X", responseCode);
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_CHECKPROPCODEL_EXIT );
    return responseCode;    
    }

/**
Validates the data type for a given property code.
@return EMTPRespCodeOK if the parent handle matches the store id, or another MTP response code if not
*/
TMTPResponseCode CMTPSendObjectInfo::MatchStoreAndParentL() const
    {
    TMTPResponseCode ret = EMTPRespCodeOK;
    const TUint32 storeId(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    const TUint32 parentHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    
    if( (EMTPOpCodeSendObjectPropList == iOperationCode) || (EMTPOpCodeSendObjectInfo == iOperationCode) )
    	{
		if(storeId != KMTPStorageDefault)
			{
			if(!iSingletons.StorageMgr().IsReadWriteStorage(storeId))
				{
				ret = EMTPRespCodeStoreReadOnly;
				}
			}
		
		 // this checking is only valid when the second parameter is not a special value.
		if ((EMTPRespCodeOK == ret) && (parentHandle != KMTPHandleAll && parentHandle != KMTPHandleNone))
			{
			//does not take owernship
			CMTPObjectMetaData* parentObjInfo = iRequestChecker->GetObjectInfo(parentHandle);
			__ASSERT_DEBUG(parentObjInfo, Panic(EMTPDpObjectNull));
			
			if (parentObjInfo->Uint(CMTPObjectMetaData::EStorageId) != storeId)      
				{
				ret = EMTPRespCodeInvalidObjectHandle;
				}
			}
    	}

    return ret;
    }

/**
Reserves space for and assigns an object handle to the received object, then
sends a success response.
*/
void CMTPSendObjectInfo::ReserveObjectL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_RESERVEOBJECTL_ENTRY );
    iReceivedObject->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
    iReceivedObject->SetDesCL(CMTPObjectMetaData::ESuid, iFullPath);
    
    if(iIsFolder)
        {
        SetPropertiesL();
        TUint32 handle = iFramework.ObjectMgr().HandleL(iFullPath);
        if (handle != KMTPHandleNone)
            {
            // The folder is already in DB
            iReceivedObject->SetUint(CMTPObjectMetaData::EHandle, handle);
            iFramework.ObjectMgr().ModifyObjectL(*iReceivedObject);
            }
        else
            {
            iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObject, iObjectSize);
            iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObject);
            }
        }
    else
        {
        iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObject, iObjectSize);    
        iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
        iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
        }
    TUint32 parameters[3];
    parameters[0] = iStorageId;
    parameters[1] = iParentHandle;
    parameters[2] = iReceivedObject->Uint(CMTPObjectMetaData::EHandle);
    SendResponseL(EMTPRespCodeOK, (sizeof(parameters) / sizeof(parameters[0])), parameters);
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_RESERVEOBJECTL_EXIT );
    }
    
void CMTPSendObjectInfo::CreateFsObjectL()
    {
    if (iIsFolder)
        {
        if (!Exists(iFullPath))
            {
            LEAVEIFERROR(iFramework.Fs().MkDirAll(iFullPath),
                    OstTraceExt1( TRACE_ERROR, CMTPSENDOBJECTINFO_CREATEFSOBJECTL, "Makes directories failed for %S", iFullPath));      
            }
        }
    else
        {
        delete iFileReceived;
        iFileReceived = NULL;
        iFileReceived = CMTPTypeFile::NewL(iFramework.Fs(), iFullPath, EFileWrite);
        iFileReceived->SetSizeL(iObjectSize);
        }
    }
    
void CMTPSendObjectInfo::Rollback()
    {
    if(iIsFolder)
        {
        OstTrace0( TRACE_NORMAL, CMTPSENDOBJECTINFO_ROLLBACK, "Rollback the dir created." );
        iFramework.Fs().RmDir(iFullPath);
        // If it is folder, delete it from MTP database, i.e ObjectStore.
        TRAP_IGNORE(iFramework.ObjectMgr().RemoveObjectL(iFullPath));
        }
    else
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPSENDOBJECTINFO_ROLLBACK, "Rollback the file created." );
        delete iFileReceived;
        iFileReceived = NULL;
        // Delete this object from file system.
        iFramework.Fs().Delete(iFullPath);
        TRAP_IGNORE(iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObject));
        }
    }
    
TMTPResponseCode CMTPSendObjectInfo::ErrorToMTPError(TInt aError) const
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
Sets the read only status on the current file to match the sent object.
*/
void CMTPSendObjectInfo::SetPropertiesL()
    {
    OstTraceFunctionEntry0( CMTPSENDOBJECTINFO_SETPROPERTIESL_ENTRY );
    TEntry entry;
    LEAVEIFERROR(iFramework.Fs().Entry(iFullPath, entry),
            OstTraceExt1( TRACE_ERROR, CMTPSENDOBJECTINFO_SETPROPERTIESL, "can't get entry details from %S", iFullPath));
    
    TUint16 assoc(EMTPAssociationTypeUndefined);
	if (entry.IsDir())
		{
		assoc = EMTPAssociationTypeGenericFolder;
		}
	iReceivedObject->SetUint(CMTPObjectMetaData::EFormatSubCode, assoc);    
        
    if (iName.Length() == 0)
    {
    	if (entry.IsDir())
    	{
    		TParsePtrC pathParser(iFullPath.Left(iFullPath.Length() - 1)); // Ignore the trailing "\".
    		iName = pathParser.Name();
    	}
    	else
    	{
        	TParsePtrC pathParser(iFullPath);
        	iName = pathParser.Name();
    	}
    }    
    
    if (iProtectionStatus ==  EMTPProtectionNoProtection ||
        iProtectionStatus == EMTPProtectionReadOnly)
        {
        entry.iAtt &= ~(KEntryAttNormal | KEntryAttReadOnly);
        if (iProtectionStatus == EMTPProtectionNoProtection)
            {                        
            entry.iAtt |= KEntryAttNormal;
            }
        else
            {
            entry.iAtt |= KEntryAttReadOnly;
            }
        if ( iFileReceived )
            {
            User::LeaveIfError(iFileReceived->File().SetAtt(entry.iAtt, ~entry.iAtt));
            }
        else
            {
        LEAVEIFERROR(iFramework.Fs().SetAtt(iFullPath, entry.iAtt, ~entry.iAtt),
                OstTraceExt2( TRACE_ERROR, DUP1_CMTPSENDOBJECTINFO_SETPROPERTIESL, "set attribute %d for %S failed!", (TInt32)entry.iAtt, iFullPath));      
            }
        }
    
    if ( EMTPHidden == iHiddenStatus )
        {
		entry.iAtt &= ~KEntryAttHidden;
        entry.iAtt |= KEntryAttHidden;
        if ( iFileReceived )
            {
            User::LeaveIfError(iFileReceived->File().SetAtt(entry.iAtt, ~entry.iAtt));
            }
        else
            {
            User::LeaveIfError(iFramework.Fs().SetAtt(iFullPath, entry.iAtt, ~entry.iAtt));
            }
        }
    
    if(iDateMod != NULL && iDateMod->Length())
       {
       TTime modifiedTime;
       iDpSingletons.MTPUtility().MTPTimeStr2TTime(*iDateMod, modifiedTime);
       if ( iFileReceived )
           { 
           User::LeaveIfError(iFileReceived->File().SetModified( modifiedTime ));
           }
       else
           {
           User::LeaveIfError(iFramework.Fs().SetModified(iFullPath, modifiedTime));
           }
       }  
    
    iReceivedObject->SetDesCL(CMTPObjectMetaData::EName, iName);
    
    OstTraceFunctionExit0( CMTPSENDOBJECTINFO_SETPROPERTIESL_EXIT );
    }

