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
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdeltadatamgrTraces.h"
#endif

//! Size of a PUID in bytes
static const TInt KMTPPuidSize = 16;



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
    OstTraceFunctionEntry0( CMTPDELTADATAMGR_CONSTRUCTL_ENTRY );
	OstTraceFunctionExit0( CMTPDELTADATAMGR_CONSTRUCTL_EXIT );
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
	}
/**
Create the MTP Delta Data Table
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::CreateDeltaDataTableL()
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_CREATEDELTADATATABLEL_ENTRY );
	
	
	iDeltaTableBatched.Close();
	if(!DBUtility::IsTableExistsL(iDatabase, KMTPDeltaDataTable))
		{
		_LIT(KSQLCreateMTPDeltaDataTableText,"CREATE TABLE MTPDeltaDataTable (SuidId BIGINT , OpCode TINYINT )");		
		LEAVEIFERROR(iDatabase.Execute(KSQLCreateMTPDeltaDataTableText),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_CREATEDELTADATATABLEL, "MTPDeltaDataTable create error!" ));

		if(!DBUtility::IsIndexExistsL(iDatabase, KMTPDeltaDataTable, KSQLPuidIndexName))
			{
			_LIT(KSQLCreateReferenceIndexText,"CREATE UNIQUE INDEX PuidIndex on MTPDeltaDataTable (SuidId)");
			LEAVEIFERROR(iDatabase.Execute(KSQLCreateReferenceIndexText),
			        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_CREATEDELTADATATABLEL, "PuidIndex on MTPDeltaDataTable create error!" ));       
			}
		}
	LEAVEIFERROR(iDeltaTableBatched.Open(iDatabase, KMTPDeltaDataTable, RDbRowSet::EUpdatable),
			        OstTrace0( TRACE_ERROR, DUP2_CMTPDELTADATAMGR_CREATEDELTADATATABLEL, "MTPDeltaDataTable open error!" ));       
		
	OstTraceFunctionExit0( CMTPDELTADATAMGR_CREATEDELTADATATABLEL_EXIT );
	}

/**
Create the Anchor Id Table anchor Id will be stored here
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::CreateAnchorIdTableL()
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_CREATEANCHORIDTABLEL_ENTRY );
	
	iAnchorTableBatched.Close();
	if(!DBUtility::IsTableExistsL(iDatabase, KAnchorIdTable))
		{
		_LIT(KSQLCreateAnchorIdTableText,"CREATE TABLE AnchorIdTable (anchorid INTEGER, curindex INTEGER, identifier INTEGER)");
		LEAVEIFERROR(iDatabase.Execute(KSQLCreateAnchorIdTableText),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_CREATEANCHORIDTABLEL, "TABLE AnchorIdTable create error!" ));
		        
			
		if(!DBUtility::IsIndexExistsL(iDatabase, KAnchorIdTable, KSQLIdentifierIndexName))
			{
			_LIT(KSQLCreateRefIndexText,"CREATE UNIQUE INDEX IdentifierIndex on AnchorIdTable (identifier)");
			LEAVEIFERROR(iDatabase.Execute(KSQLCreateRefIndexText),
			        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_CREATEANCHORIDTABLEL, "INDEX IdentifierIndex on AnchorIdTable create error!" ));
			}
		}
	LEAVEIFERROR(iAnchorTableBatched.Open(iDatabase, KAnchorIdTable, RDbRowSet::EUpdatable),
		OstTrace0( TRACE_ERROR, DUP2_CMTPDELTADATAMGR_CREATEANCHORIDTABLEL, "AnchorIdTable open error!" ));       
		
	OstTraceFunctionExit0( CMTPDELTADATAMGR_CREATEANCHORIDTABLEL_EXIT );
	}

/**
Add a new anchor ID to the AnchorIdTable
@param aAnchorId The anchor ID
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::InsertAnchorIdL(TInt aAnchorId, TInt aIdentifier)
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_INSERTANCHORIDL_ENTRY );
	
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(!(iAnchorTableBatched.SeekL(aIdentifier)))
		{
		iAnchorTableBatched.InsertL();
		iAnchorTableBatched.SetColL(1, aAnchorId);
		iAnchorTableBatched.SetColL(2, 0);
		iAnchorTableBatched.SetColL(3, aIdentifier);
		iAnchorTableBatched.PutL();
		}
	OstTraceFunctionExit0( CMTPDELTADATAMGR_INSERTANCHORIDL_EXIT );
	}

/**
Overwrite the anchor Id with new one
@param aAnchorId The new anchor ID
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::UpdateAnchorIdL(TInt aAnchorId, TInt aIdentifier)
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_UPDATEANCHORIDL_ENTRY );
	
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.UpdateL();
		iAnchorTableBatched.SetColL(1, aAnchorId);
		iAnchorTableBatched.PutL();
		}
	OstTraceFunctionExit0( CMTPDELTADATAMGR_UPDATEANCHORIDL_EXIT );
	}

/**
Get the anchor ID with specified identifier
@param aIdentifier The identifier of the anchor
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C TInt CMtpDeltaDataMgr::GetAnchorIdL(TInt aIdentifier)
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETANCHORIDL_ENTRY );
	
	TInt anchorId = 0;
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.GetL();
		anchorId = iAnchorTableBatched.ColInt32(1);
		}
	OstTraceFunctionExit0( CMTPDELTADATAMGR_GETANCHORIDL_EXIT );
	return anchorId;
	}

/**
Overwrite the old index  with new one
@leave One of the system wide error codes, if a processing failure occurs.
*/	
EXPORT_C void CMtpDeltaDataMgr::UpdatePersistentIndexL(TInt aCurindex, TInt aIdentifier)
	{	
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_UPDATEPERSISTENTINDEXL_ENTRY );
	
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.UpdateL();
		iAnchorTableBatched.SetColL(2, aCurindex);
		iAnchorTableBatched.PutL();
		}
	OstTraceFunctionExit0( CMTPDELTADATAMGR_UPDATEPERSISTENTINDEXL_EXIT );
	}
	
