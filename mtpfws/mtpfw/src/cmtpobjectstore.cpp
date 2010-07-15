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
#include <f32file.h>
#include <s32file.h>
#include "tmtptypeobjecthandle.h"
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/cmtptypearray.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/mtpobjectmgrquerytypes.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptypeuint32.h>
#include "cmtphandleallocator.h"
#include "cmtpobjectstore.h"
#include "dbutility.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpdataprovider.h"
#include "cmtpdpidstore.h"
#include "cmtppkgidstore.h"
#include "cmtpdeltadatamgr.h"
#include <e32hashtab.h>
#include "cmtpstoragemgr.h"

_LIT(KMTPDbDriveLocation, "c:");
_LIT(KMTPBackSlash, "\\");
_LIT(KMTPHandleObjectDbName, "MTPObjectStore.db");
_LIT(KMTPNoBackupFolderName, "nobackup");
_LIT(KSQLHandleTableName, "HandleStore");
_LIT(KSQLCreateHandleTableText, "CREATE TABLE HandleStore (HandleId UNSIGNED INTEGER, SuidHash UNSIGNED INTEGER, Suid LONG VARCHAR, DataProviderId UNSIGNED TINYINT, FormatCode UNSIGNED SMALLINT, FormatSubCode UNSIGNED SMALLINT, StorageId UNSIGNED INTEGER, Modes UNSIGNED TINYINT, POUID BIGINT, ParentHandle UNSIGNED INTEGER, DPFlag UNSIGNED TINYINT, NonConsumable UNSIGNED TINYINT, Name VARCHAR(255))");
_LIT(KSQLHandleId, "HandleIndex");
_LIT(KSQLCreateHandleIndexText,"CREATE UNIQUE INDEX HandleIndex on HandleStore (HandleId)");
_LIT(KSQLSuidHash, "SuidIndex");
_LIT(KSQLCreateSuidIndexText,"CREATE INDEX SuidIndex on HandleStore (SuidHash)");
_LIT(KSQLParentHandle, "ParentHandleIndex");
_LIT(KSQLCreateParentHandleText,"CREATE INDEX ParentHandleIndex on HandleStore (ParentHandle)");
_LIT(KMTPFormat, "MTP");
__FLOG_STMT(_LIT8(KComponent,"MTPObjectStore");)
const TInt KMaxLimitCommitInEnumeration = 1024;
const TInt KMaxLimitCommitAfterEnumeration = 256;
const TInt KMaxLimitCompactInEnumeration = 2048;
const TInt KMaxLimitCompactAfterEnumeration = 1024;
const TInt KSnapshotGranularity = 128; 
const TInt KMaxLimitSnapshotSize = 50000;






CMTPObjectStore::CSnapshotWorker* CMTPObjectStore::CSnapshotWorker::NewL(CMTPObjectStore* aObjectStore, TBool aOnlyRoot)
    {
    CSnapshotWorker* self = new (ELeave) CMTPObjectStore::CSnapshotWorker(aObjectStore, aOnlyRoot);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;

    }

void CMTPObjectStore::CSnapshotWorker::RunL()
    {
    iObjectStore->CleanDBSnapshotL(iOnlyRoot);
    }

void CMTPObjectStore::CSnapshotWorker::DoCancel()
    {
    //nothing to do
    }

TInt CMTPObjectStore::CSnapshotWorker::RunErr()
    {
    return KErrNone;
    }

void CMTPObjectStore::CSnapshotWorker::ActiveSelf()
    {
    TRequestStatus *status = &iStatus;
    iStatus = KRequestPending;
    User::RequestComplete(status, KErrNone);
    SetActive();
    }

CMTPObjectStore::CSnapshotWorker::CSnapshotWorker(CMTPObjectStore* aObjectStore, TBool aOnlyRoot):
        CActive(EPriorityLow), iObjectStore(aObjectStore), iOnlyRoot(aOnlyRoot)
    {

    }

void CMTPObjectStore::CSnapshotWorker::ConstructL()
    {
    CActiveScheduler::Add(this);
    }

/**
 MTP object meta data store factory method. 
 @return A pointer to a new CMTPObjectStore instance, ownership IS transferred.
 @leave One of the system wide error codes, if a processing failure occurs.
 */
