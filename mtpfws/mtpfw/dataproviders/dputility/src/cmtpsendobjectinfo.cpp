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


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"SendObjectInfo");)

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
    __FLOG(_L8("~CMTPSendObjectInfo - Entry"));
    __FLOG_2(_L8("iProgress:%d NoRollback:%d"),iProgress,iNoRollback);
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
    __FLOG(_L8("~CMTPSendObjectInfo - Exit"));
    __FLOG_CLOSE; 
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
    __FLOG(_L8("CheckRequestL - Entry"));
    TMTPResponseCode result = CheckSendingStateL();
    
    if (result != EMTPRespCodeOK) 
        {
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
        
    __FLOG_VA((_L8("Result = 0x%04X"), result));
    __FLOG(_L8("CheckRequestL - Exit"));
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
    __FLOG(_L8("ServiceL - Entry"));
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
    __FLOG(_L8("ServiceL - Exit"));
    }

/**
Second-phase construction
*/        
void CMTPSendObjectInfo::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry")); 
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
    __FLOG(_L8("ConstructL - Exit"));
    }

/**
Override to match both the SendObjectInfo and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPSendObjectInfo::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
    {
    __FLOG(_L8("Match - Entry"));
    TBool result = EFalse;
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
    if ((operationCode == EMTPOpCodeSendObjectInfo || 
        operationCode == EMTPOpCodeSendObject ||
        operationCode == EMTPOpCodeSendObjectPropList) &&
        &iConnection == &aConnection)
        {
        result = ETrue;
        }
    __FLOG(_L8("Match - Exit"));
    return result;    
    }

/**
Override to handle the response phase of SendObjectInfo and SendObject requests
@return EFalse
*/
TBool CMTPSendObjectInfo::DoHandleResponsePhaseL()
    {
    __FLOG(_L8("DoHandleResponsePhaseL - Entry"));
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
        
    __FLOG(_L8("DoHandleResponsePhaseL - Exit"));
    return EFalse;
    }

/**
Override to handle the completing phase of SendObjectInfo and SendObject requests
@return ETrue if succesfully received the file, otherwise EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleCompletingPhaseL()
    {
    __FLOG(_L8("DoHandleCompletingPhaseL - Entry"));
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
    
    __FLOG_2(_L8("DoHandleCompletingPhaseL - Exit result:%d progress:%d"),result,iProgress);
    return result;    
    }


/**
Verify if the SendObject request comes after SendObjectInfo request
@return EMTPRespCodeOK if SendObject request comes after a valid SendObjectInfo request, otherwise
EMTPRespCodeNoValidObjectInfo
*/
TMTPResponseCode CMTPSendObjectInfo::CheckSendingStateL()
    {
    __FLOG(_L8("CheckSendingState - Entry"));
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
        User::Leave( KErrGeneral );
        }
    __FLOG(_L8("CheckSendingState - Exit"));
    return result;    
    }

/**
SendObjectInfo request handler
*/
void CMTPSendObjectInfo::ServiceSendObjectInfoL()
    {
    __FLOG(_L8("ServiceSendObjectInfoL - Entry"));
    delete iObjectInfo;
    iObjectInfo = NULL;
    iObjectInfo = CMTPTypeObjectInfo::NewL();
    iCancelled = EFalse;
    ReceiveDataL(*iObjectInfo);
    iProgress = EObjectInfoInProgress;
    __FLOG(_L8("ServiceSendObjectInfoL - Exit"));
    }

/**
SendObjectPropList request handler
*/
void CMTPSendObjectInfo::ServiceSendObjectPropListL()
    {
    __FLOG(_L8("ServiceSendObjectPropListL - Entry"));
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    iCancelled = EFalse;
    iReceivedObject->SetUint(CMTPObjectMetaData::EFormatCode, iRequest->Uint32(TMTPTypeRequest::ERequestParameter3));
    ReceiveDataL(*iObjectPropList);
    iProgress = EObjectInfoInProgress;
    __FLOG(_L8("ServiceSendObjectPropListL - Exit"));
    }
    
