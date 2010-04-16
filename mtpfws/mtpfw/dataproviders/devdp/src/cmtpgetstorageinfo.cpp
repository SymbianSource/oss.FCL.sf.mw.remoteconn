// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <mtp/tmtptyperequest.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtptypestorageinfo.h>
#include <mtp/cmtptypestring.h>

#include "cmtpgetstorageinfo.h"
#include "cmtpstoragemgr.h"
#include "mtpdevicedpconst.h"
#include "mtpdevdppanic.h"
#include "rmtpdevicedpsingletons.h"
#include "cmtpdevicedpconfigmgr.h"

//MTP should reserve some disk space to prevent ood monitor popup
//'Out of memory' note.When syncing music through ovi suite,
//sometimes device screen get freeze with this note
//If you need to adjust this value,please also update the definition
//in file 'cmtptypefile.cpp'
const TInt KFreeSpaceThreshHoldValue(11*1024*1024);//11M

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"GetStorageInfo");)

/**
Verification data for GetStorageInfo request
*/
const TMTPRequestElementInfo KMTPGetStorageInfoPolicy[] = 
    {
        {TMTPTypeRequest::ERequestParameter1, EMTPElementTypeStorageId, EMTPElementAttrNone, 0, 0, 0}
    };

/**
Two-phase construction method
@param aPlugin	The data provider plugin
@param aFramework	The data provider framework
@param aConnection	The connection from which the request comes
@return a pointer to the created request processor object
*/    
MMTPRequestProcessor* CMTPGetStorageInfo::NewL(
											MMTPDataProviderFramework& aFramework,
											MMTPConnection& aConnection)
	{
	CMTPGetStorageInfo* self = new (ELeave) CMTPGetStorageInfo(aFramework, aConnection);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
/**
Destructor
*/	
CMTPGetStorageInfo::~CMTPGetStorageInfo()
	{	
	delete iStorageInfo;
	iSingletons.Close();
	__FLOG_CLOSE;
	}
	
/**
Standard c++ constructor
*/	
CMTPGetStorageInfo::CMTPGetStorageInfo(
									MMTPDataProviderFramework& aFramework,
									MMTPConnection& aConnection)
	:CMTPRequestProcessor(aFramework, aConnection,sizeof(KMTPGetStorageInfoPolicy)/sizeof(TMTPRequestElementInfo), KMTPGetStorageInfoPolicy)
	{
	}

/**
GetStorageInfo request handler
Build storage info data set and send the data to the initiator
*/		
void CMTPGetStorageInfo::ServiceL()
	{
	TUint32 storageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);	
	
	if ( iSingletons.StorageMgr().LogicalStorageId(storageId)  )
	    {
	    if ( iSingletons.StorageMgr().LogicalStorageOwner(storageId) == iFramework.DataProviderId() )
	        {
	        BuildStorageInfoL();
	        SendDataL(*iStorageInfo);		
	        }
	    else
	        {
	        SendResponseL(EMTPRespCodeStoreNotAvailable);
	        }
	    }
	else if ( iSingletons.StorageMgr().PhysicalStorageId(storageId) )
	    {
	    if ( iSingletons.StorageMgr().PhysicalStorageOwner(storageId) == iFramework.DataProviderId() )
	        {
	        BuildStorageInfoL();
	        SendDataL(*iStorageInfo);		
	        }
	    else
	        {
	        SendResponseL(EMTPRespCodeStoreNotAvailable);
	        }
	    }
	else
	    {
        // Storage Id is not valid
        SendResponseL(EMTPRespCodeInvalidStorageID);
        }
	}

/**
Second-phase construction
*/		
void CMTPGetStorageInfo::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	iStorageInfo = CMTPTypeStorageInfo::NewL();
	iSingletons.OpenL();
	}

/**
Populate the storage info data set
*/	
void CMTPGetStorageInfo::BuildStorageInfoL()
	{
	iIsCDrive = EFalse;
	SetupDriveVolumeInfoL();
    SetStorageTypeL();
    SetFileSystemTypeL();
    SetAccessCapabilityL();
    SetMaxCapacityL();
    SetFreeSpaceInBytesL();
    SetFreeSpaceInObjectsL();
    SetStorageDescriptionL();
    SetVolumeIdentifierL();		
	}

