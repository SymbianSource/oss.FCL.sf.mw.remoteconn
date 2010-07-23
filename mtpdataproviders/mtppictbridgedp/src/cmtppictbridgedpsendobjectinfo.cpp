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


#include <f32file.h>
#include <bautils.h>
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
#include "ptpdef.h"
#include "cmtppictbridgedpsendobjectinfo.h"
#include "mtppictbridgedppanic.h"
#include "mtppictbridgedpconst.h"
#include "cmtppictbridgeprinter.h"
#include "cptpserver.h"
#include "cmtpconnection.h"
#include "cmtpconnectionmgr.h"

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
MMTPRequestProcessor* CMTPPictBridgeDpSendObjectInfo::NewL(
    MMTPDataProviderFramework& aFramework,
    MMTPConnection& aConnection,
    CMTPPictBridgeDataProvider& aDataProvider)
    {
    CMTPPictBridgeDpSendObjectInfo* self = new (ELeave) CMTPPictBridgeDpSendObjectInfo(aFramework, aConnection, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPPictBridgeDpSendObjectInfo::~CMTPPictBridgeDpSendObjectInfo()
    {
    __FLOG_VA((_L8(">> CMTPPictBridgeDpSendObjectInfo::~CMTPPictBridgeDpSendObjectInfo iProgress=%d iNoRollback=%d "), iProgress, iNoRollback));
    __FLOG_2(_L8("iProgress:%d NoRollback:%d"),iProgress,iNoRollback);
    
    if ((iProgress == EObjectInfoSucceed || 
        iProgress == EObjectInfoFail || 
        iProgress == EObjectInfoInProgress) && !iNoRollback)
        {
        // Not finished SendObjectInfo/PropList SendObject pair detected.
        Rollback();
        }
 
    iDpSingletons.Close();
    delete iDateModP;
    delete iFileReceivedP;
    delete iParentSuidP;    
    delete iReceivedObjectP;
    delete iObjectInfoP;
    delete iObjectPropList;

    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::~CMTPPictBridgeDpSendObjectInfo"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
@param aFramework    The data provider framework
@param aConnection    The connection from which the request comes
*/    
CMTPPictBridgeDpSendObjectInfo::CMTPPictBridgeDpSendObjectInfo(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider):
    CMTPRequestProcessor(aFramework, aConnection, 0, NULL),
    iPictBridgeDP(aDataProvider)
    {
    }

/**
Verify the request
@return EMTPRespCodeOK if request is verified, otherwise one of the error response codes
*/    
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::CheckRequestL"));    
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
        iElements     = NULL;
        }

    if (iElements)
        {
        result = CMTPRequestProcessor::CheckRequestL();    
        }

    if ( EMTPRespCodeOK == result )
        {
        result = MatchStoreAndParentL();
        }

    if (( EMTPRespCodeOK == result ) && ( EMTPOpCodeSendObjectPropList == iOperationCode ))
        {
        iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        TUint32 objectSizeHigh = Request().Uint32(TMTPTypeRequest::ERequestParameter4);
        TUint32 objectSizeLow = Request().Uint32(TMTPTypeRequest::ERequestParameter5);
        iObjectSize = MAKE_TUINT64(objectSizeHigh, objectSizeLow);
        if (IsTooLarge(iObjectSize))
            {
             result = EMTPRespCodeObjectTooLarge;            
            }       
        }
    // If the previous request is not SendObjectInfo, SendObject fails
    if (result == EMTPRespCodeOK && iOperationCode == EMTPOpCodeSendObject)
        {
        if (iPreviousTransactionID + 1 != Request().Uint32(TMTPTypeRequest::ERequestTransactionID))
            {
            result = EMTPRespCodeNoValidObjectInfo;
            }
        }
        
    __FLOG_VA((_L8("<< CMTPPictBridgeDpSendObjectInfo::CheckRequestL 0x%04x"), result));    

    return result;    
    }
    
TBool CMTPPictBridgeDpSendObjectInfo::HasDataphase() const
    {
    return ETrue;
    }

/**
SendObjectInfo/SendObject request handler
NOTE: SendObjectInfo has to be comes before SendObject requests.  To maintain the state information
between the two requests, the two requests are combined together in one request processor.
*/    
void CMTPPictBridgeDpSendObjectInfo::ServiceL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::ServiceL"));
    if (iProgress == EObjectNone)
        {
        if ( EMTPOpCodeSendObjectInfo == iOperationCode )
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
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::ServiceL"));    
    }

/**
Second-phase construction
*/        
void CMTPPictBridgeDpSendObjectInfo::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::ConstructL"));      
    iExpectedSendObjectRequest.SetUint16(TMTPTypeRequest::ERequestOperationCode, EMTPOpCodeSendObject);
    iReceivedObjectP = CMTPObjectMetaData::NewL();
    iReceivedObjectP->SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
    iDpSingletons.OpenL(iFramework);
    iNoRollback = EFalse;
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::ConstructL"));    
    }

