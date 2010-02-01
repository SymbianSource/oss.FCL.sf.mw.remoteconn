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
*/

#include <mtp/cmtpstoragemetadata.h>
#include <mtp/mtpstoragemgrquerytypes.h>

/**
Default constructor.
*/
EXPORT_C TMTPStorageMgrQueryParams::TMTPStorageMgrQueryParams() :
    iStorageSuid(KNullDesC),
    iStorageSystemType(CMTPStorageMetaData::ESystemTypeUndefined)
    {
    
    }

/**
Constructor.
@param aStorageSuid The storage System Unique IDentifier (SUID). A value of 
@see KNullDesC indicates that the query is targetting all storages.
@param aStorageSystemType The storage system type. A value of 
@see CMTPStorageMetaData::ESystemTypeUndefined indicates that the query is 
targeting all storage systems
*/
EXPORT_C TMTPStorageMgrQueryParams::TMTPStorageMgrQueryParams(const TDesC& aStorageSuid, TUint32 aStorageSystemType) :
    iStorageSuid(aStorageSuid),
    iStorageSystemType(aStorageSystemType)
    {
    
    }
  
/**
Sets the storage System Unique IDentifier (SUID).
@param aStorageSuid The storage System Unique IDentifier (SUID). A value of 
@see KNullDesC indicates that the query is targetting all storages.
*/
EXPORT_C void TMTPStorageMgrQueryParams::SetStorageSuid(const TDesC& aStorageSuid)
    {
    iStorageSuid.Set(aStorageSuid);
    }

/**
Sets the storage system type.
@param aStorageSystemType The storage system type. A value of 
@see CMTPStorageMetaData::ESystemTypeUndefined indicates that the query is 
targeting all storage systems
*/
EXPORT_C void TMTPStorageMgrQueryParams::SetStorageSystemType(TUint32 aStorageSystemType)
    {
    iStorageSystemType = aStorageSystemType;
    }
  
/**
Provides the storage System Unique IDentifier (SUID).
@param The storage SUID.
*/
EXPORT_C const TDesC& TMTPStorageMgrQueryParams::StorageSuid() const
    {
    return iStorageSuid;
    }

/**
Provides the storage system type.
@param The storage system type.
*/
EXPORT_C TUint32 TMTPStorageMgrQueryParams::StorageSystemType() const
    {
    return iStorageSystemType;
    }
