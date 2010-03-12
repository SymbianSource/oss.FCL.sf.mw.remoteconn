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
#include <pathinfo.h> // PathInfo
#include <sysutil.h>

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypestring.h>

#include "cmtpimagedpmoveobject.h"
#include "mtpimagedppanic.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedpthumbnailcreator.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"MoveObject");)

/**
Verification data for the MoveObject request
*/    
const TMTPRequestElementInfo KMTPMoveObjectPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeStorageId, EMTPElementAttrWrite, 0, 0, 0},
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir | EMTPElementAttrWrite, 1, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
MMTPRequestProcessor* CMTPImageDpMoveObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpMoveObject* self = new (ELeave) CMTPImageDpMoveObject(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/	
CMTPImageDpMoveObject::~CMTPImageDpMoveObject()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::~CMTPImageDpMoveObject")); 
    delete iDest;
    delete iFileMan;
    delete iObjectInfo;
    __FLOG(_L8("<< CMTPImageDpMoveObject::~CMTPImageDpMoveObject"));
    __FLOG_CLOSE;
    }

/**
Standard c++ constructor
*/	
CMTPImageDpMoveObject::CMTPImageDpMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy),
    iDataProvider(aDataProvider)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }
	
/**
 Second phase constructor
*/
void CMTPImageDpMoveObject::ConstructL()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::ConstructL")); 
    iFileMan = CFileMan::NewL(iFramework.Fs());
    iObjectInfo = CMTPObjectMetaData::NewL();
    __FLOG(_L8("<< CMTPImageDpMoveObject::ConstructL")); 
    }

TMTPResponseCode CMTPImageDpMoveObject::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::CheckRequestL"));
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    if (EMTPRespCodeOK == responseCode)
        {
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        // Check whether object handle is valid
        responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *iObjectInfo);
        }
    else if(EMTPRespCodeInvalidObjectHandle == responseCode) //we only check the parent handle
        {
        responseCode = EMTPRespCodeInvalidParentObject;
        }
    
    __FLOG_VA((_L8("CheckRequestL - Exit with responseCode = 0x%04X"), responseCode));
    __FLOG(_L8("<< CMTPImageDpCopyObject::CheckRequestL"));
    return responseCode;
    }

/**
MoveObject request handler
*/		
void CMTPImageDpMoveObject::ServiceL()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::ServiceL"));
    
    TMTPResponseCode ret = MoveObjectL();
    SendResponseL(ret);
    
    __FLOG(_L8("<< CMTPImageDpMoveObject::ServiceL")); 
    }

/**
A helper function of MoveObjectL.
@param aNewFileName the new file name after the object is moved.
*/
TMTPResponseCode CMTPImageDpMoveObject::MoveFileL(const TDesC& aOldFileName, const TDesC& aNewFileName)	
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::MoveFileL"));
    
    TMTPResponseCode responseCode = EMTPRespCodeOK;    
    /**
     * File system process the move operation has two different ways:
     * 1. move object between the same storage.
     *    under this situation, move operatoin will triggers the EFsRename event by file system.
     *    then harvester of MdS will directly update the uri of mde object.      
     *    
     * 2. move object tetween the different storage.
     *    uder this situation, move operation will triggers four events by file system:
     *    a. EFsFileReplace event for old uri object.
     *    b. EFsFileReplace event for new uri object.
     *    c. EFsFileSetModified event for DateModified property of object.
     *    d. EFsDelete event for old uri object.
     *    
     *    then harvester of MdS will generate four actions for the above events:
     *    a. EMdsFileReplaced action for old uri object, this action is a null operation, because no object associates with old uri.
     *    b. EMdsFileReplaced action for new uri object, this action will add a new mde object into database and 
     *       put this action into queue of image plugin to wait for execution.
     *    c. EmdsFileModified action for new object, this action will update the DateModified property of mde object and
     *       put this action into queue of image plugin to wait for execution.
     *    d. EmdsFileDeleted action for old object, this action will directly delete old mde object from database.
     */
    
    /**
     * image dp should update object properties in database to avoid race condition with MdS,
     * if move faile, it should rollback.
     */
    
    TUint oldStoradId = iObjectInfo->Uint(CMTPObjectMetaData::EStorageId);
    TUint oldParentHandle = iObjectInfo->Uint(CMTPObjectMetaData::EParentHandle);    
    iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewFileName);
    iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
    iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
    iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);
    __FLOG_VA((_L16("CMTPImageDpMoveObject::MoveFileL - Update object info:%S"), &aNewFileName));
    
    TInt ret = MoveImageFile(aOldFileName, *iDest);
    if (ret != KErrNone)
        {
        //rollback
        __FLOG_VA((_L16("CMTPImageDpMoveObject::MoveFileL - Rollback")));
        iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aOldFileName);
        iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, oldStoradId);
        iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, oldParentHandle);
        iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);       
        responseCode = EMTPRespCodeGeneralError;        
        }
    __FLOG_VA((_L8("CMTPImageDpMoveObject::MoveFileL - MoveImageFile:%d"), ret));
    
    __FLOG(_L8("<< CMTPImageDpMoveObject::MoveFileL")); 	
    return responseCode;
    }
