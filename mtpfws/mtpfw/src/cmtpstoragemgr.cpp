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

#include <bautils.h>
#include <mtp/cmtptypestring.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpprotocolconstants.h>

#include "cmtpdataprovidercontroller.h"
#include "cmtpstoragemgr.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpstoragemgrTraces.h"
#endif


// Class constants.

// StorageID bit manipulation patterns.
static const TUint32    KLogicalIdMask(0x0000FFFF);
static const TUint32    KPhysicalIdMask(0xFFFF0000);

static const TUint      KLogicalNumberMask(0x000000FF);
static const TUint      KLogicalOwnerShift(8);
static const TUint      KPhysicalNumberShift(16);
static const TUint      KPhysicalOwnerShift(24);
static const TUint8     KMaxOwnedStorages(0xFF);

/**
MTP data provider framework storage manager factory method.
@return A pointer to an MTP data provider framework storage manager. Ownership 
IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMgr* CMTPStorageMgr::NewL()
    {
    CMTPStorageMgr* self = new(ELeave) CMTPStorageMgr();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPStorageMgr::~CMTPStorageMgr()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_CMTPSTORAGEMGR_DES_ENTRY );
    iPhysicalStorageNumbers.Reset();
    iStorages.ResetAndDestroy();
    iSingletons.Close();
    OstTraceFunctionExit0( CMTPSTORAGEMGR_CMTPSTORAGEMGR_DES_EXIT );
    }

/**
Extracts the storage number of the logical storage ID encoded in the specified
StorageID.
@param aStorageId The storage ID.
@return The storage number.
*/
EXPORT_C TUint CMTPStorageMgr::LogicalStorageNumber(TUint32 aStorageId) 
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_LOGICALSTORAGENUMBER_ENTRY );
    OstTraceFunctionExit0( CMTPSTORAGEMGR_LOGICALSTORAGENUMBER_EXIT );
    return (aStorageId & KLogicalNumberMask);
    }

/**
Extracts the ID of the data provider responsible for the logical storage ID 
encoded in the specified StorageID.
@param aStorageId The storage ID.
@return The data provider owner ID.
*/    
EXPORT_C TUint CMTPStorageMgr::LogicalStorageOwner(TUint32 aStorageId) 
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_LOGICALSTORAGEOWNER_ENTRY );
    return ((aStorageId & KLogicalIdMask) >> KLogicalOwnerShift);
    }

/**
Extracts the storage number of the physical storage ID encoded in the specified
StorageID.
@param aStorageId The storage ID.
@return The storage number.
*/
EXPORT_C TUint CMTPStorageMgr::PhysicalStorageNumber(TUint32 aStorageId) 
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_PHYSICALSTORAGENUMBER_ENTRY );
    return ((aStorageId & KPhysicalIdMask) >> KPhysicalNumberShift);
    }
    
/**
Extracts the ID of the data provider responsible for the physical storage ID 
encoded in the specified StorageID.
@param aStorageId The storage ID.
@return The data provider owner ID.
*/
EXPORT_C TUint CMTPStorageMgr::PhysicalStorageOwner(TUint32 aStorageId) 
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_PHYSICALSTORAGEOWNER_ENTRY );
    return ((aStorageId & KPhysicalIdMask) >> KPhysicalOwnerShift);
    }

/**
Sets the default MTP StorageID. This should be set once at start up and not 
subsequently changed.
@param aStorageId The system default MTP StorageID.
@panic USER 0, in debug builds only, if the default StorageID is set more than
once.
*/    
EXPORT_C void CMTPStorageMgr::SetDefaultStorageId(TUint32 aStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_SETDEFAULTSTORAGEID_ENTRY );
    iDefaultStorageId = aStorageId;
    OstTrace1(TRACE_NORMAL, CMTPSTORAGEMGR_SETDEFAULTSTORAGEID, 
            "Default StorageId = 0x%08X", aStorageId);
    OstTraceFunctionExit0( CMTPSTORAGEMGR_SETDEFAULTSTORAGEID_EXIT );
    }