CMTPObjectStore* CMTPObjectStore::NewL()
	{
	CMTPObjectStore* self = new (ELeave) CMTPObjectStore();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
 Destructor.
 */
CMTPObjectStore::~CMTPObjectStore()
	{
	delete iCompactor;
	delete iReferenceMgr;
	delete iHandleAllocator;
	delete iMtpDeltaDataMgr;
	delete iDPIDStore;
	delete iPkgIDStore;
	delete iSentinal;
	delete iSnapshotWorker;
	TRAP_IGNORE(CommitTransactionL());
	iDatabase.Compact();
	iBatched.Close();
	iBatched_SuidHashID.Close();
	CloseDb();
	iSingletons.Close();
	iNonPersistentDPList.Close();
	iEnumeratingCacheObjList.ResetAndDestroy();
	__FLOG_CLOSE;
	}

/**
 Provides a reference to the object meta data store database.
 @return The object information store database.
 */
RDbDatabase& CMTPObjectStore::Database()
	{
	return iDatabase;
	}

/**
 Provides a reference to the DPID store object.
 @return The DPID store.
 */
CMTPDPIDStore& CMTPObjectStore::DPIDStore() const
	{
	return *iDPIDStore;
	}

CMTPPkgIDStore& CMTPObjectStore::PkgIDStore() const
	{
	return *iPkgIDStore;
	}

/**
 Provides a reference to the reference manager object.
 @return The reference manager.
 */
CMTPReferenceMgr& CMTPObjectStore::ReferenceMgr() const
	{
	return *iReferenceMgr;
	}

/**
Provides a reference to the MTP delta Data manager object.
@return The MTP delta data manager.
*/ 
CMtpDeltaDataMgr* CMTPObjectStore:: MtpDeltaDataMgr() const
	{
	return iMtpDeltaDataMgr;
	}

void CMTPObjectStore::RemoveNonPersistentObjectsL(TUint /*aDataProviderId*/)
	{
	}

void CMTPObjectStore::MarkNonPersistentObjectsL(TUint aDataProviderId, TUint32)
	{
	TInt result = iNonPersistentDPList.InsertInOrder(aDataProviderId);
	if(result != KErrAlreadyExists)
		{
		User::LeaveIfError(result);
		}
	}

void CMTPObjectStore::MarkDPLoadedL(TUint aDataProviderId, TBool aFlag)
	{
	__FLOG(_L8("MarkDPFlafFalseL - Entry"));
	if (!aFlag)
		{
		_LIT(KSQLMarkfalgDPFalse, "UPDATE HandleStore SET DPFlag = %u WHERE DataProviderId = %u");
		iSqlStatement.Format(KSQLMarkfalgDPFalse, aFlag, aDataProviderId);
		User::LeaveIfError(iDatabase.Execute(iSqlStatement));
		}
	__FLOG(_L8("MarkNonPersistentObjectsL - Exit"));
	}

TBool CMTPObjectStore::FilterObject(const RDbTable& aCurrRow,const TUint32 aStorageID,const TUint32 aFormatCode,const TUint32 aDpID) const
    {
    return  ( ((KMTPStorageAll==aStorageID) ||(aCurrRow.ColUint32(EObjectStoreStorageId)== aStorageID)) 
            
            &&((0 == aFormatCode)||(aCurrRow.ColUint16(EObjectStoreFormatCode) == aFormatCode)) 
            
            &&((aDpID==KMTPDataProviderAll)||(aCurrRow.ColUint8(EObjectStoreDataProviderId) == aDpID ))
            );
    }


void CMTPObjectStore::TraverseL(const TMTPObjectMgrQueryParams& aParams, MTraverseAction& action) const
	{
	RDbTable temp;
	CleanupClosePushL(temp);
	temp.Open(iDatabase, KSQLHandleTableName, RDbRowSet::EUpdatable);

	TUint32 tmpStorageID = 0;
	TBool bStorageAvailable = ETrue;
	
	if (aParams.iParentHandle == 0)
		{//ParentHandle is not used to filter the objects. Need to iterate throught all objects in the storage to filter based on format/dpid/storatgeid.
		//Do not using index to iterate through the whole storage is faster than index iteration.
		temp.FirstL();
		while (!temp.AtEnd())
			{
			temp.GetL();
			if ((MTraverseAction::EAllDP == action.Target()) || (temp.ColUint8(EObjectStoreDPFlag) == 1))//If the DP is loaded or ation for all dps.
				{

				if((aParams.iStorageId == KMTPStorageAll) &&(aParams.iFormatCode == 0)&&(iSingletons.DpController().DataProviderL(temp.ColUint8(EObjectStoreDataProviderId)).SupportedCodes( EServiceIDs ).Count() != 0))
				    {
				    temp.NextL();
				    continue;
				    }
				
				//check StorageID ,DPID and Formatcode
				if ( !FilterObject(temp,aParams.iStorageId,aParams.iFormatCode,aParams.iDpId) )
				    {
						temp.NextL();
						continue;
				    }
				    
				//verify the whether the storageID is validated.   
				if(tmpStorageID != temp.ColUint32(EObjectStoreStorageId))
					{
					tmpStorageID = temp.ColUint32(EObjectStoreStorageId);
					bStorageAvailable = iSingletons.StorageMgr().ValidStorageId( tmpStorageID );
					}
				if(!bStorageAvailable)
					{
					temp.NextL();
					continue;
					}
								
				action.DoL(temp); 
				}
			temp.NextL();
			}
		}
	else
		{//Fetch the root directory
		temp.SetIndex(KSQLParentHandle);
		if (temp.SeekL((TUint) aParams.iParentHandle))
			{//every storageID has the same root directory handles.KMTPHandleNoParent, therefore, need to filter the storageID
			//if it is not KMTPStorageAll
			while (!temp.AtEnd())
				{
				temp.GetL();
				if (temp.ColUint32(EObjectStoreParentHandle) == aParams.iParentHandle)
					{
					if ((MTraverseAction::EAllDP == action.Target()) || (temp.ColUint8(EObjectStoreDPFlag) == 1))//If the DP is loaded or ation for all dps.
						{
						
            if((aParams.iStorageId == KMTPStorageAll) &&(aParams.iFormatCode == 0)&&(iSingletons.DpController().DataProviderL(temp.ColUint8(EObjectStoreDataProviderId)).SupportedCodes( EServiceIDs ).Count() != 0))
                {
                temp.NextL();
                continue;
                }
		                
						//check StorageID ,DPID and Formatcode
						if ( !FilterObject(temp,aParams.iStorageId,aParams.iFormatCode,aParams.iDpId) )
						    {
								temp.NextL();
								continue;
						    }
						    
						//verify the whether the storageID is validated.   
						if(tmpStorageID != temp.ColUint32(EObjectStoreStorageId))
							{
							tmpStorageID = temp.ColUint32(EObjectStoreStorageId);
							bStorageAvailable = iSingletons.StorageMgr().ValidStorageId( tmpStorageID );
							}
						if(!bStorageAvailable)
							{
							temp.NextL();
							continue;
							}
		                
						action.DoL(temp);
						}
					temp.NextL();
					}
				else
					{
					break;//has jumped over the range of handles with the same parent handle
					}
				}
			}
		}
	CleanupStack::PopAndDestroy(&temp);
	//Since we fetch all handles as requested, therefore, we do not open aContext to indicate the query is completed.
	}

TUint CMTPObjectStore::CountL(const TMTPObjectMgrQueryParams& aParams) const
	{
	TUint i = 0;
	TCountAction action(i);
	TraverseL(aParams, action);
	return i;
	}

void CMTPObjectStore::CommitReservedObjectHandleL(CMTPObjectMetaData& aObject)
	{
	TFileName suid;
	suid.CopyLC(aObject.DesC(CMTPObjectMetaData::ESuid));
	TUint32 handle = HandleL(suid);
	if (handle != KMTPHandleNone)
	    {
	    __FLOG(_L8("CommitReserverd leave for duplicate suid."));
	    User::Leave(KErrAlreadyExists);
	    }
	TUint32 suidHash = DefaultHash::Des16(suid);
	
	//After the PutL called the cursor's position is not well defined.
	iCachedHandle = 0;
	iCachedSuidHash = 0;
	TInt64 id = iHandleAllocator->NextPOUIDL();
	aObject.SetUint(CMTPObjectMetaData::EIdentifier, id);

	handle = aObject.Uint(CMTPObjectMetaData::EHandle);
	CleanupStack::PushL(TCleanupItem(CMTPObjectStore::DBUpdateFailRecover, &iBatched));
	iBatched.InsertL();
	iBatched.SetColL(EObjectStoreHandleId, handle);
	iBatched.SetColL(EObjectStoreSUIDHash, suidHash);
	DbColWriteStreamL(iBatched, EObjectStoreSUID, aObject.DesC(CMTPObjectMetaData::ESuid));	
	iBatched.SetColL(EObjectStoreDataProviderId, aObject.Uint(CMTPObjectMetaData::EDataProviderId));
	iBatched.SetColL(EObjectStoreFormatCode, aObject.Uint(CMTPObjectMetaData::EFormatCode));
	iBatched.SetColL(EObjectStoreFormatSubCode, aObject.Uint(CMTPObjectMetaData::EFormatSubCode));
	iBatched.SetColL(EObjectStoreStorageId, aObject.Uint(CMTPObjectMetaData::EStorageId));
	iBatched.SetColL(EObjectStoreModes,aObject.Uint(CMTPObjectMetaData::EModes));
	iBatched.SetColL(EObjectStorePOUID, id);
	iBatched.SetColL(EObjectStoreParentHandle, aObject.Uint(CMTPObjectMetaData::EParentHandle));
	iBatched.SetColL(EObjectStoreDPFlag, 1);
	iBatched.SetColL(EObjectStoreNonConsumable, aObject.Uint(CMTPObjectMetaData::ENonConsumable));
	iBatched.SetColL(EObjectStoreName, aObject.DesC(CMTPObjectMetaData::EName));
	
	iBatched.PutL();
	CleanupStack::Pop(&iBatched);
	IncTranOpsNumL();

	}

/*
 This API is designed intended for coping with the MTP operation GetObjectHandles which has 3 parameters.
 1. StorageID
 2. FormatCode
 3. ParentHandle.

 The first parameter contains the StorageID of the storage for which the list of Object Handles is desired. 
 A value of 0xFFFFFFFF may be used to indicate that a list of Object Handles of all objects on all storages 
 should be returned. If a storage is specified and the storage is unavailable, this operation should return Store_Not_Available.

 The second parameter is optional, and contains an Object Format datacode. Object Formats are described
 in section 4, "Object Formats". If the second parameter contains a non-0x00000000 value, it specifies that 
 a list of object handles referencing objects of a certain object format is desired. If the parameter is not used, 
 it should contain a value of 0x00000000 and objects should be included in the response dataset regardless 
 of their object format. If this parameter is not supported, the responder should return a response code of 
 Specification_By_Format_Unsupported.

 The third parameter may be used to restrict the list of objects that are returned by this operation to objects 
 directly contained in a particular folder (Association). If this parameter contains a non-0x00000000 value, 
 the responder should return a list of objects which have as their ParentObject the folder (Association) 
 that is identified by this parameter. If the number of objects that are contained in the root of a storage is desired, 
 a value of 0xFFFFFFFF may be passed in this operation, which indicates that only those objects with no ParentObject
 should be returned. If the first parameter indicates that all storages are included in this query, then a value of 0xFFFFFFFF
 should return a list of all objects at the root level of all storages. If this parameter is unused, it should contain a value of 0x00000000. 

 If the third parameter is unsupported and a non-0x00000000 value is sent in this operation, a response of 
 Parameter_Unsupported should be returned. If the use of the third parameter is supported, but the value 
 contained does not reference an actual object on the device, a response of Invalid_ObjectHandle should be returned. 
 If the use of the third parameter is supported and it contains a valid Object Handle, but the object referenced 
 is not of type Association, then a response of Invalid_ParentObject should be returned.


 */
void CMTPObjectStore::GetObjectHandlesL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& /*aContext*/, RArray<TUint>& aHandles) const
	{
	TGetHandlesAction action(aHandles);
	TraverseL(aParams, action);
	}

