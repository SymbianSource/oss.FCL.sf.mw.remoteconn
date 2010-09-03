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
 @internalTechnology
*/

#ifndef CMTPREFERENCEMGR_H
#define CMTPREFERENCEMGR_H

#include <e32base.h>
#include <d32dbms.h>
#include <mtp/mmtpreferencemgr.h>

#include "mtpframeworkconst.h"
#include "rmtpframework.h"

class RDbDatabase;
class CMTPObjectStore;

/** 
Implements the MTP reference manager interface.

@internalTechnology
 
*/
class CMTPReferenceMgr : public CBase, public MMTPReferenceMgr
    {
public:    
    static CMTPReferenceMgr* NewL(CMTPObjectStore& aObjectStore);
    ~CMTPReferenceMgr();
    
    void RemoveReferencesL(TUint aHandle);
    
public: // From MMTPReferenceMgr

    CMTPTypeArray* ReferencesLC(const TMTPTypeUint32& aFromHandle) const;
    CDesCArray* ReferencesLC(const TDesC& aParentSuid) const;
    void RemoveReferencesL(const TDesC& aSuid);
    void SetReferenceL(const TDesC& aFromSuid, const TDesC& aToSuid);
    void SetReferencesL(const TDesC& aParentSuid, const CDesCArray& aToSuids);
    void SetReferencesL(const TMTPTypeUint32& aFromHandle, const CMTPTypeArray& aToHandles);
    
private:
    CMTPReferenceMgr(CMTPObjectStore& aObjectStore);
    void ConstructL();
    
private:    
    //helpers
    void CreateTableL();
    void CreateIndexL();
    
    CMTPTypeArray* GetReferencesLC(TUint aHandle) const;
    CDesCArray* GetReferencesInDesLC(TUint aHandle) const;
    void GetReferencesL(TUint aHandle, RArray<TUint>& aToHandles) const;
    void SetReferencesL(TUint aHandle, const RArray<TUint>& aToHandles);
    void SetReferenceL(TUint aHandle, TUint aToHandles);
    //void AdjustReferencesL(RArray<TUint32>& aHandles) const;
	void IncTranOpsNumL();
	
private: // Owned
  
    TBuf<KMTPMaxSqlStatementLen>    iSqlStatement;
    
private: // Not owned

    RDbDatabase*                    iDatabase; 
    CMTPObjectStore&			    iObjectStore;
    mutable RDbTable 				iBatched;
    };
#endif // CMTPREFERENCEMGR_H