/**
Creates a mapping between the specified Symbian OS drive number and MTP 
StorageID.
@param aDriveNumber The Symbian OS drive number.
@param aStorageId The MTP StorageID.
@leave One of the sysem wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMTPStorageMgr::SetDriveMappingL(TDriveNumber aDriveNumber, TUint32 aStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_SETDRIVEMAPPINGL_ENTRY );
    iMapDriveToStorage[aDriveNumber] = aStorageId;
    OstTraceExt2(TRACE_NORMAL, CMTPSTORAGEMGR_SETDRIVEMAPPINGL, 
            "Drive number %d = StorageID 0x%08X", (int)aDriveNumber, (int)aStorageId);
    OstTraceFunctionExit0( CMTPSTORAGEMGR_SETDRIVEMAPPINGL_EXIT );
    }

/**
Sets the framework storages owner identifier. This should be set once at start 
up and not subsequently changed.
@param aDataProviderId The framework storages owner identifier.
@panic USER 0, in debug builds only, if the framework storages owner identifier
is set more than once.
*/    
EXPORT_C void CMTPStorageMgr::SetFrameworkId(TUint aDataProviderId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_SETFRAMEWORKID_ENTRY );
    __ASSERT_DEBUG((iFrameworkId == KErrNotFound), User::Invariant());
    iFrameworkId = aDataProviderId;
    OstTrace1(TRACE_NORMAL, CMTPSTORAGEMGR_SETFRAMEWORKID, 
                "System storages owner DP Id = %d", aDataProviderId);    
    OstTraceFunctionExit0( CMTPSTORAGEMGR_SETFRAMEWORKID_EXIT );
    }    
    
EXPORT_C TUint32 CMTPStorageMgr::AllocateLogicalStorageIdL(TUint aDataProviderId, TDriveNumber aDriveNumber, const CMTPStorageMetaData& aStorage)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL_TDRIVERNUMBER_ENTRY );
    TUint id(AllocateLogicalStorageIdL(aDataProviderId, PhysicalStorageId(aDriveNumber), aStorage));
    OstTraceFunctionExit0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL_EXIT );
    return id;
    }

EXPORT_C TUint32 CMTPStorageMgr::AllocateLogicalStorageIdL(TUint aDataProviderId, TUint32 aPhysicalStorageId, const CMTPStorageMetaData& aStorage)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL_TUINT32_ENTRY );
    //if support uninstall DP, comment the below assert.
    //__ASSERT_DEBUG((aDataProviderId < iSingletons.DpController().Count()), User::Invariant());
    
    // Resolve the physical storage.
    CMTPStorageMetaData& physical(StorageMetaDataL(aPhysicalStorageId));
    // Validate the SUID and storage type.
    if (iStorages.Find(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), StorageKeyMatchSuid) != KErrNotFound)
        {
        // SUID is not unique.
        OstTrace0( TRACE_ERROR, CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, "SUID is not unique" );
        User::Leave(KErrAlreadyExists);
        }
    else if (aStorage.Uint(CMTPStorageMetaData::EStorageSystemType) != physical.Uint(CMTPStorageMetaData::EStorageSystemType))
        {
        // Physical/logical storage type mis-match.
        OstTrace0( TRACE_ERROR, DUP2_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, "Physical/logical storage type mis-match" );   
        User::Leave(KErrArgument);
        }
    else if (aStorage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
        {   
        // Validate that the SUID path exists.
        if (!BaflUtils::PathExists(iSingletons.Fs(), aStorage.DesC(CMTPStorageMetaData::EStorageSuid)))
            {
            OstTrace0( TRACE_ERROR, DUP3_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, "the SUID path doesn't exist" );
            User::Leave(KErrPathNotFound);
            }
     
        // Validate that the SUID path corresponds to the physical storage drive.
        TInt storageDrive(DriveNumber(aPhysicalStorageId));
        TParse p;
        LEAVEIFERROR(p.Set(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), NULL, NULL),
                OstTrace0( TRACE_ERROR, DUP4_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, "storage can't be parsed!"));       
        TInt suidDrive(0);
        LEAVEIFERROR(iSingletons.Fs().CharToDrive(TChar(p.Drive()[0]), suidDrive),
                OstTrace0( TRACE_ERROR, DUP5_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, "CharToDrive failed!" ));
                
        if (suidDrive != storageDrive)
            {
            // SUID path/physical storage drive mis-match.
            OstTraceExt2( TRACE_ERROR, DUP6_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, 
                    "SUID path %d /physical storage drive %d  mis-match", suidDrive, storageDrive);     
            User::Leave(KErrArgument);
            }
        }
    
    // Allocate a logical StorageId.
    TInt32 id(AllocateLogicalStorageId(aDataProviderId, aPhysicalStorageId));
    LEAVEIFERROR(id,
            OstTraceExt2( TRACE_ERROR, DUP7_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL, 
                    "allocate logical storageId for Dp %d, Physical StorageId %d failed!", aDataProviderId, aPhysicalStorageId));

    // Create the logical storage meta-data.
    CMTPStorageMetaData* logical(CMTPStorageMetaData::NewLC(aStorage));
    logical->SetUint(CMTPStorageMetaData::EStorageId, id);
    
    // Store the logical storage meta-data.
    iStorages.InsertInOrderL(logical, StorageOrder);
    CleanupStack::Pop(logical);
    
    // Associate the logical and physical storages.
    RArray<TUint> logicals;
    CleanupClosePushL(logicals);
    physical.GetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, logicals);
    logicals.InsertInOrderL(id);
    physical.SetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, logicals);
    CleanupStack::PopAndDestroy(&logicals);
    
