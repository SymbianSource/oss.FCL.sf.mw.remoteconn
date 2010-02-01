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

#include "cmtppkgidstore.h"
#include "dbutility.h"

/**
Two-phase construction
@param aDatabase    The reference to the database object
@return pointer to the created <pkgid, dpid> map instance
*/
CMTPPkgIDStore* CMTPPkgIDStore::NewL(RDbDatabase& aDatabase)
    {
    CMTPPkgIDStore* self = new (ELeave) CMTPPkgIDStore(aDatabase);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


/**
Standard c++ constructor
*/  
CMTPPkgIDStore::CMTPPkgIDStore(RDbDatabase& aDatabase)
    :iDatabase(aDatabase)
    {
    }

/**
Second phase constructor.
*/    
void CMTPPkgIDStore::ConstructL()
    {
    CreatePkgIDStoreTableL();
    }

/**
Create table to store the mapping from object handle to other properties 
(data provider id, storage id, formatcode, etc.)
*/    
void CMTPPkgIDStore::CreatePkgIDStoreTableL()
    {   
    _LIT(KSQLPkgIDTableName, "PkgIDStore");
    if (!DBUtility::IsTableExistsL(iDatabase, KSQLPkgIDTableName))
        {
        _LIT(KSQLCreatePkgIDTableText,
            "CREATE TABLE PkgIDStore (DataProviderId UNSIGNED INTEGER, PkgId UNSIGNED INTEGER)");
        User::LeaveIfError(iDatabase.Execute(KSQLCreatePkgIDTableText));            
        }
    _LIT(KSQLGetPKGID, "SELECT * FROM PkgIDStore");
    iSqlStatement.Format(KSQLGetPKGID);    
    RDbView view;
    CleanupClosePushL(view);
    User::LeaveIfError(view.Prepare(iDatabase, TDbQuery(iSqlStatement)));
    User::LeaveIfError(view.Evaluate());
    while (view.NextL())
        {
        view.GetL();
        iDPIDs.AppendL(view.ColInt64(EPkgIDStoreDataProviderId));
        iPkgIDs.AppendL(view.ColInt64(EPkgIDStorePKGID));
        }
    CleanupStack::PopAndDestroy(&view); 
    }
    
/**
Destructor.
*/    
CMTPPkgIDStore::~CMTPPkgIDStore()
    {
    iDPIDs.Close();
    iPkgIDs.Close();
    }


void CMTPPkgIDStore::InsertPkgIdL(TUint aDPId, TUint aPkgId)
    {       
    TInt index = 0;
    TRAPD(err, index = iDPIDs.FindL(aDPId));
    if(KErrNotFound == err)
        {
        _LIT(KSQLInsertPkgIDObjectText, "INSERT INTO PkgIDStore (DataProviderId, PkgId) VALUES (%u, %u)");
        iSqlStatement.Format(KSQLInsertPkgIDObjectText, aDPId, aPkgId);
        User::LeaveIfError(iDatabase.Execute(iSqlStatement));
        iDPIDs.AppendL(aDPId);
        iPkgIDs.AppendL(aPkgId);
        }
    else
        {
        if(aPkgId != iPkgIDs[index])
            {
            _LIT(KSQLSetPkgIDObjectText, "UPDATE PkgIDStore SET PkgId = %u WHERE DataProviderId = %u");
            iSqlStatement.Format(KSQLSetPkgIDObjectText, aPkgId,aDPId);
            User::LeaveIfError(iDatabase.Execute(iSqlStatement));
            iPkgIDs[index] = aPkgId;
            }
        }
    }

const RArray<TUint>& CMTPPkgIDStore::DPIDL() const
    {
    __ASSERT_DEBUG((iPkgIDs.Count() == iDPIDs.Count()), User::Invariant());
    return iDPIDs;
    }

TUint CMTPPkgIDStore::PKGIDL (TUint aIndex) const
    {
    __ASSERT_DEBUG((aIndex < iPkgIDs.Count()), User::Invariant());
    return iPkgIDs[aIndex];
    }

TInt CMTPPkgIDStore::RemoveL(TUint aDpId)
    {
    TInt index = iDPIDs.Find(aDpId);
    if(KErrNotFound != index)
        {
        iDPIDs.Remove(index);
        iPkgIDs.Remove(index);
        _LIT(KSQLDeleteObjectText, "DELETE FROM PkgIDStore WHERE DataProviderId = %u");
        iSqlStatement.Format(KSQLDeleteObjectText, aDpId);
        User::LeaveIfError(iDatabase.Execute(iSqlStatement));
        }
    return index;
    }
