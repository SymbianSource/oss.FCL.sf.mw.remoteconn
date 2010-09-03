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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpcopyobjectTraces.h"
#endif


const TInt RollbackFuncCnt = 1;

/**
Verification data for the CopyObject request
*/
const TMTPRequestElementInfo KMTPCopyObjectPolicy[] = 
    {
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
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_CMTPIMAGEDPCOPYOBJECT_DES_ENTRY );
    delete iDest;
    delete iFileMan;
    delete iSrcObjectInfo;
    delete iTargetObjectInfo;
    iRollbackActionL.Close();
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_CMTPIMAGEDPCOPYOBJECT_DES_EXIT );
    }
    
/**
Standard c++ constructor
*/	
CMTPImageDpCopyObject::CMTPImageDpCopyObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider) :
    CMTPRequestProcessor(aFramework, aConnection, sizeof(KMTPCopyObjectPolicy)/sizeof(TMTPRequestElementInfo), KMTPCopyObjectPolicy),
    iFramework(aFramework),
    iDataProvider(aDataProvider)
    {
    }
    
/**
Second phase constructor
*/
void CMTPImageDpCopyObject::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_CONSTRUCTL_ENTRY );
    iFileMan = CFileMan::NewL(iFramework.Fs());
    iSrcObjectInfo = CMTPObjectMetaData::NewL();
    iRollbackActionL.ReserveL(RollbackFuncCnt);
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_CONSTRUCTL_EXIT );
    }

TMTPResponseCode CMTPImageDpCopyObject::CheckRequestL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_CHECKREQUESTL_ENTRY );
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
    
    OstTrace1( TRACE_FLOW, CMTPIMAGEDPCOPYOBJECT_CHECKREQUESTL, 
            "CheckRequestL - Exit with responseCode = 0x%04X", responseCode );
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_CHECKREQUESTL_EXIT );
    return responseCode;
    }

/**
CopyObject request handler
*/      
void CMTPImageDpCopyObject::ServiceL()
    {   
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_SERVICEL_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_SERVICEL_EXIT );
    }
    
/**
Copy object operation
@return the object handle of the resulting object.
*/
TMTPResponseCode CMTPImageDpCopyObject::CopyObjectL(TUint32& aNewHandle)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_COPYOBJECTL_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_COPYOBJECTL_EXIT );
    return responseCode;
    }

/**
A helper function of CopyObjectL.
@param aNewFileName the new full filename after copy.
@return objectHandle of new copy of object.
*/
TUint32 CMTPImageDpCopyObject::CopyFileL(const TDesC& aOldFileName, const TDesC& aNewFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_COPYFILEL_ENTRY );
    TCleanupItem anItem(FailRecover, reinterpret_cast<TAny*>(this));
    CleanupStack::PushL(anItem);
    
    GetPreviousPropertiesL(aOldFileName);
    LEAVEIFERROR(iFileMan->Copy(aOldFileName, *iDest),
            OstTraceExt3( TRACE_ERROR, CMTPIMAGEDPCOPYOBJECT_COPYFILEL, 
                    "Copy %S to %S failed! error code %d", aOldFileName, *iDest, munged_err));
            
    iRollbackActionL.AppendL(RollBackFromFsL);
    SetPreviousPropertiesL(aNewFileName);
    
    iFramework.ObjectMgr().InsertObjectL(*iTargetObjectInfo);
    //check object whether it is a new image object
    if (MTPImageDpUtilits::IsNewPicture(*iTargetObjectInfo))
        {
        //increate new pictures count
        iDataProvider.IncreaseNewPictures(1);
        }    
    
    CleanupStack::Pop(this);
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_COPYFILEL_EXIT );
    return iTargetObjectInfo->Uint(CMTPObjectMetaData::EHandle);
    }

/**
Retrieve the parameters of the request
*/	
void CMTPImageDpCopyObject::GetParametersL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_GETPARAMETERSL_ENTRY );
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
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_GETPARAMETERSL_EXIT );
    }
    
/**
Get a default parent object, ff the request does not specify a parent object, 
*/
void CMTPImageDpCopyObject::SetDefaultParentObjectL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_SETDEFAULTPARENTOBJECTL_ENTRY );
    TDriveNumber drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    LEAVEIFERROR(drive,
            OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPCOPYOBJECT_SETDEFAULTPARENTOBJECTL, "No driver number for %d! error code %d",iStorageId, munged_err));
    TChar driveLetter;
    iFramework.Fs().DriveToChar(drive, driveLetter);
    TFileName driveBuf;
    driveBuf.Append(driveLetter);
    driveBuf = BaflUtils::RootFolderPath(driveBuf.Left(1));
    delete iDest;
    iDest = NULL;
    iDest = driveBuf.AllocL();
    iNewParentHandle = KMTPHandleNoParent;
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_SETDEFAULTPARENTOBJECTL_EXIT );
    }
    
