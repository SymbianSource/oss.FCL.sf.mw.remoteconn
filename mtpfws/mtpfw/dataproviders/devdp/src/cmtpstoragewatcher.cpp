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

#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/mtpdataproviderapitypes.h>

#include "cmtpdataprovider.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpframeworkconfig.h"
#include "cmtpstoragemgr.h"
#include "cmtpobjectmgr.h"
#include "cmtpstoragewatcher.h"
#include "cmtpdevicedpconfigmgr.h"


// Class constants.
__FLOG_STMT(_LIT8(KComponent,"StorageWatcher");)
static const TBool KAllDrives(ETrue);
static const TBool KAvailableDrives(EFalse);

const TInt KFolderExclusionGranularity = 8;

/**
MTP system storage watcher factory method.
@return A pointer to an MTP system storage watcher object. Ownership IS 
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPStorageWatcher* CMTPStorageWatcher::NewL(MMTPDataProviderFramework& aFramework)
    {
    CMTPStorageWatcher* self = new (ELeave) CMTPStorageWatcher(aFramework);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPStorageWatcher::~CMTPStorageWatcher()
    {
    __FLOG(_L8("~CMTPStorageWatcher - Entry"));
    Cancel();
    delete iFolderExclusionList;
    iDpSingletons.Close();
    iDrivesExcluded.Close();
    iFrameworkSingletons.Close();
    iDevDpSingletons.Close();
    __FLOG(_L8("~CMTPStorageWatcher - Exit"));
    __FLOG_CLOSE;
    }
    
void CMTPStorageWatcher::EnumerateStoragesL()
    {
    __FLOG(_L8("EnumerateStoragesL - Entry"));

    //Use Hash to replace it
    AppendFolderExclusionListL();
    
    // Retrieve the drive exclusion list.
    iFrameworkSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::EExcludedStorageDrives, iDrivesExcluded);
    iDrivesExcluded.Sort();
    
    // Claim system storages ownership.
    CMTPStorageMgr& mgr(iFrameworkSingletons.StorageMgr());
    mgr.SetFrameworkId(iFramework.DataProviderId());
    
    /* 
    Enumerate the initial drive set. 
    
        1.   Enumerate each known drive as a physical storage.
    */
    iDrivesConfig = DriveConfigurationL(KAllDrives);
    CMTPStorageMetaData* storage = CMTPStorageMetaData::NewLC();
    storage->SetUint(CMTPStorageMetaData::EStorageSystemType, CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
    _LIT(KSuidTemplate, "?:");
    RBuf suid;
    suid.CleanupClosePushL();
    suid.Assign((KSuidTemplate().AllocL()));
    
    for (TInt drive(0); (drive < KMaxDrives); drive++)
        {
        const TUint32 mask(1 << drive);
        if (iDrivesConfig & mask)
            {
            TChar driveChar;
            User::LeaveIfError(iFramework.Fs().DriveToChar(drive, driveChar));
            suid[0] = driveChar;
            storage->SetDesCL(CMTPStorageMetaData::EStorageSuid, suid);
                    
            TUint32 id(mgr.AllocatePhysicalStorageIdL(iFramework.DataProviderId(), *storage));
            mgr.SetDriveMappingL(static_cast<TDriveNumber>(drive), id);
            }
        }
        
    CleanupStack::PopAndDestroy(&suid);
    CleanupStack::PopAndDestroy(storage);
    
    /* 
        2.  If so configured, enumerate a single logical storage for each of 
            the available drives.
    */
    if (iAllocateLogicalStorages)
        {
        iDrivesConfig = DriveConfigurationL(KAvailableDrives);

        for (TInt drive(0); (drive < KMaxDrives); drive++)
            {
            const TUint32 mask(1 << drive);
            if (iDrivesConfig & mask)
                {
                StorageAvailableL(static_cast<TDriveNumber>(drive));
                }
            }
        }
    
    // Set the default storage.
    TUint defaultDrive;
    iFrameworkSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::EDefaultStorageDrive, defaultDrive);
    
    if ( defaultDrive <= EDriveZ )
        {
        // Default drive is specified by drive number.. retrieve from manager..
        if (iAllocateLogicalStorages)
            {
            mgr.SetDefaultStorageId(mgr.FrameworkStorageId(static_cast<TDriveNumber>(defaultDrive)));
            }
        else
            {
            mgr.SetDefaultStorageId(mgr.PhysicalStorageId(static_cast<TDriveNumber>(defaultDrive)));
            }
        }
    else
       {
       // Default drive is specified by storage number
       mgr.SetDefaultStorageId(defaultDrive);
       }
       
    __FLOG(_L8("EnumerateStoragesL - Exit"));
    }