void CMTPObjectStore::GetObjectSuidsL(const TMTPObjectMgrQueryParams& aParams, RMTPObjectMgrQueryContext& /*aContext*/, CDesCArray& aSuids) const
	{
	//take the similar approach for GetObjectHandlesL, but need to pay attention for the memory usage, since every SUID might take
	//maximum 255 bytes, if there are totally 10K objects, the maximum memory usage will be 2.5M, consider to get it in multiple calls instead of one.
	TGetSuidsAction action(aSuids);
	TraverseL(aParams, action);
	}

TUint32 CMTPObjectStore::HandleL(const TDesC& aSuid) const
	{
	TUint32 handle = KMTPHandleNone;
	if (LocateBySuidL(aSuid))
		{
		handle = iBatched_SuidHashID.ColUint32(EObjectStoreHandleId);
		}
	return handle;
	}

void CMTPObjectStore::DBUpdateFailRecover(TAny* aTable)
	{
	reinterpret_cast<RDbTable*> (aTable)->Cancel();
	}

void CMTPObjectStore::InsertObjectL(CMTPObjectMetaData& aObject)
	{
	__FLOG(_L8("InsertObjectL - Entry"));
	iCachedHandle = 0;
	iCachedSuidHash = 0;
	TBool needToInsert = EFalse;
	TBool needUpdateOwner = EFalse;
	TUint dpId(aObject.Uint(CMTPObjectMetaData::EDataProviderId));

	if ((aObject.DesC(CMTPObjectMetaData::ESuid)).Length() > KMaxFileName)
	{
	// The length of object uid should not excceeds KMaxFileName
	User::Leave( KErrBadName );
	}

	TFileName suid;
	suid.CopyLC(aObject.DesC(CMTPObjectMetaData::ESuid));
	TUint32 suidHash = DefaultHash::Des16(suid);
	TUint32 parentHandle = aObject.Uint(CMTPObjectMetaData::EParentHandle);
	TUint32 handle = KMTPHandleNone, handleInDB = KMTPHandleAll;
	TInt64 id = 0;
	// Check if the dp is enumerating
	if (iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted && iCacheExist)
		{
		//it is in the object enumeration phase. 
		// if it's see if we have an object with the same SUID
		//When the MTP server startup the object enumeration, it will fetch all of the object's (SUIDHash and Handles) into in-memory ordered array.
		//this function is to try to match the handleID for the incoming suidHash/SUID/DPID, if matched in memory, it will return the handleID
		//considering the possible conflicting of the SUID for different file name, it need to consult the SUID in the table for eactly match.
		//Therefore, for each object matching, it need one table lookup and file name cope and match operation.
		//If it matched, do nothing for the table, but remove it from the in-memory ordered array.
		//if there is no match, it is a new object, insert it into table.
		iSentinal->iObjSuiIdHash = suidHash;
		iSentinal->iSuidPtr.Set(aObject.DesC(CMTPObjectMetaData::ESuid));
		TInt found = iEnumeratingCacheObjList.FindInOrder(iSentinal, TLinearOrder<CEnumertingCacheItem>(CEnumertingCacheItem::Compare));
		if (KErrNotFound != found)
			{//There is a match
			if(NULL == iEnumeratingCacheObjList[found]->iSuid)
				{//need extra check for hash collision
				handleInDB = HandleL(aObject.DesC(CMTPObjectMetaData::ESuid));
				}
			handle = iEnumeratingCacheObjList[found]->iObjHandleId;
			if((handleInDB != KMTPHandleAll) && (handleInDB != handle)) //hash collision
				{
				needToInsert = ETrue;
				}
			else
				{
				aObject.SetUint(CMTPObjectMetaData::EHandle, handle);
				id = iEnumeratingCacheObjList[found]->iPOUID;
				aObject.SetUint(CMTPObjectMetaData::EIdentifier, id);
				if(iEnumeratingCacheObjList[found]->iFormatcode != aObject.Uint(CMTPObjectMetaData::EFormatCode) ||
					 iEnumeratingCacheObjList[found]->iObjParentId != aObject.Uint(CMTPObjectMetaData::EParentHandle))
					{//have different owner
					needUpdateOwner = ETrue;
					}
				delete iEnumeratingCacheObjList[found];
				iEnumeratingCacheObjList.Remove(found);
				}
			__FLOG_VA(_L8("Found in Snapshot"));
			}
		else
			{//This is a totally new object. insert it after check the db to prevent user wrong operation
			handle = HandleL(aObject.DesC(CMTPObjectMetaData::ESuid));
			if (handle == KMTPHandleNone)
				{
				needToInsert = ETrue;
				}
			else
				{
				aObject.SetUint(CMTPObjectMetaData::EHandle, handle);
				CMTPObjectMetaData* object(CMTPObjectMetaData::NewLC());
				if(ObjectL(aObject.DesC(CMTPObjectMetaData::ESuid), *object))
					{
					if(object->Uint(CMTPObjectMetaData::EDataProviderId) != aObject.Uint(CMTPObjectMetaData::EDataProviderId))
						{
						needUpdateOwner = ETrue;
						}
					}
				CleanupStack::PopAndDestroy(object);
				}
			__FLOG_VA(_L8("Not Found in Snapshot"));
			}
		__FLOG_VA((_L8("InsertObjectL Under enmueration, needUpdateOwner %d needToInsert %d"), needUpdateOwner, needToInsert));
		}
	else
		{
		handle = HandleL(aObject.DesC(CMTPObjectMetaData::ESuid));
		if (handle != KMTPHandleNone)
			{
				//Leaves if id already exists in suid map table 
			User::Leave(KErrAlreadyExists);
			}
		// dp is not enumerating, do a plain insert
		needToInsert = ETrue;
		__FLOG_VA((_L8("InsertObjectL After enmueration, needUpdateOwner %d needToInsert %d"), needUpdateOwner, needToInsert));
		}
		
	if (needToInsert)//needToInsert and needUpdateOwner can't be true at same time
		{
		TUint32 parentHandle(aObject.Uint(CMTPObjectMetaData::EParentHandle));
		handle = iHandleAllocator->NextIdL(dpId);
		id = iHandleAllocator->NextPOUIDL();
		aObject.SetUint(CMTPObjectMetaData::EHandle, handle);
		aObject.SetUint(CMTPObjectMetaData::EIdentifier, id);
		CleanupStack::PushL(TCleanupItem(CMTPObjectStore::DBUpdateFailRecover, &iBatched));
		iBatched.InsertL();
		iBatched.SetColL(EObjectStoreHandleId, handle);
		iBatched.SetColL(EObjectStoreSUIDHash, suidHash);
		DbColWriteStreamL(iBatched, EObjectStoreSUID, aObject.DesC(CMTPObjectMetaData::ESuid));
		iBatched.SetColL(EObjectStoreDataProviderId, aObject.Uint(CMTPObjectMetaData::EDataProviderId));
		iBatched.SetColL(EObjectStoreFormatCode, aObject.Uint(CMTPObjectMetaData::EFormatCode));
		iBatched.SetColL(EObjectStoreFormatSubCode, aObject.Uint(CMTPObjectMetaData::EFormatSubCode));
		iBatched.SetColL(EObjectStoreStorageId, aObject.Uint(CMTPObjectMetaData::EStorageId));
		iBatched.SetColL(EObjectStoreModes, aObject.Uint(CMTPObjectMetaData::EModes));
		iBatched.SetColL(EObjectStorePOUID, id);
		iBatched.SetColL(EObjectStoreParentHandle, parentHandle);
		iBatched.SetColL(EObjectStoreDPFlag, 1);
		iBatched.SetColL(EObjectStoreNonConsumable, aObject.Uint(CMTPObjectMetaData::ENonConsumable));
		iBatched.SetColL(EObjectStoreName, aObject.DesC(CMTPObjectMetaData::EName));
		iBatched.PutL();
		CleanupStack::Pop(&iBatched);
		IncTranOpsNumL();
		}
	else if(needUpdateOwner)
		{
		if(iBatched.SeekL(static_cast<TUint>(handle)))
			{
			CleanupStack::PushL(TCleanupItem(CMTPObjectStore::DBUpdateFailRecover, &iBatched));
			iBatched.UpdateL();
			iBatched.SetColL(EObjectStoreDataProviderId, aObject.Uint(CMTPObjectMetaData::EDataProviderId));
			iBatched.SetColL(EObjectStoreFormatCode, aObject.Uint(CMTPObjectMetaData::EFormatCode));
			iBatched.SetColL(EObjectStoreFormatSubCode, aObject.Uint(CMTPObjectMetaData::EFormatSubCode));
			iBatched.SetColL(EObjectStoreStorageId, aObject.Uint(CMTPObjectMetaData::EStorageId));
			iBatched.SetColL(EObjectStoreModes, aObject.Uint(CMTPObjectMetaData::EModes));
			iBatched.SetColL(EObjectStoreNonConsumable, aObject.Uint(CMTPObjectMetaData::ENonConsumable));
			iBatched.SetColL(EObjectStoreName, aObject.DesC(CMTPObjectMetaData::EName));
			iBatched.SetColL(EObjectStoreParentHandle, aObject.Uint(CMTPObjectMetaData::EParentHandle));
			iBatched.PutL();
			CleanupStack::Pop(&iBatched);
			IncTranOpsNumL();	
			}
		}
	if ((needToInsert || needUpdateOwner) && IsMediaFormat(aObject.Uint(CMTPObjectMetaData::EFormatCode)))
		{
		if (iUpdateDeltaDataTable)
			{
			iMtpDeltaDataMgr->UpdateDeltaDataTableL(id, CMtpDeltaDataMgr::EAdded);
			}
		}

	__FLOG(_L8("InsertObjectL - Exit"));
	}

