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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"StorageMgr");)

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
    __FLOG(_L8("~CMTPStorageMgr - Entry"));
    iPhysicalStorageNumbers.Reset();
    iStorages.ResetAndDestroy();
    iSingletons.Close();
    __FLOG(_L8("~CMTPStorageMgr - Exit"));
    __FLOG_CLOSE;
    }

/**
Extracts the storage number of the logical storage ID encoded in the specified
StorageID.
@param aStorageId The storage ID.
@return The storage number.
*/
EXPORT_C TUint CMTPStorageMgr::LogicalStorageNumber(TUint32 aStorageId) 
    {
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
    __FLOG(_L8("SetDefaultStorageId - Entry"));
    iDefaultStorageId = aStorageId;
    __FLOG_VA((_L8("Default StorageId = 0x%08X"), aStorageId));
    __FLOG(_L8("SetDefaultStorageId - Exit"));
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
    __FLOG(_L8("DefineDriveNumberMapping - Entry"));
    iMapDriveToStorage[aDriveNumber] = aStorageId;
    __FLOG_VA((_L8("Drive number %d = StorageID 0x%08X"), aDriveNumber, aStorageId));
    __FLOG(_L8("DefineDriveNumberMapping - Exit"));
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
    __FLOG(_L8("SetFrameworkStoragesOwner - Entry"));
    __ASSERT_DEBUG((iFrameworkId == KErrNotFound), User::Invariant());
    iFrameworkId = aDataProviderId;
    __FLOG_VA((_L8("System storages owner DP Id = %d"), aDataProviderId));
    __FLOG(_L8("SetFrameworkStoragesOwner - Exit"));
    }    
    
EXPORT_C TUint32 CMTPStorageMgr::AllocateLogicalStorageIdL(TUint aDataProviderId, TDriveNumber aDriveNumber, const CMTPStorageMetaData& aStorage)
    {
    __FLOG(_L8("AllocateLogicalStorageIdL - Entry"));
    TUint id(AllocateLogicalStorageIdL(aDataProviderId, PhysicalStorageId(aDriveNumber), aStorage));
    __FLOG(_L8("AllocateLogicalStorageIdL - Exit"));
    return id;
    }

EXPORT_C TUint32 CMTPStorageMgr::AllocateLogicalStorageIdL(TUint aDataProviderId, TUint32 aPhysicalStorageId, const CMTPStorageMetaData& aStorage)
    {
    __FLOG(_L8("AllocateLogicalStorageIdL - Entry"));
    //if support uninstall DP, comment the below assert.
    //__ASSERT_DEBUG((aDataProviderId < iSingletons.DpController().Count()), User::Invariant());
    
    // Resolve the physical storage.
    CMTPStorageMetaData& physical(StorageMetaDataL(aPhysicalStorageId));
    // Validate the SUID and storage type.
    if (iStorages.Find(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), StorageKeyMatchSuid) != KErrNotFound)
        {
        // SUID is not unique.
        User::Leave(KErrAlreadyExists);
        }
    else if (aStorage.Uint(CMTPStorageMetaData::EStorageSystemType) != physical.Uint(CMTPStorageMetaData::EStorageSystemType))
        {
        // Physical/logical storage type mis-match.
        User::Leave(KErrArgument);
        }
    else if (aStorage.Uint(CMTPStorageMetaData::EStorageSystemType) == CMTPStorageMetaData::ESystemTypeDefaultFileSystem)
        {   
        // Validate that the SUID path exists.
        if (!BaflUtils::PathExists(iSingletons.Fs(), aStorage.DesC(CMTPStorageMetaData::EStorageSuid)))
            {
            User::Leave(KErrPathNotFound);
            }
     
        // Validate that the SUID path corresponds to the physical storage drive.
        TInt storageDrive(DriveNumber(aPhysicalStorageId));
        TParse p;
        User::LeaveIfError(p.Set(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), NULL, NULL));
        TInt suidDrive(0);
        User::LeaveIfError(iSingletons.Fs().CharToDrive(TChar(p.Drive()[0]), suidDrive));
        if (suidDrive != storageDrive)
            {
            // SUID path/physical storage drive mis-match.
            User::Leave(KErrArgument);
            }
        }
    
    // Allocate a logical StorageId.
    TInt32 id(AllocateLogicalStorageId(aDataProviderId, aPhysicalStorageId));
    User::LeaveIfError(id);
    
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
    
