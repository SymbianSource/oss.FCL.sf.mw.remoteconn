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

#include "cmtpimagedpcopyobject.h"
#include "mtpimagedppanic.h"
#include "mtpimagedputilits.h"
#include "cmtpimagedp.h"

__FLOG_STMT(_LIT8(KComponent,"CopyObject");)

const TInt RollbackFuncCnt = 1;

/**
Verification data for the CopyObject request
*/
const TMTPRequestElementInfo KMTPCopyObjectPolicy[] = 
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
MMTPRequestProcessor* CMTPImageDpCopyObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpCopyObject* self = new (ELeave) CMTPImageDpCopyObject(aFramework, aConnection,aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor
*/	
CMTPImageDpCopyObject::~CMTPImageDpCopyObject()
    {	
    __FLOG(_L8(">> CMTPImageDpCopyObject::~CMTPImageDpCopyObject"));
    delete iDest;
    delete iFileMan;
    delete iSrcObjectInfo;
    delete iTargetObjectInfo;
    iRollbackActionL.Close();
    __FLOG(_L8("<< CMTPImageDpCopyObject::~CMTPImageDpCopyObject"));
    __FLOG_CLOSE;
    
    }
    
/**
Standard c++ constructor
*/	
CMTPImageDpCopyObject::CMTPImageDpCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy),
    iFramework(aFramework),
    iDataProvider(aDataProvider)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    }
    
/**
Second phase constructor
*/
void CMTPImageDpCopyObject::ConstructL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::ConstructL"));
    iFileMan = CFileMan::NewL(iFramework.Fs());
    iSrcObjectInfo = CMTPObjectMetaData::NewL();
    iRollbackActionL.ReserveL(RollbackFuncCnt);
    __FLOG(_L8("<< CMTPImageDpCopyObject::ConstructL"));
    }

TMTPResponseCode CMTPImageDpCopyObject::CheckRequestL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::CheckRequestL"));
    TMTPResponseCode responseCode = CMTPRequestProcessor::CheckRequestL();
    if (EMTPRespCodeOK == responseCode)
        {
        TUint32 objectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
        // Check whether object handle is valid
        responseCode = MTPImageDpUtilits::VerifyObjectHandleL(iFramework, objectHandle, *iSrcObjectInfo);
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
CopyObject request handler
*/      
void CMTPImageDpCopyObject::ServiceL()
    {   
    __FLOG(_L8(">> CMTPImageDpCopyObject::ServiceL"));
    TUint32 handle = KMTPHandleNone;
    TMTPResponseCode responseCode = CopyObjectL(handle);
    if(responseCode == EMTPRespCodeOK)
        {
        SendResponseL(EMTPRespCodeOK, 1, &handle);
        }
    else
        {
        SendResponseL(responseCode);
        }
    __FLOG(_L8("<< CMTPImageDpCopyObject::ServiceL"));
    }
    
/**
Copy object operation
@return the object handle of the resulting object.
*/
TMTPResponseCode CMTPImageDpCopyObject::CopyObjectL(TUint32& aNewHandle)
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::CopyObjectL"));
    TMTPResponseCode responseCode = EMTPRespCodeOK;
    aNewHandle = KMTPHandleNone;
    
    GetParametersL();
    
    iNewFileName.Append(*iDest);
    const TDesC& oldFileName = iSrcObjectInfo->DesC(CMTPObjectMetaData::ESuid);
    TParsePtrC fileNameParser(oldFileName);
    
    if((iNewFileName.Length() + fileNameParser.NameAndExt().Length()) <= iNewFileName.MaxLength())
        {
        iNewFileName.Append(fileNameParser.NameAndExt());
        responseCode = CanCopyObjectL(oldFileName, iNewFileName);	
        }
    else
        {
        responseCode = EMTPRespCodeGeneralError;
        }
        
    
    if(responseCode == EMTPRespCodeOK)
        {
        aNewHandle = CopyFileL(oldFileName, iNewFileName);
        }
    __FLOG(_L8("<< CMTPImageDpCopyObject::CopyObjectL"));
    return responseCode;
    }

/**
A helper function of CopyObjectL.
@param aNewFileName the new full filename after copy.
@return objectHandle of new copy of object.
*/
TUint32 CMTPImageDpCopyObject::CopyFileL(const TDesC& aOldFileName, const TDesC& aNewFileName)
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::CopyFileL"));
    TCleanupItem anItem(FailRecover, reinterpret_cast<TAny*>(this));
    CleanupStack::PushL(anItem);
    
    GetPreviousPropertiesL(aOldFileName);
    User::LeaveIfError(iFileMan->Copy(aOldFileName, *iDest));
    iRollbackActionL.Append(RollBackFromFsL);
    SetPreviousPropertiesL(aNewFileName);
    
    iFramework.ObjectMgr().InsertObjectL(*iTargetObjectInfo);
    //check object whether it is a new image object
    if (MTPImageDpUtilits::IsNewPicture(*iTargetObjectInfo))
        {
        //increate new pictures count
        iDataProvider.IncreaseNewPictures(1);
        }    
    
    __FLOG(_L8("<< CMTPImageDpCopyObject::CopyFileL"));
    CleanupStack::Pop(this);
    return iTargetObjectInfo->Uint(CMTPObjectMetaData::EHandle);
    }