void CMTPObjectStore::IncTranOpsNumL()
	{
	iTransactionOps++;
	if (iTransactionOps % iMaxCommitLimit == 0)
		{
		CommitTransactionL();
		if (iTransactionOps % iMaxCompactLimit == 0)
			{
			User::LeaveIfError(iDatabase.Compact());
			}
		BeginTransactionL();
		}
	}

void CMTPObjectStore::BeginTransactionL()
	{
	if (!iDatabase.InTransaction())
		{
		User::LeaveIfError(iDatabase.Begin());
		}
	}

void CMTPObjectStore::CommitTransactionL()
	{
	__FLOG(_L8("CommitTransactionL Entry"));
	if (iDatabase.InTransaction())
		{
		User::LeaveIfError(iDatabase.Commit());
		}
	__FLOG(_L8("CommitTransactionL Exit"));
	}

void CMTPObjectStore::InsertObjectsL(RPointerArray<CMTPObjectMetaData>& aObjects)
	{
	TInt count = aObjects.Count();
	for (TInt i = 0; i < count; i++)
		{
		InsertObjectL(*aObjects[i]);
		}
	}

void CMTPObjectStore::ModifyObjectL(const CMTPObjectMetaData& aObject)
	{
	TUint32 handle = aObject.Uint(CMTPObjectMetaData::EHandle);
	TFileName suid;
	suid.CopyLC(aObject.DesC(CMTPObjectMetaData::ESuid));
	TUint32 suidHash = DefaultHash::Des16(suid);

	if (LocateByHandleL(handle))
		{
		//To avoid this modification will not generate duplicate SUID
		TUint32 handle2 = HandleL(suid);
		if (handle2 != KMTPHandleNone && handle2 != handle)
		    {
		    __FLOG(_L8("ModifyObjectL leave for duplicate suid."));
		    User::Leave(KErrAlreadyExists); 
		    }
		
		//After the PutL called the cursor's position is not well defined.
		iCachedHandle = 0;
		iCachedSuidHash = 0;
		TInt64 id = iBatched.ColInt64(EObjectStorePOUID);
		TUint32 suidHashOld = iBatched.ColUint32(EObjectStoreSUIDHash);
		TUint32 parentOld = iBatched.ColUint32(EObjectStoreParentHandle);
		CleanupStack::PushL(TCleanupItem(CMTPObjectStore::DBUpdateFailRecover, &iBatched));
		iBatched.UpdateL();
		if(suidHashOld != suidHash)
			{//change on index column will impact the performace
			iBatched.SetColL(EObjectStoreSUIDHash, suidHash);
			}
		if(parentOld != aObject.Uint(CMTPObjectMetaData::EParentHandle))
			{//change on index column will impact the performace
			iBatched.SetColL(EObjectStoreParentHandle, aObject.Uint(CMTPObjectMetaData::EParentHandle));
			}

		DbColWriteStreamL(iBatched, EObjectStoreSUID, aObject.DesC(CMTPObjectMetaData::ESuid));		
		iBatched.SetColL(EObjectStoreDataProviderId, aObject.Uint(CMTPObjectMetaData::EDataProviderId));
		iBatched.SetColL(EObjectStoreFormatCode, aObject.Uint(CMTPObjectMetaData::EFormatCode));
		iBatched.SetColL(EObjectStoreFormatSubCode, aObject.Uint(CMTPObjectMetaData::EFormatSubCode));
		iBatched.SetColL(EObjectStoreStorageId, aObject.Uint(CMTPObjectMetaData::EStorageId));
		iBatched.SetColL(EObjectStoreModes, aObject.Uint(CMTPObjectMetaData::EModes));
		iBatched.SetColL(EObjectStoreDPFlag, 1);
		iBatched.SetColL(EObjectStoreNonConsumable, aObject.Uint(CMTPObjectMetaData::ENonConsumable));
		iBatched.SetColL(EObjectStoreName, aObject.DesC(CMTPObjectMetaData::EName));
		iBatched.PutL();
		CleanupStack::Pop(&iBatched);
		IncTranOpsNumL();
		if (aObject.Uint(CMTPObjectMetaData::EObjectMetaDataUpdate) && IsMediaFormat(aObject.Uint(CMTPObjectMetaData::EFormatCode)))
			{
			if (iUpdateDeltaDataTable)
				{
				iMtpDeltaDataMgr->UpdateDeltaDataTableL(id, CMtpDeltaDataMgr::EModified);
				}
			}
		}
	else
		{
		User::Leave(KErrNotFound);
		}
	}

TBool CMTPObjectStore::ObjectL(const TMTPTypeUint32& aHandle, CMTPObjectMetaData& aBuf) const
	{
	TUint32 handleId = aHandle.Value();
	aBuf.SetUint(CMTPObjectMetaData::EHandle, handleId);
	return GetObjectL(handleId, aBuf);
	}