/**
Override to match both the SendObjectInfo and SendObject requests
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPPictBridgeDpSendObjectInfo::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::Match"));    
    TBool result = EFalse;
    TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
    if ((operationCode == EMTPOpCodeSendObjectInfo || 
        operationCode == EMTPOpCodeSendObject ||
        operationCode == EMTPOpCodeSendObjectPropList) &&
        &iConnection == &aConnection)
        {
        result = ETrue;
        }
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::Match"));    
    return result;    
    }

/**
Override to handle the response phase of SendObjectInfo and SendObject requests
@return EFalse
*/
TBool CMTPPictBridgeDpSendObjectInfo::DoHandleResponsePhaseL()
    {
    __FLOG_VA((_L8(">> CMTPPictBridgeDpSendObjectInfo::DoHandleResponsePhaseL iProgress==%d opCode==0x%x"),iProgress, iOperationCode));
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
        }
    else if (iProgress == ESendObjectInProgress)
        {
        successful = DoHandleSendObjectCompleteL();
        iProgress = (successful ? ESendObjectSucceed : ESendObjectFail);
        }        
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::DoHandleResponsePhaseL"));    
    return EFalse;
    }

/**
Override to handle the completing phase of SendObjectInfo and SendObject requests
@return ETrue if succesfully received the file, otherwise EFalse
*/    
TBool CMTPPictBridgeDpSendObjectInfo::DoHandleCompletingPhaseL()
    {
    __FLOG_VA((_L8(">> CMTPPictBridgeDpSendObjectInfo::DoHandleCompletingPhaseL iProgress==%d opCode==0x%x"),iProgress, iOperationCode));
    TBool result = ETrue;
    CMTPRequestProcessor::DoHandleCompletingPhaseL();
    if (iProgress == EObjectInfoSucceed)
        {
        if (( iOperationCode == EMTPOpCodeSendObjectInfo ) || ( iOperationCode == EMTPOpCodeSendObjectPropList ))
            {
            iPreviousTransactionID = Request().Uint32(TMTPTypeRequest::ERequestTransactionID);
            }
        result = EFalse;
        }
    else if (iProgress == ESendObjectSucceed)
        {
        iPictBridgeDP.PtpServer()->Printer()->DpsObjectReceived(iReceivedObjectP->Uint(CMTPObjectMetaData::EHandle));
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
    
    __FLOG_2(_L8("<< CMTPPictBridgeDpSendObjectInfo::DoHandleCompletingPhaseL result:%d progress %d"),result,iProgress);    
    return result;    
    }

/**
Verify if the SendObject request comes after SendObjectInfo request
@return EMTPRespCodeOK if SendObject request comes after a valid SendObjectInfo request, otherwise
EMTPRespCodeNoValidObjectInfo
*/
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::CheckSendingStateL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::CheckSendingStateL"));    
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
            // Not finished SendObjectInfo/SendObject pair detected, need to remove the object reservation that was created, unless the object already existed
            if (!iNoRollback )
                {
                __FLOG(_L8("  CMTPPictBridgeDpSendObjectInfo::CheckSendingStateL ... Rolling back!"));
                Rollback();
                }

            delete iObjectInfoP;
            iObjectInfoP = NULL;
            delete iObjectPropList;
            iObjectPropList = NULL;
            iProgress = EObjectNone;
            }
        }
    else 
        {
        User::Leave( KErrGeneral );
        }
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::CheckSendingStateL"));    
    return result;    
    }

/**
SendObjectInfo request handler
*/
void CMTPPictBridgeDpSendObjectInfo::ServiceSendObjectInfoL()
    {
    delete iObjectInfoP;
    iObjectInfoP = NULL;
    iObjectInfoP = CMTPTypeObjectInfo::NewL();
    ReceiveDataL(*iObjectInfoP);
    iProgress = EObjectInfoInProgress;
    }

