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

#ifndef CMTPOBJECTSTORE_H
#define CMTPOBJECTSTORE_H

#include <badesca.h>
#include <d32dbms.h>

#include "mtpframeworkconst.h"
#include "rmtpframework.h"
#include "cmtpreferencemgr.h"
class CFileStore;
class CMTPHandleAllocator;
class CMTPObjectMetaData;
class CMTPPuidMgr;
class CMTPReferenceMgr;
class CMTPTypeArray;
class RMTPObjectMgrQueryContext;
class TMTPTypeUint32;
class TMTPTypeUint128;
class TMTPObjectMgrQueryParams;
class CMtpDeltaDataMgr;
class CMTPDPIDStore;
class CMTPPkgIDStore;
/** 
 Implements the MTP object meta data store, which manages the storage/mapping of object handle and suid, and puid using DMBS.
 Note, the CMTPObjectMgr acts as a shim class which forwards all the requests to the CMTPObjectStore. 
 The reason is that later on, the class CMTPObjectStore will also implement the reference manager and puid manager
 functionalities.  Having a single class implements all these will enable us to use a single database with multiple 
 tables.

 @internalComponent
 
 */
class CMTPObjectStore : public CBase
	{
	class CDbCompactor : public CBase
		{
	public:
		inline static CDbCompactor* NewL(RDbNamedDatabase* aDatabase)
			{
			CDbCompactor* self = new (ELeave) CDbCompactor(aDatabase);
			return self;
			}
		inline void Compact()
			{
			if (!((++iCompactCnt) % KCompactCnt))
				{
				RDbDatabase::TSize size = iDatabase->Size();
				iDatabase->Compact();
				}
			}
		inline ~CDbCompactor()
			{
			}
	private:
		inline CDbCompactor(RDbNamedDatabase* aDatabase) : iDatabase(aDatabase)
			{
			}

	private:
		RDbNamedDatabase* iDatabase;
		TUint8 iCompactCnt;
		static const TUint8 KCompactCnt = 32; //Use this value make the performance better haven't test other value
		};

	class MTraverseAction
		{
	public:
		typedef enum {EOnlyLoadedDP, EAllDP} TActionTarget;
		MTraverseAction(TActionTarget aTarget) : iActionTarget(aTarget)
			{
			}
		TActionTarget Target()
			{
			return iActionTarget;
			}
		virtual void DoL(RDbTable& aTable) = 0;
		
	private:
		TActionTarget iActionTarget;
		};
	class TCountAction : public MTraverseAction
		{
	public:
		TCountAction(TUint& aCounter, TActionTarget aTarget = EOnlyLoadedDP) : MTraverseAction(aTarget), iCounter(aCounter)
			{
			}
		void DoL(RDbTable& /*aTable*/)
			{
			++iCounter;
			}
	private:
		TUint& iCounter;
		};

	class TGetHandlesAction : public MTraverseAction
		{
	public:
		TGetHandlesAction(RArray<TUint>& aHandles, TActionTarget aTarget = EOnlyLoadedDP) : MTraverseAction(aTarget), iHandles(aHandles)
			{
			}
		void DoL(RDbTable& aTable)
			{
			iHandles.AppendL(aTable.ColUint32(EObjectStoreHandleId));
			}
	private:
		RArray<TUint>& iHandles;
		};

	class TGetSuidsAction : public MTraverseAction
		{
	public:
		TGetSuidsAction(CDesCArray& aSuids, TActionTarget aTarget = EOnlyLoadedDP) : MTraverseAction(aTarget), iSuids(aSuids)
			{
			}
		void DoL(RDbTable& aTable)
			{
			TFileName buf;
			RDbColReadStream suid;
			suid.OpenLC(aTable, EObjectStoreSUID);
			suid.ReadL(buf, aTable.ColLength(EObjectStoreSUID));
			suid.Close();
			CleanupStack::PopAndDestroy();
			iSuids.AppendL(buf);
			}
	private:
		CDesCArray& iSuids;
		};
	
	class TDelAction : public MTraverseAction
		{
	public:
		TDelAction(CMTPObjectStore& aStore, TActionTarget aTarget = EOnlyLoadedDP) : MTraverseAction(aTarget), iStore(aStore)
			{
			}
		void DoL(RDbTable& aTable)
			{
			//no need to call GetL already all it in TraverseL
			iStore.ReferenceMgr().RemoveReferencesL(aTable.ColUint32(EObjectStoreHandleId));
			aTable.DeleteL();
			iStore.IncTranOpsNumL();
			}
	private:
		CMTPObjectStore& iStore;
		};
	
	class CSnapshotWorker: public CActive
        {
        public:
            static CSnapshotWorker* NewL(CMTPObjectStore* aObjectStore, TBool aOnlyRoot); 
            
            void RunL();
            TInt RunError(TInt aErr);
            void ActiveSelf();
            ~CSnapshotWorker();
			
        private:
            CSnapshotWorker(CMTPObjectStore* aObjectStore, TBool aOnlyRoot);
            void ConstructL();

        private:
            void DoCancel();
            
        private:
        CMTPObjectStore* iObjectStore;
        TBool            iOnlyRoot;
        };
public:

	static CMTPObjectStore* NewL();
	~CMTPObjectStore();

public:

	RDbDatabase& Database();
	CMTPDPIDStore& DPIDStore() const;
	CMTPPkgIDStore& PkgIDStore() const;
	CMTPReferenceMgr& ReferenceMgr() const;
	CMtpDeltaDataMgr* MtpDeltaDataMgr() const;
	void RestorePersistentObjectsL(TUint aDataProviderId);
	void RemoveObjectsByStorageIdL(TUint32 aStorageId);
	void RemoveNonPersistentObjectsL(TUint aDataProviderId);
	void MarkNonPersistentObjectsL(TUint aDataProviderId, TUint32 aStorageId);
	void EstablishDBSnapshotL(TUint32 aStorageId);
	void CleanDBSnapshotL(TBool aOnlyRoot = EFalse);
    void RemoveUndefinedObjectsL();
	void ObjectsEnumComplete();
	void MarkDPLoadedL(TUint aDataProviderId, TBool aFlag);

public:

	TUint CountL(const TMTPObjectMgrQueryParams& aParams) const;
	void GetObjectHandlesL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, RArray<TUint>& aHandles) const;
	void GetObjectSuidsL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& aContext, CDesCArray& aSuids) const;
	void TraverseL(const TMTPObjectMgrQueryParams& aParams, MTraverseAction& action) const;
	void CalcFreeHandlesL(TUint aDataProviderId);
	void CommitReservedObjectHandleL(CMTPObjectMetaData& aObject);
	TUint32 HandleL(const TDesC& aSuid) const;
	TUint32 HandleL(TUint32 aSuidHash, const TDesC& aSuid) const;
	void InsertObjectL(CMTPObjectMetaData& aObject);
	void InsertObjectsL(RPointerArray<CMTPObjectMetaData>& aObjects);
	void ModifyObjectL(const CMTPObjectMetaData& aObject);
	TBool ObjectL(const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aObject) const;
	TBool ObjectL(const TDesC& aSuid, CMTPObjectMetaData& aObject) const;
	TBool ObjectExistsL(const TUint32 aHandle);
	TMTPTypeUint128 PuidL(const TUint32 aHandle);
	TMTPTypeUint128 PuidL(const TDesC& aSuid);

	const TPtrC ObjectSuidL(TUint32 aHandle) const;
	void RemoveObjectL(const TMTPTypeUint32& aHandle);
	void RemoveObjectL(const TDesC& aSuid);
	void RemoveObjectsL(const CDesCArray& aSuids);
	void RemoveObjectsL(TUint aDataProviderId);
	void ReserveObjectHandleL(CMTPObjectMetaData& aObject, TUint64 aSpaceRequired);
	void UnreserveObjectHandleL(const CMTPObjectMetaData& aObject);
	void IncTranOpsNumL();
	TBool IsMediaFormat(TUint32 aFormatCode);

	void BeginTransactionL();
	void CommitTransactionL();

	void CleanL();
	TUint ObjectOwnerId(const TMTPTypeUint32& aHandle) const;
