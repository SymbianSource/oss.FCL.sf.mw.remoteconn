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

#ifndef CMTPDPIDSTORE_H
#define CMTPDPIDSTORE_H  
/** 
Implements the MTP object manager interface.
@internalComponent
 
*/
#include <d32dbms.h>
#include "mtpframeworkconst.h"

class RDbDatabase;

class CMTPDPIDStore :  public CBase        
    {
public:   
 
    static CMTPDPIDStore* NewL(RDbDatabase& aDatabase);    
    ~CMTPDPIDStore();

public:     
    void InsertDPIDObjectL(TUint aDpId, TUint aUid);    
    TUint DPIDL(TUint32 aUid, TBool& aFlag) const;

private: 
	enum TDPIDStore
        {
        EDPIDStoreDataProviderId         = 1,
        EDPIDStoreUid                    = 2        
        };   
	CMTPDPIDStore(RDbDatabase& aDatabase);
    void CreateDPIDStoteTableL();
    void ConstructL();   
private:
	RDbDatabase&                    iDatabase;	
	mutable TBuf<KMTPMaxSqlStatementLen>    iSqlStatement;   
	RArray<TInt64> iDPIDs;
	RArray<TInt64> iDPUIDs;
    };
#endif // CMTPDPIDSTORE_H