/**
Initiates storage change notice subscription.
*/
void CMTPStorageWatcher::Start()
    {
    __FLOG(_L8("Start - Entry"));
    if (!(iState & EStarted))
        {
        __FLOG(_L8("Starting RFs notifier"));
        TRequestStatus* status(&iStatus);
        User::RequestComplete(status, KErrNone);
        SetActive();
        iState |= EStarted;
        }
    __FLOG(_L8("Start - Exit"));    
    }
    
void CMTPStorageWatcher::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));
    __FLOG(_L8("Stopping RFs notifier"));
    iFrameworkSingletons.Fs().NotifyChangeCancel();
    iState &= (!EStarted);
    __FLOG(_L8("DoCancel - Exit"));
    }

/**
Append all DPs folder exclusion list strings in Device DP
 */
void CMTPStorageWatcher::AppendFolderExclusionListL()
    {
    CDesCArraySeg* folderExclusionSets = new (ELeave) CDesCArraySeg(KFolderExclusionGranularity);
    CleanupStack::PushL(folderExclusionSets);
    CMTPDataProviderController& dps(iFrameworkSingletons.DpController());
    TUint currentDpIndex = 0, count = dps.Count();
    while (currentDpIndex < count)
        {
        CMTPDataProvider& dp(dps.DataProviderByIndexL(currentDpIndex));
        if(KMTPImplementationUidDeviceDp != dp.ImplementationUid().iUid)
            {
            folderExclusionSets->Reset();
            dp.Plugin().SupportedL(EFolderExclusionSets,*folderExclusionSets);
            for(TInt i = 0; i < folderExclusionSets->Count(); ++i)
                {
                TPtrC16 excludedFolder = (*folderExclusionSets)[i];
                iFolderExclusionList->AppendL(excludedFolder);
                }
            }
        currentDpIndex++;
        }
    CleanupStack::PopAndDestroy(folderExclusionSets);
    }

/**
Handles leaves occurring in RunL.
@param aError leave error code
@return KErrNone
*/
#ifdef __FLOG_ACTIVE
TInt CMTPStorageWatcher::RunError(TInt aError)
#else
TInt CMTPStorageWatcher::RunError(TInt /*aError*/)
#endif
    {
    __FLOG(_L8("RunError - Entry"));
    __FLOG_VA((_L8("Error = %d"), aError));

    // Ignore the error, meaning that the storages may not be accurately accounted for
    RequestNotification();

    __FLOG(_L8("RunError - Exit"));
    return KErrNone;
    }
    
void CMTPStorageWatcher::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    const TUint32 previous(iDrivesConfig);
    const TUint32 current(DriveConfigurationL(KAvailableDrives));
    if (current != previous)
        {        
        const TUint32 changed(current ^ previous);
        const TUint32 added(changed & current);
        const TUint32 removed(changed & previous);
        TInt i(KMaxDrives);
        while (i--)
            {
            const TUint32 mask(1 << i);
            if (added & mask)
                {
                StorageAvailableL(static_cast<TDriveNumber>(i));
                }
            else if (removed & mask)
                {
                StorageUnavailableL(static_cast<TDriveNumber>(i));
                }
            }
        }
    iDrivesConfig = current;
    RequestNotification();
    __FLOG(_L8("RunL - Exit"));
    }
    