private:

	/**
	 Enumeration representing the column fields in the object store DBMS, this definition must exactly match the sequence
	 defined in KSQLCreateHandleTableText
	 //During insert a object, only the following items is provided in the CMTPObjectMetaData
	 //DPId
	 //Format
	 //FormatSubcode
	 //StorageID
	 //Modes
	 //ParentHandle
	 //
	 //Handle is allocated by MTP framework
	 //NonConsumable is inserted only by FileDP, for objects manged by LicenseeDP, its values is meaningless
	 */
	enum TObjectStore
		{
		EObjectStoreHandleId = 1,
		EObjectStoreSUIDHash = 2,
		EObjectStoreSUID = 3,
		EObjectStoreDataProviderId = 4,
		EObjectStoreFormatCode = 5,
		EObjectStoreFormatSubCode = 6,
		EObjectStoreStorageId = 7,
		EObjectStoreModes = 8,
		EObjectStorePOUID = 9,
		EObjectStoreParentHandle = 10,
		EObjectStoreDPFlag = 11,
		EObjectStoreNonConsumable = 12,
		EObjectStoreName = 13
		};

	class CEnumertingCacheItem : public CBase
		{
	public:
		static CEnumertingCacheItem* NewLC(TUint32 aSuidHash, TUint32 aHandle, TUint32 aParent, TUint32 aFormat, TUint64 aId, TUint8 aDpID)
			{
			CEnumertingCacheItem* self = new (ELeave) CEnumertingCacheItem(aSuidHash, aHandle, aParent, aFormat, aId, aDpID);
			CleanupStack::PushL(self);
			return self;
			}
		static CEnumertingCacheItem* NewL(TUint32 aSuidHash, TUint32 aHandle, TUint32 aParent, TUint32 aFormat, TUint64 aId, TUint8 aDpID)
			{
			CEnumertingCacheItem* self = CEnumertingCacheItem::NewLC(aSuidHash, aHandle, aParent, aFormat, aId, aDpID);
			CleanupStack::Pop();
			return self;
			}
		static TInt Compare(const CEnumertingCacheItem& aFirst, const CEnumertingCacheItem& aSecond);
		CEnumertingCacheItem(TUint32 aSuidHash, TUint32 aHandle, TUint32 aParent, TUint32 aFormat, TUint64 aId, TUint8 aDpID);
		~CEnumertingCacheItem()
			{
			delete iSuid;
			}
		TUint32 iObjHandleId;
		TUint32 iObjParentId;
		TUint32 iObjSuiIdHash;
		TUint32 iFormatcode;
		TUint64 iPOUID;
		HBufC* iSuid;
		TPtrC iSuidPtr;
		TUint8 iDpID;
		};

	CMTPObjectStore();
	void ConstructL();