/**
SendObject request handler
*/    
void CMTPSendObjectInfo::ServiceSendObjectL()
    {
    __FLOG(_L8("ServiceSendObjectL - Entry"));
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
    __FLOG(_L8("ServiceSendObjectL - Exit"));
    }

/**
Get a default parent object, if the request does not specify a parent object.
*/
void CMTPSendObjectInfo::GetDefaultParentObjectL()
    {    
    __FLOG(_L8("GetDefaultParentObjectL - Entry"));
    if (iStorageId == KMTPStorageDefault)
        {
        iStorageId = iFramework.StorageMgr().DefaultStorageId();
        }
    TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
    User::LeaveIfError(drive);

    // Obtain the root of the drive.  Logical storages can sometimes have a filesystem root
    // other than <drive>:\ .  For example an MP3 DP might have a root of c:\media\music\
    // The DevDP needs to be aware of this when handling associations (folders) so they are
    // created in the correct location on the filesystem.
    delete iParentSuid;
    iParentSuid = NULL;
    iParentSuid=(iFramework.StorageMgr().StorageL(iStorageId).DesC(CMTPStorageMetaData::EStorageSuid)).AllocL();
    iReceivedObject->SetUint(CMTPObjectMetaData::EParentHandle, KMTPHandleNoParent);
    __FLOG(_L8("GetDefaultParentObjectL - Exit"));        
    }

/**
Get parent object and storage id
@return EMTPRespCodeOK if successful, otherwise, EMTPRespCodeInvalidParentObject
*/
TMTPResponseCode CMTPSendObjectInfo::GetParentObjectAndStorageIdL()
    {
    __FLOG(_L8("GetParentObjectAndStorageIdL - Entry"));    
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

    __FLOG_VA((_L8("iParentSuid = %S"), iParentSuid));
    __FLOG(_L8("GetParentObjectAndStorageIdL - Exit"));    
    return EMTPRespCodeOK;
    }

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectInfoCompleteL()
    {
    __FLOG(_L8("DoHandleSendObjectInfoCompleteL - Entry"));    
    TBool result(ETrue);
    TUint16 format(iObjectInfo->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    result = iDpSingletons.ExclusionMgrL().IsFormatValid(TMTPFormatCode(format));
    
    if (result)
        {
        __FLOG_VA((_L8("ASSOCIATION TYPE IS: %X"), iObjectInfo->Uint16L(CMTPTypeObjectInfo::EAssociationType)));        
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
            __FLOG_1(_L8("Fail to create fs object %d"),err);
            SendResponseL(ErrorToMTPError(err));
            result = EFalse;
            }
        else
            {
            ReserveObjectL();
            }
        }
    __FLOG(_L8("DoHandleSendObjectInfoCompleteL - Exit"));
    return result;    
    }