/**
move object operations
@return A valid MTP response code.
*/
TMTPResponseCode CMTPImageDpMoveObject::MoveObjectL()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::MoveObjectL"));
    TMTPResponseCode responseCode = EMTPRespCodeOK;
    GetParametersL();
    RBuf newObjectName;
    newObjectName.CreateL(KMaxFileName);
    newObjectName.CleanupClosePushL();
    newObjectName = *iDest;
    TFileName oldFileName = iObjectInfo->DesC(CMTPObjectMetaData::ESuid);
    TParsePtrC fileNameParser(oldFileName);
    if((newObjectName.Length() + fileNameParser.NameAndExt().Length()) <= newObjectName.MaxLength())
        {
        newObjectName.Append(fileNameParser.NameAndExt());
        responseCode = CanMoveObjectL(oldFileName, newObjectName);
        }
    else
        {
        responseCode = EMTPRespCodeGeneralError;
        }
        
    if(responseCode == EMTPRespCodeOK)
        {
        responseCode = MoveFileL(oldFileName, newObjectName);
        }
    CleanupStack::PopAndDestroy(); // newObjectName.
    __FLOG(_L8("<< CMTPImageDpMoveObject::MoveObjectL")); 	
    return responseCode;
    }

/**
Retrieve the parameters of the request
*/	
void CMTPImageDpMoveObject::GetParametersL()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::GetParametersL"));
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPImageDpRequestCheckNull));
    
    TUint32 objectHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    iNewParentHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
    
    if(iNewParentHandle == 0)
        {
        SetDefaultParentObjectL();
        }
    else	
        {
        CMTPObjectMetaData* parentObjectInfo = iRequestChecker->GetObjectInfo(iNewParentHandle);
        __ASSERT_DEBUG(parentObjectInfo, Panic(EMTPImageDpObjectNull));
        delete iDest;
        iDest = NULL;
        iDest = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();
        }
    __FLOG(_L8("<< CMTPImageDpMoveObject::GetParametersL"));
    }
    
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPImageDpMoveObject::SetDefaultParentObjectL()
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::SetDefaultParentObjectL"));  
    const CMTPStorageMetaData& storage = iFramework.StorageMgr().StorageL(iStorageId);
    delete iDest;
    iDest = NULL;
    iDest = storage.DesC(CMTPStorageMetaData::EStorageSuid).AllocL();
    iNewParentHandle = KMTPHandleNoParent;
    __FLOG(_L8("<< CMTPImageDpMoveObject::SetDefaultParentObjectL"));  
    }

/**
Check if we can move the file to the new location
*/
TMTPResponseCode CMTPImageDpMoveObject::CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::CanMoveObjectL"));     
    TMTPResponseCode result = EMTPRespCodeOK;

    TEntry fileEntry;
    User::LeaveIfError(iFramework.Fs().Entry(aOldName, fileEntry));
    TDriveNumber drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    User::LeaveIfError(drive);
    TVolumeInfo volumeInfo;
    User::LeaveIfError(iFramework.Fs().Volume(volumeInfo, drive));
    
    if(volumeInfo.iFree < fileEntry.FileSize())
        {
        result = EMTPRespCodeStoreFull;
        }
    else if (BaflUtils::FileExists(iFramework.Fs(), aNewName))			
        {
        result = EMTPRespCodeInvalidParentObject;
        }
    __FLOG_VA((_L8("CanMoveObjectL - Exit with response code 0x%04X"), result));
    __FLOG(_L8("<< CMTPImageDpMoveObject::CanMoveObjectL"));     
    return result;	
    }

TInt CMTPImageDpMoveObject::MoveImageFile(const TDesC& aOldImageName, const TDesC& aNewImageName)
    {
    __FLOG(_L8(">> CMTPImageDpMoveObject::MoveImageFile"));
    __FLOG_VA((_L8("move image src: %S dest: %S"), &aOldImageName, &aNewImageName));        
    __FLOG(_L8("<< CMTPImageDpMoveObject::MoveImageFile"));
    return iFileMan->Move(aOldImageName, aNewImageName);
    }

// End of file