#ifdef OST_TRACE_COMPILER_IN_USE
    HBufC8* buf(HBufC8::NewLC(aStorage.DesC(CMTPStorageMetaData::EStorageSuid).Length()));
    buf->Des().Copy(aStorage.DesC(CMTPStorageMetaData::EStorageSuid));
    OstTraceExt2(TRACE_NORMAL, DUP8_CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL_TUINT32,
            "Allocated logical StorageID 0x%08X for storage SUID %s", (TUint)id, *buf);
    CleanupStack::PopAndDestroy(buf);
#endif // OST_TRACE_COMPILER_IN_USE    
    OstTraceFunctionExit0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEIDL_TUINT32_EXIT );
    return id;
    }

EXPORT_C TUint32 CMTPStorageMgr::AllocatePhysicalStorageIdL(TUint aDataProviderId, const CMTPStorageMetaData& aStorage)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEIDL_ENTRY );
    
    // Validate the SUID.
    if (iStorages.Find(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), StorageKeyMatchSuid) != KErrNotFound)
        {
        // SUID is not unique.
        OstTrace0( TRACE_ERROR, DUP1_CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEIDL, "SUID is not unique" );
        User::Leave(KErrAlreadyExists);
        }
    
    // Allocate a physical StorageId.
    TInt32 id(AllocatePhysicalStorageId(aDataProviderId));
    LEAVEIFERROR(id,
            OstTrace1( TRACE_ERROR, DUP2_CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEIDL, "Allocate Physical StorageId for Dp %d failed", aDataProviderId));
            
    
    // Create the physical storage meta-data.
    CMTPStorageMetaData* physical(CMTPStorageMetaData::NewLC(aStorage));
    const RArray<TUint> noStorages;
    physical->SetUint(CMTPStorageMetaData::EStorageId, id);
    physical->SetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, noStorages);
    
    // Store the physical storage meta-data.
    iStorages.InsertInOrderL(physical, StorageOrder);
    CleanupStack::Pop(physical);
    
    OstTrace1(TRACE_NORMAL, CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEIDL,
            "Allocated physical StorageID 0x%08X", id);
    OstTraceFunctionExit0( CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEIDL_EXIT );
    return id;
    }

EXPORT_C TInt CMTPStorageMgr::DeallocateLogicalStorageId(TUint aDataProviderId, TUint32 aLogicalStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_DEALLOCATELOGICALSTORAGEID_ENTRY );
    TInt ret(KErrArgument);
    
    // Validate the StorageID.
    if (LogicalStorageId(aLogicalStorageId))
        {
        ret = iStorages.FindInOrder(aLogicalStorageId, StorageOrder);
        if (ret != KErrNotFound)
            {
            // Validate the storage owner.
            if (LogicalStorageOwner(iStorages[ret]->Uint(CMTPStorageMetaData::EStorageId)) != aDataProviderId)
                {
                ret = KErrAccessDenied;
                }
            else
                {
                TRAPD(err, RemoveLogicalStorageL(ret));
                ret = err;
                }
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_DEALLOCATELOGICALSTORAGEID_EXIT );
    return ret;
    }