/**
Handling the completing phase of SendObjectPropList request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectPropListCompleteL()
    {
    __FLOG(_L8("DoHandleSendObjectPropListCompleteL - Entry"));
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
            __FLOG_1(_L8("Fail to create fs object %d"),err);
            SendResponseL(ErrorToMTPError(err));
            result = EFalse;
            }
        else
            {
            ReserveObjectL();
            }
        }
        
    __FLOG(_L8("DoHandleSendObjectPropListCompleteL - Exit"));
    return result;    
    }
    
/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/    
TBool CMTPSendObjectInfo::DoHandleSendObjectCompleteL()
    {
    __FLOG(_L8("DoHandleSendObjectCompleteL - Entry"));
    TBool result(ETrue);
        
    if (!iIsFolder)
        {        
        
        TEntry fileEntry;
        User::LeaveIfError(iFramework.Fs().Entry(iFullPath, fileEntry));

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
        __FLOG(_L8("It is a cancel for sendObject."));
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
            __FLOG_VA((_L8("File Name %S"), &iFullPath));
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
    __FLOG(_L8("DoHandleSendObjectCompleteL - Exit"));
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
    __FLOG(_L8("GetFullPathNameL - Entry"));
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

#ifdef __FLOG_ACTIVE
    TFileName tempName;
    tempName.Copy(iFullPath);
    tempName.Collapse();
    __FLOG_VA((_L8("iFullPath = %S, Result = %d"), &tempName, result));
    __FLOG(_L8("GetFullPathNameL - Exit"));
#endif
    return result;
    }

/**
Check if the object is too large
@return ETrue if yes, otherwise EFalse
*/
TBool CMTPSendObjectInfo::IsTooLarge(TUint64 aObjectSize) const
    {
    __FLOG(_L8("IsTooLarge - Entry"));
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
        User::LeaveIfError(drive);
        iFramework.Fs().FileSystemSubType(drive, fsname);        
        
        const TUint64 KMaxFatFileSize = 0xFFFFFFFF; //Maximal file size supported by all FAT filesystems (4GB-1)
        _LIT(KFsFAT16, "FAT16");
        _LIT(KFsFAT32, "FAT32");
        
        if((fsname.CompareF(KFsFAT16) == 0 || fsname.CompareF(KFsFAT32) == 0) && aObjectSize > KMaxFatFileSize)
            {
            ret = ETrue;
            }
        }
    __FLOG_VA((_L8("Result = %d"), ret));
    __FLOG(_L8("IsTooLarge - Exit"));
    return ret;
    }
    
/**
Check if the file already exists on the storage.
@return ETrue if file is exists, otherwise EFalse
*/
TBool CMTPSendObjectInfo::Exists(const TDesC& aName) const
    {
    __FLOG(_L8("Exists - Entry"));
    // This detects both files and folders
    TBool ret(EFalse); 
    ret = BaflUtils::FileExists(iFramework.Fs(), aName);
    __FLOG_VA((_L8("Result = %d"), ret));
    __FLOG(_L8("Exists - Exit"));
    return ret;
    }

/**
Check if the property list is valid and extract properties (file name)
@param aInvalidParameterIndex if invalid, contains the index of the property.  Undefined, if it is valid.
@return if error, one of the error response code; otherwise EMTPRespCodeOK
*/
TMTPResponseCode CMTPSendObjectInfo::VerifyObjectPropListL(TInt& aInvalidParameterIndex)
    {
    __FLOG(_L8("VerifyObjectPropListL - Entry"));
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
    __FLOG_VA((_L8("Result = 0x%04X"), responseCode));
    __FLOG(_L8("VerifyObjectPropListL - Exit"));
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
    __FLOG(_L8("ExtractPropertyL - Entry"));
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
    __FLOG_VA((_L8("Result = 0x%04X"), responseCode));
    __FLOG(_L8("ExtractPropertyL - Exit"));
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
    __FLOG(_L8("CheckPropCode - Entry"));
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
    __FLOG_VA((_L8("Result = 0x%04X"), responseCode));
    __FLOG(_L8("CheckPropCode - Exit"));
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
    __FLOG(_L8("ReserveObjectL - Entry"));    
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
    __FLOG(_L8("ReserveObjectL - Exit"));    
    }
    
void CMTPSendObjectInfo::CreateFsObjectL()
    {
    if (iIsFolder)
        {
        if (!Exists(iFullPath))
            {
            User::LeaveIfError(iFramework.Fs().MkDirAll(iFullPath));
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
        __FLOG(_L8("Rollback the dir created."));
        iFramework.Fs().RmDir(iFullPath);
        // If it is folder, delete it from MTP database, i.e ObjectStore.
        TRAP_IGNORE(iFramework.ObjectMgr().RemoveObjectL(iFullPath));
        }
    else
        {
        __FLOG(_L8("Rollback the file created."));
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
    __FLOG(_L8("SetPropertiesL - Entry"));

    TEntry entry;
    User::LeaveIfError(iFramework.Fs().Entry(iFullPath, entry));  
    
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
            User::LeaveIfError(iFramework.Fs().SetAtt(iFullPath, entry.iAtt, ~entry.iAtt));
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
    
    __FLOG(_L8("SetPropertiesL - Exit"));
    }