/**
Check if we can copy the file to the new location
*/
TMTPResponseCode CMTPImageDpCopyObject::CanCopyObjectL(const TDesC& aOldName, const TDesC& aNewName) const
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL_ENTRY );
    TMTPResponseCode result = EMTPRespCodeOK;
    
    TEntry fileEntry;
    LEAVEIFERROR(iFramework.Fs().Entry(aOldName, fileEntry),
            OstTraceExt2( TRACE_ERROR, DUP1_CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL, 
                    "Gets the entry details for %S failed! error code %d", aOldName, munged_err ));
    TDriveNumber drive(static_cast<TDriveNumber>(iFramework.StorageMgr().DriveNumber(iStorageId)));
    LEAVEIFERROR(drive,
            OstTraceExt2( TRACE_ERROR, DUP2_CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL, 
                    "Gets drive for storage %d failed! error code %d", iStorageId, munged_err ));
    TVolumeInfo volumeInfo;
    LEAVEIFERROR(iFramework.Fs().Volume(volumeInfo, drive),
            OstTraceExt2( TRACE_ERROR, DUP3_CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL, 
                    "Gets volume information for driver %d failed! error code %d", drive, munged_err ));            
    
    if(volumeInfo.iFree < fileEntry.FileSize())
        {
        result = EMTPRespCodeStoreFull;
        }
    else if (BaflUtils::FileExists(iFramework.Fs(), aNewName))			
        {
        result = EMTPRespCodeInvalidParentObject;
        }
	OstTrace1( TRACE_NORMAL, CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL, 
	        "CanCopyObjectL - Exit with response code 0x%04X", result );
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_CANCOPYOBJECTL_EXIT );
    return result;	
    }
    
/**
Save the object properties before doing the copy
*/
void CMTPImageDpCopyObject::GetPreviousPropertiesL(const TDesC& aOldFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_GETPREVIOUSPROPERTIESL_ENTRY );
    LEAVEIFERROR(iFramework.Fs().Modified(aOldFileName, iDateModified),
            OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPCOPYOBJECT_GETPREVIOUSPROPERTIESL, 
                    "Gets the last modification date and time failed for %S! error code %d", aOldFileName, munged_err ));                        
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_GETPREVIOUSPROPERTIESL_EXIT );
    }
    
/**
Set the object properties after doing the copy
*/
void CMTPImageDpCopyObject::SetPreviousPropertiesL(const TDesC& aNewFileName)
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPCOPYOBJECT_SETPREVIOUSPROPERTIESL_ENTRY );      
    LEAVEIFERROR(iFramework.Fs().SetModified(aNewFileName, iDateModified),
            OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPCOPYOBJECT_SETPREVIOUSPROPERTIESL, 
                    "Sets the date and time for %S failed. Error code %d", aNewFileName, munged_err));
    iTargetObjectInfo = CMTPObjectMetaData::NewL();
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EDataProviderId, iSrcObjectInfo->Uint(CMTPObjectMetaData::EDataProviderId));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EFormatCode, iSrcObjectInfo->Uint(CMTPObjectMetaData::EFormatCode));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EFormatSubCode, iSrcObjectInfo->Uint(CMTPObjectMetaData::EFormatSubCode));
    iTargetObjectInfo->SetDesCL(CMTPObjectMetaData::EName, iSrcObjectInfo->DesC(CMTPObjectMetaData::EName));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::ENonConsumable, iSrcObjectInfo->Uint(CMTPObjectMetaData::ENonConsumable));
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EParentHandle, iNewParentHandle);
    iTargetObjectInfo->SetUint(CMTPObjectMetaData::EStorageId, iStorageId);
    iTargetObjectInfo->SetDesCL(CMTPObjectMetaData::ESuid, aNewFileName);
    OstTraceFunctionExit0( CMTPIMAGEDPCOPYOBJECT_SETPREVIOUSPROPERTIESL_EXIT );
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
    LEAVEIFERROR(iFramework.Fs().Delete(iNewFileName),
            OstTraceExt2( TRACE_ERROR, CMTPIMAGEDPCOPYOBJECT_ROLLBACKFROMFSL, 
                    "delete %S failed! error code %d", iNewFileName, munged_err));
    }

void CMTPImageDpCopyObject::RollBackFromFsL(CMTPImageDpCopyObject* aObject)
    {
    aObject->RollBackFromFsL();
    }

// End of file

