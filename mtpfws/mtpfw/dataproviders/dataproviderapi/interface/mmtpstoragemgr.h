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

/**
 @file
 @publishedPartner
 @released
*/

#ifndef MMTPSTORAGEMGR_H
#define MMTPSTORAGEMGR_H

#include <f32file.h>

#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mtpstoragemgrquerytypes.h>

class CMTPTypeString;

/** 
Defines the MTP data provider framework storage manager interface. The storage 
manager controls the allocation of physical and logical MTP StorageIDs, and 
their mapping to storage media, such as Symbian OS file system drives.

MTP StorageIDs are represented as unsigned 32-bit integers and are subdivided 
into two 16-bit fields. The most significant 16-bits represents the physical 
StorageID and the least significant 16-bits represents the StorageID of a 
logical partition of that physical storage. 
@publishedPartner
@released
*/
class MMTPStorageMgr
    {
public:
    
    /**
    Creates a new logical MTP StorageID partition on the specified Symbian OS 
    file system drive, to be owned and managed by the specified data provider.
    @param aDataProviderId The identifier of the data provider which will be 
    responsible for the logical MTP StorageID.
    @param aDriveNumber The Symbian OS file system drive on which the logical 
    MTP StorageID partition is to be created.
    @return The fully formed MTP StorageID of the new logical storage 
    partition.
    @param aStorage The storage meta-data. This must minimally specify the 
    @see EStorageSystemType and @see EStorageSuid elements. The 
    @see EStorageSystemType must match that of the corresponding physical 
    MTP StorageID. If @see EStorageSystemType is specified as 
    @see ESystemTypeDefaultFileSystem then @see EStorageSuid is validated as a 
    fully formed file system folder path (e.g. "C:\Media") which must 
    correspond to the specified file system drive.
    @leave KErrAlreadyExists, if the storage SUID is already defined.
    @leave KErrArgument if @see EStorageSystemType is not 
    @see ESystemTypeDefaultFileSystem, or if @see EStorageSuid is not a valid
    file system folder path, or does not match the specified drive.
    @leave KErrNotFound if the specified drive does not exist.
    @leave KErrPathNotFound, if @see EStorageSystemType is 
    @see ESystemTypeDefaultFileSystem and the path specified by 
    @see EStorageSuid does not exist on the Symbian OS filesystem. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual TUint32 AllocateLogicalStorageIdL(TUint aDataProviderId, TDriveNumber aDriveNumber, const CMTPStorageMetaData& aStorage) = 0;
    
    /**
    Creates a new logical MTP StorageID partition on the specified physical MTP
    StorageID, to be owned and managed by the specified data provider.
    @param aDataProviderId The identifier of the data provider which will be 
    responsible for the logical MTP StorageID.
    @param aPhysicalStorageId The physical MTP StorageID on which the logical 
    MTP StorageID partition is to be created.
    @param aStorage The storage meta-data. This must minimally specify the 
    @see EStorageSystemType and @see EStorageSuid elements. The 
    @see EStorageSystemType must match that of the corresponding physical 
    MTP StorageID. If @see EStorageSystemType is specified as 
    @see ESystemTypeDefaultFileSystem then @see EStorageSuid is validated as a 
    fully formed file system folder path (e.g. "C:\Media") which must 
    correspond to the specified file system drive.
    @return The fully formed MTP StorageID of the new logical storage 
    partition.
    @leave KErrAlreadyExists, if the storage SUID is already defined.
    @leave KErrArgument if @see EStorageSystemType is not 
    @see ESystemTypeDefaultFileSystem, or if @see EStorageSuid is not a valid
    file system folder path, or does not match the specified drive.
    @leave KErrNotFound if the specified physical StorageID does not exist.
    @leave KErrPathNotFound, if @see EStorageSystemType is 
    @see ESystemTypeDefaultFileSystem and the path specified by 
    @see EStorageSuid does not exist on the Symbian OS filesystem. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual TUint32 AllocateLogicalStorageIdL(TUint aDataProviderId, TUint32 aPhysicalStorageId, const CMTPStorageMetaData& aStorage) = 0;
    
    /**
    Creates a new physical MTP StorageID to be owned and managed by the 
    specified data provider.
    @param aDataProviderId The identifier of the data provider which will be 
    responsible for the physical MTP StorageID.
    @param aStorage The storage meta-data. 
    @return The physical MTP StorageID.
    @leave KErrAlreadyExists, if the storage SUID is already defined.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual TUint32 AllocatePhysicalStorageIdL(TUint aDataProviderId, const CMTPStorageMetaData& aStorage) = 0;
    
    /**
    Releases the specified fully formed logical MTP StorageID. 
    @param aDataProviderId The identifier of the data provider which is 
    responsible for the logical MTP StorageID.
    @param aLogicalStorageId The fully formed logical MTP StorageID to be 
    deallocated.
    @return KErrNone, if successful.
    @return KErrNotFound, if the specified logical MTP StorageID is not 
    defined.
    @return KErrAccessDenied, if the specified data provider is not the owner of
    the specified logical MTP StorageID.
    @return one of the system wide error codes, if a processing failure occurs.
    */
    virtual TInt DeallocateLogicalStorageId(TUint aDataProviderId, TUint32 aLogicalStorageId) = 0;
    
    /**
    Releases all logical MTP StorageIDs which represent partitions of the 
    specified physical MTP StorageID and which are owned by the specified data 
    provider.
    @param aDataProviderId The identifier of the data provider which is 
    responsible for the logical MTP StorageIDs.
    @param aPhysicalStorageId The physical MTP StorageID.
    */
    virtual void DeallocateLogicalStorageIds(TUint aDataProviderId, TUint32 aPhysicalStorageId) = 0;
    
    /**
    Releases the specified physical MTP StorageID, along with all associated
    logical storage partititions. 
    @param aDataProviderId The identifier of the data provider which is 
    responsible for the physical MTP StorageID.
    @param aPhysicalStorageId The physical MTP StorageID to be released.
    @return KErrNone if successful.
    @return KErrNotFound, if the specified sical MTP StorageID is not 
    defined.
    @return KErrAccessDenied if the specified data provider is not the owner of
    the specified physical MTP StorageID.
    @return one of the system wide error codes, if a processing failure occurs.
    */
    virtual TInt DeallocatePhysicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId) = 0;

    /**
    Provides the default MTP StorageID, as determined by the value of the 
    @see MTPFrameworkConfig::EDefaultStorageDrive framework configurability 
    parameter. This may either be a physical or fully formed logical MTP 
    StorageID, depending on the value of the 
    @see MTPFrameworkConfig::ELogicalStorageIdsAllocationEnable framework 
    configurability parameter.
    @return The default MTP StorageID.
    */
    virtual TUint32 DefaultStorageId() const = 0;

    /**
    Provides the Symbian OS drive number associated with the specified MTP 
    StorageID. 
    @param aStorageId The physical or fully formed logical MTP StorageID.
    @return The Symbian OS drive number associated with the specified MTP 
    StorageID.
    @return KErrNotFound, if the MTP StorageID does not exist, or is not 
    associated with a Symbian OS drive number.
    */
    virtual TInt DriveNumber(TUint32 aStorageId) const = 0;

    /**
    Provides the fully formed MTP StorageID created by the MTP data provider 
    framework on the specified Symbian OS drive. If the 
    @see MTPFrameworkConfig::ELogicalStorageIdsAllocationEnable framework 
    configurability parameter is set, then an MTP StorageID is created on each 
    of the available Symbian OS file system drives as they become available. If
    not set then the creation of all logical MTP StorageIDs is deferred to the 
    active data providers.
    @param aDriveNumber The drive number for which the corresponding logical
    MTP StorageID is required.
    @return The fully formed MTP StorageID of the logical storage which 
    corresponds to the specified Symbian OS drive number.
    @return KErrNotFound, if the Symbian OS drive number does not correspond 
    to an MTP data provider framework created default logical MTP StorageID.
    */
    virtual TInt32 FrameworkStorageId(TDriveNumber aDriveNumber) const = 0;
    
    /**
    Provides an ordered list of the Symbian OS file system drives which are
    available for use as MTP storages. This comprises the sub-set of available
    Symbian OS file system drives which are left exposed by the the @see
    MTPFrameworkConfig::EExcludedStorageDrives framework configurability 
    parameter.
    @param aDrives On succesful completion, the list of available Symbian OS 
    file system drives.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetAvailableDrivesL(RArray<TDriveNumber>& aDrives) const = 0;
    
    /**
    Provides an ordered list of the storage meta-data for all available logical
    MTP StorageIDs which match the specified query criteria. 
    @param aParams The query parameters.
    @param aStorages On succesful completion, the list of matching storage 
    meta-data. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetLogicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const = 0;
    
    /**
    Provides an ordered list of the storage meta-data for all available 
    physical MTP StorageIDs which match the specified query criteria. 
    @param aParams The query parameters.
    @param aStorages On succesful completion, the list of matching storage 
    meta-data. 
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void GetPhysicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const = 0;
    
    /**
    Provides the logical StorageID component (i.e. the least significant 
    16-bits) of the specified 32-bit MTP StorageID.
    @param aStorageId The MTP StorageID.
    @return The logical MTP StorageID, if successful.
    */
    virtual TUint32 LogicalStorageId(TUint32 aStorageId) const = 0;
    
    /**
    Provides the the fully formed 32-bit logical StorageID associated with the
    specified storage System Unique IDentifier (SUID).
    @param aStorageId The MTP StorageID.
    @return If successful, the fully formed MTP StorageID of the logical 
    storage which corresponds to the specified storage UID.
    KErrNotFound, if the specified storage UID is not defined.
    */
    virtual TInt32 LogicalStorageId(const TDesC& aStorageSuid) const = 0;

    /**
    Provides the physical MTP StorageID associated with the specified 
    Symbian OS drive number.
    @param aDriveNumber The Symbian OS drive number.
    @return If successful, the physical MTP StorageID associated with the 
    specified Symbian OS drive number.
    KErrNotFound, if the Symbian OS drive number is not associated with an MTP
    StorageID.
    */
    virtual TInt32 PhysicalStorageId(TDriveNumber aDriveNumber) const = 0;
   
    /**
    Provides the physical StorageID component (i.e. the most significant 
    16-bits) of the specified 32-bit MTP StorageID.
    @param aStorageId The MTP StorageID.
    @return The physical MTP StorageID.
    */
    virtual TUint32 PhysicalStorageId(TUint32 aStorageId) const = 0;
    
    /**
    Provides the storage meta-data for the specified logical MTP StorageID.
    @param aStorageId The physical or fully formed logical MTP StorageID.
    @leave KErrNotFound if the specified StorageID does not exist.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual const CMTPStorageMetaData& StorageL(TUint32 aStorageId) const = 0;
   
    /**
    Constructs a fully formed 32-bit MTP StorageID from the specified physical 
    and logical MTP StorageIDs.
    @param aPhysicalStorageId The physical MTP StorageID. The least significant
    16-bits are ignored.
    @param aLogicalStorageId The logical MTP StorageID. The most significant 
    16-bits are ignored.
    @return The fully formed 32-bit MTP StorageID.
    */
    virtual TUint32 StorageId(TUint32 aPhysicalStorageId, TUint32 aLogicalStorageId) const = 0;
    
    /**
    Indicates if the specified StorageID is defined.
    @param aStorageId The physical or fully formed logical MTP StorageID.
    @return ETrue if the StorageID exists, otherwise EFalse.
    */
    virtual TBool ValidStorageId(TUint32 aStorageId) const = 0;

    /**
    Generates an MTP volume identifier string of which the leading 128 
    characters are guaranteed to be unique amongst all storages presented to a 
    conected MTP initiator.
    @param aDataProviderId The identifier of the data provider.
    @param aStorageId The MTP StorageID associated with the volume identifier.
    @param aVolumeIdSuffix The data provider supplied volume identifier string.
    @return The MTP volume identifier string. Ownership IS transferred.
    @leave KErrAccessDenied if the specified data provider is not the owner of
    the specified StorageID.
    @leave KErrNotFound if the specified MTP StorageID does not exist.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual CMTPTypeString* VolumeIdL(TUint aDataProviderId, TUint32 aStorageId, const TDesC& aVolumeIdSuffix) const = 0;
    };
    
#endif // MMTPSTORAGEMGR_H
