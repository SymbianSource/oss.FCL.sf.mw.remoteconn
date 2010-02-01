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

#ifndef CMTPPKGIDSTORE_H
#define CMTPPKGIDSTORE_H  
/** 
Implements the MTP package ID store interface.
@internalComponent
 
*/
//#include <e32base.h>
#include <d32dbms.h>
#include "mtpframeworkconst.h"

class RDbDatabase;

/**
 * Mapping of Installed data provider <sis package id, dp id>
 */
class CMTPPkgIDStore :  public CBase        
    {
public:   
    static CMTPPkgIDStore* NewL(RDbDatabase& aDatabase);    
    ~CMTPPkgIDStore();

public:     
    void InsertPkgIdL(TUint aDpId, TUint aPkgId);
    const RArray<TUint>& DPIDL() const;
    TUint PKGIDL (TUint aIndex) const;
    TInt  RemoveL(TUint aDpId);

private: 
    enum TPkgIDStore
        {
        EPkgIDStoreDataProviderId   = 1,
        EPkgIDStorePKGID            = 2
        };   
    CMTPPkgIDStore(RDbDatabase& aDatabase);
    void CreatePkgIDStoreTableL();
    void ConstructL();   
private:
    RDbDatabase&  iDatabase;   
    mutable TBuf<KMTPMaxSqlStatementLen> iSqlStatement;   
    RArray<TUint> iDPIDs;
    RArray<TUint> iPkgIDs;
    };
    
#endif // CMTPPKGIDSTORE_H