EXPORT_C void CMTPStorageMgr::DeallocateLogicalStorageIds(TUint aDataProviderId, TUint32 aPhysicalStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_DEALLOCATELOGICALSTORAGEIDS_ENTRY );
    TInt ret(iStorages.FindInOrder(aPhysicalStorageId, StorageOrder));
    if (ret != KErrNotFound)
        {
        const RArray<TUint>& logicals(iStorages[ret]->UintArray(CMTPStorageMetaData::EStorageLogicalIds));
        TUint count(logicals.Count());
        while (count)
            {
            const TUint KIdx(count - 1);
            if (LogicalStorageOwner(logicals[KIdx]) == aDataProviderId)
                {
                DeallocateLogicalStorageId(aDataProviderId, logicals[KIdx]);
                }
            count--;
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_DEALLOCATELOGICALSTORAGEIDS_EXIT );
    }

EXPORT_C TInt CMTPStorageMgr::DeallocatePhysicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_DEALLOCATEPHYSICALSTORAGEID_ENTRY );
    TInt ret(KErrArgument);
    
    // Validate the StorageID.
    if (!LogicalStorageId(aPhysicalStorageId))
        {
        ret = iStorages.FindInOrder(aPhysicalStorageId, StorageOrder);
        if (ret != KErrNotFound)
            {
            // Validate the storage owner.
            if (PhysicalStorageOwner(iStorages[ret]->Uint(CMTPStorageMetaData::EStorageId)) != aDataProviderId)
                {
                ret = KErrAccessDenied;
                }
            else
                {
                // Deallocate all associated logical storages.
                const RArray<TUint>& logicals(iStorages[ret]->UintArray(CMTPStorageMetaData::EStorageLogicalIds));
                TUint count(logicals.Count());
                while (count)
                    {
                    const TUint KIdx(--count);
                    DeallocateLogicalStorageId(aDataProviderId, logicals[KIdx]);
                    }
                
                // Delete the storage.
                delete iStorages[ret];
                iStorages.Remove(ret);
                }
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_DEALLOCATEPHYSICALSTORAGEID_EXIT );
    return ret;
    }

EXPORT_C TUint32 CMTPStorageMgr::DefaultStorageId() const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_DEFAULTSTORAGEID_ENTRY );
    OstTraceFunctionExit0( CMTPSTORAGEMGR_DEFAULTSTORAGEID_EXIT );
    return iDefaultStorageId;
    }

EXPORT_C TInt CMTPStorageMgr::DriveNumber(TUint32 aStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_DRIVENUMBER_ENTRY );
    TInt drive(KErrNotFound);
    if (PhysicalStorageOwner(aStorageId) == iFrameworkId)
        {
        const TUint32 KPhysicalId(PhysicalStorageId(aStorageId));
        const TUint KCount(iMapDriveToStorage.Count());
        for (TUint i(0); ((i < KCount) && (drive == KErrNotFound)); i++)
            {
            if (PhysicalStorageId(iMapDriveToStorage[i]) == KPhysicalId)
                {
                drive = i;
                }
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_DRIVENUMBER_EXIT );
    return drive;
    }

EXPORT_C TInt32 CMTPStorageMgr::FrameworkStorageId(TDriveNumber aDriveNumber) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_FRAMEWORKSTORAGEID_ENTRY );
    TInt32 ret(KErrNotFound);
    TInt32 id(iMapDriveToStorage[aDriveNumber]);
    if ((id != KErrNotFound) && (LogicalStorageId(id)))
        {
        ret = id;
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_FRAMEWORKSTORAGEID_EXIT );
    return ret;
    }

EXPORT_C void CMTPStorageMgr::GetAvailableDrivesL(RArray<TDriveNumber>& aDrives) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_GETAVAILABLEDRIVESL_ENTRY );
    aDrives.Reset();
    for (TUint i(0); (i < iMapDriveToStorage.Count()); i++)
        {
        if (iMapDriveToStorage[i] != KErrNotFound)
            {
            aDrives.AppendL(static_cast<TDriveNumber>(i));
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_GETAVAILABLEDRIVESL_EXIT );
    }

EXPORT_C void CMTPStorageMgr::GetLogicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_GETLOGICALSTORAGESL_ENTRY );
    aStorages.Reset();
    const TBool KAllStorages(aParams.StorageSuid() == KNullDesC);
    const TBool KAllStorageSystemTypes(aParams.StorageSystemType() == CMTPStorageMetaData::ESystemTypeUndefined);
    const TUint KCount(iStorages.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        const CMTPStorageMetaData& storage(*iStorages[i]);
        if (((KAllStorages) || (storage.DesC(CMTPStorageMetaData::EStorageSuid) == aParams.StorageSuid())) &&
            ((KAllStorageSystemTypes) || (storage.Uint(CMTPStorageMetaData::EStorageSystemType) == aParams.StorageSystemType())) &&
            (LogicalStorageId(storage.Uint(CMTPStorageMetaData::EStorageId))))
            {
            aStorages.AppendL(iStorages[i]);
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_GETLOGICALSTORAGESL_EXIT );
    }

