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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpstoragewatcherTraces.h"
#endif



// Class constants.
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
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_CMTPSTORAGEWATCHER_ENTRY );
    Cancel();
    delete iFolderExclusionList;
    iDpSingletons.Close();
    iDrivesExcluded.Close();
    iFrameworkSingletons.Close();
    iDevDpSingletons.Close();
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_CMTPSTORAGEWATCHER_EXIT );
    }
    
void CMTPStorageWatcher::EnumerateStoragesL()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_ENUMERATESTORAGESL_ENTRY );

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
            LEAVEIFERROR(iFramework.Fs().DriveToChar(drive, driveChar),
                    OstTrace1( TRACE_ERROR, CMTPSTORAGEWATCHER_ENUMERATESTORAGESL, "drive %d convert to char error!", drive ));
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
       
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_ENUMERATESTORAGESL_EXIT );
    }

/**
Initiates storage change notice subscription.
*/
void CMTPStorageWatcher::Start()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_START_ENTRY );
    if (!(iState & EStarted))
        {
        OstTrace0( TRACE_NORMAL, CMTPSTORAGEWATCHER_START, "Starting RFs notifier" );
        TRequestStatus* status(&iStatus);
        User::RequestComplete(status, KErrNone);
        SetActive();
        iState |= EStarted;
        }
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_START_EXIT );
    }
    
void CMTPStorageWatcher::DoCancel()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_DOCANCEL_ENTRY );
    OstTrace0( TRACE_NORMAL, CMTPSTORAGEWATCHER_DOCANCEL, "Stopping RFs notifier" );
    iFrameworkSingletons.Fs().NotifyChangeCancel();
    iState &= (!EStarted);
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_DOCANCEL_EXIT );
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
#ifdef OST_TRACE_COMPILER_IN_USE
TInt CMTPStorageWatcher::RunError(TInt aError)
#else
TInt CMTPStorageWatcher::RunError(TInt /*aError*/)
#endif
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_RUNERROR_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPSTORAGEWATCHER_RUNERROR, "Error = %d", aError );    

    // Ignore the error, meaning that the storages may not be accurately accounted for
    RequestNotification();

    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_RUNERROR_EXIT );
    return KErrNone;
    }
    
void CMTPStorageWatcher::RunL()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_RUNL_ENTRY );
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
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_RUNL_EXIT );
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
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_CONSTRUCTL_ENTRY );
    iFrameworkSingletons.OpenL();
    iFrameworkSingletons.FrameworkConfig().GetValueL(CMTPFrameworkConfig::ELogicalStorageIdsAllocationEnable, iAllocateLogicalStorages);
    
    iDpSingletons.OpenL(iFramework);
    iDevDpSingletons.OpenL(iFramework);
    iFolderExclusionList = iDevDpSingletons.ConfigMgr().GetArrayValueL(CMTPDeviceDpConfigMgr::EFolderExclusionList); 

    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_CONSTRUCTL_EXIT );
    }
    
TUint32 CMTPStorageWatcher::DriveConfigurationL(TBool aAllDrives) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_DRIVECONFIGURATIONL_ENTRY );
    TUint32     config(0);
    TDriveList  drives;
    RFs&        fs(iFrameworkSingletons.Fs());
    User::LeaveIfError(fs.DriveList(drives));
    TInt i(KMaxDrives);
    while (i--)
        {        
        OstTraceExt2( TRACE_NORMAL, CMTPSTORAGEWATCHER_DRIVECONFIGURATIONL, 
                "Drive number %d, available = 0x%02d", i, drives[i] );
        if ((drives[i]) &&
            (!Excluded(static_cast<TDriveNumber>(i))))
            {
            TDriveInfo info;
            LEAVEIFERROR(fs.Drive(info, i),
                    OstTrace1( TRACE_ERROR, DUP2_CMTPSTORAGEWATCHER_DRIVECONFIGURATIONL, "can't get info for drive %d", i ));
                    
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
    OstTraceExt2( TRACE_NORMAL, DUP1_CMTPSTORAGEWATCHER_DRIVECONFIGURATIONL, 
            "Drives list = 0x%08X, AllDrives = %d", config, (TUint32)aAllDrives );    
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_DRIVECONFIGURATIONL_EXIT );
    return config;
    }

TBool CMTPStorageWatcher::Excluded(TDriveNumber aDriveNumber) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_EXCLUDED_ENTRY );
    TBool ret(iDrivesExcluded.FindInOrder(aDriveNumber) != KErrNotFound);
    OstTraceExt2( TRACE_NORMAL, CMTPSTORAGEWATCHER_EXCLUDED, 
            "Drive = %d, excluded = %d", aDriveNumber, ret );      
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_EXCLUDED_EXIT );
    return ret;
    }
    
void CMTPStorageWatcher::RequestNotification()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_REQUESTNOTIFICATION_ENTRY );
    _LIT(KPath, "?:\\..");
    iFrameworkSingletons.Fs().NotifyChange(ENotifyEntry, iStatus, KPath);
    SetActive();
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_REQUESTNOTIFICATION_EXIT );
    }
 
