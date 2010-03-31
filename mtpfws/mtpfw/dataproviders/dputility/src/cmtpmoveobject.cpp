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

#include "cmtpstoragemgr.h"
#include "cmtpmoveobject.h"
#include "mtpdppanic.h"


__FLOG_STMT(_LIT8(KComponent,"MoveObject");)

/**
Verification data for the MoveObject request
*/    
const TMTPRequestElementInfo KMTPMoveObjectPolicy[] = 
    {
    	{TMTPTypeRequest::ERequestParameter1, EMTPElementTypeObjectHandle, EMTPElementAttrFileOrDir | EMTPElementAttrWrite, 0, 0, 0},   	
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
	delete iDest;
	delete iFileMan;
	delete iPathToMove;
	delete iNewRootFolder;
	iSingletons.Close();
	__FLOG_CLOSE;
	}

/**
Standard c++ constructor
*/	
CMTPMoveObject::CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy),
	iMoveObjectIndex(0)
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
	
    __FLOG(_L8("CheckRequestL - Exit"));
	return result;	
	} 

/**
MoveObject request handler
*/		
void CMTPMoveObject::ServiceL()
	{	
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
	}

/**
 Second phase constructor
*/
void CMTPMoveObject::ConstructL()
    {
	iSingletons.OpenL();
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
	User::LeaveIfError(iFileMan->Move(suid, *iDest));
	SetPreviousPropertiesL(aNewFileName);
	iObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewFileName);
	iObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
	iObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
	iFramework.ObjectMgr().ModifyObjectL(*iObjectInfo);
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
	TBool isFolder = EFalse;
	User::LeaveIfError(BaflUtils::IsFolder(iFramework.Fs(), suid, isFolder));	
				
	if(!isFolder)
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
		
		if(!isFolder)
			{
			MoveFileL(newObjectName);
			SendResponseL(responseCode);
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