#ifdef __FLOG_ACTIVE
    HBufC8* buf(HBufC8::NewLC(aStorage.DesC(CMTPStorageMetaData::EStorageSuid).Length()));
    buf->Des().Copy(aStorage.DesC(CMTPStorageMetaData::EStorageSuid));
    __FLOG_VA((_L8("Allocated logical StorageID 0x%08X for storage SUID %S"), id, buf));
    CleanupStack::PopAndDestroy(buf);
#endif // __FLOG_ACTIVE    
    __FLOG(_L8("AllocateLogicalStorageIdL - Exit"));
    return id;
    }

EXPORT_C TUint32 CMTPStorageMgr::AllocatePhysicalStorageIdL(TUint aDataProviderId, const CMTPStorageMetaData& aStorage)
    {
    __FLOG(_L8("AllocatePhysicalStorageIdL - Entry"));
    
    // Validate the SUID.
    if (iStorages.Find(aStorage.DesC(CMTPStorageMetaData::EStorageSuid), StorageKeyMatchSuid) != KErrNotFound)
        {
        // SUID is not unique.
        User::Leave(KErrAlreadyExists);
        }
    
    // Allocate a physical StorageId.
    TInt32 id(AllocatePhysicalStorageId(aDataProviderId));
    User::LeaveIfError(id);
    
    // Create the physical storage meta-data.
    CMTPStorageMetaData* physical(CMTPStorageMetaData::NewLC(aStorage));
    const RArray<TUint> noStorages;
    physical->SetUint(CMTPStorageMetaData::EStorageId, id);
    physical->SetUintArrayL(CMTPStorageMetaData::EStorageLogicalIds, noStorages);
    
    // Store the physical storage meta-data.
    iStorages.InsertInOrderL(physical, StorageOrder);
    CleanupStack::Pop(physical);
    
    __FLOG_VA((_L8("Allocated physical StorageID 0x%08X"), id));
    __FLOG(_L8("AllocatePhysicalStorageIdL - Exit"));
    return id;
    }

EXPORT_C TInt CMTPStorageMgr::DeallocateLogicalStorageId(TUint aDataProviderId, TUint32 aLogicalStorageId)
    {
    __FLOG(_L8("DeallocateLogicalStorageId - Entry"));
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
    __FLOG(_L8("DeallocateLogicalStorageId - Exit"));
    return ret;
    }

EXPORT_C void CMTPStorageMgr::DeallocateLogicalStorageIds(TUint aDataProviderId, TUint32 aPhysicalStorageId)
    {
    __FLOG(_L8("DeallocateLogicalStorageIds - Entry"));
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
    __FLOG(_L8("DeallocateLogicalStorageIds - Exit"));
    }

EXPORT_C TInt CMTPStorageMgr::DeallocatePhysicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId)
    {
    __FLOG(_L8("DeallocatePhysicalStorageId - Entry"));
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
    __FLOG(_L8("DeallocatePhysicalStorageId - Exit"));
    return ret;
    }

EXPORT_C TUint32 CMTPStorageMgr::DefaultStorageId() const
    {
    __FLOG(_L8("DefaultStorageId - Entry"));
    __FLOG(_L8("DefaultStorageId - Exit"));
    return iDefaultStorageId;
    }

EXPORT_C TInt CMTPStorageMgr::DriveNumber(TUint32 aStorageId) const
    {
    __FLOG(_L8("DriveNumber - Entry"));
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
    __FLOG(_L8("DriveNumber - Exit"));
    return drive;
    }

EXPORT_C TInt32 CMTPStorageMgr::FrameworkStorageId(TDriveNumber aDriveNumber) const
    {
    __FLOG(_L8("FrameworkStorageId - Entry"));
    TInt32 ret(KErrNotFound);
    TInt32 id(iMapDriveToStorage[aDriveNumber]);
    if ((id != KErrNotFound) && (LogicalStorageId(id)))
        {
        ret = id;
        }
    __FLOG(_L8("FrameworkStorageId - Exit"));
    return ret;
    }

EXPORT_C void CMTPStorageMgr::GetAvailableDrivesL(RArray<TDriveNumber>& aDrives) const
    {
    __FLOG(_L8("GetAvailableDrivesL - Entry"));
    aDrives.Reset();
    for (TUint i(0); (i < iMapDriveToStorage.Count()); i++)
        {
        if (iMapDriveToStorage[i] != KErrNotFound)
            {
            aDrives.AppendL(static_cast<TDriveNumber>(i));
            }
        }
    __FLOG(_L8("GetAvailableDrivesL - Exit"));
    }

