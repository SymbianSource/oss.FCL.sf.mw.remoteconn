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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpmoveobjectTraces.h"
#endif



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
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_CMTPMOVEOBJECT_ENTRY );
	Cancel();
	iDpSingletons.Close();
	iSingletons.Close();
	
	delete iTimer;
	delete iNewFileName;
	delete iDest;
	delete iFileMan;
	delete iPathToMove;
	delete iNewRootFolder;	
	OstTraceFunctionExit0( CMTPMOVEOBJECT_CMTPMOVEOBJECT_EXIT );
	}

/**
Standard c++ constructor
*/	
CMTPMoveObject::CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPMoveObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPMoveObjectPolicy),
	iMoveObjectIndex(0), iTimer(NULL)
	{
	}

TMTPResponseCode CMTPMoveObject::CheckRequestL()
	{
    OstTraceFunctionEntry0( CMTPMOVEOBJECT_CHECKREQUESTL_ENTRY );
	TMTPResponseCode result = CMTPRequestProcessor::CheckRequestL();
	if (EMTPRespCodeOK != result)
		{
		OstTraceFunctionExit0( CMTPMOVEOBJECT_CHECKREQUESTL_EXIT );
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
		LEAVEIFERROR(BaflUtils::IsFolder(iFramework.Fs(), suid, iIsFolder),
		        OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_CHECKREQUESTL, "can't judge whether %S is folder", suid));
		        
		if(!iIsFolder)
			{
			if(iDpSingletons.MovingBigFileCache().IsOnGoing())
				{
				OstTrace0( TRACE_NORMAL, CMTPMOVEOBJECT_CHECKREQUESTL, 
				        "CheckRequestL - A big file moving is ongoing, respond with access denied" );
				result = EMTPRespCodeAccessDenied;
				}
			}
		}

	OstTraceFunctionExit0( DUP1_CMTPMOVEOBJECT_CHECKREQUESTL_EXIT );
	return result;	
	} 

/**
MoveObject request handler
*/		
void CMTPMoveObject::ServiceL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_SERVICEL_ENTRY );
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
	OstTraceFunctionExit0( CMTPMOVEOBJECT_SERVICEL_EXIT );
	}

/**
 Second phase constructor
*/
void CMTPMoveObject::ConstructL()
    {
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_CONSTRUCTL_ENTRY );
	iSingletons.OpenL();
	iDpSingletons.OpenL(iFramework);
    OstTraceFunctionExit0( CMTPMOVEOBJECT_CONSTRUCTL_EXIT );
    }
    

/**
A helper function of MoveObjectL.
@param aNewFileName the new file name after the object is moved.
*/
void CMTPMoveObject::MoveFileL(const TDesC& aNewFileName)	
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_MOVEFILEL_ENTRY );
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	GetPreviousPropertiesL(suid);
	
	if(iFramework.StorageMgr().DriveNumber(iObjectInfo->Uint(CMTPObjectMetaData::EStorageId)) ==
			iFramework.StorageMgr().DriveNumber(iStorageId))
		//Move file to the same storage
		{
		LEAVEIFERROR(iFileMan->Move(suid, *iDest),
		        OstTraceExt2( TRACE_ERROR, CMTPMOVEOBJECT_MOVEFILEL, "move %S to %S failed!", suid, *iDest ));
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
		
		LEAVEIFERROR(iFileMan->Move(suid, *iDest, CFileMan::EOverWrite, iStatus),
		        OstTraceExt2( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_MOVEFILEL, "move %S to %S failed!", suid, *iDest));
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
	OstTraceFunctionExit0( CMTPMOVEOBJECT_MOVEFILEL_EXIT );
	}

/**
A helper function of MoveObjectL.
@param aNewFolderName the new file folder name after the folder is moved.
*/
void CMTPMoveObject::MoveFolderL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_MOVEFOLDERL_ENTRY );
	
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

	OstTraceFunctionExit0( CMTPMOVEOBJECT_MOVEFOLDERL_EXIT );
	}
		
