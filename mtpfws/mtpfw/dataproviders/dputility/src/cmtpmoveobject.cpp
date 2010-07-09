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
#include "cmtpmoveobject.h"
#include "mtpdppanic.h"


__FLOG_STMT(_LIT8(KComponent,"MoveObject");)

/**
Verification data for the MoveObject request
*/    
const TMTPRequestElementInfo KMTPMoveObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir, 0, 0, 0},   	
        {TMTPTypeRequest::ERequestParameter2, EMTPElementTypeStorageId, EMTPElementAttrWrite, 0, 0, 0},                
        {TMTPTypeRequest::ERequestParameter3, EMTPElementTypeObjectHandle, EMTPElementAttrDir, 1, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/     
EXPORT_C MMTPRequestProcessor* CMTPMoveObject::NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection)
	{
	CMTPMoveObject* self = new (ELeave) CMTPMoveObject(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);	
	return self;
	}


/**
Destructor
*/	
EXPORT_C CMTPMoveObject::~CMTPMoveObject()
	{
	Cancel();
	iDpSingletons.Close();
	iSingletons.Close();
	
	delete iTimer;
	delete iNewFileName;
	delete iDest;
	delete iFileMan;
	delete iPathToMove;
	delete iNewRootFolder;	
	__FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPMoveObject::CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy),
	iMoveObjectIndex(0), iTimer(NULL)
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	}

TMTPResponseCode CMTPMoveObject::CheckRequestL()
	{
    __FLOG(_L8("CheckRequestL - Entry"));
	TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK != result)
		{
		__FLOG(_L8("CheckRequestL with error- Exit"));
		return result;
		}
	
	const TUint32 KObjectHandle = Request().Uint32(TMTPTypeRequest::ERequestParameter1);
	//not taking owernship
	iObjectInfo = iRequestChecker->GetObjectInfo(KObjectHandle); 
	__ASSERT_DEBUG(iObjectInfo, Panic(EMTPDpObjectNull));	
	if(!iSingletons.StorageMgr().IsReadWriteStorage(iObjectInfo->Uint(CMTPObjectMetaData::EStorageId)))
		{
		result = EMTPRespCodeStoreReadOnly;
		}
	
	if ( (EMTPRespCodeOK == result) && (!iSingletons.StorageMgr().IsReadWriteStorage(Request().Uint32(TMTPTypeRequest::ERequestParameter2))) )
		{
		result = EMTPRespCodeStoreReadOnly;
		}
	
	if(result == EMTPRespCodeOK)
		{
		const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
		iIsFolder = EFalse;
		User::LeaveIfError(BaflUtils::IsFolder(iFramework.Fs(), suid, iIsFolder));
		if(!iIsFolder)
			{
			if(iDpSingletons.MovingBigFileCache().IsOnGoing())
				{
				__FLOG(_L8("CheckRequestL - A big file moving is ongoing, respond with access denied"));
				result = EMTPRespCodeAccessDenied;
				}
			}
		}
	
    __FLOG(_L8("CheckRequestL - Exit"));
	return result;	
	} 

/**
MoveObject request handler
*/		
void CMTPMoveObject::ServiceL()
	{
	__FLOG(_L8("ServiceL - Entry"));
	TMTPResponseCode ret = EMTPRespCodeOK;
	TRAPD(err, ret = MoveObjectL());
	if (err != KErrNone)
		{
		SendResponseL(EMTPRespCodeAccessDenied);
		}
	else if (EMTPRespCodeOK != ret)
		{
		SendResponseL(ret);
		}
	__FLOG(_L8("ServiceL - Exit"));
	}

/**
 Second phase constructor
*/
void CMTPMoveObject::ConstructL()
    {
	iSingletons.OpenL();
	iDpSingletons.OpenL(iFramework);
    }
    