/**
Set the drive volume info in the storage info data set
*/
void CMTPGetStorageInfo::SetupDriveVolumeInfoL()
	{
	TUint32 storageId = Request().Uint32(TMTPTypeRequest::ERequestParameter1);	
	TInt driveNo(iFramework.StorageMgr().DriveNumber(storageId));
	User::LeaveIfError(driveNo);
	RFs& fs = iFramework.Fs();
	User::LeaveIfError(fs.Drive(iDriveInfo, driveNo));
	User::LeaveIfError(fs.Volume(iVolumeInfo, driveNo));
	
	const TInt KCDriveNo = 2;
	if(KCDriveNo == driveNo)
		{
		iDriveInfo.iType = EMediaRom;
		iIsCDrive = ETrue;
		}
	}
	
/**
Set the storage type in the storage info data set
*/
void CMTPGetStorageInfo::SetStorageTypeL()
	{
	TUint16 storageType = EMTPStorageUndefined;
	switch(iDriveInfo.iType)
		{
		case EMediaNotPresent:
			User::Leave(KErrDisMounted);
			break;
		case EMediaUnknown:		
			break;
			
		case EMediaCdRom:
			storageType = EMTPStorageRemovableROM;			
			 break;
		case EMediaRam:
		case EMediaNANDFlash:
			storageType = EMTPStorageFixedRAM;
			break;
		case EMediaRom:
			storageType = EMTPStorageFixedROM;
			break;
		case EMediaHardDisk:
		case EMediaFlash:					
		case EMediaRemote:
		case EMediaFloppy:
			storageType = EMTPStorageRemovableRAM;
			break;
		default:
			break;
		}
	TMTPTypeUint16 mtpStorageType(storageType);	
	iStorageInfo->SetL(CMTPTypeStorageInfo::EStorageType, mtpStorageType);
	}

/**
Set the file system type in the storage info data set
*/
void CMTPGetStorageInfo::SetFileSystemTypeL()
	{
	TMTPTypeUint16 mtpFileSystemType(EMTPFileSystemGenericHierarchical);	
	iStorageInfo->SetL(CMTPTypeStorageInfo::EFileSystemType, mtpFileSystemType);	
	}
	

/**
Set the access capability in the storage info data set
*/
void CMTPGetStorageInfo::SetAccessCapabilityL()
	{
	TMTPTypeUint16 mtpStorageType;
	iStorageInfo->GetL(CMTPTypeStorageInfo::EStorageType, mtpStorageType);
	TUint16 storageType = mtpStorageType.Value();
	TInt accessCapability = EAccessCapabilityReadWrite;
	if(storageType == EMTPStorageFixedROM || storageType == EMTPStorageRemovableROM)
		{
		accessCapability = EAccessCapabilityReadOnlyWithoutDeletion;
		}
	TMTPTypeUint16 mtpAccessCapability(accessCapability);	
	iStorageInfo->SetL(CMTPTypeStorageInfo::EAccessCapability, mtpAccessCapability);		
	}

/**
Set the max capacity in the storage info data set
*/
void CMTPGetStorageInfo::SetMaxCapacityL()
	{
	TMTPTypeUint64 mtpMaxCapacity(iVolumeInfo.iSize);
	iStorageInfo->SetL(CMTPTypeStorageInfo::EMaxCapacity, mtpMaxCapacity);
	}
		
/**
Set the free space of the drive in the storage info data set
*/
void CMTPGetStorageInfo::SetFreeSpaceInBytesL()
	{
	TMTPTypeUint64 mtpFreeSpace;
	if(iIsCDrive)
	    {
	    mtpFreeSpace.Set(0);
	    }
	else
	    {
	    //Exclude the reserved disk space when reporting free space
	    TInt64 free = (iVolumeInfo.iFree > KFreeSpaceThreshHoldValue) ?
	        (iVolumeInfo.iFree - KFreeSpaceThreshHoldValue) : 0;
	    mtpFreeSpace.Set(free);
	    }
	__FLOG_2(_L8("SetFreeSpaceInBytesL volume free:%d report:%d"),
	        iVolumeInfo.iFree,mtpFreeSpace.Value());
	iStorageInfo->SetL(CMTPTypeStorageInfo::EFreeSpaceInBytes, mtpFreeSpace);	
	}
	
