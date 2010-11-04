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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpcopyobjectTraces.h"
#endif


/**
Verification data for the CopyObject request
*/
const TMTPRequestElementInfo KMTPCopyObjectPolicy[] = 
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
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_CMTPCOPYOBJECT_DES_ENTRY );
	Cancel();
	iDpSingletons.Close();
	iSingletons.Close();
	
	delete iTimer;
	delete iDest;
	delete iNewFileName;
	delete iFileMan;

	OstTraceFunctionExit0( CMTPCOPYOBJECT_CMTPCOPYOBJECT_DES_EXIT );
	}

/**
Standard c++ constructor
*/	
CMTPCopyObject::CMTPCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection) :
	CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy),
	iTimer(NULL)
	{
	}



TMTPResponseCode CMTPCopyObject::CheckRequestL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_CHECKREQUESTL_ENTRY );
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
			LEAVEIFERROR(BaflUtils::IsFolder(iFramework.Fs(), suid, iIsFolder),
			        OstTraceExt1( TRACE_ERROR, DUP1_CMTPCOPYOBJECT_CHECKREQUESTL, "can't judge whether %S is a folder", suid ));
			if(!iIsFolder)
				{
				if(iDpSingletons.CopyingBigFileCache().IsOnGoing())
					{
					OstTrace0( TRACE_NORMAL, CMTPCOPYOBJECT_CHECKREQUESTL, 
					        "CheckRequestL - A big file copying is ongoing, respond with access denied" );				
					result = EMTPRespCodeAccessDenied;
					}
				}
			}
		CleanupStack::PopAndDestroy(object); 
		}
	OstTraceFunctionExit0( CMTPCOPYOBJECT_CHECKREQUESTL_EXIT );
	return result;	
	} 

/**
CopyObject request handler
*/		
void CMTPCopyObject::ServiceL()
	{	
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_SERVICEL_ENTRY );
	TUint32 handle = KMTPHandleNone;
	TMTPResponseCode responseCode = EMTPRespCodeOK;
	TRAPD(err, responseCode = CopyObjectL(handle));
	if(err != KErrNone)
		{
		SendResponseL(EMTPRespCodeAccessDenied);
		}
	else if(responseCode != EMTPRespCodeOK)
		{
		OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_SERVICEL, "ServiceL, sending response with respond code %d", responseCode );
		SendResponseL(responseCode);
		}
	else if (iIsFolder)
		{
		OstTrace1( TRACE_NORMAL, DUP1_CMTPCOPYOBJECT_SERVICEL, "ServiceL, sending response with handle=%d, respond code OK", handle );
		SendResponseL(EMTPRespCodeOK, 1, &handle);
		}
	OstTraceFunctionExit0( CMTPCOPYOBJECT_SERVICEL_EXIT );
	}


/**
 Second phase constructor
*/
void CMTPCopyObject::ConstructL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_CONSTRUCTL_ENTRY );
	iSingletons.OpenL();
	iDpSingletons.OpenL(iFramework);
	OstTraceFunctionExit0( CMTPCOPYOBJECT_CONSTRUCTL_EXIT );
	}

	
/**
A helper function of CopyObjectL.
@param aNewFileName the new full filename after copy.
@return objectHandle of new copy of object.
*/
void CMTPCopyObject::CopyFileL(const TDesC& aNewFileName)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_COPYFILEL_ENTRY );
	delete iNewFileName;
	iNewFileName = NULL;
	iNewFileName = aNewFileName.AllocL(); // Store the new file name	
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	GetPreviousPropertiesL(suid);
	
	LEAVEIFERROR(iFileMan->Copy(suid, *iDest, CFileMan::EOverWrite, iStatus),
	        OstTraceExt2( TRACE_ERROR, CMTPCOPYOBJECT_COPYFILEL, "copy %S to %S failed!", suid, *iDest));
	if ( !IsActive() )
	{  
	SetActive();
	}
	
	delete iTimer;
	iTimer = NULL;
	iTimer = CPeriodic::NewL(EPriorityStandard);
	TTimeIntervalMicroSeconds32 KCopyObjectIntervalNone = 0;	
	iTimer->Start(TTimeIntervalMicroSeconds32(KCopyObjectTimeOut), KCopyObjectIntervalNone, TCallBack(CMTPCopyObject::OnTimeoutL, this));

	OstTraceFunctionExit0( CMTPCOPYOBJECT_COPYFILEL_EXIT );
	}