private:

	TBool LocateByHandleL(const TUint aHandle, const TBool aReadTable = ETrue) const;
	TBool LocateBySuidL(const TDesC& aSuid) const;

	void BuildObjectMetaDataL(CMTPObjectMetaData& aBuf, const RDbTable& aTable) const;
	void InitializeDbL();
	void CreateDbL(const TDesC& aFileName);
	TInt OpenDb(const TDesC& aFileName);
	void CloseDb();

	void CreateHandleTableL();
	void CreateHandleIndexL();
	void CreateStorageTableL();
	void CreateStorageIndexL();

	void GetFullPathName(const TDesC& aName, TFileName& aFileName) const;

	TBool GetObjectL(TUint32 aHandle, CMTPObjectMetaData& aObject) const;

	static void DBUpdateFailRecover(TAny* aTable);
    TBool IsInvalidHandle( TUint32 aHandle ) const;
    TBool FilterObject(const RDbTable& aCurrRow,const TUint32 aStorageID,const TUint32 aFormatCode,const TUint32 aDpID) const;
    void DbColWriteStreamL(RDbTable& aTable, TDbColNo aCol, const TDesC16& aDes);
    void DbColReadStreamL(const RDbTable& aTable, TDbColNo aCol, TDes16& aDes) const;    

private:

	static const TUint KQueryWindowForeSlots = 512;
	static const TUint KQueryWindowRearSlots = 0;
	
	mutable RDbTable						iBatched;
	mutable RDbTable						iBatched_SuidHashID;
	mutable RDbNamedDatabase				iDatabase;
	mutable TBuf<KMTPMaxSqlStatementLen>	iSqlStatement;
	mutable TUint32							iCachedHandle;
	mutable TUint32							iCachedSuidHash;
	
	RArray<TUint>							iNonPersistentDPList;
	//The following object is used for enumerating cache, will be closed after the enumeration is completed.
	RPointerArray<CEnumertingCacheItem>		iEnumeratingCacheObjList;
	RMTPFramework							iSingletons;
	
	
	TUint32									iTransactionOps;
	TUint32									iMaxCommitLimit;
	TUint32									iMaxCompactLimit;
	TBool									iUpdateDeltaDataTable;
	TBool									iCacheExist;
	
	CMTPReferenceMgr*						iReferenceMgr;
	CMTPHandleAllocator*					iHandleAllocator;
	CEnumertingCacheItem*					iSentinal;
	CMTPDPIDStore*							iDPIDStore;
	CMTPPkgIDStore*							iPkgIDStore;
	CMtpDeltaDataMgr* 						iMtpDeltaDataMgr;
	CDbCompactor*							iCompactor;
	TInt                                    iSnapshotCleanPos;
	CSnapshotWorker*                        iSnapshotWorker;
	mutable TFileName                       iSuidBuf;
	TBool                                   iCleanUndefined;
	TBool                                   iDbOpened;
	};

#endif // CMTPOBJECTSTORE_H
