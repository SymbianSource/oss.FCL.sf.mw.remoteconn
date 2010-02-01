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

#include <mtp/cmtpstoragemetadata.h>
    
/**
The element meta-data table content.
*/
const CMTPMetaData::TElementMetaData CMTPStorageMetaData::KElements[CMTPStorageMetaData::ENumElements] = 
    {
        {0, CMTPStorageMetaData::EUint},        // EAttributes
        {0, CMTPStorageMetaData::EDesCArray},   // EExcludedAreas
        {1, CMTPStorageMetaData::EUint},        // EStorageId
        {0, CMTPStorageMetaData::EUintArray},   // ELogicalIds
        {0, CMTPStorageMetaData::EDesC},        // EStorageSuid
        {2, CMTPStorageMetaData::EUint},        // EStorageSystemType
    };
    
/**
MTP storage meta-data factory method. This method creates an 
empty storage meta-data record.
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewL()
    {
    CMTPStorageMetaData* self = CMTPStorageMetaData::NewLC();
    CleanupStack::Pop(self); // self
    return self;
    }

/**
MTP storage meta-data factory method. This method creates an 
empty storage meta-data record. A pointer to the meta-data is placed on the 
cleanup stack.
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewLC()
    {
    CMTPStorageMetaData* self = new(ELeave) CMTPStorageMetaData();
    CleanupStack::PushL(self);
    self->CMTPMetaData::ConstructL();
    return self;
    }
    
/**
MTP storage meta-data factory method. 
@param aStorageSystemType The storage system type.
@param aStorageSuid The storage System Unique IDentifier (SUID).
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewL(TUint aStorageSystemType, const TDesC& aStorageSuid)
    {
    CMTPStorageMetaData* self = CMTPStorageMetaData::NewLC(aStorageSystemType, aStorageSuid);
    CleanupStack::Pop(self);
    return self;
    }
    
/**
MTP storage meta-data factory method. A pointer to the meta-data is 
placed on the cleanup stack.
@param aStorageSystemType The storage system type.
@param aStorageSuid The storage System Unique IDentifier (SUID).
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewLC(TUint aStorageSystemType, const TDesC& aStorageSuid)
    {
    CMTPStorageMetaData* self = new(ELeave) CMTPStorageMetaData();
    CleanupStack::PushL(self);
    self->ConstructL(aStorageSystemType, aStorageSuid);
    return self;
    }
    
/**
MTP storage meta-data copy factory method. 
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewL(const CMTPStorageMetaData& aStorage)
    {
    CMTPStorageMetaData* self = CMTPStorageMetaData::NewLC(aStorage);
    CleanupStack::Pop(self);
    return self;
    }
    
/**
MTP storage meta-data copy factory method. A pointer to the meta-data
is placed on the cleanup stack.
@param aStorage .
@return A pointer to a new CMTPStorageMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPStorageMetaData* CMTPStorageMetaData::NewLC(const CMTPStorageMetaData& aStorage)
    {
    CMTPStorageMetaData* self = new(ELeave) CMTPStorageMetaData();
    CleanupStack::PushL(self);
    self->CMTPMetaData::ConstructL(aStorage);
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPStorageMetaData::~CMTPStorageMetaData()
    {

    } 

/**
Constructor.
*/
CMTPStorageMetaData::CMTPStorageMetaData() :
    CMTPMetaData(KElements, ENumElements)
    {
    
    }
    
/**
Second phase constructor.
@param aStorageSystemType The storage system type.
@param aStorageSuid The storage System Unique IDentifier (SUID).
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPStorageMetaData::ConstructL(TUint aStorageSystemType, const TDesC& aStorageSuid)
    {
    CMTPMetaData::ConstructL();
    SetUint(EStorageSystemType, aStorageSystemType);
    SetDesCL(EStorageSuid, aStorageSuid);
    }
