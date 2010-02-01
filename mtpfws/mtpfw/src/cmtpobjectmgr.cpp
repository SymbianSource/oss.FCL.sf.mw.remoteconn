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

#include "cmtpobjectmgr.h"
#include "cmtpdpidstore.h"
#include "cmtpobjectstore.h"
#include "tmtptypeobjecthandle.h"
#include "cmtppkgidstore.h"
/**
MTP object manager information record factory method. This method creates an 
empty object information record.
@return A pointer to a new CMTPObjectMetaData instance, ownership IS transferred.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPObjectMgr* CMTPObjectMgr::NewL()
    {
    CMTPObjectMgr* self = new (ELeave) CMTPObjectMgr();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor
*/    
CMTPObjectMgr::~CMTPObjectMgr()
    {
    delete iObjectStore;
    }

/**
Provides a reference to the object meta data store.
@return The object information store.
*/
CMTPObjectStore& CMTPObjectMgr::ObjectStore()
    {
    return *iObjectStore;
    }

void CMTPObjectMgr::RestorePersistentObjectsL(TUint aDataProviderId)
    {
    iObjectStore->RestorePersistentObjectsL(aDataProviderId);
    }

EXPORT_C void CMTPObjectMgr::RemoveObjectsByStorageIdL(TUint32 aStorageId)
	{
    iObjectStore->RemoveObjectsByStorageIdL(aStorageId);
	}

void CMTPObjectMgr::RemoveNonPersistentObjectsL(TUint aDataProviderId)
	{
	iObjectStore->RemoveNonPersistentObjectsL(aDataProviderId);
	}

void CMTPObjectMgr::MarkNonPersistentObjectsL(TUint aDataProviderId, TUint32 aStorageId)
	{
	iObjectStore->MarkNonPersistentObjectsL(aDataProviderId, aStorageId);
	}
      
void CMTPObjectMgr::MarkDPLoadedL(TUint aDataProviderId, TBool aFlag)
	{
	iObjectStore->MarkDPLoadedL(aDataProviderId, aFlag);
	}
TUint CMTPObjectMgr::CountL(const TMTPObjectMgrQueryParams& aParams) const
    {
    return iObjectStore->CountL(aParams);
    }
    
void CMTPObjectMgr::CommitReservedObjectHandleL(CMTPObjectMetaData& aObjectInfo)
    {
    iObjectStore->CommitReservedObjectHandleL(aObjectInfo);
    }
    
void CMTPObjectMgr::GetObjectHandlesL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, RArray<TUint>& aHandles) const
    {
    iObjectStore->GetObjectHandlesL(aParams, aContext, aHandles);
    }
    
void CMTPObjectMgr::GetObjectSuidsL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, CDesCArray& aSuids) const
    {
    iObjectStore->GetObjectSuidsL(aParams, aContext, aSuids);
    }
    
TUint32 CMTPObjectMgr::HandleL(const TDesC& aSuid) const
    {
    return iObjectStore->HandleL(aSuid);
    }
    
void CMTPObjectMgr::InsertObjectL(CMTPObjectMetaData& aObject)
    {
    iObjectStore->InsertObjectL(aObject);
    }
    
void CMTPObjectMgr::InsertObjectsL(RPointerArray<CMTPObjectMetaData>& aObjects)
    {
    iObjectStore->InsertObjectsL(aObjects);
    }
void CMTPObjectMgr::InsertDPIDObjectL(TUint aDpId, TUint aUid)
    {
    iObjectStore->DPIDStore().InsertDPIDObjectL(aDpId,aUid);
    }  

void CMTPObjectMgr::InsertPkgIDObjectL(TUint aDpId, TUint aPkgId)
    {
    iObjectStore->PkgIDStore().InsertPkgIdL(aDpId,aPkgId);
    }
    
void CMTPObjectMgr::ModifyObjectL(const CMTPObjectMetaData& aObject)
    {
    iObjectStore->ModifyObjectL(aObject);
    }
    
TBool CMTPObjectMgr::ObjectL(const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aBuf) const
    {
    return iObjectStore->ObjectL(aHandle, aBuf);
    }
    
TBool CMTPObjectMgr::ObjectL(const TDesC& aSuid, CMTPObjectMetaData& aBuf) const            
    {
    return iObjectStore->ObjectL(aSuid, aBuf);
    }
    
TUint CMTPObjectMgr::ObjectOwnerId(const TMTPTypeUint32& aHandle) const
    {
    return iObjectStore->ObjectOwnerId(aHandle);
    }

TUint CMTPObjectMgr::DPIDL(const TUint aUid, TBool& aFlag) const
    {
    return iObjectStore->DPIDStore().DPIDL(aUid, aFlag);
    }    

TMTPTypeUint128 CMTPObjectMgr::PuidL(TUint32 aHandle) const
    {
    return iObjectStore->PuidL(aHandle);
    }
    
    
TMTPTypeUint128 CMTPObjectMgr::PuidL(TInt64 /*aObjectUid*/) const
    {
    User::Leave(KErrNotSupported);
    return TMTPTypeUint128(0);
    }
    
TMTPTypeUint128 CMTPObjectMgr::PuidL(const TDesC& aSuid) const
    {
    return iObjectStore->PuidL(aSuid);
    }
    
void CMTPObjectMgr::RemoveObjectL(const TMTPTypeUint32& aHandle)
    {
    iObjectStore->RemoveObjectL(aHandle);
    }
    
void CMTPObjectMgr::RemoveObjectL(const TDesC& aSuid)
    {
    iObjectStore->RemoveObjectL(aSuid);
    }
    
void CMTPObjectMgr::RemoveObjectsL(const CDesCArray& aSuids)
    {
    iObjectStore->RemoveObjectsL(aSuids);
    }
    
void CMTPObjectMgr::RemoveObjectsL(TUint aDataProviderId)
    {
    iObjectStore->RemoveObjectsL(aDataProviderId);
    }
    
void CMTPObjectMgr::ReserveObjectHandleL(CMTPObjectMetaData& aObjectInfo, TUint64 aSpaceRequired)
    {
    iObjectStore->ReserveObjectHandleL(aObjectInfo, aSpaceRequired);
    }
    
void CMTPObjectMgr::UnreserveObjectHandleL(const CMTPObjectMetaData& aObjectInfo)
    {
    iObjectStore->UnreserveObjectHandleL(aObjectInfo);
    }

/**
Standard C++ constructor
*/    	
CMTPObjectMgr::CMTPObjectMgr()
    {
    }

/**
second phase construction
*/    
void CMTPObjectMgr::ConstructL()
    {
    iObjectStore = CMTPObjectStore::NewL();
    }

/**
return MtpDeltaDataMgr
*/    
TAny* CMTPObjectMgr::MtpDeltaDataMgr()
    {
    return 	iObjectStore->MtpDeltaDataMgr();
    }


    