/**
ServiceSendObjectPropListL request handler
*/ 
void CMTPPictBridgeDpSendObjectInfo::ServiceSendObjectPropListL()
    {
    delete iObjectPropList;
    iObjectPropList = NULL;
    iObjectPropList = CMTPTypeObjectPropList::NewL();
    ReceiveDataL( *iObjectPropList );
    iProgress = EObjectInfoInProgress;
    }

/**
SendObject request handler
*/    
void CMTPPictBridgeDpSendObjectInfo::ServiceSendObjectL()
    {    
    ReceiveDataL(*iFileReceivedP);    
    iProgress = ESendObjectInProgress;
    }

/**
Get a default parent object, if the request does not specify a parent object.
*/
void CMTPPictBridgeDpSendObjectInfo::GetDefaultParentObjectL()
    {    
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::GetDefaultParentObjectL"));    

    if (iStorageId == KMTPStorageDefault)
        {
        iStorageId = iFramework.StorageMgr().DefaultStorageId();
        }
    TInt drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    User::LeaveIfError(drive);

    // Obtain the root of the drive.  Logical storages can sometimes have a filesystem root
    // other than <drive>:\ .  For example an MP3 DP might have a root of c:\media\music\

    delete iParentSuidP;
    iParentSuidP = NULL;
    iParentSuidP=(iFramework.StorageMgr().StorageL(iStorageId).DesC(CMTPStorageMetaData::EStorageSuid)).AllocL();
    iReceivedObjectP->SetUint(CMTPObjectMetaData::EParentHandle, KMTPHandleNoParent);
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::GetDefaultParentObjectL"));            
    }

/**
Get parent object and storage id
@return EMTPRespCodeOK if successful, otherwise, EMTPRespCodeInvalidParentObject
*/
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::GetParentObjectAndStorageIdL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::GetParentObjectAndStorageIdL"));    
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPPictBridgeDpRequestCheckNull));

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
        delete iParentSuidP;
        iParentSuidP = NULL;
        iParentSuidP = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();
        iReceivedObjectP->SetUint(CMTPObjectMetaData::EParentHandle, iParentHandle);
        }

    __FLOG_VA((_L16("<< CMTPPictBridgeDpSendObjectInfo::GetParentObjectAndStorageIdL %S"), iParentSuidP));    
    return EMTPRespCodeOK;
    }

/**
Handling the completing phase of SendObjectInfo request
@return ETrue if the specified object can be saved on the specified location, otherwise, EFalse
*/    
TBool CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectInfoCompleteL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectInfoCompleteL"));    
    TBool result(ETrue);
    TUint16 format(iObjectInfoP->Uint16L(CMTPTypeObjectInfo::EObjectFormat));
    
    result = IsFormatValid(TMTPFormatCode(format));
    
    if (result)
        {
        delete iDateModP;
        iDateModP = NULL;
        iDateModP = iObjectInfoP->StringCharsL(CMTPTypeObjectInfo::EDateModified).AllocL();
    
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
        iObjectSize = iObjectInfoP->Uint32L(CMTPTypeObjectInfo::EObjectCompressedSize);
        if (IsTooLarge(iObjectSize))
            {
            SendResponseL(EMTPRespCodeObjectTooLarge);
            result = EFalse;            
            }
        }

    if (result)
        {
        iProtectionStatus = iObjectInfoP->Uint16L(CMTPTypeObjectInfo::EProtectionStatus);
        if (iProtectionStatus !=  EMTPProtectionNoProtection &&
            iProtectionStatus != EMTPProtectionReadOnly)
            {
            SendResponseL(EMTPRespCodeParameterNotSupported);
            result = EFalse;
            }
        }

    if (result)
        {
        result = GetFullPathNameL(iObjectInfoP->StringCharsL(CMTPTypeObjectInfo::EFilename));
        if (!result)
            {        
            // File and/or parent pathname invalid.
            SendResponseL(EMTPRespCodeInvalidDataset);
            }
        }

    // pictbridge objects should be overwritten
/*
    if (result)
        {    
        result &= !Exists(iFullPath);
        if (!result)
            {        
            // Object with the same name already exists.
            __FLOG(_L8("   no rollback"));
			iNoRollback = ETrue;
            SendResponseL(EMTPRespCodeAccessDenied);
            }
        }
  */
    
    if (result)
        {
        iReceivedObjectP->SetUint(CMTPObjectMetaData::EFormatCode, format);
        iPictBridgeDP.PtpServer()->Printer()->DpsDiscovery(iFullPath, &iConnection);
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
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectInfoCompleteL"));    
    return result;    
    }


