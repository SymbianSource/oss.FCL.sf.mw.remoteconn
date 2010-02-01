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

#ifndef MTPSTORAGEMGRQUERYTYPES_H
#define MTPSTORAGEMGRQUERYTYPES_H

#include <e32cmn.h>

/**
Defines the MTP data provider framework object store enumeration manager object
query parameters.
*/
class TMTPStorageMgrQueryParams
    {
public:

    IMPORT_C TMTPStorageMgrQueryParams();
    IMPORT_C TMTPStorageMgrQueryParams(const TDesC& aStorageSuid, TUint32 aStorageSystemType);
    
    IMPORT_C void SetStorageSuid(const TDesC& aStorageSuid);
    IMPORT_C void SetStorageSystemType(TUint32 aStorageSystemType);
    
    IMPORT_C const TDesC& StorageSuid() const;
    IMPORT_C TUint32 StorageSystemType() const;

private:
    
    /**
    The storage System Unique IDentifier (SUID). 
    */
    TPtrC   iStorageSuid;

    /**
    The storage system type.
    */
    TUint32 iStorageSystemType;
    };
    
#endif // MTPSTORAGEMGRQUERYTYPES_H