EXPORT_C void CMTPStorageMgr::GetPhysicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_GETPHYSICALSTORAGESL_ENTRY );
    aStorages.Reset();
    const TBool KAllStorages(aParams.StorageSuid() == KNullDesC);
    const TBool KAllStorageSystemTypes(aParams.StorageSystemType() == CMTPStorageMetaData::ESystemTypeUndefined);
    const TUint KCount(iStorages.Count());
    for (TUint i(0); (i < KCount); i++)
        {
        const CMTPStorageMetaData& storage(*iStorages[i]);
        if (((KAllStorages) || (storage.DesC(CMTPStorageMetaData::EStorageSuid) == aParams.StorageSuid())) &&
            ((KAllStorageSystemTypes) || (storage.Uint(CMTPStorageMetaData::EStorageSystemType) == aParams.StorageSystemType())) &&
            (!LogicalStorageId(storage.Uint(CMTPStorageMetaData::EStorageId))))
            {
            aStorages.AppendL(iStorages[i]);
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_GETPHYSICALSTORAGESL_EXIT );
    }

EXPORT_C TUint32 CMTPStorageMgr::LogicalStorageId(TUint32 aStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_LOGICALSTORAGEID_TUINT32_ENTRY );
    OstTraceFunctionExit0( CMTPSTORAGEMGR_LOGICALSTORAGEID_TUINT32_EXIT );
    return (aStorageId & KLogicalIdMask);
    }

EXPORT_C TInt32 CMTPStorageMgr::LogicalStorageId(const TDesC& aStorageSuid) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_LOGICALSTORAGEID_TDESC_ENTRY );
    TInt32 id(KErrNotFound);
    TInt idx(iStorages.Find(aStorageSuid, StorageKeyMatchSuid));
    if (idx != KErrNotFound)
        {
        id = iStorages[idx]->Uint(CMTPStorageMetaData::EStorageId);
        if (!LogicalStorageId(id))
            {
            id = KErrNotFound;
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_LOGICALSTORAGEID_TDESC_EXIT );
    return id;
    }

EXPORT_C TInt32 CMTPStorageMgr::PhysicalStorageId(TDriveNumber aDriveNumber) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_PHYSICALSTORAGEID_TDRIVENUMBER_ENTRY );
    TInt32 storageId(iMapDriveToStorage[aDriveNumber]);
    if (storageId != KErrNotFound)
        {
        storageId = PhysicalStorageId(storageId);
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_PHYSICALSTORAGEID_TDRIVENUMBER_EXIT );
    return storageId;
    }

EXPORT_C TUint32 CMTPStorageMgr::PhysicalStorageId(TUint32 aStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_PHYSICALSTORAGEID_TUINT32_ENTRY );
    OstTraceFunctionExit0( CMTPSTORAGEMGR_PHYSICALSTORAGEID_TUINT32_EXIT );
    return (aStorageId & KPhysicalIdMask);
    }

EXPORT_C const CMTPStorageMetaData& CMTPStorageMgr::StorageL(TUint32 aStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_STORAGEL_ENTRY );
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    LEAVEIFERROR(idx,
            OstTrace1( TRACE_ERROR, CMTPSTORAGEMGR_STORAGEL, "can't find storage with StorageId %d", idx ));
    OstTraceFunctionExit0( CMTPSTORAGEMGR_STORAGEL_EXIT );
    return *iStorages[idx];
    }

EXPORT_C TUint32 CMTPStorageMgr::StorageId(TUint32 aPhysicalStorageId, TUint32 aLogicalStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_STORAGEID_ENTRY );
    OstTraceFunctionExit0( CMTPSTORAGEMGR_STORAGEID_EXIT );
    return (aPhysicalStorageId | aLogicalStorageId);
    }

