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



#include "cmtpdpidstore.h"
#include "dbutility.h"
#include "mtpdebug.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpdpidstoreTraces.h"
#endif



_LIT(KSQLDPIDTableName, "DPIDStore");
_LIT(KSQLInsertDPIDObjectText, "INSERT INTO DPIDStore (DataProviderId, Uid) VALUES (%u, %u)");
_LIT(KSQLCreateDPIDTableText, "CREATE TABLE DPIDStore (DataProviderId UNSIGNED INTEGER, Uid UNSIGNED INTEGER )");
_LIT(KSQLGetDPIDUID, "SELECT * FROM DPIDStore");

/**
Two-phase construction
@param aDatabase	The reference to the database object
@return	pointer to the created suid map instance
*/
CMTPDPIDStore* CMTPDPIDStore::NewL(RDbDatabase& aDatabase)
	{
	CMTPDPIDStore* self = new (ELeave) CMTPDPIDStore(aDatabase);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}


/**
Standard c++ constructor
*/	
CMTPDPIDStore::CMTPDPIDStore(RDbDatabase& aDatabase)
	:iDatabase(aDatabase)
	{
	}

/**
Second phase constructor.
*/    
void CMTPDPIDStore::ConstructL()
    {    
    CreateDPIDStoteTableL();
    }

/**
Create the table for storing the mapping from object handle to other properties 
(data provider id, storage id, formatcode, etc.)
*/    
void CMTPDPIDStore::CreateDPIDStoteTableL()
	{	
	if (!DBUtility::IsTableExistsL(iDatabase, KSQLDPIDTableName))
		{
		LEAVEIFERROR(iDatabase.Execute(KSQLCreateDPIDTableText),
		        OstTrace0( TRACE_ERROR, CMTPDPIDSTORE_CREATEDPIDSTOTETABLEL, "TABLE DPIDStore create failed!" ));
		}
	iSqlStatement.Format(KSQLGetDPIDUID);    
    RDbView view;
    CleanupClosePushL(view);
    LEAVEIFERROR(view.Prepare(iDatabase, TDbQuery(iSqlStatement)),
            OstTrace0( TRACE_ERROR, DUP1_CMTPDPIDSTORE_CREATEDPIDSTOTETABLEL, "view for DPIDStore prepare failed!" ));
    LEAVEIFERROR(view.Evaluate(),
            OstTrace0( TRACE_ERROR, DUP2_CMTPDPIDSTORE_CREATEDPIDSTOTETABLEL, "view evaluate failed!" ));
	while (view.NextL())
        {
        view.GetL();
       	iDPIDs.AppendL(view.ColInt64(EDPIDStoreDataProviderId));
		iDPUIDs.AppendL(view.ColInt64(EDPIDStoreUid));
        }
	CleanupStack::PopAndDestroy(&view);	
	}
	
/**
Destructor.
*/    
CMTPDPIDStore::~CMTPDPIDStore()
    {  
	iDPIDs.Close();
	iDPUIDs.Close();
    }


void CMTPDPIDStore::InsertDPIDObjectL(TUint aDPId, TUint aUid)
	{		
	iSqlStatement.Format(KSQLInsertDPIDObjectText, aDPId, aUid);
	TInt err(iDatabase.Execute(iSqlStatement));
	iDatabase.Compact();
	iDPIDs.AppendL(aDPId);
	iDPUIDs.AppendL(aUid);
	}	
  
TUint CMTPDPIDStore::DPIDL(TUint32 aUid, TBool& aFlag) const
    {
	TUint tempDPId = 3;
	aFlag = ETrue;
	TRAPD(err, tempDPId = iDPUIDs.FindL(aUid));
	if(err == KErrNone)
		{
		tempDPId = iDPIDs[tempDPId];
		}
	else
		{
		if(iDPIDs.Count()>0)
			{
			tempDPId = iDPIDs[iDPIDs.Count() - 1];
			tempDPId++;			
			}
		aFlag = EFalse;
		}
	return tempDPId;
    } 