/**
move object operations
@return A valid MTP response code.
*/
TMTPResponseCode CMTPMoveObject::MoveObjectL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_MOVEOBJECTL_ENTRY );
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
		LEAVEIFERROR(BaflUtils::MostSignificantPartOfFullName(suid, rightMostFolderName),
		        OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_MOVEOBJECTL, "extract most significant part of %S failed", suid));
		        
		if((newObjectName.Length() + rightMostFolderName.Length() + 1) <= newObjectName.MaxLength())
			{
			newObjectName.Append(rightMostFolderName);
			// Add backslash.
			_LIT(KBackSlash, "\\");
			newObjectName.Append(KBackSlash);
			}
		}
		
	iNewRootFolder = newObjectName.AllocL();
	OstTraceExt1( TRACE_NORMAL, CMTPMOVEOBJECT_MOVEOBJECTL, "%S", *iNewRootFolder );

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
	OstTraceFunctionExit0( CMTPMOVEOBJECT_MOVEOBJECTL_EXIT );
	return responseCode;
	}

/**
Retrieve the parameters of the request
*/	
void CMTPMoveObject::GetParametersL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_GETPARAMETERSL_ENTRY );
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
	OstTraceFunctionExit0( CMTPMOVEOBJECT_GETPARAMETERSL_EXIT );
	}
	
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPMoveObject::SetDefaultParentObjectL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_SETDEFAULTPARENTOBJECTL_ENTRY );
	const CMTPStorageMetaData& storageMetaData( iFramework.StorageMgr().StorageL(iStorageId) );
	const TDesC& driveBuf( storageMetaData.DesC(CMTPStorageMetaData::EStorageSuid) );
	delete iDest;
	iDest = NULL;
	iDest = driveBuf.AllocL();
	iNewParentHandle = KMTPHandleNoParent;
	OstTraceFunctionExit0( CMTPMOVEOBJECT_SETDEFAULTPARENTOBJECTL_EXIT );
	}

/**
Check if we can move the file to the new location
*/
TMTPResponseCode CMTPMoveObject::CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_CANMOVEOBJECTL_ENTRY );
	TMTPResponseCode result = EMTPRespCodeOK;

	TEntry fileEntry;
	LEAVEIFERROR(iFramework.Fs().Entry(aOldName, fileEntry),
	        OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_CANMOVEOBJECTL, "can't get entry details from %S", aOldName));
	TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
	LEAVEIFERROR(drive,
	        OstTrace1( TRACE_ERROR, DUP2_CMTPMOVEOBJECT_CANMOVEOBJECTL, "can't get driver number for storage %d", iStorageId));
	TVolumeInfo volumeInfo;
	LEAVEIFERROR(iFramework.Fs().Volume(volumeInfo, drive),
	        OstTrace1( TRACE_ERROR, DUP3_CMTPMOVEOBJECT_CANMOVEOBJECTL, "can't get volume info for drive %d", drive));        
	
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
	OstTraceFunctionExit0( CMTPMOVEOBJECT_CANMOVEOBJECTL_EXIT );
	OstTrace1( TRACE_NORMAL, CMTPMOVEOBJECT_CANMOVEOBJECTL, "response code 0x%04X", result );
	return result;	
	}

/**
Save the object properties before moving
*/
void CMTPMoveObject::GetPreviousPropertiesL(const TDesC& aFileName)
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_GETPREVIOUSPROPERTIESL_ENTRY );
	LEAVEIFERROR(iFramework.Fs().Modified(aFileName, iPreviousModifiedTime),
	        OstTraceExt1( TRACE_ERROR, CMTPMOVEOBJECT_GETPREVIOUSPROPERTIESL, 
	                "Can't get the last modification date and time for %S", aFileName));
	if ( iIsFolder )
	    {
	    TEntry fileEntry;
	    User::LeaveIfError(iFramework.Fs().Entry( aFileName, fileEntry ));
	    iIsHidden = fileEntry.IsHidden();
	    }
	OstTraceFunctionExit0( CMTPMOVEOBJECT_GETPREVIOUSPROPERTIESL_EXIT );
	}

