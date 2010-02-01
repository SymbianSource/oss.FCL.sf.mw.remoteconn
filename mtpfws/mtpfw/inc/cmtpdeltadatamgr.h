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
// cmtpdeltadatamgr.h
// 
//

/**
 @file
 @publishedPartner
 @released
*/


#ifndef CMTPDELTADATAMGR_H
#define CMTPDELTADATAMGR_H

#include "mtpdebug.h"

#include <e32base.h>
#include <d32dbms.h>
#include "dbutility.h"
#include "mtpframeworkconst.h"
#include <mtp/cmtptypearray.h>
#include <mtp/tmtptypeuint128.h>
class RDbDatabase;
class TMTPTypeUint32;

class CMtpDeltaDataMgr : public CBase
{
private:
    CMtpDeltaDataMgr (RDbDatabase& aDatabase);
    void ConstructL();
    

public:    
    static CMtpDeltaDataMgr* NewL(RDbDatabase& aDatabase);
    ~CMtpDeltaDataMgr();
    /**
    The enumeration for operations on file .
    */
    enum TOpCode
    {
    EAdded=0,
    EDeleted=1,
    EModified=2,
    }; 
    
    /**
    updates the delta data table with the params , if the suid identifier param passesd is
    a not found in the table a new entry will be created with opcode as EAdded.
    */
    void UpdateDeltaDataTableL(TInt64 aSuidId, TOpCode aOpCode);
    
    IMPORT_C void CreateDeltaDataTableL();
    IMPORT_C void CreateAnchorIdTableL();
    IMPORT_C void InsertAnchorIdL(TInt aAnchorId, TInt aIdentifier);
    IMPORT_C void UpdateAnchorIdL(TInt aAnchorId, TInt aIdentifier);
    IMPORT_C TInt GetAnchorIdL(TInt aIdentifier);
    IMPORT_C void UpdatePersistentIndexL(TInt aCurindex, TInt aIdentifier);
    IMPORT_C TInt GetPersistentIndexL(TInt aIdentifier);
    
    IMPORT_C TInt GetAddedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aAddedPuidIdArray);
    IMPORT_C TInt GetDeletedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aDeletedPuidIdArray);
    IMPORT_C TInt GetModifiedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aModifiedPuidIdArray);
    IMPORT_C TInt GetChangedPuidsL(TInt aMaxArraySize, TInt &aPosition, CMTPTypeArray& aModifiedPuidIdArray, CMTPTypeArray& aDeletedPuidArray);    
    
    IMPORT_C void ResetMTPDeltaDataTableL();

private: // Not owned

    RDbDatabase& iDatabase; 
    TBuf<KMTPMaxSqlStatementLen>    iSqlStatement;
    RArray <TMTPTypeUint128> iSuidIdArray;
   
    RDbView iView;
    TBool iNeedToSendMore;
    TInt iTotalRows;    
    RDbTable iDeltaTableBatched;
    RDbTable iAnchorTableBatched;
   /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
};

#endif // CMTPDELTADATAMGR_H