TBool CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectPropListCompleteL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectPropListCompleteL"));
    TBool result(ETrue);
    TMTPResponseCode responseCode( GetParentObjectAndStorageIdL() );
    if ( responseCode != EMTPRespCodeOK )
        {
        SendResponseL( responseCode );
        result = EFalse;
        }
    
    if ( result )
        {
        TInt invalidParameterIndex = KErrNotFound;
        responseCode = VerifyObjectPropListL( invalidParameterIndex );
        result = ( responseCode == EMTPRespCodeOK );
        if ( !result )
            {
            TUint32 parameters[4];
            parameters[0] = 0;
            parameters[1] = 0;
            parameters[2] = 0;
            parameters[3] = invalidParameterIndex;
            SendResponseL( responseCode, 4, parameters );
            }
        }

    if ( result )
        {
        iReceivedObjectP->SetUint(CMTPObjectMetaData::EFormatCode, iRequest->Uint32( TMTPTypeRequest::ERequestParameter3 ));
        iPictBridgeDP.PtpServer()->Printer()->DpsDiscovery(iFullPath, &iConnection);
        TRAPD(err, CreateFsObjectL());
        if ( err != KErrNone )
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

    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectPropListCompleteL"));
    return result;
    }

/**
*/
TBool CMTPPictBridgeDpSendObjectInfo::IsFormatValid(TMTPFormatCode aFormat) const
    {
    __FLOG_VA((_L8(">> CMTPPictGetObjectPropDesc::IsFormatValid %d"),aFormat));
    TInt count(sizeof(KMTPValidCodeExtensionMappings) / sizeof(KMTPValidCodeExtensionMappings[0]));        
    for(TInt i = 0; i < count; i++)
        {
        if (KMTPValidCodeExtensionMappings[i].iFormatCode == aFormat)
            {
            __FLOG(_L8("<< CMTPPictGetObjectPropDesc::IsFormatValid ETrue"));
            return ETrue;
            }
        }
    __FLOG(_L8("<< CMTPPictGetObjectPropDesc::IsFormatValid EFalse"));
    return EFalse;
    }
    
/**
Handling the completing phase of SendObject request
@return ETrue if the object has been successfully saved on the device, otherwise, EFalse
*/    
TBool CMTPPictBridgeDpSendObjectInfo::DoHandleSendObjectCompleteL()
    {
    __FLOG_VA((_L8(">> CMTPPictGetObjectPropDesc::DoHandleSendObjectCompleteL size=%d cancelled=%d"), iObjectSize, iCancelled));
    TBool result(ETrue);
                
    delete iFileReceivedP;
    iFileReceivedP = NULL;
        
    TEntry fileEntry;
    User::LeaveIfError(iFramework.Fs().Entry(iFullPath, fileEntry));

    if (fileEntry.FileSize() != iObjectSize)
        {
        __FLOG_VA((_L8("   sizes differ %d!=%d"),fileEntry.FileSize(), iObjectSize));
        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
         
        iFramework.Fs().Delete(iFullPath);
        iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObjectP);
        TMTPResponseCode responseCode = EMTPRespCodeObjectTooLarge;
        if (fileEntry.FileSize() < iObjectSize)
            {
            responseCode = EMTPRespCodeInvalidDataset;
            }
        SendResponseL(responseCode);
        result = EFalse;
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
        iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        Rollback();
        SendResponseL(EMTPRespCodeTransactionCancelled);        
        }
    else if (result && !iCancelled)
        {
         iFramework.RouteRequestUnregisterL(iExpectedSendObjectRequest, iConnection);
        
        //The MTP spec states that it is not mandatory for SendObjectInfo
        //to be followed by a SendObject.  An object is reserved in the ObjectStore on 
        //receiving a SendObjectInfo request, but we only commit it 
        //on receiving the corresponding SendObject request.  With Associations however 
        //we commit the object straight away as the SendObject phase is often absent 
        //with folder creation.


        TRAPD(err, iFramework.ObjectMgr().CommitReservedObjectHandleL(*iReceivedObjectP));
        __FLOG_VA((_L8(" Entry error %d"),err));        
        if( KErrAlreadyExists == err )
            {
            iReceivedObjectP->SetUint(CMTPObjectMetaData::EHandle, iFramework.ObjectMgr().HandleL(iFullPath));
            }
        else 
            {
            User::LeaveIfError(err);
            }
        SendResponseL(EMTPRespCodeOK);
        }
    __FLOG(_L8("<< CMTPPictGetObjectPropDesc::DoHandleSendObjectCompleteL"));
    return result;
    }

