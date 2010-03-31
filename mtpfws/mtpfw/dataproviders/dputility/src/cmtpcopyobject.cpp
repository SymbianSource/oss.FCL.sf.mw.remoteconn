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

#include "cmtpfsentrycache.h"
#include "cmtpstoragemgr.h"
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
	__FLOG(_L8("~CMTPCopyObject - Entry"));
	Cancel();
	iDpSingletons.Close();
	iSingletons.Close();
	
	delete iTimer;
	delete iDest;
	delete iNewFileName;
	delete iFileMan;
	
	__FLOG(_L8("~CMTPCopyObject - Exit"));
	__FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPCopyObject::CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy),
	iTimer(NULL)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	}



TMTPResponseCode CMTPCopyObject::CheckRequestL()
	{
	__FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
	if ( (EMTPRespCodeOK == result) && (!iSingletons.StorageMgr().IsReadWriteStorage(Request().Uint32(TMTPTypeRequest::ERequestParameter2))) )
		{
		result = EMTPRespCodeStoreReadOnly;
		}
	if(result == EMTPRespCodeOK)
		{
		const TUint32 KHandle(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
		CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
		if(iFramework.ObjectMgr().ObjectL(KHandle, *object))
			{
			const TDesC& suid(object->DesC(CMTPObjectMetaData::ESuid));
			iIsFolder = EFalse;
			User::LeaveIfError(BaflUtils::IsFolder(iFramework.Fs(), suid, iIsFolder));
			if(!iIsFolder)
				{
				if(iDpSingletons.CopyingBigFileCache().IsOnGoing())
					{
					__FLOG(_L8("CheckRequestL - A big file copying is ongoing, respond with access denied"));
					result = EMTPRespCodeAccessDenied;
					}
				}
			}
		CleanupStack::PopAndDestroy(object); 
		}
	__FLOG(_L8("CheckRequestL - Exit"));
	return result;	
	} 

/**
CopyObject request handler
*/		
void CMTPCopyObject::ServiceL()
	{	
	__FLOG(_L8("ServiceL - Entry"));
	TUint32 handle = KMTPHandleNone;
	TMTPResponseCode responseCode = CopyObjectL(handle);
	if(responseCode != EMTPRespCodeOK)
		{
		__FLOG_VA((_L8("ServiceL, sending response with respond code %d"), responseCode));
		SendResponseL(responseCode);
		}
	else if (iIsFolder)
		{
		__FLOG_VA((_L8("ServiceL, sending response with handle=%d, respond code OK"), handle));
		SendResponseL(EMTPRespCodeOK, 1, &handle);
		}
	__FLOG(_L8("ServiceL - Exit"));
	}


/**
 Second phase constructor
*/
void CMTPCopyObject::ConstructL()
	{
	iSingletons.OpenL();
	iDpSingletons.OpenL(iFramework);
	}

	
/**
A helper function of CopyObjectL.
@param aNewFileName the new full filename after copy.
@return objectHandle of new copy of object.
*/
void CMTPCopyObject::CopyFileL(const TDesC& aNewFileName)
	{
	__FLOG(_L8("CopyFileL - Entry"));
	delete iNewFileName;
	iNewFileName = NULL;
	iNewFileName = aNewFileName.AllocL(); // Store the new file name	
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	GetPreviousPropertiesL(suid);
	
	User::LeaveIfError(iFileMan->Copy(suid, *iDest, CFileMan::EOverWrite, iStatus));
	if ( !IsActive() )
	{  
	SetActive();
	}
	
	delete iTimer;
	iTimer = NULL;
	iTimer = CPeriodic::NewL(EPriorityStandard);
	TTimeIntervalMicroSeconds32 KCopyObjectIntervalNone = 0;	
	iTimer->Start(TTimeIntervalMicroSeconds32(KCopyObjectTimeOut), KCopyObjectIntervalNone, TCallBack(CMTPCopyObject::OnTimeoutL, this));
	
	__FLOG(_L8("CopyFileL - Exit"));
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
	
	if(!iIsFolder)
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
		
		if(!iIsFolder) // It is a file.
			{
			CopyFileL(newObjectName);
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

/**
 Call back function, called when the timer expired for big file copying.
 Send response to initiator and cache the target file entry info, which is used to send response 
 to getobjectproplist and getobjectinfo.
*/
TInt CMTPCopyObject::OnTimeoutL(TAny* aPtr)
	{
	CMTPCopyObject* copyObjectProcessor = static_cast<CMTPCopyObject*>(aPtr);
	copyObjectProcessor->DoOnTimeoutL();
	return KErrNone;
	}

void CMTPCopyObject::DoOnTimeoutL()
	{
	__FLOG(_L8("DoOnTimeoutL - Entry"));
	
	if (iTimer)
		{
		if (iTimer->IsActive())
			{
			iTimer->Cancel();
			}
		delete iTimer;
		iTimer = NULL;
		}
	
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	TEntry fileEntry;
	User::LeaveIfError(iFramework.Fs().Entry(suid, fileEntry));
	TUint32 handle = KMTPHandleNone;
	handle = UpdateObjectInfoL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	
	// Cache the target file entry info, which is used to send response to getobjectproplist and getobjectinfo
	aCache.SetOnGoing(ETrue);
	aCache.SetTargetHandle(handle);
	aCache.SetFileEntry(fileEntry);
	
	__FLOG_VA((_L8("UpdateFSEntryCache, sending response with handle=%d, respond code OK for a big file copy"), handle));
	SendResponseL(EMTPRespCodeOK, 1, &handle);
	
	__FLOG(_L8("DoOnTimeoutL - Exit"));
	}

/**
 CMTPCopyObject::RunL
*/
void CMTPCopyObject::RunL()
	{
	__FLOG(_L8("RunL - Entry"));
	
	User::LeaveIfError(iStatus.Int());
	SetPreviousPropertiesL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	// Check to see if we are copying a big file
	if(aCache.IsOnGoing())
		{
		__FLOG(_L8("RunL - Big file copy complete"));
		aCache.SetOnGoing(EFalse);
		aCache.SetTargetHandle(KMTPHandleNone);
		}	
	else
		{
		//Cancel the timer
		if(iTimer)
			{
			if (iTimer->IsActive())
				{
				iTimer->Cancel();
				}
			delete iTimer;
			iTimer = NULL;
			}
		
		TUint32 handle = UpdateObjectInfoL(*iNewFileName);
		__FLOG_VA((_L8("RunL, sending response with handle=%d, respond code OK for a normal file copy"), handle));
		SendResponseL(EMTPRespCodeOK, 1, &handle);
		}
	__FLOG(_L8("RunL - Exit"));
	}

/**
Override to handle the complete phase of copy object
@return EFalse
*/
TBool CMTPCopyObject::DoHandleCompletingPhaseL()
	{
	CMTPRequestProcessor::DoHandleCompletingPhaseL();
	
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	if(aCache.IsOnGoing())
		{
		return EFalse;
		}
	else
		{
		return ETrue;
		}
	}

/**
Override to match CopyObject request
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPCopyObject::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
	{
	__FLOG(_L8("Match - Entry"));
	TBool result = EFalse;
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	if ((operationCode == EMTPOpCodeCopyObject) && &iConnection == &aConnection)
	{
	result = ETrue;
	}    
	__FLOG_VA((_L8("Match -- Exit with result = %d"), result));
	return result;    
	}