TBool CMTPObjectStore::ObjectExistsL(const TUint32 aHandle)
	{
	return LocateByHandleL(aHandle, EFalse);
	}


void CMTPObjectStore::BuildObjectMetaDataL(CMTPObjectMetaData& aBuf, const RDbTable& aTable) const
	{
	aBuf.SetUint(CMTPObjectMetaData::EHandle, aTable.ColUint32(EObjectStoreHandleId));

	TFileName suid;
    DbColReadStreamL(aTable, EObjectStoreSUID, suid);
    aBuf.SetDesCL(CMTPObjectMetaData::ESuid, suid);

	aBuf.SetUint(CMTPObjectMetaData::EFormatCode, aTable.ColUint16(EObjectStoreFormatCode));
	aBuf.SetUint(CMTPObjectMetaData::EFormatSubCode, aTable.ColUint16(EObjectStoreFormatSubCode));
	aBuf.SetUint(CMTPObjectMetaData::EDataProviderId, aTable.ColUint8(EObjectStoreDataProviderId));
	aBuf.SetUint(CMTPObjectMetaData::EParentHandle, aTable.ColUint32(EObjectStoreParentHandle));
	aBuf.SetUint(CMTPObjectMetaData::EStorageId, aTable.ColUint32(EObjectStoreStorageId));
	aBuf.SetUint(CMTPObjectMetaData::EModes, aTable.ColUint8(EObjectStoreModes));
	aBuf.SetUint(CMTPObjectMetaData::EIdentifier, aTable.ColInt64(EObjectStorePOUID));
	aBuf.SetUint(CMTPObjectMetaData::ENonConsumable, aTable.ColUint8(EObjectStoreNonConsumable));
	aBuf.SetDesCL(CMTPObjectMetaData::EName, aTable.ColDes(EObjectStoreName));
	}

TBool CMTPObjectStore::ObjectL(const TDesC& aSuid, CMTPObjectMetaData& aBuf) const
	{
	if (LocateBySuidL(aSuid))
		{
		BuildObjectMetaDataL(aBuf, iBatched_SuidHashID);
		return ETrue;
		}
	return EFalse;
	}

const TPtrC CMTPObjectStore::ObjectSuidL(TUint32 aHandle) const
	{
	//iBatched owns the memory of Suid ?
	if (!LocateByHandleL(aHandle))
		{
		User::Leave(KErrNotFound);
		}
    DbColReadStreamL(iBatched, EObjectStoreSUID, iSuidBuf);
	return iSuidBuf;
	}

TMTPTypeUint128 CMTPObjectStore::PuidL(TUint32 aHandle)
	{
	if (!LocateByHandleL(aHandle))
		{
		User::Leave(KErrNotFound);
		}
	TUint64 highHalfPOUID = static_cast<TUint64> (iBatched.ColInt64(EObjectStorePOUID));
	// We actually use the first 64 bits to represent the PUID. this will represent 2^64=16 G xG objects, it is reasonable 
	//to assume the Phone, as a resource-constrained device, never will reach that number, therefore, we choose
	//only 64 bit to represent the POUID Of the objects on phone.
	TMTPTypeUint128 result;
	result.Set(1, highHalfPOUID);	//for the ONB
	return result;
	}

TMTPTypeUint128 CMTPObjectStore::PuidL(const TDesC& aSuid)
	{
	if (!LocateBySuidL(aSuid))
		{
		User::Leave(KErrNotFound);
		}
	TUint64 highHalfPOUID = static_cast<TUint64> (iBatched_SuidHashID.ColInt64(EObjectStorePOUID));
	// We actually use the first 64 bits to represent the PUID. this will represent 2^64=16 G xG objects, it is reasonable 
	//to assume the Phone, as a resource-constrained device, never will reach that number, therefore, we choose
	//only 64 bit to represent the POUID Of the objects on phone.
	TMTPTypeUint128 result;
	result.Set(1, highHalfPOUID);	//for the ONB
	return result;
	}

void CMTPObjectStore::RemoveObjectL(const TMTPTypeUint32& aHandle)
    {
    __FLOG_VA((_L8("RemoveObjectL Entry Handle = 0x%x"), aHandle.Value()));
    if (LocateByHandleL(aHandle.Value()))
        {
        if (iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted &&
                IsMediaFormat(iBatched.ColUint16(EObjectStoreFormatCode)))
            {
            
            iMtpDeltaDataMgr->UpdateDeltaDataTableL(iBatched.ColInt64(EObjectStorePOUID), CMtpDeltaDataMgr::EDeleted);
            }
        iCachedSuidHash = 0;
        iCachedHandle = 0;
        iReferenceMgr->RemoveReferencesL(aHandle.Value());
        iBatched.DeleteL();
        __FLOG(_L8("RemoveObjectL From iBacthed"));
        IncTranOpsNumL();
        }
    __FLOG(_L8("RemoveObjectL Exit"));
    }

void CMTPObjectStore::RemoveObjectL(const TDesC& aSuid)
	{
	if(LocateBySuidL(aSuid))
		{
		if (iSingletons.DpController().EnumerateState() != CMTPDataProviderController::EEnumeratedFulllyCompleted &&
			IsMediaFormat(iBatched_SuidHashID.ColUint16(EObjectStoreFormatCode)))
			{
			iMtpDeltaDataMgr->UpdateDeltaDataTableL(iBatched_SuidHashID.ColInt64(EObjectStorePOUID), CMtpDeltaDataMgr::EDeleted);
			}
		iCachedSuidHash = 0;
		iCachedHandle = 0;
		//no need to call GetL already all it in LocateBySuidL
		iReferenceMgr->RemoveReferencesL(iBatched_SuidHashID.ColUint32(EObjectStoreHandleId));
		iBatched_SuidHashID.DeleteL();
		IncTranOpsNumL();
		}
	}

void CMTPObjectStore::RemoveObjectsL(const CDesCArray& aSuids)
	{
	TUint i(aSuids.Count());
	while (i--)
		{
		RemoveObjectL(aSuids[i]);
		}
	}

void CMTPObjectStore::RemoveObjectsL(TUint aDataProviderId)
	{
	iCachedSuidHash = 0;
	iCachedHandle = 0;
	TMTPObjectMgrQueryParams params(KMTPStorageAll, 0, 0, aDataProviderId);
	TDelAction action(*this, MTraverseAction::EAllDP);
	TraverseL(params, action);
	}

void CMTPObjectStore::RemoveObjectsByStorageIdL(TUint32 aStorageId)
	{
	iCachedSuidHash = 0;
	iCachedHandle = 0;
	TMTPObjectMgrQueryParams params(aStorageId, 0, 0);
	TDelAction action(*this);
	TraverseL(params, action);
	}

void CMTPObjectStore::ReserveObjectHandleL(CMTPObjectMetaData& aObject, TUint64 /*aSpaceRequired*/)
	{
	const TUint dp(aObject.Uint(CMTPObjectMetaData::EDataProviderId));
	const TUint32 id(iHandleAllocator->NextIdL(dp));
	aObject.SetUint(CMTPObjectMetaData::EHandle, id);
	}

void CMTPObjectStore::UnreserveObjectHandleL(const CMTPObjectMetaData& /*aObjectInfo*/)
	{

	}

/**
 Clean unloaded data provider contents from object store
 */