EXPORT_C void CMTPStorageMgr::GetLogicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const
    {
    __FLOG(_L8("GetLogicalStoragesL - Entry"));
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
    __FLOG(_L8("GetLogicalStoragesL - Exit"));
    }

EXPORT_C void CMTPStorageMgr::GetPhysicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const
    {
    __FLOG(_L8("GetPhysicalStoragesL - Entry"));
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
    __FLOG(_L8("GetPhysicalStoragesL - Exit"));
    }

EXPORT_C TUint32 CMTPStorageMgr::LogicalStorageId(TUint32 aStorageId) const
    {
    __FLOG(_L8("LogicalStorageId - Entry"));
    __FLOG(_L8("LogicalStorageId - Exit"));
    return (aStorageId & KLogicalIdMask);
    }

EXPORT_C TInt32 CMTPStorageMgr::LogicalStorageId(const TDesC& aStorageSuid) const
    {
    __FLOG(_L8("LogicalStorageId - Entry"));
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
    __FLOG(_L8("LogicalStorageId - Exit"));
    return id;
    }

EXPORT_C TInt32 CMTPStorageMgr::PhysicalStorageId(TDriveNumber aDriveNumber) const
    {
    __FLOG(_L8("PhysicalStorageId - Entry"));
    TInt32 storageId(iMapDriveToStorage[aDriveNumber]);
    if (storageId != KErrNotFound)
        {
        storageId = PhysicalStorageId(storageId);
        }
    __FLOG(_L8("PhysicalStorageId - Exit"));
    return storageId;
    }

EXPORT_C TUint32 CMTPStorageMgr::PhysicalStorageId(TUint32 aStorageId) const
    {
    __FLOG(_L8("PhysicalStorageId - Entry"));
    __FLOG(_L8("PhysicalStorageId - Exit"));
    return (aStorageId & KPhysicalIdMask);
    }

EXPORT_C const CMTPStorageMetaData& CMTPStorageMgr::StorageL(TUint32 aStorageId) const
    {
    __FLOG(_L8("StorageL - Entry"));
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    User::LeaveIfError(idx);
    __FLOG(_L8("StorageL - Exit"));
    return *iStorages[idx];
    }

EXPORT_C TUint32 CMTPStorageMgr::StorageId(TUint32 aPhysicalStorageId, TUint32 aLogicalStorageId) const
    {
    __FLOG(_L8("StorageId - Entry"));
    __FLOG(_L8("StorageId - Exit"));
    return (aPhysicalStorageId | aLogicalStorageId);
    }

EXPORT_C TBool CMTPStorageMgr::ValidStorageId(TUint32 aStorageId) const
    {
    __FLOG(_L8("ValidStorageId - Entry"));
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    if(KErrNotFound == idx)
    	{
    	__FLOG(_L8("ValidStorageId - False Exit"));
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
  
    __FLOG(_L8("ValidStorageId - Exit"));
    
    return ret;
    }
    
EXPORT_C CMTPTypeString* CMTPStorageMgr::VolumeIdL(TUint aDataProviderId, TUint32 aStorageId, const TDesC& aVolumeIdSuffix) const
    {
    __FLOG(_L8("VolumeIdL - Entry"));

    // Validate the StorageId.
    TUint owner(LogicalStorageId(aStorageId) ? LogicalStorageOwner(aStorageId) : PhysicalStorageOwner(aStorageId));
    if (!ValidStorageId(aStorageId))
        {
        User::Leave(KErrNotFound);
        }
    else if (aDataProviderId != owner)
        {
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
    __FLOG(_L8("VolumeIdL - Exit"));
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    iSingletons.OpenL();
    for (TUint i(0); (i < KMaxDrives); i++)
        {
        iMapDriveToStorage[i] = KErrNotFound;
        }
    __FLOG(_L8("ConstructL - Exit"));
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("AllocateLogicalStorageId - Entry"));
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("AllocateLogicalStorageId - Exit"));
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("AllocatePhysicalStorageId - Entry"));
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
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("AllocatePhysicalStorageId - Exit"));
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
    __FLOG(_L8("RemoveLogicalStorageL - Entry"));
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
    __FLOG(_L8("RemoveLogicalStorageL - Entry"));
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
    __FLOG(_L8("StorageMetaDataL - Entry"));
    TInt idx(iStorages.FindInOrder(aStorageId, StorageOrder));
    User::LeaveIfError(idx);
    __FLOG(_L8("StorageMetaDataL - Exit"));
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
