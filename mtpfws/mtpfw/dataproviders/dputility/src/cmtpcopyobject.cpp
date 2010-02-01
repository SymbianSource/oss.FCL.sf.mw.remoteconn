// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mmtpstoragemgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/cmtptypestring.h>

#include "cmtpcopyobject.h"
#include "mtpdppanic.h"

__FLOG_STMT(_LIT8(KComponent,"CopyObject");)

/**
Verification data for the CopyObject request
*/
const TMTPRequestElementInfo KMTPCopyObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir, 0, 0, 0},   	
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
EXPORT_C MMTPRequestProcessor* CMTPCopyObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPCopyObject* self = new (ELeave) CMTPCopyObject(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);	
	return self;
	}


/**
Destructor
*/	
EXPORT_C CMTPCopyObject::~CMTPCopyObject()
	{	
	delete iDest;
	delete iFileMan;

	__FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPCopyObject::CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	}


/**
CopyObject request handler
*/		
void CMTPCopyObject::ServiceL()
	{	
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
	}


/**
 Second phase constructor
*/
void CMTPCopyObject::ConstructL()
    {

    }

	
/**
A helper function of CopyObjectL.
@param aNewFileName the new full filename after copy.
@return objectHandle of new copy of object.
*/
TUint32 CMTPCopyObject::CopyFileL(const TDesC& aNewFileName)
	{
	__FLOG(_L8("CopyFileL - Entry"));
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	GetPreviousPropertiesL(suid);
	User::LeaveIfError(iFileMan->Copy(suid, *iDest));
	SetPreviousPropertiesL(aNewFileName);
	
	TUint32 handle = UpdateObjectInfoL(aNewFileName);
	
	__FLOG(_L8("CopyFileL - Exit"));
	
	return handle;
	}

/**
A helper function of CopyObjectL.
@param aNewFolderName the new full file folder name after copy.
@return objecthandle of new copy of the folder.
*/
TUint32 CMTPCopyObject::CopyFolderL(const TDesC& aNewFolderName)
	{
	__FLOG(_L8("CopyFolderL - Entry"));
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	TUint32 handle;
	if (iObjectInfo->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId())
		{
		GetPreviousPropertiesL(suid);
		User::LeaveIfError(iFramework.Fs().MkDir(aNewFolderName));
		SetPreviousPropertiesL(aNewFolderName);	
		handle = UpdateObjectInfoL(aNewFolderName);
		}
	else
		{
		handle = iFramework.ObjectMgr().HandleL(aNewFolderName);
		}
	__FLOG(_L8("CopyFolderL - Exit"));
	return handle;
	}
		