void CMTPObjectStore::CleanL()
	{
	__FLOG(_L8("CleanL - Entry"));
	
	RemoveUndefinedObjectsL();
	Swi::RSisRegistrySession sisSession;
	User::LeaveIfError(sisSession.Connect());
	CleanupClosePushL(sisSession);

	const RArray<TUint>& loadedDPIDs = iPkgIDStore->DPIDL();
	RArray<TUint> unInstalledDpIDs;
	CleanupClosePushL(unInstalledDpIDs);
	for (TUint idx(0); (idx < loadedDPIDs.Count()); ++idx)
		{
		TUid pkgUid = TUid::Uid(iPkgIDStore->PKGIDL(idx));
		if (!sisSession.IsInstalledL(pkgUid))
			{
			//DP is uninstalled, remove DP related data from database.
			TUint thisID = loadedDPIDs[idx];
			__FLOG_1(_L("Data provider[%d] is removed from device!"),thisID);
			unInstalledDpIDs.AppendL(thisID);
			}
		}
	for (TUint index = 0; index < unInstalledDpIDs.Count(); ++index)
		{
		RemoveObjectsL(unInstalledDpIDs[index]);
		iPkgIDStore->RemoveL(unInstalledDpIDs[index]);
		}
	CleanupStack::PopAndDestroy(&unInstalledDpIDs);
	CleanupStack::PopAndDestroy(&sisSession);
	__FLOG(_L8("CleanL - Exit"));
	}

TUint CMTPObjectStore::ObjectOwnerId(const TMTPTypeUint32& aHandle) const
    {
    if (!LocateByHandleL(aHandle.Value()))
        {
        return 0xff;
        }
    return iBatched.ColUint32(EObjectStoreDataProviderId);
    }
/**
 Standard c++ constructor
 */
CMTPObjectStore::CMTPObjectStore()
	{
	}

/**
 Second phase constructor.
 */
void CMTPObjectStore::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	iMaxCommitLimit = KMaxLimitCommitInEnumeration;
	iMaxCompactLimit = KMaxLimitCompactInEnumeration;
	iSingletons.OpenL();
	InitializeDbL();
	iCompactor = CDbCompactor::NewL(&iDatabase);
	iMtpDeltaDataMgr = CMtpDeltaDataMgr::NewL(iDatabase);
	iReferenceMgr = CMTPReferenceMgr::NewL(*this);
	iDPIDStore = CMTPDPIDStore::NewL(iDatabase);
	iPkgIDStore = CMTPPkgIDStore::NewL(iDatabase);
	User::LeaveIfError(iBatched.Open(iDatabase, KSQLHandleTableName, RDbRowSet::EUpdatable));
	User::LeaveIfError(iBatched.SetIndex(KSQLHandleId));
	User::LeaveIfError(iBatched_SuidHashID.Open(iDatabase, KSQLHandleTableName, RDbRowSet::EUpdatable));
	User::LeaveIfError(iBatched_SuidHashID.SetIndex(KSQLSuidHash));
	iHandleAllocator = CMTPHandleAllocator::NewL(*this);
	iSentinal = CEnumertingCacheItem::NewL(0, 0, 0, 0, 0, 0);
	BeginTransactionL();
	}

/**
 Initialises the database, it creates the table and index if the database does not exist, otherwise,
 it open the existing table and index
 */
void CMTPObjectStore::InitializeDbL()
	{
	TFileName fullName;
	GetFullPathName(KMTPNoBackupFolderName, fullName);
	BaflUtils::EnsurePathExistsL(iSingletons.Fs(), fullName);
	fullName.Append(KMTPBackSlash);
	fullName.Append(KMTPHandleObjectDbName);
	TInt err = KErrNone;
	if (!BaflUtils::FileExists(iSingletons.Fs(), fullName))
		{
		CreateDbL(fullName);
		}
	else
		{
		err = OpenDb(fullName);
		if (err==KErrNone && iDatabase.IsDamaged())
			{
			err = iDatabase.Recover();
			}
		if(KErrNone == err)
			{
			err = iDatabase.Compact();
			if(KErrNone != err)
				{//the DB file is corrupt
				BaflUtils::DeleteFile(iSingletons.Fs(), fullName);
				}
			}
		}
	
	if (err != KErrNone)
		{
		CloseDb();
		CreateDbL(fullName);
		}
	}

/**
 Create the database with the specified database name
 @param aFileName    The name of the database to create
 */
void CMTPObjectStore::CreateDbL(const TDesC& aFileName)
	{
	BaflUtils::EnsurePathExistsL(iSingletons.Fs(), aFileName);

	User::LeaveIfError(iDatabase.Replace(iSingletons.Fs(), aFileName, KMTPFormat));
	// Create table and index
	CreateHandleTableL();
	CreateHandleIndexL();
	iUpdateDeltaDataTable = ETrue;
	}

/**
 Open the database with the specified database name
 @param aFileName    The name of the database to open
 */
TInt CMTPObjectStore::OpenDb(const TDesC& aFileName)
	{
	TInt err = iDatabase.Open(iSingletons.Fs(), aFileName, KMTPFormat);
	if(KErrNone == err)
		{
		TRAP(err,
			CreateHandleTableL();
			CreateHandleIndexL();
			)
		}
	iUpdateDeltaDataTable = ETrue;
	return err;
	}

/**
 Close the current opened database
 */
void CMTPObjectStore::CloseDb()
	{
	iDatabase.Close();
	}

/**
 Create the table for storing the mapping from object handle to other properties (data provider id, storage id, formatcode, etc.)
 */
void CMTPObjectStore::CreateHandleTableL()
	{
	if (!DBUtility::IsTableExistsL(iDatabase, KSQLHandleTableName))
		{
		User::LeaveIfError(iDatabase.Execute(KSQLCreateHandleTableText));
		}
	}

/**
 Create three index on the table: 1. Handle, 2. SuidHash, 3. ParentHandle
 */
void CMTPObjectStore::CreateHandleIndexL()
	{
	if (DBUtility::IsTableExistsL(iDatabase, KSQLHandleTableName))
		{
		if (!DBUtility::IsIndexExistsL(iDatabase, KSQLHandleTableName, KSQLHandleId))
			{
			User::LeaveIfError(iDatabase.Execute(KSQLCreateHandleIndexText));
			}

		if (!DBUtility::IsIndexExistsL(iDatabase, KSQLHandleTableName, KSQLSuidHash))
			{
			User::LeaveIfError(iDatabase.Execute(KSQLCreateSuidIndexText));
			}

		if (!DBUtility::IsIndexExistsL(iDatabase, KSQLHandleTableName, KSQLParentHandle))
			{
			User::LeaveIfError(iDatabase.Execute(KSQLCreateParentHandleText));
			}
		}
	else
		{
		User::Leave(KErrNotFound);
		}
	}

/**
 Get the full path of the database 
 @param aFileName    The file name of the database to be retrieved
 */
void CMTPObjectStore::GetFullPathName(const TDesC& aName, TFileName& aFileName) const
	{
	iSingletons.Fs().PrivatePath(aFileName);
	aFileName.Insert(0, KMTPDbDriveLocation);
	aFileName.Append(aName);
	}

/**
 Initialize the handle allocator for this DP with the minimum object ID to use.
 This must not be called while an initiator has an open session or it could cause
 an object ID to be reused.
 @param aDataProviderId Data provider ID whose handle allocator must be initialized
 */
void CMTPObjectStore::RestorePersistentObjectsL(TUint)
	{

	}

TBool CMTPObjectStore::LocateByHandleL(const TUint aHandle, const TBool aReadTable /*default = ETrue*/) const
	{
	__FLOG_VA((_L8("LocateByHandleL - Entry aHandle 0x%x"), aHandle));
	TBool result = EFalse;
	if(IsInvalidHandle(aHandle))
		{
		__FLOG_VA((_L8("LocateByHandleL - Exit result 0x%x"), result));
		return result;
		}
	
	if (iCachedHandle == aHandle)
		{
		__FLOG(_L8("CacheHit"));
		result = ETrue;
		}
	else
		{
		if (iBatched.SeekL(aHandle))
			{
			iCachedHandle = aHandle;
			result = ETrue;
			}
		else
			{
			iCachedHandle = 0;
			}
		}
	if (result && aReadTable)
		{
		iBatched.GetL();
		}
	__FLOG_VA((_L8("LocateByHandleL - Exit result 0x%x"), result));
	return result;
	}

