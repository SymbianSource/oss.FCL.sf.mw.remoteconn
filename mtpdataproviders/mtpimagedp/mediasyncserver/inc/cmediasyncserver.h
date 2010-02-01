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

#ifndef CMEDIASYNCSERVER_H
#define CMEDIASYNCSERVER_H

#include <e32base.h>
#include <e32std.h>

#include "cmediasyncserverdef.h"

class RFs;
class CMediaSyncObserver;
class CMediaSyncDatabase;

class CMediaSyncServer : public CPolicyServer
    {
public:

    ~CMediaSyncServer();

    static void RunServerL();
    static CMediaSyncServer* NewLC(RFs& aFs);
    
    CMediaSyncObserver* MediaSyncObserver() const;
    CMediaSyncDatabase* MediaSyncDatabase() const;
  //  void AddSession();
  //  void DropSession();
    
    TBool NeedFullSync();
    void ClearFullSyncFlag();
    
private: // From CPolicyServer
    
   CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
    
private:
    
    CMediaSyncServer();
    void ConstructL(RFs& aFs);
    
private: //has ownership
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMediaSyncDatabase* iDb;
    CMediaSyncObserver* iObserver;
    TBool               iNeedFullSync; 
    };

#endif /*CMEDIASYNCSERVER_H*/