/**
Set the object properties after moving
*/
void CMTPMoveObject::SetPreviousPropertiesL(const TDesC& aFileName)
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_SETPREVIOUSPROPERTIESL_ENTRY );
	LEAVEIFERROR(iFramework.Fs().SetModified(aFileName, iPreviousModifiedTime),
	        OstTraceExt1( TRACE_ERROR, CMTPMOVEOBJECT_SETPREVIOUSPROPERTIESL, "Sets the date and time for %S failed", aFileName));
	if ( iIsFolder && iIsHidden )
	    {
	    TEntry fileEntry;
	    User::LeaveIfError(iFramework.Fs().Entry( aFileName, fileEntry ));
	    fileEntry.iAtt &= ~KEntryAttHidden;
	    fileEntry.iAtt |= KEntryAttHidden;
	    User::LeaveIfError(iFramework.Fs().SetAtt( aFileName, fileEntry.iAtt, ~fileEntry.iAtt));
	    }
	OstTraceFunctionExit0( CMTPMOVEOBJECT_SETPREVIOUSPROPERTIESL_EXIT );
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
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_DOONTIMEOUTL_ENTRY );
	
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
	LEAVEIFERROR(iFramework.Fs().Entry(suid, fileEntry),
	        OstTraceExt1( TRACE_ERROR, DUP1_CMTPMOVEOBJECT_DOONTIMEOUTL, "Gets the entry details for %S failed!", suid));
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
	
	OstTrace0( TRACE_NORMAL, CMTPMOVEOBJECT_DOONTIMEOUTL, 
	        "UpdateFSEntryCache, sending response with respond code OK for a big file move" );
	SendResponseL(EMTPRespCodeOK);
	
	OstTraceFunctionExit0( CMTPMOVEOBJECT_DOONTIMEOUTL_EXIT );
	}

/**
 CMTPMoveObject::RunL
*/
void CMTPMoveObject::RunL()
	{
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_RUNL_ENTRY );
	
	LEAVEIFERROR(iStatus.Int(),
	        OstTrace1( TRACE_ERROR, DUP2_CMTPMOVEOBJECT_RUNL, "wrong istatus %d", iStatus.Int()));
	SetPreviousPropertiesL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.MovingBigFileCache();
	// Check to see if we are moving a big file
	if(aCache.IsOnGoing())
		{
		OstTrace0( TRACE_NORMAL, CMTPMOVEOBJECT_RUNL, "RunL - Big file move complete" );
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

		OstTrace0( TRACE_NORMAL, DUP1_CMTPMOVEOBJECT_RUNL, 
		        "RunL, sending response with respond code OK for a normal file move" );
		SendResponseL(EMTPRespCodeOK);
		}
	OstTraceFunctionExit0( CMTPMOVEOBJECT_RUNL_EXIT );
	}

#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPMoveObject::RunError(TInt aErr)
#else
TInt CMTPMoveObject::RunError(TInt /*aErr*/)
#endif
    {
    OstTrace1(TRACE_ERROR, CMTPMOVEOBJECT_RUNERROR,"CMTPMoveObject::RunError is %d", aErr );
	TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
    return KErrNone;
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
	OstTraceFunctionEntry0( CMTPMOVEOBJECT_MATCH_ENTRY );
	TBool result = EFalse;
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	if ((operationCode == EMTPOpCodeMoveObject) && &iConnection == &aConnection)
	{
	result = ETrue;
	}    
	OstTrace1( TRACE_NORMAL, CMTPMOVEOBJECT_MATCH, "with result = %d", result );
	OstTraceFunctionExit0( CMTPMOVEOBJECT_MATCH_EXIT );
	return result;
	}