EXPORT_C TBool CMTPStorageMgr::ValidStorageId(TUint32 aStorageId) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_VALIDSTORAGEID_ENTRY );
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    if(KErrNotFound == idx)
    	{
    	OstTraceFunctionExit0( CMTPSTORAGEMGR_VALIDSTORAGEID_EXIT );
    	return EFalse;
    	}
    
    _LIT(KSeperator,"\\");
	TBool ret = ETrue;
	if(iStorages[idx]->Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
		{
		const TDesC& KSuid(iStorages[idx]->DesC(CMTPStorageMetaData::EStorageSuid));
		if(LogicalStorageId(aStorageId) || (KSuid.Right(1) == KSeperator))
			{
			ret = BaflUtils::PathExists(iSingletons.Fs(), KSuid);
			}
		else if(KSuid.Length() >= KMaxFileName)
			{
			ret = EFalse;
			}
		else
			{
			TFileName buf;
			buf.Append(KSuid);
            buf.Append(KSeperator);
            
            ret = BaflUtils::PathExists(iSingletons.Fs(), buf);
			}
		}
    
    OstTraceFunctionExit0( DUP1_CMTPSTORAGEMGR_VALIDSTORAGEID_EXIT );
    return ret;
    }
    
EXPORT_C CMTPTypeString* CMTPStorageMgr::VolumeIdL(TUint aDataProviderId, TUint32 aStorageId, const TDesC& aVolumeIdSuffix) const
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_VOLUMEIDL_ENTRY );

    // Validate the StorageId.
    TUint owner(LogicalStorageId(aStorageId) ? LogicalStorageOwner(aStorageId) : PhysicalStorageOwner(aStorageId));
    if (!ValidStorageId(aStorageId))
        {
        OstTrace1( TRACE_ERROR, CMTPSTORAGEMGR_VOLUMEIDL, "invalid storage id %d", aStorageId);
        User::Leave(KErrNotFound);
        }
    else if (aDataProviderId != owner)
        {
        OstTrace1( TRACE_ERROR, DUP1_CMTPSTORAGEMGR_VOLUMEIDL, "Dp %d not owner", aDataProviderId);
        User::Leave(KErrAccessDenied);
        }
    
    // Generate a unique volume ID.
    RBuf16 buffer;
    buffer.CreateL(KMTPMaxStringCharactersLength);
    CleanupClosePushL(buffer);
    buffer.Format(_L("%08X"), aStorageId); 
           
    if (aVolumeIdSuffix.Length() != 0)
        {
        // Append the separator and suffix, truncating if necessary.
        buffer.Append(_L("-"));
        buffer.Append(aVolumeIdSuffix.Left(KMTPMaxStringCharactersLength - buffer.Length()));
        }

    CMTPTypeString* volumeId = CMTPTypeString::NewL(buffer);
	CleanupStack::PopAndDestroy(&buffer);  
    OstTraceFunctionExit0( CMTPSTORAGEMGR_VOLUMEIDL_EXIT );
    return volumeId;
    }   
    
/**
Constructor.
*/
CMTPStorageMgr::CMTPStorageMgr() :
    iFrameworkId(KErrNotFound)
    {
    }
    
/**
Second phase constructor.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPStorageMgr::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_CONSTRUCTL_ENTRY );
    iSingletons.OpenL();
    for (TUint i(0); (i < KMaxDrives); i++)
        {
        iMapDriveToStorage[i] = KErrNotFound;
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_CONSTRUCTL_EXIT );
    }
    
/**
Allocates a new 32-bit logical StorageId for the storage owner as a partition 
of the specified physical MTP StorageID.
@param aDataProviderId The storage owner data provider identifier.
@param aPhysicalStorageId The physical MTP StorageID.
@return The new logical StorageId.
@return KErrNotFound, if the specified physical MTP StorageID does not exist
@return KErrOverflow, if the maximum number of storages would be exceeded.
*/
TInt32 CMTPStorageMgr::AllocateLogicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEID_ENTRY );
    TInt ret(iStorages.FindInOrder(aPhysicalStorageId, StorageOrder));
    if (ret != KErrNotFound)
        {
        // Scan for the first available storage number.
        const RArray<TUint>& logicalIds(iStorages[ret]->UintArray(CMTPStorageMetaData::EStorageLogicalIds));
        TUint num(1);
        do
            {
            ret = EncodeLogicalStorageId(aPhysicalStorageId, aDataProviderId, num);
            }
        while ((logicalIds.FindInOrder(ret) != KErrNotFound) &&
                (++num <= KMaxOwnedStorages));
                
        if (num >= KMaxOwnedStorages)
            {
            ret = KErrOverflow;
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_ALLOCATELOGICALSTORAGEID_EXIT );
    return ret;
    }
    
