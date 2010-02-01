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

#include <mtp/cmtpobjectmetadata.h>
#include <mtp/mtpprotocolconstants.h>
    
/**
The element meta-data table content.
*/
const CMTPMetaData::TElementMetaData CMTPObjectMetaData::KElements[CMTPObjectMetaData::ENumElements] = 
    {
        {0, CMTPObjectMetaData::EUint}, // EDataProviderId
        {1, CMTPObjectMetaData::EUint}, // EFormatCode
        {2, CMTPObjectMetaData::EUint}, // EFormatSubCode
        {3, CMTPObjectMetaData::EUint}, // EHandle
        {4, CMTPObjectMetaData::EUint}, // EModes
        {5, CMTPObjectMetaData::EUint}, // EParentHandle
        {6, CMTPObjectMetaData::EUint}, // EStorageId
        {7, CMTPObjectMetaData::EUint}, // EStorageVolumeId
        {0, CMTPObjectMetaData::EDesC}, // ESuid
        {8, CMTPObjectMetaData::EUint}, // EUserRating @deprecated 
        {9, CMTPObjectMetaData::EUint}, // EBuyNow @deprecated 
        {10, CMTPObjectMetaData::EUint}, // EPlayed @deprecated  
        {11, CMTPObjectMetaData::EUint},// EIdentifier
        //{0, CMTPObjectMetaData::EInt},// EParentId
        {12, CMTPObjectMetaData::EUint},// ENonConsumable
        {13, CMTPObjectMetaData::EUint},// EObjectMetaDataUpdate
        {1, CMTPObjectMetaData::EDesC},// EName
    };
    
/**
MTP object manager meta-data factory method. This method creates an 
empty object meta-data record.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewL()
    {
    CMTPObjectMetaData* self = CMTPObjectMetaData::NewLC();
    CleanupStack::Pop(self); // self
    return self;
    }

/**
MTP object manager meta-data factory method. This method creates an 
empty object meta-data record. A pointer to the meta-data is
placed on the cleanup stack.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewLC()
    {
    CMTPObjectMetaData* self = new(ELeave) CMTPObjectMetaData();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }
    
/**
MTP object manager meta-data factory method. 
@param aDataProviderId The identifier of the data provider which owns the data 
object.
@param aFormatCode The MTP object format code of the data object.
@param aStorageId The MTP storage ID of the storage media on which the data 
object resides.
@param aSuid A buffer containing the SUID of the data object.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewL(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid)
    {
    CMTPObjectMetaData* self = CMTPObjectMetaData::NewLC(aDataProviderId, aFormatCode, aStorageId, aSuid);
    CleanupStack::Pop(self);
    return self;
    }
    
/**
MTP object manager meta-data factory method. A pointer to the meta-data is 
placed on the cleanup stack.
@param aDataProviderId The identifier of the data provider which owns the data 
object.
@param aFormatCode The MTP object format code of the data object.
@param aStorageId The MTP storage ID of the storage media on which the data 
object resides.
@param aSuid A buffer containing the SUID of the data object.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewLC(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid)
    {
    CMTPObjectMetaData* self = new(ELeave) CMTPObjectMetaData();
    CleanupStack::PushL(self);
    self->ConstructL(aDataProviderId, aFormatCode, aStorageId, aSuid);
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPObjectMetaData::~CMTPObjectMetaData()
    {

    } 

/**
Constructor.
*/
CMTPObjectMetaData::CMTPObjectMetaData() :
    CMTPMetaData(KElements, ENumElements)
    {
    
    }
    
/**
Second phase constructor.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPObjectMetaData::ConstructL()
    {
    CMTPMetaData::ConstructL();
    SetUint(EParentHandle, KMTPHandleNoParent);
    //SetInt(EParentId, KErrNotFound);
    }
    
/**
Second phase constructor.
@param aDataProviderId The identifier of the data provider which owns the data 
object.
@param aFormatCode The MTP object format code of the data object.
@param aStorageId The MTP storage ID of the storage media on which the data 
object resides.
@param aSuid A buffer containing the SUID of the data object.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPObjectMetaData::ConstructL(TUint aDataProviderId, TUint aFormatCode, TUint aStorageId, const TDesC& aSuid)
    {
    ConstructL();
    SetUint(EDataProviderId, aDataProviderId);
    SetUint(EFormatCode, aFormatCode);
    SetUint(EStorageId, aStorageId);
    SetDesCL(ESuid, aSuid);
    }
    
/**
@deprecated
Second phase constructor.
@param aSuid A buffer containing the SUID of the data object.
@param aUserRating  user rating of the data object.
@param aBuyNow The MTP object BuyNow code of the data object.
@param aPlayCount The MTP object aPlayCount code of the data object.
@leave One of the system wide error code, if a processing failure occurs.
*/
void CMTPObjectMetaData::ConstructL(const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount)
    {
     ConstructL();

     SetUint(EUserRating, aUserRating);
     SetUint(EBuyNow, aBuyNow);
     SetUint(EPlayCount, aPlayCount);
     SetDesCL(ESuid, aSuid);
    }

/**
@deprecated
MTP object manager meta-data factory method.
@param aSuid A buffer containing the SUID of the data object.
@param aUserRating  user rating of the data object.
@param aBuyNow The MTP object BuyNow code of the data object.
@param aPlayCount The MTP object aPlayCount code of the data object.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewL(const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount)
    {
    CMTPObjectMetaData* self = CMTPObjectMetaData::NewLC(aSuid, aUserRating, aBuyNow, aPlayCount);
    CleanupStack::Pop(self);
    return self;
    }

/**
@deprecated
MTP object manager meta-data factory method. A pointer to the meta-data is 
placed on the cleanup stack.
@param aSuid A buffer containing the SUID of the data object.
@param aUserRating  user rating of the data object.
@param aBuyNow The MTP object BuyNow code of the data object.
@param aPlayCount The MTP object aPlayCount code of the data object.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C CMTPObjectMetaData* CMTPObjectMetaData::NewLC(const TDesC& aSuid, TUint aUserRating, TUint aBuyNow, TUint aPlayCount)
    {
    CMTPObjectMetaData* self = new(ELeave) CMTPObjectMetaData();
    CleanupStack::PushL(self);
    self->ConstructL(aSuid, aUserRating, aBuyNow, aPlayCount);
    return self;
    }


    
    
    
    
    
    
    
    
    
    