/**
Get the full path name of the object to be saved
@param aFileName, on entry, contains the file name of the object,
on return, contains the full path name of the object to be saved
@return ETrue if the name is valid, EFalse otherwise
*/
TBool CMTPPictBridgeDpSendObjectInfo::GetFullPathNameL(const TDesC& aFileName)
    {
    __FLOG_VA((_L16(">> CMTPPictBridgeDpSendObjectInfo::GetFullPathNameL file %S"), &aFileName));
    TBool result(EFalse);
    if (aFileName.Length() > 0)
        {
        iFullPath = *iParentSuidP;
        if (iFullPath.Length() + aFileName.Length() < iFullPath.MaxLength())
            {
            iFullPath.Append(aFileName);
            result = iFramework.Fs().IsValidName(iFullPath);
            }
        }

    __FLOG_VA((_L16("<< CMTPPictBridgeDpSendObjectInfo::GetFullPathNameL full path %S"), &iFullPath));
    return result;
    }

/**
Check if the object is too large
@return ETrue if yes, otherwise EFalse
*/
TBool CMTPPictBridgeDpSendObjectInfo::IsTooLarge(TUint64 aObjectSize) const
    {
    TBool ret(aObjectSize > KMaxTInt64);
    return ret;
    }
    
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::VerifyObjectPropListL( TInt& aInvalidParameterIndex )
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::VerifyObjectPropListL"));
    TMTPResponseCode responseCode( EMTPRespCodeOK );
    const TUint KCount( iObjectPropList->NumberOfElements() );
    iObjectPropList->ResetCursor();
    for ( TUint i(0); (i < KCount); i++ )
        {
        CMTPTypeObjectPropListElement& KElement=iObjectPropList->GetNextElementL();
        const TUint32 KHandle( KElement.Uint32L(CMTPTypeObjectPropListElement::EObjectHandle) );
        aInvalidParameterIndex = i;
        if ( KHandle != KMTPHandleNone )
            {
            responseCode = EMTPRespCodeInvalidObjectHandle;  
            break;
            }
        responseCode = CheckPropCodeL( KElement );
        if ( responseCode != EMTPRespCodeOK )
            {
            break;
            }
        responseCode = ExtractPropertyL(KElement);
        if ( responseCode != EMTPRespCodeOK )
            {
            break;
            }  
        }

    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::VerifyObjectPropListL"));
    return responseCode;
    }

TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::ExtractPropertyL( const CMTPTypeObjectPropListElement& aElement )
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::ExtractPropertyL"));
    TMTPResponseCode responseCode(EMTPRespCodeOK);
    switch ( aElement.Uint16L(CMTPTypeObjectPropListElement::EPropertyCode) )
        {
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
        delete iDateModP;
        iDateModP = NULL;
        iDateModP = aElement.StringL(CMTPTypeObjectPropListElement::EValue).AllocL();
        break;
    case EMTPObjectPropCodeName:
    	iName = aElement.StringL(CMTPTypeObjectPropListElement::EValue);
    	break;
    default:
        break;
        }

    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::ExtractPropertyL"));
    return responseCode;
    }
    
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::CheckPropCodeL( const CMTPTypeObjectPropListElement& aElement ) const
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::CheckPropCodeL"));
    TMTPResponseCode responseCode( EMTPRespCodeOK );
    switch( aElement.Uint16L( CMTPTypeObjectPropListElement::EPropertyCode ))
        {
        case EMTPObjectPropCodeStorageID:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            else if ( iStorageId != aElement.Uint32L(CMTPTypeObjectPropListElement::EValue) )
                {
                responseCode = EMTPRespCodeInvalidDataset;
                }
            }
            break;
        case EMTPObjectPropCodeObjectFormat:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
        else if (Request().Uint32(TMTPTypeRequest::ERequestParameter3) != aElement.Uint16L(CMTPTypeObjectPropListElement::EValue))
                {
                responseCode = EMTPRespCodeInvalidDataset;
                }
            }
            break;
        case EMTPObjectPropCodeObjectSize:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT64 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
        else if (iObjectSize != aElement.Uint64L(CMTPTypeObjectPropListElement::EValue))
                {
                responseCode = EMTPRespCodeInvalidDataset;
                }
            }
            break;
        case EMTPObjectPropCodeParentObject:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT32 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            else if( Request().Uint32(TMTPTypeRequest::ERequestParameter2) != aElement.Uint32L(CMTPTypeObjectPropListElement::EValue) )
                {
                responseCode = EMTPRespCodeInvalidDataset;
                }
            }
            break;
        case EMTPObjectPropCodePersistentUniqueObjectIdentifier:
            {
            responseCode = EMTPRespCodeAccessDenied;
            }
            break;
        case EMTPObjectPropCodeProtectionStatus:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT16 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            }
            break;
        case EMTPObjectPropCodeDateModified:
        case EMTPObjectPropCodeObjectFileName:
        case EMTPObjectPropCodeName:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeString )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            }
            break;
        case EMTPObjectPropCodeNonConsumable:
            {
            if ( aElement.Uint16L(CMTPTypeObjectPropListElement::EDatatype) != EMTPTypeUINT8 )
                {
                responseCode = EMTPRespCodeInvalidObjectPropFormat;
                }
            }
            break;
        default:
            {
            responseCode = EMTPRespCodeInvalidObjectPropCode;
            }
            break;
        }
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::CheckPropCodeL"));
    return responseCode; 
    }

TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::MatchStoreAndParentL() const
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::MatchStoreAndParentL"));
    TMTPResponseCode ret = EMTPRespCodeOK;
    const TUint32 storeId(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    const TUint32 parentHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter2));
    if (parentHandle != KMTPHandleAll && parentHandle != KMTPHandleNone)
        {
        CMTPObjectMetaData* parentObjInfo = iRequestChecker->GetObjectInfo(parentHandle);
        __ASSERT_DEBUG(parentObjInfo, Panic(EMTPPictBridgeDpObjectNull));

        if (parentObjInfo->Uint(CMTPObjectMetaData::EStorageId) != storeId)   
            {
            ret = EMTPRespCodeInvalidObjectHandle;
            }
        }
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::MatchStoreAndParentL"));
    return ret;
    }

/**
Reserves space for and assigns an object handle to the received object, then
sends a success response.
*/
void CMTPPictBridgeDpSendObjectInfo::ReserveObjectL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::ReserveObjectL"));    
    iReceivedObjectP->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
    iReceivedObjectP->SetDesCL(CMTPObjectMetaData::ESuid, iFullPath);
    
    iFramework.ObjectMgr().ReserveObjectHandleL(*iReceivedObjectP, iObjectSize);    
    
    iExpectedSendObjectRequest.SetUint32(TMTPTypeRequest::ERequestSessionID, iSessionId);
    iFramework.RouteRequestRegisterL(iExpectedSendObjectRequest, iConnection);
    TUint32 parameters[3];
    parameters[0] = iStorageId;
    parameters[1] = iParentHandle;
    parameters[2] = iReceivedObjectP->Uint(CMTPObjectMetaData::EHandle);
    SendResponseL(EMTPRespCodeOK, (sizeof(parameters) / sizeof(parameters[0])), parameters);
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::ReserveObjectL"));    
    }
    
void CMTPPictBridgeDpSendObjectInfo::CreateFsObjectL()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::CreateFsObjectL"));    
    delete iFileReceivedP;
    iFileReceivedP = NULL;
    iFileReceivedP = CMTPTypeFile::NewL(iFramework.Fs(), iFullPath, EFileWrite);
    iFileReceivedP->SetSizeL(iObjectSize);
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::CreateFsObjectL"));    
    }
    
void CMTPPictBridgeDpSendObjectInfo::Rollback()
    {
    __FLOG(_L8(">> CMTPPictBridgeDpSendObjectInfo::Rollback"));
    // Delete this object from file system.
    delete iFileReceivedP;
    iFileReceivedP=NULL;
    TInt err=iFramework.Fs().Delete(iFullPath);
    __FLOG_VA((_L16(">> CMTPPictBridgeDpSendObjectInfo::Rollback deleted %S with return code %d"), &iFullPath, err));
    TRAP_IGNORE(iFramework.ObjectMgr().UnreserveObjectHandleL(*iReceivedObjectP));
    __FLOG(_L8("<< CMTPPictBridgeDpSendObjectInfo::Rollback"));    
    }
    
TMTPResponseCode CMTPPictBridgeDpSendObjectInfo::ErrorToMTPError(TInt aError) const
    {
    TMTPResponseCode resp = EMTPRespCodeGeneralError;
    
    switch (aError)
        {
    case KErrAccessDenied:
        resp = EMTPRespCodeAccessDenied;
        break;
        
    case KErrDiskFull:
        resp = EMTPRespCodeStoreFull;
        break;

    default:
        resp = EMTPRespCodeGeneralError;
        break;
        }
        
    return resp;
    }