/**
A helper function of CopyObjectL.
@param aNewFolderName the new full file folder name after copy.
@return objecthandle of new copy of the folder.
*/
TUint32 CMTPCopyObject::CopyFolderL(const TDesC& aNewFolderName)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_COPYFOLDERL_ENTRY );
	const TDesC& suid(iObjectInfo->DesC(CMTPObjectMetaData::ESuid));
	TUint32 handle;
	if (iObjectInfo->Uint(CMTPObjectMetaData::EDataProviderId) == iFramework.DataProviderId())
		{
		GetPreviousPropertiesL(suid);
		LEAVEIFERROR(iFramework.Fs().MkDir(aNewFolderName),
		        OstTraceExt1( TRACE_ERROR, CMTPCOPYOBJECT_COPYFOLDERL, "create directory %S failed!", aNewFolderName));
		SetPreviousPropertiesL(aNewFolderName);	
		handle = UpdateObjectInfoL(aNewFolderName);
		}
	else
		{
		handle = iFramework.ObjectMgr().HandleL(aNewFolderName);
		}
	OstTraceFunctionExit0( CMTPCOPYOBJECT_COPYFOLDERL_EXIT );
	return handle;
	}
		
/**
Copy object operation
@return the object handle of the resulting object.
*/
TMTPResponseCode CMTPCopyObject::CopyObjectL(TUint32& aNewHandle)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_COPYOBJECTL_ENTRY );
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
		LEAVEIFERROR(BaflUtils::MostSignificantPartOfFullName(suid, rightMostFolderName),
		        OstTraceExt1( TRACE_ERROR, CMTPCOPYOBJECT_COPYOBJECTL, "can't extract most significant part from %S", suid));    
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
	OstTraceFunctionExit0( CMTPCOPYOBJECT_COPYOBJECTL_EXIT );
	return responseCode;
	}

/**
Retrieve the parameters of the request
*/	
void CMTPCopyObject::GetParametersL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_GETPARAMETERSL_ENTRY );
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
	OstTraceFunctionExit0( CMTPCOPYOBJECT_GETPARAMETERSL_EXIT );
	}
	
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPCopyObject::SetDefaultParentObjectL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_SETDEFAULTPARENTOBJECTL_ENTRY );

	const CMTPStorageMetaData& storageMetaData( iFramework.StorageMgr().StorageL(iStorageId) );
	const TDesC& driveBuf( storageMetaData.DesC(CMTPStorageMetaData::EStorageSuid) );
	delete iDest;
	iDest = NULL;
	iDest = driveBuf.AllocL();
	iNewParentHandle = KMTPHandleNoParent;
	    
	OstTraceFunctionExit0( CMTPCOPYOBJECT_SETDEFAULTPARENTOBJECTL_EXIT );
	}

/**
Check if we can copy the file to the new location
*/
TMTPResponseCode CMTPCopyObject::CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_CANCOPYOBJECTL_ENTRY );
	TMTPResponseCode result = EMTPRespCodeOK;

	TEntry fileEntry;
	LEAVEIFERROR(iFramework.Fs().Entry(aOldName, fileEntry),
	        OstTraceExt1( TRACE_ERROR, DUP1_CMTPCOPYOBJECT_CANCOPYOBJECTL, "Can't get entry details for %S", aOldName ));     
	TInt drive(iFramework.StorageMgr().DriveNumber(iStorageId));
	LEAVEIFERROR(drive,
	        OstTraceExt2( TRACE_ERROR, DUP2_CMTPCOPYOBJECT_CANCOPYOBJECTL, 
	                "Get driver number for storage %d failed! error code %d", iStorageId, drive));
	        
	TVolumeInfo volumeInfo;
	LEAVEIFERROR(iFramework.Fs().Volume(volumeInfo, drive),
	        OstTrace1( TRACE_ERROR, DUP3_CMTPCOPYOBJECT_CANCOPYOBJECTL, "can't get volume information for driver %d", drive));

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
	OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_CANCOPYOBJECTL, "response code 0x%04X", result );	
	OstTraceFunctionExit0( CMTPCOPYOBJECT_CANCOPYOBJECTL_EXIT );
	return result;	
	}

/**
Save the object properties before doing the copy
*/
void CMTPCopyObject::GetPreviousPropertiesL(const TDesC& aFileName)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_GETPREVIOUSPROPERTIESL_ENTRY );
	LEAVEIFERROR(iFramework.Fs().Modified(aFileName, iPreviousModifiedTime),
	        OstTraceExt1( TRACE_ERROR, CMTPCOPYOBJECT_GETPREVIOUSPROPERTIESL, "Can't get the last modification date and time for %S", aFileName));
	if ( iIsFolder )
	    {
        TEntry fileEntry;
        User::LeaveIfError(iFramework.Fs().Entry( aFileName, fileEntry ));
        iIsHidden = fileEntry.IsHidden();
	    }
	        
	OstTraceFunctionExit0( CMTPCOPYOBJECT_GETPREVIOUSPROPERTIESL_EXIT );
	}