TBool CMTPObjectStore::LocateBySuidL(const TDesC& aSuid) const
	{
	TBool result = EFalse;
	TFileName suid;
	suid.CopyLC(aSuid);
	TUint32 suidHash = DefaultHash::Des16(suid);
	if (iCachedSuidHash == suidHash && iCachedSuidHash != 0) //the hash may generate 0 and we use 0 as a sentinel
		{
		iBatched_SuidHashID.GetL();
	    DbColReadStreamL(iBatched_SuidHashID, EObjectStoreSUID, suid);	    
		if (suid.CompareF(aSuid) == 0 )
			{
			result = ETrue;
			}
		}
	if (!result)
		{
		if (iBatched_SuidHashID.SeekL(static_cast<TUint> (suidHash)))
			{//found, but there might be multiple entries since SUIDhash might possible conflict.
			while (!iBatched_SuidHashID.AtEnd())
				{
				iBatched_SuidHashID.GetL();
		        DbColReadStreamL(iBatched_SuidHashID, EObjectStoreSUID, suid);				
				if (suid.CompareF(aSuid) == 0)
					{
					result = ETrue;
					iCachedSuidHash = suidHash;
					break;
					}
				else if (iBatched_SuidHashID.ColUint32(EObjectStoreSUIDHash) == suidHash)
					{
					iBatched_SuidHashID.NextL();
					}
				else
					{
					iCachedSuidHash = 0;
					break;//Not found
					}
				}
			}
		else
			{
			iCachedSuidHash = 0;
			}
		}
	return result;
	}

/**
 Get an object for the current query
 @param aBuf if found, contains the pointer to the created object info,
 @return ETrue if the object is found, otherwise, EFalse
 */

TBool CMTPObjectStore::GetObjectL(TUint32 aHandle, CMTPObjectMetaData& aObject) const
	{
	if (LocateByHandleL(aHandle))
		{
		BuildObjectMetaDataL(aObject, iBatched);
		return ETrue;
		}
	return EFalse;
	}

/**
 Determine if the object is of WMP supported media format
 @param aObject the object meta data
 @return ETrue if it is of WMP supported media format. EFalse otherwise.
 */
TBool CMTPObjectStore::IsMediaFormat(TUint32 aFormatCode)
	{
	switch (aFormatCode)
		{
		//case EMTPFormatCodeUndefined:
		//case EMTPFormatCodeAssociation:
		case EMTPFormatCodeAIFF:
		case EMTPFormatCodeWAV:
		case EMTPFormatCodeMP3:
		case EMTPFormatCodeAVI:
		case EMTPFormatCodeMPEG:
		case EMTPFormatCodeASF:
		case EMTPFormatCodeEXIFJPEG:
		case EMTPFormatCodeTIFFEP:
		case EMTPFormatCodeFlashPix:
		case EMTPFormatCodeBMP:
		case EMTPFormatCodeCIFF:
		case EMTPFormatCodeGIF:
		case EMTPFormatCodeJFIF:
		case EMTPFormatCodeCD:
		case EMTPFormatCodePICT:
		case EMTPFormatCodePNG:
		case EMTPFormatCodeTIFF:
		case EMTPFormatCodeTIFFIT:
		case EMTPFormatCodeJP2:
		case EMTPFormatCodeJPX:
		case EMTPFormatCodeUndefinedFirmware:
		case EMTPFormatCodeWindowsImageFormat:
		case EMTPFormatCodeUndefinedAudio:
		case EMTPFormatCodeWMA:
		case EMTPFormatCodeOGG:
		case EMTPFormatCodeAAC:
		case EMTPFormatCodeAudible:
		case EMTPFormatCodeWMV:
		case EMTPFormatCodeMP4Container:
		case EMTPFormatCodeMP2:
		case EMTPFormatCode3GPContainer:
		case EMTPFormatCodeAbstractMultimediaAlbum:
		case EMTPFormatCodeAbstractImageAlbum:
		case EMTPFormatCodeAbstractAudioAlbum:
		case EMTPFormatCodeAbstractVideoAlbum:
		case EMTPFormatCodeAbstractAudioVideoPlaylist:
		case EMTPFormatCodeAbstractAudioPlaylist:
		case EMTPFormatCodeAbstractVideoPlaylist:
		case EMTPFormatCodeWPLPlaylist:
		case EMTPFormatCodeM3UPlaylist:
		case EMTPFormatCodeMPLPlaylist:
		case EMTPFormatCodeASXPlaylist:
		case EMTPFormatCodePLSPlaylist:
			return ETrue;

		default:
			return EFalse;
		}
	}

void CMTPObjectStore::CalcFreeHandlesL(TUint aDataProviderId)
	{
	TMTPTypeObjectHandle handleType(0, aDataProviderId);
	TUint32 minHandleForDP = handleType.Value();
	iCachedSuidHash = 0;
	iCachedHandle = 0;
	TUint32 preHandle = minHandleForDP, curHandle = 0;
	if (iBatched.SeekL((TUint) minHandleForDP, RDbTable::ELessThan))
		{
		iBatched.NextL();
		while (!iBatched.AtEnd())
			{
			iBatched.GetL();
			curHandle = iBatched.ColUint32(EObjectStoreHandleId);
			TMTPTypeObjectHandle handleType(curHandle);
			if (handleType.DpId() == aDataProviderId)
				{
				if(++ preHandle < curHandle)//base on the handle is allocated continuously
					{
					if(!iHandleAllocator->AppendHandleBlockL(aDataProviderId, preHandle, curHandle - preHandle))
						{
						break;
						}
					preHandle = curHandle;
					}
				iBatched.NextL();
				}
			else
				{
				//Has gone over the current DPid, break;
				break;
				}
			}
		}
	}



void CMTPObjectStore::EstablishDBSnapshotL(TUint32 aStorageId)
    {
    //Currently, i only do this for File DP since it is non-persistent, later, if we take the proposal that 
    //1. FileDP is the last DP to be enumerated.
    //2. FileDP will san the whole file system, and will try to enumerate all of the objects(might on behalf of another DP) if the objects is still not
    // in the object store after all other DP finish its enumeration.
    //3. Then notify the related DP about the newly added objects by notification API;
    __FLOG(_L8("EstablishDBSnapshotL - Entry"));
    RDbTable temp;
    CleanupClosePushL(temp);
    User::LeaveIfError(temp.Open(iDatabase, KSQLHandleTableName, RDbRowSet::EUpdatable));
    if(!iCacheExist)
        {
        TInt32 count = temp.CountL(RDbRowSet::EQuick);
        iEnumeratingCacheObjList.ReserveL(count);
        }
    temp.FirstL();

    while (!temp.AtEnd())
        {
        temp.GetL();
        if (temp.ColUint8(EObjectStoreDPFlag) == 1 
                && (KMTPStorageAll == aStorageId || temp.ColUint32(EObjectStoreStorageId) == aStorageId))
            {
            TUint32 handleID = temp.ColUint32(EObjectStoreHandleId);
            TUint32 parentID = temp.ColUint32(EObjectStoreParentHandle);
            TInt64 pUID = temp.ColInt64(EObjectStorePOUID);
            iHandleAllocator->SetIdL(handleID, pUID);
            CEnumertingCacheItem* item = CEnumertingCacheItem::NewLC(
                    temp.ColUint32(EObjectStoreSUIDHash), handleID, parentID,
                    temp.ColUint16(EObjectStoreFormatCode), pUID, temp.ColUint8(EObjectStoreDataProviderId));
            TInt result = iEnumeratingCacheObjList.InsertInOrder(item, TLinearOrder<CEnumertingCacheItem>(CEnumertingCacheItem::Compare));
            if (KErrAlreadyExists == result) //hash collision
                {
                TInt found = iEnumeratingCacheObjList.FindInOrder(item, TLinearOrder<CEnumertingCacheItem>(CEnumertingCacheItem::Compare));
                CEnumertingCacheItem* colliItem = iEnumeratingCacheObjList[found];
                TFileName suid;
                if (colliItem->iSuid == NULL)
                    {
                    if (LocateByHandleL(colliItem->iObjHandleId))
                        {
                        DbColReadStreamL(iBatched, EObjectStoreSUID, suid);
                        colliItem->iSuid = suid.AllocL();
                        colliItem->iSuidPtr.Set(*colliItem->iSuid);
                        }
                    }

                DbColReadStreamL(temp, EObjectStoreSUID, suid);
                
                item->iSuid = suid.AllocL();
                
                item->iSuidPtr.Set(*item->iSuid);
                result = iEnumeratingCacheObjList.InsertInOrder(item, TLinearOrder<CEnumertingCacheItem>(CEnumertingCacheItem::Compare));
                }
            
            if(result != KErrAlreadyExists)
                {
                User::LeaveIfError(result);
                CleanupStack::Pop(item);
                }
            else
                {
                CleanupStack::PopAndDestroy(item);
                }

            }
        temp.NextL();
        }

    CleanupStack::PopAndDestroy(&temp);
    iCacheExist = ETrue;
    __FLOG_VA((_L8("EstablishDBSnapshotL - Exit build %d items"), iEnumeratingCacheObjList.Count()));
    }
