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
 @internalComponent
*/

#ifndef CMTPSTORAGEMGR_H
#define CMTPSTORAGEMGR_H

#include <e32base.h>
#include <f32file.h>
#include <mtp/mmtpstoragemgr.h>

#include "mtpdebug.h"
#include "rmtpframework.h"

/** 
Implements the MTP storage manager interface.
@internalComponent
 
*/
class CMTPStorageMgr : 
    public CBase, 
    public MMTPStorageMgr
    {
public:

    IMPORT_C static CMTPStorageMgr* NewL();
    IMPORT_C ~CMTPStorageMgr();
    
    IMPORT_C static TUint LogicalStorageNumber(TUint32 aStorageId);
    IMPORT_C static TUint LogicalStorageOwner(TUint32 aStorageId);
    IMPORT_C static TUint PhysicalStorageNumber(TUint32 aStorageId);
    IMPORT_C static TUint PhysicalStorageOwner(TUint32 aStorageId);
    
    IMPORT_C void SetDefaultStorageId(TUint32 aStorageId);
    IMPORT_C void SetDriveMappingL(TDriveNumber aDriveNumber, TUint32 aStorageId);
    IMPORT_C void SetFrameworkId(TUint aDataProviderId);
    
public: // From MMTPStorageMgr

    IMPORT_C TUint32 AllocateLogicalStorageIdL(TUint aDataProviderId, TDriveNumber aDriveNumber, const CMTPStorageMetaData& aStorage);
    IMPORT_C TUint32 AllocateLogicalStorageIdL(TUint aDataProviderId, TUint32 aPhysicalStorageId, const CMTPStorageMetaData& aStorage);
    IMPORT_C TUint32 AllocatePhysicalStorageIdL(TUint aDataProviderId, const CMTPStorageMetaData& aStorage);
    IMPORT_C TInt DeallocateLogicalStorageId(TUint aDataProviderId, TUint32 aLogicalStorageId);
    IMPORT_C void DeallocateLogicalStorageIds(TUint aDataProviderId, TUint32 aPhysicalStorageId);
    IMPORT_C TInt DeallocatePhysicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId);
    IMPORT_C TUint32 DefaultStorageId() const;
    IMPORT_C TInt DriveNumber(TUint32 aStorageId) const;
    IMPORT_C TInt32 FrameworkStorageId(TDriveNumber aDriveNumber) const;
    IMPORT_C void GetAvailableDrivesL(RArray<TDriveNumber>& aDrives) const;
    IMPORT_C void GetLogicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const;
    IMPORT_C void GetPhysicalStoragesL(const TMTPStorageMgrQueryParams& aParams, RPointerArray<const CMTPStorageMetaData>& aStorages) const;
    IMPORT_C TUint32 LogicalStorageId(TUint32 aStorageId) const;
    IMPORT_C TInt32 LogicalStorageId(const TDesC& aStorageSuid) const;
    IMPORT_C TInt32 PhysicalStorageId(TDriveNumber aDriveNumber) const;
    IMPORT_C TUint32 PhysicalStorageId(TUint32 aStorageId) const;
    IMPORT_C const CMTPStorageMetaData& StorageL(TUint32 aStorageId) const;
    IMPORT_C TUint32 StorageId(TUint32 aPhysicalStorageId, TUint32 aLogicalStorageId) const;
    IMPORT_C TBool ValidStorageId(TUint32 aStorageId) const;
    IMPORT_C CMTPTypeString* VolumeIdL(TUint aDataProviderId, TUint32 aStorageId, const TDesC& aVolumeIdSuffix) const;
          
public:
    IMPORT_C TBool IsReadWriteStorage(TUint32 aStorageId) const; 
    
private:

    CMTPStorageMgr();
    void ConstructL();
    
    TInt32 AllocateLogicalStorageId(TUint aDataProviderId, TUint32 aPhysicalStorageId);
    TInt32 AllocatePhysicalStorageId(TUint aDataProviderId);
    TUint32 EncodeLogicalStorageId(TUint32 aPhysicalStorageId, TUint aDataProviderId, TUint aStorageNumber);
    TUint32 EncodeLogicalStorageNumber(TUint aStorageNumber);
    TUint32 EncodeLogicalStorageOwner(TUint aDataProviderId);
    TUint32 EncodePhysicalStorageId(TUint aDataProviderId, TUint aStorageNumber);
    TUint32 EncodePhysicalStorageNumber(TUint aStorageNumber);
    TUint32 EncodePhysicalStorageOwner(TUint aDataProviderId);
    void RemoveLogicalStorageL(TUint aIdx);
    CMTPStorageMetaData& StorageMetaDataL(TUint32 aStorageId);
    
    static TBool StorageKeyMatchSuid(const TDesC* aSuid, const CMTPStorageMetaData& aStorage);
    static TInt StorageOrder(const CMTPStorageMetaData& aL, const CMTPStorageMetaData& aR);
    static TInt StorageOrder(const TUint32* aKey, const CMTPStorageMetaData& aStorage);
    
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The default MTP storage ID.
    */
    TUint32                             iDefaultStorageId;
    
    /**
    The framework storages (drives) owning data provider ID.
    */
    TInt                                iFrameworkId;

    /**
    The Symbian OS drive number to default MTP StorageID mapping table. This 
    StorageID may either be the physical MTP StorageID or the default logical 
    MTP StorageID.
    */
    TFixedArray<TInt, KMaxDrives>       iMapDriveToStorage;
    
    /**
    The physical storage numbers allocation table. Indexed by physical storage 
    owner data provider ID.
    */
    RArray<TUint>                       iPhysicalStorageNumbers;
    
    /**
    The framework singletons.
    */
    RMTPFramework                       iSingletons;
    
    /**
    The storages table.
    */
    RPointerArray<CMTPStorageMetaData>  iStorages;
    };
    
#endif // CMTPSTORAGEMGR_H
