// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include "cmtpdeltadatamgr.h"
//! Size of a PUID in bytes
static const TInt KMTPPuidSize = 16;


__FLOG_STMT(_LIT8(KComponent,"MTPDeltaDataMgr:");)

_LIT(KMTPDeltaDataTable, "MTPDeltaDataTable");
_LIT(KSQLPuidIndexName, "PuidIndex");
_LIT(KSQLIdentifierIndexName, "IdentifierIndex");
_LIT(KAnchorIdTable, "AnchorIdTable");
_LIT (KDeleteDeltaTable, "DELETE FROM MTPDeltaDataTable");
/**
Standard c++ constructor
*/	
CMtpDeltaDataMgr::CMtpDeltaDataMgr(RDbDatabase& aDatabase)
	:iDatabase(aDatabase)
 	{
 	}

 
/**
Second-phase construction
@leave One of the system wide error codes, if a processing failure occurs.
*/	
void CMtpDeltaDataMgr::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	__FLOG(_L8("ConstructL - Exit"));
	}


/**
Two-phase construction
@param aDatabase	The reference to the database object
@return	pointer to the created CMtpDeltaDataMgr instance
*/
CMtpDeltaDataMgr* CMtpDeltaDataMgr::NewL(RDbDatabase& aDatabase)
	{
	CMtpDeltaDataMgr* self = new (ELeave) CMtpDeltaDataMgr(aDatabase);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


/**
Destructor
*/	
CMtpDeltaDataMgr::~CMtpDeltaDataMgr()
	{
	iDeltaTableBatched.Close();
	iAnchorTableBatched.Close();
	iView.Close();
	iSuidIdArray.Close();
	__FLOG_CLOSE;
	}
/**
Create the MTP Delta Data Table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::CreateDeltaDataTableL()
	{
	__FLOG(_L8("CreateDeltaDataTableL - Entry"));
	
	iDeltaTableBatched.Close();
	if(!DBUtility::IsTableExistsL(iDatabase, KMTPDeltaDataTable))
		{
		_LIT(KSQLCreateMTPDeltaDataTableText,"CREATE TABLE MTPDeltaDataTable (SuidId BIGINT , OpCode TINYINT )");		
		User::LeaveIfError(iDatabase.Execute(KSQLCreateMTPDeltaDataTableText));
		
		if(!DBUtility::IsIndexExistsL(iDatabase, KMTPDeltaDataTable, KSQLPuidIndexName))
			{
			_LIT(KSQLCreateReferenceIndexText,"CREATE UNIQUE INDEX PuidIndex on MTPDeltaDataTable (SuidId)");
			User::LeaveIfError(iDatabase.Execute(KSQLCreateReferenceIndexText));
			}
		}
	iDeltaTableBatched.Open(iDatabase, KMTPDeltaDataTable, RDbRowSet::EUpdatable);
		
	__FLOG(_L8("CreateDeltaDataTableL - Exit"));
	}

/**
Create the Anchor Id Table anchor Id will be stored here
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::CreateAnchorIdTableL()
	{
	__FLOG(_L8("CreateAnchorIdTableL - Entry"));
	iAnchorTableBatched.Close();
	if(!DBUtility::IsTableExistsL(iDatabase, KAnchorIdTable))
		{
		_LIT(KSQLCreateAnchorIdTableText,"CREATE TABLE AnchorIdTable (anchorid INTEGER, curindex INTEGER, identifier INTEGER)");
		User::LeaveIfError(iDatabase.Execute(KSQLCreateAnchorIdTableText));
			
		if(!DBUtility::IsIndexExistsL(iDatabase, KAnchorIdTable, KSQLIdentifierIndexName))
			{
			_LIT(KSQLCreateRefIndexText,"CREATE UNIQUE INDEX IdentifierIndex on AnchorIdTable (identifier)");
			User::LeaveIfError(iDatabase.Execute(KSQLCreateRefIndexText));
			}
		}
	iAnchorTableBatched.Open(iDatabase, KAnchorIdTable, RDbRowSet::EUpdatable);
		
	__FLOG(_L8("CreateAnchorIdTableL - Exit"));
	}

/**
Add a new anchor ID to the AnchorIdTable
@param aAnchorId The anchor ID
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::InsertAnchorIdL(TInt aAnchorId, TInt aIdentifier)
	{
	__FLOG(_L8("InsertAnchorIdL - Entry"));	
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(!(iAnchorTableBatched.SeekL(aIdentifier)))
		{
		iAnchorTableBatched.InsertL();
		iAnchorTableBatched.SetColL(1, aAnchorId);
		iAnchorTableBatched.SetColL(2, 0);
		iAnchorTableBatched.SetColL(3, aIdentifier);
		iAnchorTableBatched.PutL();
		}
	__FLOG(_L8("InsertAnchorIdL - Exit"));
	}

/**
Overwrite the anchor Id with new one
@param aAnchorId The new anchor ID
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::UpdateAnchorIdL(TInt aAnchorId, TInt aIdentifier)
	{
	__FLOG(_L8("UpdateAnchorIdL - Entry"));
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.UpdateL();
		iAnchorTableBatched.SetColL(1, aAnchorId);
		iAnchorTableBatched.PutL();
		}
	__FLOG(_L8("UpdateAnchorIdL - Exit"));
	}

/**
Get the anchor ID with specified identifier
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C TInt CMtpDeltaDataMgr::GetAnchorIdL(TInt aIdentifier)
	{
	__FLOG(_L8("GetAnchorIdL - Entry"));
	TInt anchorId = 0;
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.GetL();
		anchorId = iAnchorTableBatched.ColInt32(1);
		}
	__FLOG(_L8("GetAnchorIdL - Exit"));
	return anchorId;
	}

/**
Overwrite the old index  with new one
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::UpdatePersistentIndexL(TInt aCurindex, TInt aIdentifier)
	{	
	__FLOG(_L8("UpdatePersistentIndexL - Entry"));
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.UpdateL();
		iAnchorTableBatched.SetColL(2, aCurindex);
		iAnchorTableBatched.PutL();
		}
	__FLOG(_L8("UpdatePersistentIndexL - Exit"));
	}
	
/**
returns the stored index 
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetPersistentIndexL(TInt aIdentifier)
	{
	__FLOG(_L8("GetPersistentIndexL - Entry"));

	TInt currIndex = 0;
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.GetL();
		currIndex = iAnchorTableBatched.ColInt32(2);
		}
	__FLOG(_L8("GetPersistentIndexL - Exit"));		
	return currIndex;
	}

/**
Add the Opcode and SuidId to the MTPDeltaDataTable
@param aSuidId The suid identifier of the object to be added
@param aOpCode operation code 
@leave  One of the system wide error codes, if a processing failure occurs.
*/
void CMtpDeltaDataMgr::UpdateDeltaDataTableL(TInt64 aSuidId, TOpCode aOpCode)
	{
	__FLOG(_L8("UpdateDeltaDataTableL - Entry"));
	if(!DBUtility::IsTableExistsL(iDatabase, KMTPDeltaDataTable))
		return;
		
	iDeltaTableBatched.SetIndex(KSQLPuidIndexName);
	if(iDeltaTableBatched.SeekL(aSuidId))
		{
		iDeltaTableBatched.UpdateL();
		iDeltaTableBatched.SetColL(2, aOpCode);
		}
	else
		{
		iDeltaTableBatched.InsertL();
		iDeltaTableBatched.SetColL(1, aSuidId);
		iDeltaTableBatched.SetColL(2, aOpCode);
		}
	iDeltaTableBatched.PutL();
	__FLOG(_L8("UpdateDeltaDataTableL - Exit"));
	}

/**
@param total number of items to be  filled into aModifiedPuidIdArray and aDeletedPuidArray
@param the start position 
@param reference to modifed and deleted mtp arrays 
@return Number of remaining items to be retrieved from table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetChangedPuidsL(TInt aMaxArraySize, TInt& aPosition, CMTPTypeArray& aModifiedPuidIdArray, CMTPTypeArray& aDeletedPuidArray)
	{
	__FLOG(_L8("GetChangedPuidsL - Entry"));
	
	if(!iNeedToSendMore)
		{
		_LIT(KSQLGetAll, "SELECT * FROM MTPDeltaDataTable");
		
		User::LeaveIfError(iView.Prepare(iDatabase, TDbQuery(KSQLGetAll)));
		User::LeaveIfError(iView.EvaluateAll());
		iNeedToSendMore = ETrue;
		iView.FirstL();
		iTotalRows = iView.CountL();

		if(aPosition !=0 && aPosition < iTotalRows)
			{
			for(TInt i=0; i<aPosition; i++)
				{
				iView.NextL();
				}
			}
		}
		
	if(iTotalRows == 0 || aPosition >= iTotalRows)
		{
		iNeedToSendMore = EFalse;
		iView.Close();
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize;count++)
		{
		iView.GetL();
		//Get the data from the current row
		suidId = iView.ColInt64(1);
		TInt8 opCode = iView.ColInt8(2);	
		puidBuffer.Copy(TPtrC8((const TUint8*)&suidId, sizeof(TInt64)));
		puidBuffer.Append(TPtrC8((const TUint8*)&puidlow, sizeof(TInt64)));
		TMTPTypeUint128 puid(puidBuffer);

		if(opCode  == EDeleted)
			{
			aDeletedPuidArray.AppendL(puid);
			}
		else
			{
			aModifiedPuidIdArray.AppendL(puid);
			}
		aPosition++;

		if(aPosition == iTotalRows)
			{
			iNeedToSendMore = EFalse;
			iView.Close();
			break;	
			}
		else
			{
			//Move to the next row
			iView.NextL();
			}
		}
	
	__FLOG(_L8("GetChangedPuidsL - Exit"));
	return	(iTotalRows - aPosition);
	}

/**
@param total number of items to be  filled into aAddedPuidIdArray
@param reference to added mtp arrays 
@return Number of remaining items to be retrieved from table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetAddedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aAddedPuidIdArray)
	{
	__FLOG(_L8("GetAddedPuidsL - Entry"));
	
	if(!iNeedToSendMore)
		{
		TInt opcode = EAdded;
		_LIT(KSQLSelectAdded, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectAdded, opcode);
		
		User::LeaveIfError(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)));
		User::LeaveIfError(iView.EvaluateAll());
		iNeedToSendMore = ETrue;
		iView.FirstL();
		iTotalRows = iView.CountL();

		if(aPosition !=0 && aPosition < iTotalRows)
			{
			for(TInt i=0; i<aPosition; i++)
				{
				iView.NextL();
				}
			}
		}
		
	if(iTotalRows == 0 || aPosition >= iTotalRows)
		{
		iNeedToSendMore = EFalse;
		iView.Close();
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize;count++)
		{

		iView.GetL();
		//Get the data from the current row
		suidId = iView.ColInt64(1);
		puidBuffer.Copy(TPtrC8((const TUint8*)&suidId, sizeof(TInt64)));
		puidBuffer.Append(TPtrC8((const TUint8*)&puidlow, sizeof(TInt64)));
		TMTPTypeUint128 puid(puidBuffer);

		aAddedPuidIdArray.AppendL(puid);
		aPosition++;

		if(aPosition == iTotalRows)
			{
			iNeedToSendMore = EFalse;
			iView.Close();
			break;	
			}
		else
			{
			//Move to the next row
			iView.NextL();		
			}
		}
	
	__FLOG(_L8("GetAddedPuidsL - Exit"));
	return 	(iTotalRows - aPosition);
	}

/**
@param total number of items to be  filled into aAddedPuidIdArray
@param reference to deleted mtp arrays 
@return Number of remaining items to be retrieved from table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetDeletedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aDeletedPuidIdArray)
	{
	__FLOG(_L8("GetDeletedPuidsL - Entry"));

	if(!iNeedToSendMore)
		{
		TInt opcode = EDeleted;
		_LIT(KSQLSelectDeleted, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectDeleted, opcode);
		
		User::LeaveIfError(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)));
		User::LeaveIfError(iView.EvaluateAll());
		iNeedToSendMore = ETrue;
		iView.FirstL();
		iTotalRows = iView.CountL();

		if(aPosition !=0 && aPosition < iTotalRows)
			{
			for(TInt i=0; i<aPosition; i++)
				{
				iView.NextL();
				}
			}
		}
		
	if(iTotalRows == 0 || aPosition >= iTotalRows)
		{
		iNeedToSendMore = EFalse;
		iView.Close();
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize;count++)
		{

		iView.GetL();
		//Get the data from the current row
		suidId = iView.ColInt64(1);
		puidBuffer.Copy(TPtrC8((const TUint8*)&suidId, sizeof(TInt64)));
		puidBuffer.Append(TPtrC8((const TUint8*)&puidlow, sizeof(TInt64)));
		TMTPTypeUint128 puid(puidBuffer);

		aDeletedPuidIdArray.AppendL(puid);
		aPosition++;

		if(aPosition == iTotalRows)
			{
			iNeedToSendMore = EFalse;
			iView.Close();
			break;	
			}
		else
			{
			//Move to the next row
			iView.NextL();		
			}
		}
		
	__FLOG(_L8("GetDeletedPuidsL - Exit"));
	return 	(iTotalRows - aPosition);
	}

/**
@param total number of items to be  filled into aAddedPuidIdArray
@param reference to Modified mtp arrays 
@return Number of remaining items to be retrieved from table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetModifiedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aModifiedPuidIdArray)
	{
	__FLOG(_L8("GetDeletedPuidsL - Entry"));

	if(!iNeedToSendMore)
		{
		TInt opcode = EModified;
		_LIT(KSQLSelectModified, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectModified, opcode);
		
		User::LeaveIfError(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)));
		User::LeaveIfError(iView.EvaluateAll());
		iNeedToSendMore = ETrue;
		iView.FirstL();
		iTotalRows = iView.CountL();

		if(aPosition !=0 && aPosition < iTotalRows)
			{
			for(TInt i=0; i<aPosition; i++)
				{
				iView.NextL();
				}
			}
		}
		
	if(iTotalRows == 0 || aPosition >= iTotalRows)
		{
		iNeedToSendMore = EFalse;
		iView.Close();
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize;count++)
		{
		iView.GetL();
		//Get the data from the current row
		suidId = iView.ColInt64(1);		
		
		puidBuffer.Copy(TPtrC8((const TUint8*)&suidId, sizeof(TInt64)));
		puidBuffer.Append(TPtrC8((const TUint8*)&puidlow, sizeof(TInt64)));
		TMTPTypeUint128 puid(puidBuffer);

		aModifiedPuidIdArray.AppendL(puid);
		aPosition++;

		if(aPosition == iTotalRows)
			{
			iNeedToSendMore = EFalse;
			iView.Close();
			break;	
			}
		else
			{
			//Move to the next row
			iView.NextL();		
			}
		}
		
	__FLOG(_L8("GetDeletedPuidsL - Exit"));
	return 	(iTotalRows - aPosition);
	}

/**
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::ResetMTPDeltaDataTableL()
	{
	__FLOG(_L8("ResetMTPDeltaDataTableL - Entry"));

	iView.Close();
	iNeedToSendMore = EFalse;
	User::LeaveIfError(iDatabase.Execute(KDeleteDeltaTable));
	
	__FLOG(_L8("ResetMTPDeltaDataTableL - Exit"));
	}