/**
Retrieve the parameters of the request
*/	
void CMTPImageDpCopyObject::GetParametersL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::GetParametersL"));
    __ASSERT_DEBUG(iRequestChecker, Panic(EMTPImageDpRequestCheckNull));
    
    TUint32 objectHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
    iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
    TUint32 parentObjectHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter3);       
    
    if(parentObjectHandle == 0)
        {
        SetDefaultParentObjectL();
        }
    else	
        {
        CMTPObjectMetaData* parentObjectInfo = iRequestChecker->GetObjectInfo(parentObjectHandle);
        __ASSERT_DEBUG(parentObjectInfo, Panic(EMTPImageDpObjectNull));
        delete iDest;
        iDest = NULL;
        iDest = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();        
        iNewParentHandle = parentObjectHandle;
        }
    __FLOG(_L8("<< CMTPImageDpCopyObject::GetParametersL"));	
    }
    
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPImageDpCopyObject::SetDefaultParentObjectL()
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::SetDefaultParentObjectL"));
    TDriveNumber drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    User::LeaveIfError(drive);
    TChar driveLetter;
    iFramework.Fs().DriveToChar(drive, driveLetter);
    TFileName driveBuf;
    driveBuf.Append(driveLetter);
    driveBuf = BaflUtils::RootFolderPath(driveBuf.Left(1));
    delete iDest;
    iDest = NULL;
    iDest = driveBuf.AllocL();
    iNewParentHandle = KMTPHandleNoParent;
    __FLOG(_L8("<< CMTPImageDpCopyObject::SetDefaultParentObjectL"));
    }
    
/**
Check if we can copy the file to the new location
*/
TMTPResponseCode CMTPImageDpCopyObject::CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const
    {
    __FLOG(_L8(">> CMTPImageDpCopyObject::CanCopyObjectL"));
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
    __FLOG_VA((_L8("CanCopyObjectL - Exit with response code 0x%04X"), result));
    __FLOG(_L8("<< CMTPImageDpCopyObject::CanCopyObjectL"));  	
    return result;	
    }
    
/**
Save the object properties before doing the copy
*/
void CMTPImageDpCopyObject::GetPreviousPropertiesL(const TDesC& aOldFileName)
    {
    __FLOG(_L8("GetPreviousPropertiesL - Entry"));
    User::LeaveIfError(iFramework.Fs().Modified(aOldFileName, iDateModified));
    __FLOG(_L8("GetPreviousPropertiesL - Exit"));
    }
    
/**
Set the object properties after doing the copy
*/
void CMTPImageDpCopyObject::SetPreviousPropertiesL(const TDesC& aNewFileName)
    {
    __FLOG(_L8("SetPreviousPropertiesL - Entry"));        
    User::LeaveIfError(iFramework.Fs().SetModified(aNewFileName, iDateModified));
    
    iTargetObjectInfo = CMTPObjectMetaData::NewL();
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EDataProviderId, iSrcObjectInfo->Uint(CMTPObjectMetaData::EDataProviderId));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EFormatCode, iSrcObjectInfo->Uint(CMTPObjectMetaData::EFormatCode));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EFormatSubCode, iSrcObjectInfo->Uint(CMTPObjectMetaData::EFormatSubCode));
    iTargetObjectInfo->SetDesCL(CMTPObjectMetaData::EName, iSrcObjectInfo->DesC(CMTPObjectMetaData::EName));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::ENonConsumable, iSrcObjectInfo->Uint(CMTPObjectMetaData::ENonConsumable));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
    iTargetObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewFileName);
    __FLOG(_L8("SetPreviousPropertiesL - Exit"));
    }

void CMTPImageDpCopyObject::FailRecover(TAny* aCopyOperation)
    {
    reinterpret_cast<CMTPImageDpCopyObject*>(aCopyOperation)->RollBack();
    }

void CMTPImageDpCopyObject::RollBack()
    {
    TInt i = iRollbackActionL.Count();
    while(-- i >= 0)
        {
        TRAP_IGNORE((*iRollbackActionL[i])(this));
        }
    iRollbackActionL.Reset();
    }

void CMTPImageDpCopyObject::RollBackFromFsL()
    {
    User::LeaveIfError(iFramework.Fs().Delete(iNewFileName));
    }

void CMTPImageDpCopyObject::RollBackFromFsL(CMTPImageDpCopyObject* aObject)
    {
    aObject->RollBackFromFsL();
    }

// End of file