/**
Allocates a new 32-bit physical StorageId for the storage owner.
@param aDataProviderId The storage owner data provider identifier.
@return The new physical StorageId.
@return KErrOverflow, if the maximum number of storages would be exceeded.
@return One of the system wide error code, if a processing failure occurs.
*/
TInt32 CMTPStorageMgr::AllocatePhysicalStorageId(TUint aDataProviderId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEID_ENTRY );
    TInt32 ret(KErrNone);
    while ((iPhysicalStorageNumbers.Count() < (aDataProviderId + 1)) && (ret == KErrNone))
        {
        ret = iPhysicalStorageNumbers.Append(0);
        }
        
    if (ret == KErrNone)
        {
        if (iPhysicalStorageNumbers[aDataProviderId] < KMaxOwnedStorages)
            {
            ret = EncodePhysicalStorageId(aDataProviderId, ++iPhysicalStorageNumbers[aDataProviderId]);
            }
        else
            {
            ret = KErrOverflow;
            }
        }
    OstTraceFunctionExit0( CMTPSTORAGEMGR_ALLOCATEPHYSICALSTORAGEID_EXIT );
    return ret;
    }
	
/**
Encodes the specified physical MTP StorageID, data provider identifier, and 
storage number as a fully formed MTP StorageID.
@param aPhysicalStorageId The physical MTP StorageID.
@param aDataProviderId The data provider identifier.
@param aStorageNumber The storage number.
@return The fully formed MTP StorageID.
*/	
TUint32 CMTPStorageMgr::EncodeLogicalStorageId(TUint32 aPhysicalStorageId, TUint aDataProviderId, TUint aStorageNumber)
    {
    return (StorageId(aPhysicalStorageId, (EncodeLogicalStorageOwner(aDataProviderId) | EncodeLogicalStorageNumber(aStorageNumber))));
    }

/**
Encodes the storage identifier as the logical storage number in a fully formed 
MTP StorageID.
@param aStorageNumber The storage number.
@return The encoded logical storage number.
*/	
TUint32 CMTPStorageMgr::EncodeLogicalStorageNumber(TUint aStorageNumber)
	{
	return (aStorageNumber);
	}

/**
Encodes the specified data provider identifier as the logical storage owner 
in a fully formed MTP StorageID.
@param aDataProviderId The data provider identifier.
@return The encoded logical storage owner.
*/
TUint32 CMTPStorageMgr::EncodeLogicalStorageOwner(TUint aDataProviderId)
	{
	return (aDataProviderId << KLogicalOwnerShift);
	}
	
/**
Encodes the specified data provider identifier and storage number as an  
physical MTP StorageID.
@param aDataProviderId The data provider identifier.
@param aStorageNumber The storage number.
@return The encoded physical MTP StorageID.
*/	
TUint32 CMTPStorageMgr::EncodePhysicalStorageId(TUint aDataProviderId, TUint aStorageNumber)
    {
    return (EncodePhysicalStorageOwner(aDataProviderId) | EncodePhysicalStorageNumber(aStorageNumber));
    }

/**
Encodes the storage identifier as the physical storage number in a fully formed 
MTP StorageID.
@param aStorageNumber The storage number.
@return The encoded physical storage number.
*/	
TUint32 CMTPStorageMgr::EncodePhysicalStorageNumber(TUint aStorageNumber)
	{
	return (aStorageNumber << KPhysicalNumberShift);
	}

/**
Encodes the specified data provider identifier as the physical storage owner 
in a fully formed MTP StorageID.
@param aDataProviderId The data provider identifier.
@return The encoded physical storage owner.
*/
TUint32 CMTPStorageMgr::EncodePhysicalStorageOwner(TUint aDataProviderId)
	{
	return (aDataProviderId << KPhysicalOwnerShift);
	}
	