/**
A helper function of MoveObjectL.
@param aNewFileName the new file name after the object is moved.
*/
void CMTPMoveObject::MoveFileL(const TDesC& aNewFileName)	
	{
	__FLOG(_L8("MoveFileL - Entry"));
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	GetPreviousPropertiesL(suid);
	
	if(iFramework.StorageMgr().DriveNumber(iObjectInfo->Uint(CMTPObjectMetaData::EStorageId)) ==
			iFramework.StorageMgr().DriveNumber(iStorageId))
		//Move file to the same storage
		{
		User::LeaveIfError(iFileMan->Move(suid, *iDest));
		SetPreviousPropertiesL(aNewFileName);
		iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewFileName);
		iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
		iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
		iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);
		SendResponseL(EMTPRespCodeOK);
		}
	else
		//Move file between different storages
		{
		delete iNewFileName;
		iNewFileName = NULL;
		iNewFileName = aNewFileName.AllocL(); // Store the new file name
		
		User::LeaveIfError(iFileMan->Move(suid, *iDest, CFileMan::EOverWrite, iStatus));
		if ( !IsActive() )
		{  
		SetActive();
		}
		
		delete iTimer;
		iTimer = NULL;
		iTimer = CPeriodic::NewL(EPriorityStandard);
		TTimeIntervalMicroSeconds32 KMoveObjectIntervalNone = 0;	
		iTimer->Start(TTimeIntervalMicroSeconds32(KMoveObjectTimeOut), KMoveObjectIntervalNone, TCallBack(CMTPMoveObject::OnTimeoutL, this));		
		}
	__FLOG(_L8("MoveFileL - Exit"));
	}

/**
A helper function of MoveObjectL.
@param aNewFolderName the new file folder name after the folder is moved.
*/
void CMTPMoveObject::MoveFolderL()
	{
	__FLOG(_L8("MoveFolderL - Entry"));
	
	RBuf oldFolderName;
	oldFolderName.CreateL(KMaxFileName);
	oldFolderName.CleanupClosePushL();
	oldFolderName = iObjectInfo->DesC(CMTPObjectMetaData::ESuid);
	iPathToMove = oldFolderName.AllocL();
	
	if (iObjectInfo->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId())
		{
		GetPreviousPropertiesL(oldFolderName);
		// Remove backslash.
		oldFolderName.SetLength(oldFolderName.Length() - 1);	
		SetPreviousPropertiesL(*iNewRootFolder);
		_LIT(KBackSlash, "\\");
		oldFolderName.Append(KBackSlash);	
			
		iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, *iNewRootFolder);
		iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
		iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
		iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);
		}
	
	CleanupStack::PopAndDestroy(); // oldFolderName.
		
	__FLOG(_L8("MoveFolderL - Exit"));
	}
		
/**
move object operations
@return A valid MTP response code.
*/
TMTPResponseCode CMTPMoveObject::MoveObjectL()
	{
	__FLOG(_L8("MoveObjectL - Entry"));
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	
	GetParametersL();
				
	RBuf newObjectName;
	newObjectName.CreateL(KMaxFileName);
	newObjectName.CleanupClosePushL();
	newObjectName = *iDest;
	
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	TParsePtrC fileNameParser(suid);
	
	// Check if the object is a folder or a file.
	if(!iIsFolder)
		{
		if((newObjectName.Length() + fileNameParser.NameAndExt().Length()) <= newObjectName.MaxLength())
			{
			newObjectName.Append(fileNameParser.NameAndExt());
			}
		responseCode = CanMoveObjectL(suid, newObjectName);			
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
		
	iNewRootFolder = newObjectName.AllocL();
	__FLOG(*iNewRootFolder);
		
	if(responseCode == EMTPRespCodeOK)
		{			
		delete iFileMan;
		iFileMan = NULL;
		iFileMan = CFileMan::NewL(iFramework.Fs());
		
		if(!iIsFolder)
			{
			MoveFileL(newObjectName);
			}
		else
			{		
			MoveFolderL();
			SendResponseL(responseCode);
			}
		}
	CleanupStack::PopAndDestroy(); // newObjectName.
	__FLOG(_L8("MoveObjectL - Exit"));
	return responseCode;
	}

/**
Retrieve the parameters of the request
*/	
void CMTPMoveObject::GetParametersL()
	{
	__FLOG(_L8("GetParametersL - Entry"));
	__ASSERT_DEBUG(iRequestChecker, Panic(EMTPDpRequestCheckNull));
	
	iStorageId = Request().Uint32(TMTPTypeRequest::ERequestParameter2);
	iNewParentHandle  = Request().Uint32(TMTPTypeRequest::ERequestParameter3);

	if(iNewParentHandle == 0)
		{
		SetDefaultParentObjectL();
		}
	else	
		{
		CMTPObjectMetaData* parentObjectInfo = iRequestChecker->GetObjectInfo(iNewParentHandle);
		__ASSERT_DEBUG(parentObjectInfo, Panic(EMTPDpObjectNull));
		delete iDest;
		iDest = NULL;
		iDest = parentObjectInfo->DesC(CMTPObjectMetaData::ESuid).AllocL();
		}
	__FLOG(_L8("GetParametersL - Exit"));	
	}
	
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPMoveObject::SetDefaultParentObjectL()
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
Check if we can move the file to the new location
*/
TMTPResponseCode CMTPMoveObject::CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	__FLOG(_L8("CanMoveObjectL - Entry"));
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
	__FLOG_VA((_L8("CanMoveObjectL - Exit with response code 0x%04X"), result));
	return result;	
	}

