// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMEDIASYNCDATABASE_H
#define CMEDIASYNCDATABASE_H

#include <d32dbms.h>
#include <comms-infras/commsdebugutility.h>
#include <mdesession.h>
#include <mdccommon.h>
#include <comms-infras/commsdebugutility.h>

class CMediaSyncDataWriter;
class CMdESession;

class CMediaSyncDatabase : public CBase
    {
public:
    static CMediaSyncDatabase* NewL(RFs& aFs);
    ~CMediaSyncDatabase();

    /**        
    * Save MDS notification to database
    *
    * @param aObjectIdArray array of changed object id
    * @param aChangeType type of change related with this changed object
    *
    */
    void SaveNotificationsL(const RArray<TItemId>& aObjectIdArray, TObserverNotificationType aType, CMdESession& aSession); 
    
    /**        
    * Save MDS notification to database
    *
    * @param aObjectIdArray object IDs which are set to present statect
    * @param aPresent state: ETrue - present or  EFales - not present
    *
    */
    void SaveNotificationsL(const RArray<TItemId>& aObjectIdArray, TBool aPresent, CMdESession& aSession);     
    
    /**        
    * Delete all notification record from database
    * 
    */ 
    void RemoveAllNotificationsL();    
    
    /**
     * Delete specific notificaion by object id
     * 
     * @param aObjectId changed object id
     * 
     */
    TBool RemoveNotificationL(TItemId aObjectId, TUint aType);
    
    /**        
    * Get notification record from database
    *
    * @param aResulWriter on return contains serialized results
    * @param aIsFinished flag indicate whether all record has been fetched
    */    
    void FetchNotificationsL(CMediaSyncDataWriter& aResulWriter, TInt aMaxtFetchCount, TBool& aIsFinished);
    
    /**
     * Rollback the current transaction
     */
    void Rollback();
    
    /**
     * Check whether DB file is corrupt
     */
    inline TBool IsMssDbCorrupt() { return iDbCorrupt; }
    
    /**
     * Clear DB corrupt flag
     */    
    inline void ClearMssDbCorrupt() { iDbCorrupt = EFalse; }
    
private:
    CMediaSyncDatabase(RFs& aFs);
    void ConstructL();
    void CreateTableL(const TDesC& aDbFile);
    void CreateTabIndexL();
    
    void SaveAddNotificationsL(const RArray<TItemId>& aObjectIdArray, CMdESession& aSession);        
    void SaveAndCheckWithUriL(const RArray<TItemId>& aObjectIdArray, TUint aType, CMdESession& aSession);
    void SaveWithoutUriL(const RArray<TItemId>& aObjectIdArray, TUint aType);         
    void CompactDatabase();
    
    TBool UpdateUriColumnL(TItemId aObjectId, TUint aType, const TDesC& aUri);     
    TBool OptimizeL(TItemId aObjectId, TUint aType, const TDesC& aUri);
    inline TBool OptimizeL(TItemId aObjectId, TUint aType);
    
    static void RollbackTable(TAny* aTable);
    
private:
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    RFs&             iFs;
    RDbNamedDatabase iDatabase;
    RDbTable         iBatched;    
    TBool            iDbCorrupt;// flag that indicate whether database is corrupt
    TBool            iSavePosition;
    TDbBookmark      iBookmark;   
    TInt             iCompactCounter;
    };

#endif /*CMEDIASYNCDATABASE_H*/