/**
Set the object properties after doing the copy
*/
void CMTPCopyObject::SetPreviousPropertiesL(const TDesC& aFileName)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_SETPREVIOUSPROPERTIESL_ENTRY );
	LEAVEIFERROR(iFramework.Fs().SetModified(aFileName, iPreviousModifiedTime),
	        OstTraceExt1( TRACE_ERROR, CMTPCOPYOBJECT_SETPREVIOUSPROPERTIESL, "Sets the date and time for %S failed", aFileName));
	if ( iIsFolder && iIsHidden )
	    {
        TEntry fileEntry;
        User::LeaveIfError(iFramework.Fs().Entry( aFileName, fileEntry ));
        fileEntry.iAtt &= ~KEntryAttHidden;
        fileEntry.iAtt |= KEntryAttHidden;
        User::LeaveIfError(iFramework.Fs().SetAtt( aFileName, fileEntry.iAtt, ~fileEntry.iAtt));
	    }
	OstTraceFunctionExit0( CMTPCOPYOBJECT_SETPREVIOUSPROPERTIESL_EXIT );
	}

/**
 Update object info in the database.
*/
TUint32 CMTPCopyObject::UpdateObjectInfoL(const TDesC& aNewObjectName)
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_UPDATEOBJECTINFOL_ENTRY );
	
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
        OstTrace0( TRACE_ERROR, CMTPCOPYOBJECT_UPDATEOBJECTINFOL, "the specified object handle doesn't exist");
		User::Leave(KErrCorrupt);
		}
	TUint32 handle = objectInfo->Uint(CMTPObjectMetaData::EHandle);	
	CleanupStack::PopAndDestroy(objectInfo);
	
	OstTraceFunctionExit0( CMTPCOPYOBJECT_UPDATEOBJECTINFOL_EXIT );
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
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_DOONTIMEOUTL_ENTRY );
	
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
	        OstTraceExt1( TRACE_ERROR, DUP1_CMTPCOPYOBJECT_DOONTIMEOUTL, "Gets the entry details for %S failed!", suid));

	TUint32 handle = KMTPHandleNone;
	handle = UpdateObjectInfoL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	
	// Cache the target file entry info, which is used to send response to getobjectproplist and getobjectinfo
	aCache.SetOnGoing(ETrue);
	aCache.SetTargetHandle(handle);
	aCache.SetFileEntry(fileEntry);

	OstTrace1( TRACE_NORMAL, CMTPCOPYOBJECT_DOONTIMEOUTL, 
	        "UpdateFSEntryCache, sending response with handle=%d, respond code OK for a big file copy", handle );	
	SendResponseL(EMTPRespCodeOK, 1, &handle);
	
	OstTraceFunctionExit0( CMTPCOPYOBJECT_DOONTIMEOUTL_EXIT );
	}

/**
 CMTPCopyObject::RunL
*/
void CMTPCopyObject::RunL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_RUNL_ENTRY );
	
	LEAVEIFERROR(iStatus.Int(),
	        OstTrace1( TRACE_ERROR, DUP2_CMTPCOPYOBJECT_RUNL, "wrong istatus %d", iStatus.Int()));
	SetPreviousPropertiesL(*iNewFileName);
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	// Check to see if we are copying a big file
	if(aCache.IsOnGoing())
		{
		OstTrace0( TRACE_NORMAL, CMTPCOPYOBJECT_RUNL, "RunL - Big file copy complete" );
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
		OstTrace1( TRACE_NORMAL, DUP1_CMTPCOPYOBJECT_RUNL, 
		        "RunL, sending response with handle=%d, respond code OK for a normal file copy", handle );		
		SendResponseL(EMTPRespCodeOK, 1, &handle);
		}
	OstTraceFunctionExit0( CMTPCOPYOBJECT_RUNL_EXIT );
	}

#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPCopyObject::RunError(TInt aErr)
#else
TInt CMTPCopyObject::RunError(TInt /*aErr*/)
#endif
    {
    OstTrace1(TRACE_ERROR, CMTPCOPYOBJECT_RUNERROR,"CMTPCopyObject::RunError is %d", aErr );
	TRAP_IGNORE(SendResponseL(EMTPRespCodeGeneralError));
    return KErrNone;
    }

/**
Override to handle the complete phase of copy object
@return EFalse
*/
TBool CMTPCopyObject::DoHandleCompletingPhaseL()
	{
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_DOHANDLECOMPLETINGPHASEL_ENTRY );
	CMTPRequestProcessor::DoHandleCompletingPhaseL();
	
	CMTPFSEntryCache& aCache = iDpSingletons.CopyingBigFileCache();
	if(aCache.IsOnGoing())
		{
		OstTraceFunctionExit0( CMTPCOPYOBJECT_DOHANDLECOMPLETINGPHASEL_EXIT );
		return EFalse;
		}
	else
		{
		OstTraceFunctionExit0( DUP1_CMTPCOPYOBJECT_DOHANDLECOMPLETINGPHASEL_EXIT );
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
	OstTraceFunctionEntry0( CMTPCOPYOBJECT_MATCH_ENTRY );
	TBool result = EFalse;
	TUint16 operationCode = aRequest.Uint16(TMTPTypeRequest::ERequestOperationCode);
	if ((operationCode == EMTPOpCodeCopyObject) && &iConnection == &aConnection)
	{
	result = ETrue;
	}    
	OstTraceFunctionExit0( CMTPCOPYOBJECT_MATCH_EXIT );
	return result;    
	}
