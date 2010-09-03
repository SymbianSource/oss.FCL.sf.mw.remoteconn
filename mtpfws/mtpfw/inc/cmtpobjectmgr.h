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

#ifndef CMTPOBJECTMGR_H
#define CMTPOBJECTMGR_H

#include <e32base.h>
#include <mtp/mmtpobjectmgr.h>

#include "cmtpobjectstore.h"

class CMTPObjectStore;
class RFs;
class TMTPTypeUint32;
class CMTPPuidMgr;
class CMTPReferenceMgr;

/** 
Implements the MTP object manager interface.
@internalComponent
 
*/
class CMTPObjectMgr : 
    public CBase, 
    public MMTPObjectMgr
    {
public:   
 
    static CMTPObjectMgr* NewL();
    ~CMTPObjectMgr();

public:

    CMTPObjectStore& ObjectStore();
    void RestorePersistentObjectsL(TUint aDataProviderId);
    IMPORT_C void RemoveObjectsByStorageIdL(TUint32 aStorageId);
    void RemoveNonPersistentObjectsL(TUint aDataProviderId);
    void MarkNonPersistentObjectsL(TUint aDataProviderId ,TUint32 aStorageId);
    void MarkDPLoadedL(TUint aDataProviderId, TBool aFalg);
   
    void InsertPkgIDObjectL(TUint aDpId, TUint aPkgId);
       
    
public: // From MMTPObjectMgr

    TUint CountL(const TMTPObjectMgrQueryParams& aParams) const;
	void CommitReservedObjectHandleL(CMTPObjectMetaData& aObject);
    void GetObjectHandlesL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, RArray<TUint>& aHandles) const;
    void GetObjectSuidsL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, CDesCArray& aSuids) const;
	TUint32 HandleL(const TDesC& aSuid) const;
    void InsertObjectL(CMTPObjectMetaData& aObject);
    void InsertDPIDObjectL(TUint aDpId, TUint aUid);
    void InsertObjectsL(RPointerArray<CMTPObjectMetaData>& aObjects);
    void ModifyObjectL(const CMTPObjectMetaData& aObject);
    TBool ObjectL(const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aObject) const;
    TBool ObjectL(const TDesC& aSuid, CMTPObjectMetaData& aObject) const;
    TUint ObjectOwnerId(const TMTPTypeUint32& aHandle) const;
    TUint DPIDL(const TUint aUid, TBool& aFlag) const;    
	TMTPTypeUint128 PuidL(TUint32 aHandle) const;
	TMTPTypeUint128 PuidL(TInt64 aObjectUid) const;
	TMTPTypeUint128 PuidL(const TDesC& aSuid) const;
    void RemoveObjectL(const TMTPTypeUint32& aHandle);
    void RemoveObjectL(const TDesC& aSuid);
    void RemoveObjectsL(const CDesCArray& aSuids);
    void RemoveObjectsL(TUint aDataProviderId);
	void ReserveObjectHandleL(CMTPObjectMetaData& aObject, TUint64 aSpaceRequired);
	void UnreserveObjectHandleL(const CMTPObjectMetaData& aObject);

    TAny* MtpDeltaDataMgr();
    
private:
    
    CMTPObjectMgr();
    void ConstructL();
    
private: // Owned

    /**
    The object meta data store.
    */
    CMTPObjectStore*    iObjectStore;
    };
    
#endif // CMTPOBJECTMGR_H