/**
Removes the logical storages table entry at the specified index.
@param aIdx The storages table index.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPStorageMgr::RemoveLogicalStorageL(TUint aIdx)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_REMOVELOGICALSTORAGEL_ENTRY );
    TUint32 id(iStorages[aIdx]->Uint(CMTPStorageMetaData::EStorageId));
    
    // Disassociate the logical and physical storages.
    CMTPStorageMetaData& physical(StorageMetaDataL(PhysicalStorageId(id)));
    RArray<TUint> logicals;
    CleanupClosePushL(logicals);
    physical.GetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, logicals);
    logicals.Remove(logicals.FindInOrderL(id));
    physical.SetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, logicals);
    CleanupStack::PopAndDestroy(&logicals);
    
    // Delete the storage.
    delete iStorages[aIdx];
    iStorages.Remove(aIdx);
    OstTraceFunctionExit0( CMTPSTORAGEMGR_REMOVELOGICALSTORAGEL_EXIT );
    }
    
/**
Provides a non-const reference to the storage meta-data for the specified 
logical MTP StorageID.
@param aStorageId The physical or fully formed logical MTP StorageID.
@leave KErrNotFound if the specified StorageID does not exist.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPStorageMetaData& CMTPStorageMgr::StorageMetaDataL(TUint32 aStorageId)
    {
    OstTraceFunctionEntry0( CMTPSTORAGEMGR_STORAGEMETADATAL_ENTRY );
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    LEAVEIFERROR(idx,
            OstTrace1( TRACE_ERROR, CMTPSTORAGEMGR_STORAGEMETADATAL, "can't find storage with storageId %d", idx ));
    OstTraceFunctionExit0( CMTPSTORAGEMGR_STORAGEMETADATAL_EXIT );
    return *iStorages[idx];
    }
   
/**
Implements a storage key match identity relation using 
@see CMTPStorageMetaData::EStorageSuid.
@param aSuid The storage SUID key value.
@param aStorage The storage meta-data.
@return ETrue if the storage matches the key relation, otherwise EFalse.
*/ 
TBool CMTPStorageMgr::StorageKeyMatchSuid(const TDesC* aSuid, const CMTPStorageMetaData& aStorage)
    {
    return (*aSuid == aStorage.DesC(CMTPStorageMetaData::EStorageSuid));
    }
	
/**
Implements an @see TLinearOrder function for @see CMTPStorageMetaData objects 
based on relative @see CMTPStorageMetaData::EStorageId.
@param aL The first object instance.
@param aR The second object instance.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPStorageMgr::StorageOrder(const CMTPStorageMetaData& aL, const CMTPStorageMetaData& aR)
    {
    return (aL.Uint(CMTPStorageMetaData::EStorageId) - aR.Uint(CMTPStorageMetaData::EStorageId));
    }
	
/**
Implements an @see CMTPStorageMetaData::EStorageId key order function.
@param aKey The key value.
@param aR The storage meta-data.
@return Zero, if the two objects are equal; A negative value, if the first 
object is less than the second, or; A positive value, if the first object is 
greater than the second.
*/
TInt CMTPStorageMgr::StorageOrder(const TUint32* aKey, const CMTPStorageMetaData& aStorage)
    {
    return (*aKey - aStorage.Uint(CMTPStorageMetaData::EStorageId));
    }

EXPORT_C TBool CMTPStorageMgr::IsReadWriteStorage(TUint32 aStorageId) const
	{
	const TInt KCDrive = 2;
	TInt driveNo(DriveNumber(aStorageId));
	if(KErrNotFound == driveNo)
		return ETrue;
	
	if(KCDrive == driveNo)
		return EFalse;
	
	TDriveInfo driveInfo;
	if(iSingletons.Fs().Drive(driveInfo, driveNo) != KErrNone)
		return EFalse;
	
	TBool ret = ETrue;
	switch(driveInfo.iType)
		{
		case EMediaCdRom:
		case EMediaRom:
			ret = EFalse;
			break;
		
		//comment the blank cases.
		//case EMediaNotPresent:
		//case EMediaUnknown:	
		//case EMediaRam:
		//case EMediaNANDFlash:
		//case EMediaHardDisk:
		//case EMediaFlash:					
		//case EMediaRemote:
		//case EMediaFloppy:
		default:
			break;
		}
	
	if(ret)
		{
		TVolumeInfo volumeInfo;
		if(iSingletons.Fs().Volume(volumeInfo, driveNo) == KErrNone)
			{
			if( volumeInfo.iDrive.iMediaAtt & (KMediaAttWriteProtected | KMediaAttLocked) )
				{
				ret = EFalse;
				}
			}
		}
	    	
	return ret;
	}