/**
Constructor.
@param aConnectionMgr The MTP connection manager interface.
*/
CMTPStorageWatcher::CMTPStorageWatcher(MMTPDataProviderFramework& aFramework) :
    CActive(EPriorityStandard),
    iFramework(aFramework)
    {
    CActiveScheduler::Add(this);
    }
    
/**
Second phase constructor.
*/
void CMTPStorageWatcher::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    iFrameworkSingletons.OpenL();
    iFrameworkSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ELogicalStorageIdsAllocationEnable, iAllocateLogicalStorages);
    
    iDpSingletons.OpenL(iFramework);
    iDevDpSingletons.OpenL(iFramework);
    iFolderExclusionList = iDevDpSingletons.ConfigMgr().GetArrayValueL(CMTPDeviceDpConfigMgr::EFolderExclusionList); 

    __FLOG(_L8("ConstructL - Exit"));
    }
    
TUint32 CMTPStorageWatcher::DriveConfigurationL(TBool aAllDrives) const
    {
    __FLOG(_L8("DriveConfigurationL - Entry"));
    TUint32     config(0);
    TDriveList  drives;
    RFs&        fs(iFrameworkSingletons.Fs());
    User::LeaveIfError(fs.DriveList(drives));
    TInt i(KMaxDrives);
    while (i--)
        {        
        __FLOG_VA((_L8("Drive number %d, available = 0x%02d"), i, drives [i]));
        if ((drives[i]) &&
            (!Excluded(static_cast<TDriveNumber>(i))))
            {
            TDriveInfo info;
            User::LeaveIfError(fs.Drive(info, i));
            if ((info.iType != EMediaNotPresent) || (aAllDrives))
                {
                TVolumeInfo volumeInfo;
                if(KErrNone == fs.Volume(volumeInfo,i))
                	{
                	config |=  (1 << i);
                	}
                }
            }
        }
    __FLOG_VA((_L8("Drives list = 0x%08X, AllDrives = %d"), config, aAllDrives));
    __FLOG(_L8("DriveConfigurationL - Exit"));
    return config;
    }

TBool CMTPStorageWatcher::Excluded(TDriveNumber aDriveNumber) const
    {
    __FLOG(_L8("Excluded - Entry"));
    TBool ret(iDrivesExcluded.FindInOrder(aDriveNumber) != KErrNotFound);
    __FLOG_VA((_L8("Drive = %d, excluded = %d"), aDriveNumber, ret));
    __FLOG(_L8("Excluded - Exit"));
    return ret;
    }
    
void CMTPStorageWatcher::RequestNotification()
    {
    __FLOG(_L8("RequestNotification - Entry"));
    _LIT(KPath, "?:\\..");
    iFrameworkSingletons.Fs().NotifyChange(ENotifyEntry, iStatus, KPath);
    SetActive();
    __FLOG(_L8("RequestNotification - Exit"));
    }
 
void CMTPStorageWatcher::SendEventL(TUint16 aEvent, TUint32 aStorageId)
    {
    __FLOG(_L8("SendEventL - Entry"));
    if (iState & EStarted)
        {
        __FLOG_VA((_L8("Sending event 0x%04X for StorageID 0x%08X"), aEvent, aStorageId));
        iEvent.Reset();
        iEvent.SetUint16(TMTPTypeEvent::EEventCode, aEvent);
        iEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
        iEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
        iEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aStorageId);
        iFramework.SendEventL(iEvent);
        }
    __FLOG(_L8("SendEventL - Exit"));
    }