/**
Copy object operation
@return the object handle of the resulting object.
*/
TMTPResponseCode CMTPCopyObject::CopyObjectL(TUint32& aNewHandle)
	{
	__FLOG(_L8("CopyObjectL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	aNewHandle = KMTPHandleNone;
	
	GetParametersL();
			
	RBuf newObjectName;
	newObjectName.CleanupClosePushL();
	newObjectName.CreateL(KMaxFileName);
	newObjectName = *iDest;
	
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	TParsePtrC fileNameParser(suid);
	
	// Check if the object is a folder or a file.
	TBool isFolder = EFalse;
	User::LeaveIfError(BaflUtils::IsFolder(iFramework.Fs(), suid, isFolder));	
	
	if(!isFolder)
		{
		if((newObjectName.Length() + fileNameParser.NameAndExt().Length()) <= newObjectName.MaxLength())
			{
			newObjectName.Append(fileNameParser.NameAndExt());
			}
		}
	else // It is a folder.
		{
		TFileName rightMostFolderName;
		User::LeaveIfError(BaflUtils::MostSignificantPartOfFullName(suid, rightMostFolderName));
		if((newObjectName.Length() + rightMostFolderName.Length() + 1) <= newObjectName.MaxLength())
			{
			newObjectName.Append(rightMostFolderName);
			// Add backslash.
			_LIT(KBackSlash, "\\");
			newObjectName.Append(KBackSlash);
			}
		}
	responseCode = CanCopyObjectL(suid, newObjectName);    
	if(responseCode == EMTPRespCodeOK)
		{			
		delete iFileMan;
		iFileMan = NULL;
		iFileMan = CFileMan::NewL(iFramework.Fs());
		
		if(!isFolder) // It is a file.
			{
			aNewHandle = CopyFileL(newObjectName);
			}
		else // It is a folder.
			{
			aNewHandle = CopyFolderL(newObjectName);
			}
		}
	
	CleanupStack::PopAndDestroy(); // newObjectName.
	__FLOG(_L8("CopyObjectL - Exit"));
	return responseCode;
	}

/**
Retrieve the parameters of the request
*/	
void CMTPCopyObject::GetParametersL()
	{
	__FLOG(_L8("GetParametersL - Entry"));
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	
	TUint32 objectHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	TUint32 parentObjectHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter3);
	
	//not taking owernship
	iObjectInfo = iRequestChecker->GetObjectInfo(objectHandle); 
	__ASSERT_DEBUG(iObjectInfo, Panic(EMTPDpObjectNull));	

	if(parentObjectHandle == 0)
		{
		SetDefaultParentObjectL();
		}
	else	
		{
		CMTPObjectMetaData* parentObjectInfo = iRequestChecker->GetObjectInfo(parentObjectHandle);
		__ASSERT_DEBUG(parentObjectInfo, Panic(EMTPDpObjectNull));
		delete iDest;
		iDest = NULL;
		iDest = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();
		iNewParentHandle = parentObjectHandle;
		}
	__FLOG(_L8("GetParametersL - Exit"));	
	}
	
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPCopyObject::SetDefaultParentObjectL()
	{
	__FLOG(_L8("SetDefaultParentObjectL - Entry"));

	const CMTPStorageMetaData& storageMetaData( iFramework.StorageMgr().StorageL(iStorageId) );
	const TDesC& driveBuf( storageMetaData.DesC(CMTPStorageMetaData::EStorageSuid) );
	delete iDest;
	iDest = NULL;
	iDest = driveBuf.AllocL();
	iNewParentHandle = KMTPHandleNoParent;
	    
	__FLOG(_L8("SetDefaultParentObjectL - Exit"));
	}

/**
Check if we can copy the file to the new location
*/
TMTPResponseCode CMTPCopyObject::CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	__FLOG(_L8("CanCopyObjectL - Entry"));
	TMTPResponseCode result = EMTPRespCodeOK;

	TEntry fileEntry;
	User::LeaveIfError(iFramework.Fs().Entry(aOldName, fileEntry));
	TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
	User::LeaveIfError(drive);
	TVolumeInfo volumeInfo;
	User::LeaveIfError(iFramework.Fs().Volume(volumeInfo, drive));
	
#ifdef SYMBIAN_ENABLE_64_BIT_FILE_SERVER_API
    if(volumeInfo.iFree < fileEntry.FileSize())
#else
    if(volumeInfo.iFree < fileEntry.iSize)
#endif
		{
		result = EMTPRespCodeStoreFull;
		}
	else if (BaflUtils::FileExists(iFramework.Fs(), aNewName))			
		{
		result = EMTPRespCodeInvalidParentObject;
		}
	__FLOG_VA((_L8("CanCopyObjectL - Exit with response code 0x%04X"), result));
	return result;	
	}

/**
Save the object properties before doing the copy
*/
void CMTPCopyObject::GetPreviousPropertiesL(const TDesC& aFileName)
	{
	__FLOG(_L8("GetPreviousPropertiesL - Entry"));
	User::LeaveIfError(iFramework.Fs().Modified(aFileName, iPreviousModifiedTime));
	__FLOG(_L8("GetPreviousPropertiesL - Exit"));
	}

/**
Set the object properties after doing the copy
*/
void CMTPCopyObject::SetPreviousPropertiesL(const TDesC& aFileName)
	{
	__FLOG(_L8("SetPreviousPropertiesL - Entry"));
	User::LeaveIfError(iFramework.Fs().SetModified(aFileName, iPreviousModifiedTime));
	__FLOG(_L8("SetPreviousPropertiesL - Exit"));
	}

/**
 Update object info in the database.
*/
TUint32 CMTPCopyObject::UpdateObjectInfoL(const TDesC& aNewObjectName)
	{
	__FLOG(_L8("UpdateObjectInfoL - Entry"));	
	
	// We should not modify this object's handle, so just get a "copy".
	CMTPObjectMetaData* objectInfo(CMTPObjectMetaData::NewLC());
	const TMTPTypeUint32 objectHandle(iObjectInfo->Uint(CMTPObjectMetaData::EHandle));
	if(iFramework.ObjectMgr().ObjectL(objectHandle, *objectInfo))
		{
		objectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewObjectName);
		objectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
		//Modify storage Id.
		objectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
		iFramework.ObjectMgr().InsertObjectL(*objectInfo);
		}
	else
		{
		User::Leave(KErrCorrupt);
		}
	TUint32 handle = objectInfo->Uint(CMTPObjectMetaData::EHandle);	
	CleanupStack::PopAndDestroy(objectInfo);
	
	__FLOG(_L8("UpdateObjectInfoL - Exit"));
	
	return handle;	
	}