void CMTPStorageWatcher::SendEventL(TUint16 aEvent, TUint32 aStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_SENDEVENTL_ENTRY );
    if (iState & EStarted)
        {
        OstTraceExt2( TRACE_NORMAL, CMTPSTORAGEWATCHER_SENDEVENTL, 
                "Sending event 0x%04X for StorageID 0x%08X", (TUint32)aEvent, aStorageId );  
        iEvent.Reset();
        iEvent.SetUint16(TMTPTypeEvent::EEventCode, aEvent);
        iEvent.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
        iEvent.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
        iEvent.SetUint32(TMTPTypeEvent::EEventParameter1, aStorageId);
        iFramework.SendEventL(iEvent);
        }
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_SENDEVENTL_EXIT );
    }

/**
Configures the specified drive as an available MTP storage.
@param aDriveNumber The Symbian OS file system drive number.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPStorageWatcher::StorageAvailableL(TDriveNumber aDriveNumber)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_STORAGEAVAILABLEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "Drive = %d is available.", aDriveNumber);
    CMTPStorageMgr& mgr(iFrameworkSingletons.StorageMgr());
    TInt32 physical(mgr.PhysicalStorageId(aDriveNumber));
    _LIT(KSuidTemplate, "?:");
    // Generate the storage SUID as the drive root folder.
    RBuf suid;
    suid.CleanupClosePushL();
    suid.Assign((KSuidTemplate().AllocL()));
    TChar driveChar;
    LEAVEIFERROR(iFramework.Fs().DriveToChar(aDriveNumber, driveChar),
            OstTrace1( TRACE_ERROR, DUP6_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "driver %d convert to char error!", aDriveNumber ));       
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
    LEAVEIFERROR(physical, 
            OstTrace1( TRACE_ERROR, DUP7_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "can't get physical storage id for drive %d", aDriveNumber));
            
    TUint32 logical(physical);

    // If configured to do so, assign a logical storage ID mapping.
    if (iAllocateLogicalStorages)
        {
        OstTrace0( TRACE_NORMAL, DUP1_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "Assigning local storage ID mapping" );       
        
        // Try to read from resource file to use a specified root dir path, if available.
        RBuf rootDirPath;
        rootDirPath.CreateL(KMaxFileName);
        rootDirPath.CleanupClosePushL();
        RMTPDeviceDpSingletons devSingletons;
        devSingletons.OpenL(iFramework);
        CleanupClosePushL(devSingletons);
        TRAPD(resError, devSingletons.ConfigMgr().GetRootDirPathL(aDriveNumber, rootDirPath));
        OstTrace1( TRACE_NORMAL, DUP2_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "ResError = %d", resError);    
        if ((KErrNone == resError) && (0 < rootDirPath.Length()))
            {
            OstTrace0(TRACE_NORMAL, DUP3_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "Reading resource file succeeded");
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
                OstTrace0( TRACE_NORMAL, DUP4_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, "Overwriting SUID to specified root dir path from resource file" );                
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

        OstTraceExt2( TRACE_NORMAL, DUP5_CMTPSTORAGEWATCHER_STORAGEAVAILABLEL, 
                "Drive = %d mapped as storage 0x%08X", (TUint32)aDriveNumber, logical);
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
        
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_STORAGEAVAILABLEL_EXIT );
    }

/**
Configures the specified drive as an unavailable MTP storage.
@param aDriveNumber The Symbian OS file system drive number.
@leave One of the system wide error codes, if a processing failure occurs.
*/    
void CMTPStorageWatcher::StorageUnavailableL(TDriveNumber aDriveNumber)
{
    OstTraceFunctionEntry0( CMTPSTORAGEWATCHER_STORAGEUNAVAILABLEL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPSTORAGEWATCHER_STORAGEUNAVAILABLEL, "Drive = %d is unavailable.", aDriveNumber );
    
    CMTPStorageMgr& mgr(iFrameworkSingletons.StorageMgr());
    TInt32 physical(mgr.PhysicalStorageId(aDriveNumber));
    LEAVEIFERROR(physical,
            OstTrace1( TRACE_ERROR, DUP2_CMTPSTORAGEWATCHER_STORAGEUNAVAILABLEL, "can't get physical storage id for drive %d", aDriveNumber));
    TUint32 logical(0);
    
    // If configured to do so, assign a logical storage ID mapping.
    if (iAllocateLogicalStorages)
        {
        logical = mgr.FrameworkStorageId(aDriveNumber);

        // Deassign the logical storage ID mapping.
        mgr.DeallocateLogicalStorageIds(iFramework.DataProviderId(), physical);
        mgr.SetDriveMappingL(aDriveNumber, physical);
        OstTraceExt2( TRACE_NORMAL, DUP1_CMTPSTORAGEWATCHER_STORAGEUNAVAILABLEL, 
                "Drive = %d unmapped as storage 0x%08X", (TUint32)aDriveNumber, logical);        
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
    OstTraceFunctionExit0( CMTPSTORAGEWATCHER_STORAGEUNAVAILABLEL_EXIT );
    }