/**
Configures the specified drive as an available MTP storage.
@param aDriveNumber The Symbian OS file system drive number.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPStorageWatcher::StorageAvailableL(TDriveNumber aDriveNumber)
    {
    __FLOG(_L8("StorageAvailableL - Entry"));
    __FLOG_VA((_L8("Drive = %d is available."), aDriveNumber));
    CMTPStorageMgr& mgr(iFrameworkSingletons.StorageMgr());
    TInt32 physical(mgr.PhysicalStorageId(aDriveNumber));
    _LIT(KSuidTemplate, "?:");
    // Generate the storage SUID as the drive root folder.
    RBuf suid;
    suid.CleanupClosePushL();
    suid.Assign((KSuidTemplate().AllocL()));
    TChar driveChar;
    User::LeaveIfError(iFramework.Fs().DriveToChar(aDriveNumber, driveChar));
    driveChar.LowerCase();
    suid[0] = driveChar;
    // Create the storage meta-data.
    CMTPStorageMetaData* storage = CMTPStorageMetaData::NewLC();
    storage->SetUint(CMTPStorageMetaData::EStorageSystemType, CMTPStorageMetaData::ESystemTypeDefaultFileSystem);
    storage->SetDesCL(CMTPStorageMetaData::EStorageSuid, suid);
    if(physical == KErrNotFound)
    	{
        TUint32 id(mgr.AllocatePhysicalStorageIdL(iFramework.DataProviderId(), *storage));
        mgr.SetDriveMappingL(aDriveNumber, id);
    	}
    physical = mgr.PhysicalStorageId(aDriveNumber);

    User::LeaveIfError(physical);
    TUint32 logical(physical);

    // If configured to do so, assign a logical storage ID mapping.
    if (iAllocateLogicalStorages)
        {
        __FLOG(_L8("Assigning local storage ID mapping"));
        
        // Try to read from resource file to use a specified root dir path, if available.
        RBuf rootDirPath;
        rootDirPath.CreateL(KMaxFileName);
        rootDirPath.CleanupClosePushL();
        RMTPDeviceDpSingletons devSingletons;
        devSingletons.OpenL(iFramework);
        CleanupClosePushL(devSingletons);
        TRAPD(resError, devSingletons.ConfigMgr().GetRootDirPathL(aDriveNumber, rootDirPath));
        __FLOG_VA((_L8("ResError = %d"), resError));
        if ((KErrNone == resError) && (0 < rootDirPath.Length()))
            {
            __FLOG(_L8("Reading resource file succeeded"));
            // If there is a root directory information in rss file then check the directory exist or not. 
            // If not exists, then create it. 
            // Before doing anything, delete the leading and trailing white space.
            rootDirPath.Trim();       
            TBuf<KMaxFileName> buffer;
            buffer.Append(driveChar);
            _LIT(KSeperator,":");
            buffer.Append(KSeperator);
            buffer.Append(rootDirPath);
            TInt error = iFramework.Fs().MkDir(buffer);
            suid.Close();
            _LIT(KSuidTemplate, "?:\\");
            suid.Assign((KSuidTemplate().AllocL()));
            driveChar.LowerCase();
            suid[0] = driveChar;

            if ((KErrNone == error) || (KErrAlreadyExists == error))
                {
                __FLOG(_L8("Overwriting SUID to specified root dir path from resource file"));  
                //if dir already existed or created, make that as root directory
                suid.ReAllocL(buffer.Length());
                suid = buffer;
                }
            }
        CleanupStack::PopAndDestroy(&devSingletons);
        CleanupStack::PopAndDestroy(&rootDirPath);
        
        // Set up folder exclusion list
        CDesCArraySeg* storageExclusions = new (ELeave) CDesCArraySeg(KFolderExclusionGranularity);
        CleanupStack::PushL(storageExclusions);
        TInt excludedFolderCount = iFolderExclusionList->Count();

        for (TInt i = 0; i < excludedFolderCount; ++i)
            {
            TPtrC16 excludedFolder = (*iFolderExclusionList)[i];

            if (excludedFolder[0] == '?' ||
                excludedFolder[0] == '*' ||
                excludedFolder[0] == suid[0])
                {
                storageExclusions->AppendL(excludedFolder);
                }
            }
        for ( TInt i=0; i<storageExclusions->Count();++i)
            {
            HBufC16* temp = static_cast<TPtrC16>((*storageExclusions)[i]).AllocL();
            TPtr16 tempptr(temp->Des());
            tempptr[0] = suid[0];
            storage->SetHashPath(tempptr,i);
            delete temp;
            }
        
        storage->SetDesCL(CMTPStorageMetaData::EStorageSuid, suid);
        storage->SetDesCArrayL(CMTPStorageMetaData::EExcludedAreas, *storageExclusions);
        CleanupStack::PopAndDestroy(storageExclusions);

        // Create the logical StorageID and drive mapping.
        logical = mgr.AllocateLogicalStorageIdL(iFramework.DataProviderId(), physical, *storage);
        mgr.SetDriveMappingL(aDriveNumber, logical);

        __FLOG_VA((_L8("Drive = %d mapped as storage 0x%08X"), aDriveNumber, logical));
        }

    CleanupStack::PopAndDestroy(storage);
    CleanupStack::PopAndDestroy(&suid);
    
    // Notify the active data providers.
    if (iState & EStarted)
        {
        TMTPNotificationParamsStorageChange params = {physical};
        iFrameworkSingletons.DpController().NotifyDataProvidersL(EMTPStorageAdded, static_cast<TAny*>(&params));
        
        if(iFrameworkSingletons.DpController().EnumerateState() == CMTPDataProviderController::EEnumeratingSubDirFiles)
			{
			iDevDpSingletons.PendingStorages().InsertInOrder(logical);
			}
        }

    // Notify any connected Initiator(s).
    if (iAllocateLogicalStorages)
        {
        SendEventL(EMTPEventCodeStoreAdded, logical);
        }
        
    __FLOG(_L8("StorageAvailableL - Exit"));
    }

/**
Configures the specified drive as an unavailable MTP storage.
@param aDriveNumber The Symbian OS file system drive number.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPStorageWatcher::StorageUnavailableL(TDriveNumber aDriveNumber)
{
    __FLOG(_L8("StorageUnavailableL - Entry"));
    __FLOG_VA((_L8("Drive = %d is unavailable."), aDriveNumber));
    CMTPStorageMgr& mgr(iFrameworkSingletons.StorageMgr());
    TInt32 physical(mgr.PhysicalStorageId(aDriveNumber));
    User::LeaveIfError(physical);
    TUint32 logical(0);
    
    // If configured to do so, assign a logical storage ID mapping.
    if (iAllocateLogicalStorages)
        {
        logical = mgr.FrameworkStorageId(aDriveNumber);

        // Deassign the logical storage ID mapping.
        mgr.DeallocateLogicalStorageIds(iFramework.DataProviderId(), physical);
        mgr.SetDriveMappingL(aDriveNumber, physical);
        __FLOG_VA((_L8("Drive = %d unmapped as storage 0x%08X"), aDriveNumber, logical));
        }

    // Notify the active data providers.
    TMTPNotificationParamsStorageChange params = {physical};
    iFrameworkSingletons.DpController().NotifyDataProvidersL(EMTPStorageRemoved, static_cast<TAny*>(&params));

    TInt index = iDevDpSingletons.PendingStorages().FindInOrder( logical);
	if(KErrNotFound != index)
		{
		iDevDpSingletons.PendingStorages().Remove(index);
		}
        	
    // Notify any connected Initiator(s).
    if (iAllocateLogicalStorages)
        {
        SendEventL(EMTPEventCodeStoreRemoved, logical);
        }   
    __FLOG(_L8("StorageUnavailableL - Exit"));
    }