/**
Save the object properties before moving
*/
void CMTPMoveObject::GetPreviousPropertiesL(const TDesC& aFileName)
	{
	__FLOG(_L8("GetPreviousPropertiesL - Entry"));
	User::LeaveIfError(iFramework.Fs().Modified(aFileName, iPreviousModifiedTime));
	__FLOG(_L8("GetPreviousPropertiesL - Exit"));
	}

/**
Set the object properties after moving
*/
void CMTPMoveObject::SetPreviousPropertiesL(const TDesC& aFileName)
	{
	__FLOG(_L8("SetPreviousPropertiesL - Entry"));
	User::LeaveIfError(iFramework.Fs().SetModified(aFileName, iPreviousModifiedTime));
	__FLOG(_L8("SetPreviousPropertiesL - Exit"));
	}

/**
 Call back function, called when the timer expired for big file moving.
 Send response to initiator and cache the target file entry info, which is used to send response 
 to getobjectproplist and getobjectinfo.
*/
TInt CMTPMoveObject::OnTimeoutL(TAny* aPtr)
	{
	CMTPMoveObject* moveObjectProcessor = static_cast<CMTPMoveObject*>(aPtr);
	moveObjectProcessor->DoOnTimeoutL();
	return KErrNone;
	}

void CMTPMoveObject::DoOnTimeoutL()
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
	TUint32 handle = iObjectInfo->Uint(CMTPObjectMetaData::EHandle);
	
	iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, *iNewFileName);
	iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
	iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
	iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);
	
	CMTPFSEntryCache& aCache = iDpSingletons.MovingBigFileCache();
	
	// Cache the target file entry info, which is used to send response to getobjectproplist and getobjectinfo
	aCache.SetOnGoing(ETrue);
	aCache.SetTargetHandle(handle);
	aCache.SetFileEntry(fileEntry);	
	
	__FLOG(_L8("UpdateFSEntryCache, sending response with respond code OK for a big file move"));
	SendResponseL(EMTPRespCodeOK);
	
	__FLOG(_L8("DoOnTimeoutL - Exit"));
	}

/**
 CMTPMoveObject::RunL
*/
void CMTPMoveObject::RunL()
	{
	__FLOG(_L8("RunL - Entry"));
	
	User::LeaveIfError(iStatus.Int());
	SetPreviousPropertiesL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.MovingBigFileCache();
	// Check to see if we are moving a big file
	if(aCache.IsOnGoing())
		{
		__FLOG(_L8("RunL - Big file move complete"));
		aCache.SetOnGoing(EFalse);
		aCache.SetTargetHandle(KMTPHandleNone);
		}
	else
		{
		//Cancel the timer
		if(iTimer)
			{
			if(iTimer->IsActive())
				{
				iTimer->Cancel();
				}
			delete iTimer;
			iTimer = NULL;
			}

		iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, *iNewFileName);
		iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
		iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
		iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);

		__FLOG(_L8("RunL, sending response with respond code OK for a normal file move"));
		SendResponseL(EMTPRespCodeOK);
		}
	__FLOG(_L8("RunL - Exit"));
	}

/**
Override to handle the complete phase of move object
*/
TBool CMTPMoveObject::DoHandleCompletingPhaseL()
	{
	CMTPRequestProcessor::DoHandleCompletingPhaseL();
	
	CMTPFSEntryCache& aCache = iDpSingletons.MovingBigFileCache();
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
Override to match MoveObject request
@param aRequest    The request to match
@param aConnection The connection from which the request comes
@return ETrue if the processor can handle the request, otherwise EFalse
*/        
TBool CMTPMoveObject::Match(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) const
	{
	__FLOG(_L8("Match - Entry"));
	TBool result = EFalse;
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	if ((operationCode == EMTPOpCodeMoveObject) && &iConnection == &aConnection)
	{
	result = ETrue;
	}    
	__FLOG_VA((_L8("Match -- Exit with result = %d"), result));
	return result;
	}