/*
 * All Objects enumeration complete
 */
void CMTPObjectStore::ObjectsEnumComplete()
    {
    if(iCacheExist)
        {
        iCacheExist = EFalse;
        }
    iNonPersistentDPList.Close();
    iEnumeratingCacheObjList.ResetAndDestroy();
    iUpdateDeltaDataTable = ETrue;
    iMaxCommitLimit = KMaxLimitCommitAfterEnumeration;
    iMaxCompactLimit = KMaxLimitCompactAfterEnumeration;
    CommitTransactionL();
    User::LeaveIfError(iDatabase.Compact());
    BeginTransactionL();
    }


void CMTPObjectStore::CleanDBSnapshotL(TBool aOnlyRoot/* = EFalse*/)
    {
    //For those items left in the iEnumeratingCacheObjList, remove the object entry if the DPID of the handle is not persistent. and populate the 
    //roundtrip table if needed.
    //and then close the iEnumeratingCacheObjList to release the memory.
    //_LIT(KInsert, "CMTPObjectStore::CleanDBSnapshot");
    //volatile TTimer t(KInsert);
    __FLOG(_L8("CleanDBSnapshotL Entry"));
    if (iSnapshotWorker == NULL)
        {
        iSnapshotCleanPos = iEnumeratingCacheObjList.Count() - 1;
        iSnapshotWorker = CSnapshotWorker::NewL(this, aOnlyRoot);
        }
    
    //for (TInt i = iEnumeratingCacheObjList.Count() - 1; i >= 0; i--)
    for (TInt i = 0; iSnapshotCleanPos >= 0 && i <= KSnapshotGranularity; --iSnapshotCleanPos, ++i)
        {
        if((aOnlyRoot && iEnumeratingCacheObjList[iSnapshotCleanPos]->iObjParentId == KMTPHandleNoParent) //only root 
           ||(!aOnlyRoot)) //everything
            {
            TInt rc = iNonPersistentDPList.FindInOrder(iEnumeratingCacheObjList[iSnapshotCleanPos]->iDpID);
            if (rc != KErrNotFound)
                {//This is a non persistent DP.
                __FLOG_VA((_L8("Remove Object 0x%x"), iEnumeratingCacheObjList[iSnapshotCleanPos]->iObjHandleId));
                RemoveObjectL(iEnumeratingCacheObjList[iSnapshotCleanPos]->iObjHandleId);
                }
            }
        }
    
    if (iSnapshotCleanPos >= 0)
        {
        iSnapshotWorker->ActiveSelf();
        }
    else
        {
        //TRequestStatus *status = &aStatus;
        //User::RequestComplete(status, KErrNone);
        iSingletons.DpController().Schedule();
        iSnapshotCleanPos = 0;
        delete iSnapshotWorker;
        iSnapshotWorker = NULL;
        }
    
    __FLOG(_L8("CleanDBSnapshotL Exit"));
    }

void CMTPObjectStore::RemoveUndefinedObjectsL()
    {
    __FLOG(_L8("CompactDBSnapshotL Entry"));
    
    if (iCleanUndefined)
        {
        return;
        }
    
    TInt32 count = 0;
    RDbTable temp;
    CleanupClosePushL(temp);
    User::LeaveIfError(temp.Open(iDatabase, KSQLHandleTableName, RDbRowSet::EUpdatable));
    count = temp.CountL(RDbRowSet::EQuick);

    __FLOG_VA((_L8("Count before deletion %d "), count));
    CleanupStack::PopAndDestroy(&temp);
    
    if (count > KMaxLimitSnapshotSize)
        {
        // Delete all object with undefined format
        _LIT(KSQLDeleteObjectText, "DELETE FROM HandleStore WHERE FormatCode = %u");
        iSqlStatement.Format(KSQLDeleteObjectText, EMTPFormatCodeUndefined);
        User::LeaveIfError(iDatabase.Execute(iSqlStatement));    
        }
    
    iCleanUndefined = ETrue;

    
    __FLOG(_L8("CompactDBSnapshotL Exit"));    
    }


CMTPObjectStore::CEnumertingCacheItem::CEnumertingCacheItem(TUint32 aSuidHash, TUint32 aHandle, TUint32 aParent, TUint32 aFormat, TUint64 aId, TUint8 aDpID)
	{
	iObjSuiIdHash = aSuidHash;
	iObjHandleId = aHandle;
	iObjParentId = aParent;
	iFormatcode = aFormat;
	iPOUID = aId;
	iDpID = aDpID;
	}

TInt CMTPObjectStore::CEnumertingCacheItem::Compare(const CEnumertingCacheItem& aFirst, const CEnumertingCacheItem& aSecond)
	{
	if (aFirst.iObjSuiIdHash > aSecond.iObjSuiIdHash)
		{
		return 1;
		}
	else if (aFirst.iObjSuiIdHash < aSecond.iObjSuiIdHash)
		{
		return -1;
		}
	if ((aFirst.iSuidPtr.Length() != 0) && (aSecond.iSuidPtr.Length() != 0))
		{
		return aFirst.iSuidPtr.CompareF(aSecond.iSuidPtr);
		}
	return 0;
	}

TBool CMTPObjectStore::IsInvalidHandle( TUint32 aHandle ) const
	{
	return ( (KMTPHandleAll == aHandle) || (KMTPHandleNone == aHandle) );
	}

void CMTPObjectStore::DbColWriteStreamL(RDbTable& aTable, TDbColNo aCol, const TDesC16& aDes)
    {
    RDbColWriteStream suid;
    suid.OpenLC(aTable, aCol);
    suid.WriteL(aDes);
    suid.Close();
    CleanupStack::PopAndDestroy(); // suid
    }

void CMTPObjectStore::DbColReadStreamL(const RDbTable& aTable, TDbColNo aCol, TDes16& aDes) const
    {
    RDbColReadStream suid;
    suid.OpenLC(aTable, aCol);
    suid.ReadL(aDes, aTable.ColLength(aCol));
    suid.Close();
    CleanupStack::PopAndDestroy(); // suid
    }
