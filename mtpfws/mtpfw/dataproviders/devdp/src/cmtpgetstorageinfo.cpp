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
#include <centralrepository.h>
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
#include "mtpframeworkconst.h"
#include "mtpcommonconst.h"

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
		    if (iDriveInfo.iDriveAtt & KDriveAttRemovable)
		        {
		        //E: is set as logically removable after eMMC image updated
		        //So here we need to deal with this case to set it as FixedRam
		        if(iDriveInfo.iDriveAtt & KDriveAttInternal)
		            {
		            __FLOG(_L8("removable but internal drive, set as Fixed RAM"));
		            storageType = EMTPStorageFixedRAM;
		            }
		        else
		            {
		            __FLOG(_L8("non internal,set as removable RAM"));
		            storageType = EMTPStorageRemovableRAM;
		            }
		        }
		    else
		        {
		        __FLOG(_L8("Non removable, set as Fixed RAM"));
		        storageType = EMTPStorageFixedRAM;
		        }
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
	    CRepository* repository(NULL);
	    TInt thresholdValue(0);
	    TRAPD(err,repository = CRepository::NewL(KCRUidUiklaf));
	    if (err == KErrNone)
	        {
	        err = repository->Get(KUikOODDiskFreeSpaceWarningNoteLevelMassMemory,thresholdValue);
	        if (err == KErrNone)
	            {
	            __FLOG_1(_L8("Read from central repo:%d"),thresholdValue);
	            thresholdValue += KFreeSpaceExtraReserved;
	            }	  
	        delete repository;
	        }
	    
	    if (err != KErrNone)
	        {
	        __FLOG(_L8("Fail in read ,use default"));
	        thresholdValue = KFreeSpaceThreshHoldDefaultValue + KFreeSpaceExtraReserved;
	        }
	    
	    __FLOG_2(_L8("threshold:%d free space:%ld"),thresholdValue,iVolumeInfo.iFree);
	    //Exclude the reserved disk space when reporting free space
	    TInt64 free = (iVolumeInfo.iFree > thresholdValue) ?
	        (iVolumeInfo.iFree - thresholdValue) : 0;
	    mtpFreeSpace.Set(free);
	    __FLOG_1(_L8("set free:%ld"),free);
	    }
	
	__FLOG_2(_L8("SetFreeSpaceInBytesL volume free:%ld report:%ld"),
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
	__FLOG_1(_L8("driveNumber:%d"),driveNumber);
	
	CMTPTypeString* mtpDescription = CMTPTypeString::NewLC();
	            
	//Firstly, read name from VolumeInfo
	if (0 < iVolumeInfo.iName.Length())
	    {
	        __FLOG_1(_L8("Using standard volume name:%S"),&iVolumeInfo.iName);
	        mtpDescription->SetL(iVolumeInfo.iName);	        
	    }
	else //If name not set, set name according to type
	    {
	    TMTPTypeUint16 storageType(EMTPStorageUndefined);
	    iStorageInfo->GetL(CMTPTypeStorageInfo::EStorageType,storageType);
	    __FLOG_1(_L8("Set name according to storage type: %d"),storageType.Value());
	    
	    switch (storageType.Value())
	        {
	        case EMTPStorageFixedROM:
	            if (driveNumber == EDriveC)//Phone Memory
	                {
	                __FLOG(_L8("drive c"));
	                mtpDescription->SetL(KPhoneMemory);
	                }
	            break;
	        case EMTPStorageRemovableROM:
	            break;
	        case EMTPStorageFixedRAM: // Mass Memory
	            mtpDescription->SetL(KMassMemory);
	            break;
	        case EMTPStorageRemovableRAM: // Memory Card
	            mtpDescription->SetL(KMemoryCard);
	            break;
	        case EMTPStorageUndefined:
	        default:
	            break;
	        }
	    
	    //Finally, it the name still not set, use default value:
	    //eg, 'A drive'
	    if(mtpDescription->NumChars() == 0)
	        {
	        TChar driveChar;
	        TInt err = iFramework.Fs().DriveToChar(driveNumber,driveChar);
	        __FLOG_2(_L8("Use default name,driveNumber:%d err:%d"),driveNumber,err);
	        if (err == KErrNone)
	            {
	            TBuf<sizeof(KDefaultName) + 1> driveName;
	            driveName.Append(driveChar);
	            driveName.Append(KDefaultName);
	            mtpDescription->SetL(driveName);
	            }
	        else
	            {
	            mtpDescription->SetL(KNoName);
	            }
	        }
	    }
	
	iStorageInfo->SetL(CMTPTypeStorageInfo::EStorageDescription,*mtpDescription);
	CleanupStack::PopAndDestroy(mtpDescription);	
	
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










	

	


   	

	