/**
returns the stored index 
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C TInt CMtpDeltaDataMgr::GetPersistentIndexL(TInt aIdentifier)
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETPERSISTENTINDEXL_ENTRY );
	

	TInt currIndex = 0;
	iAnchorTableBatched.SetIndex(KSQLIdentifierIndexName);
	if(iAnchorTableBatched.SeekL(aIdentifier))
		{
		iAnchorTableBatched.GetL();
		currIndex = iAnchorTableBatched.ColInt32(2);
		}	
	OstTraceFunctionExit0( CMTPDELTADATAMGR_GETPERSISTENTINDEXL_EXIT );
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
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_UPDATEDELTADATATABLEL_ENTRY );
	
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
	OstTraceFunctionExit0( CMTPDELTADATAMGR_UPDATEDELTADATATABLEL_EXIT );
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
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETCHANGEDPUIDSL_ENTRY );
	
	
	if(!iNeedToSendMore)
		{
		_LIT(KSQLGetAll, "SELECT * FROM MTPDeltaDataTable");
		
		LEAVEIFERROR(iView.Prepare(iDatabase, TDbQuery(KSQLGetAll)),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_GETCHANGEDPUIDSL, "view for MTPDeltaDataTable prepare failure!" ));
		LEAVEIFERROR(iView.EvaluateAll(),
		        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_GETCHANGEDPUIDSL, "view evaluate failed!" ));    
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
		OstTraceFunctionExit0( CMTPDELTADATAMGR_GETCHANGEDPUIDSL_EXIT );
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize && iView.AtRow();count++)
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
	
	OstTraceFunctionExit0( DUP1_CMTPDELTADATAMGR_GETCHANGEDPUIDSL_EXIT );
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
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETADDEDPUIDSL_ENTRY );
	
	if(!iNeedToSendMore)
		{
		TInt opcode = EAdded;
		_LIT(KSQLSelectAdded, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectAdded, opcode);
		
		LEAVEIFERROR(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_GETADDEDPUIDSL, "view for MTPDeltaDataTable prepare failed!" ));	        
		LEAVEIFERROR(iView.EvaluateAll(),
		        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_GETADDEDPUIDSL, "view evaluate failed!" ));
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
		OstTraceFunctionExit0( CMTPDELTADATAMGR_GETADDEDPUIDSL_EXIT );
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize && iView.AtRow();count++)
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
	
	OstTraceFunctionExit0( DUP1_CMTPDELTADATAMGR_GETADDEDPUIDSL_EXIT );
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
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETDELETEDPUIDSL_ENTRY );

	if(!iNeedToSendMore)
		{
		TInt opcode = EDeleted;
		_LIT(KSQLSelectDeleted, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectDeleted, opcode);
		
		LEAVEIFERROR(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_GETDELETEDPUIDSL, "view for MTPDeltaDataTable prepare failed!" ));
		LEAVEIFERROR(iView.EvaluateAll(),
		        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_GETDELETEDPUIDSL, "view evaluated failed!" ));
		        
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
		OstTraceFunctionExit0( CMTPDELTADATAMGR_GETDELETEDPUIDSL_EXIT );
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize && iView.AtRow();count++)
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
		
	OstTraceFunctionExit0( DUP1_CMTPDELTADATAMGR_GETDELETEDPUIDSL_EXIT );
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
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_GETMODIFIEDPUIDSL_ENTRY );

	if(!iNeedToSendMore)
		{
		TInt opcode = EModified;
		_LIT(KSQLSelectModified, "SELECT * FROM MTPDeltaDataTable WHERE OpCode = %d");
		iSqlStatement.Format(KSQLSelectModified, opcode);
		
		LEAVEIFERROR(iView.Prepare(iDatabase, TDbQuery(iSqlStatement)),
		        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_GETMODIFIEDPUIDSL, "view for MTPDeltaDataTable prepare failed!" ));
		LEAVEIFERROR(iView.EvaluateAll(),
		        OstTrace0( TRACE_ERROR, DUP1_CMTPDELTADATAMGR_GETMODIFIEDPUIDSL, "view evaluate failed!" ));
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
		OstTraceFunctionExit0( CMTPDELTADATAMGR_GETMODIFIEDPUIDSL_EXIT );
		return 0;
		}
	
	TInt64 suidId = 0;
	TInt64 puidlow = 1;
	TBuf8<KMTPPuidSize> puidBuffer;
	
	for(TInt count=0;count <aMaxArraySize && iView.AtRow();count++)
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
		
	OstTraceFunctionExit0( DUP1_CMTPDELTADATAMGR_GETMODIFIEDPUIDSL_EXIT );
	return 	(iTotalRows - aPosition);
	}

/**
@leave One of the system wide error codes, if a processing failure occurs.
*/
EXPORT_C void CMtpDeltaDataMgr::ResetMTPDeltaDataTableL()
	{
	OstTraceFunctionEntry0( CMTPDELTADATAMGR_RESETMTPDELTADATATABLEL_ENTRY );

	iView.Close();
	iNeedToSendMore = EFalse;
	LEAVEIFERROR(iDatabase.Execute(KDeleteDeltaTable),
	        OstTrace0( TRACE_ERROR, CMTPDELTADATAMGR_RESETMTPDELTADATATABLEL, "DELETE FROM MTPDeltaDataTable failed!" ));
	
	OstTraceFunctionExit0( CMTPDELTADATAMGR_RESETMTPDELTADATATABLEL_EXIT );
	}