/**
Set the free space of in objects in the storage info data set
*/
void CMTPGetStorageInfo::SetFreeSpaceInObjectsL()
	{
	TMTPTypeUint32 mtpFreeSpaceInObj(0xFFFFFFFF); 	//we don't support this property
	iStorageInfo->SetL(CMTPTypeStorageInfo::EFreeSpaceInObjects, mtpFreeSpaceInObj);	
	}
	
/**
Set the storage description (volume name) of the drive in the storage info data set
*/
void CMTPGetStorageInfo::SetStorageDescriptionL()
	{
	__FLOG(_L8("SetStorageDescriptionL - Entry"));
    TUint32 storage(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
    TInt driveNumber = iFramework.StorageMgr().DriveNumber(storage);
	RBuf volumeName;
	volumeName.CreateL(KMaxFileName);
	volumeName.CleanupClosePushL();
	RMTPDeviceDpSingletons devSingletons;
	devSingletons.OpenL(iFramework);
	CleanupClosePushL(devSingletons);
	TRAPD(resError, devSingletons.ConfigMgr().GetFriendlyVolumeNameL(driveNumber, volumeName));
	if ((KErrNone == resError) && (0 < volumeName.Length()))
		{
		__FLOG(_L8("Using volume name from resource file"));
		CMTPTypeString* mtpDescription = CMTPTypeString::NewLC(volumeName);
		iStorageInfo->SetL(CMTPTypeStorageInfo::EStorageDescription, *mtpDescription);
		CleanupStack::PopAndDestroy(mtpDescription);
		}
	else if (0 < iVolumeInfo.iName.Length())
		{
		__FLOG(_L8("Using standard volume name"));
		CMTPTypeString* mtpDescription = CMTPTypeString::NewLC(iVolumeInfo.iName);
		iStorageInfo->SetL(CMTPTypeStorageInfo::EStorageDescription, *mtpDescription);
	    CleanupStack::PopAndDestroy(mtpDescription);
		}
		
	CleanupStack::PopAndDestroy(&devSingletons);
	CleanupStack::PopAndDestroy(&volumeName);
	__FLOG(_L8("SetStorageDescriptionL - Exit"));
	}
	
/**
Set the volume identifier of the drive in the storage info data set
*/
void CMTPGetStorageInfo::SetVolumeIdentifierL()
	{
	// Retrieve the StorageID.
    TUint32 storage(Request().Uint32(TMTPTypeRequest::ERequestParameter1));
	
	// Retrieve the volume info.
	TVolumeInfo volInfo;
	iFramework.Fs().Volume(volInfo, iFramework.StorageMgr().DriveNumber(storage));
	
	// Construct the suffix string.
	RBuf16 suffix;
	CleanupClosePushL(suffix);
	suffix.CreateMaxL(KMTPMaxStringCharactersLength);
	suffix.Format(_L("%08X"), volInfo.iUniqueID);
           
    if (volInfo.iName.Length() != 0)
        {
        // Append the separator and volume label, truncating if necessary.
        suffix.Append(_L("-"));
        suffix.Append(volInfo.iName.Left(KMTPMaxStringCharactersLength - suffix.Length()));
        }
	
	// Generate the volume ID string.
	CMTPTypeString* volId(iFramework.StorageMgr().VolumeIdL(iFramework.DataProviderId(), storage, suffix));
	CleanupStack::PopAndDestroy(&suffix);
	CleanupStack::PushL(volId);
	iStorageInfo->SetL(CMTPTypeStorageInfo::EVolumeIdentifier, *volId);
	CleanupStack::PopAndDestroy(volId);	
	}










	

	


   	

	






