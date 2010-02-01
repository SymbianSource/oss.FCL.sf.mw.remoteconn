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


#ifndef CMEDIASYNCOBSERVER_H
#define CMEDIASYNCOBSERVER_H

#include "cmediasyncserverdef.h"
#include "cmediasyncdatabase.h"

class CMediaSyncDatabase;

class CMediaSyncObserver : public CBase,
                           public MMdESessionObserver,
                           public MMdEObjectObserver,
                           public MMdEObjectPresentObserver
  {
public:
    static CMediaSyncObserver* NewL(CMediaSyncDatabase* aDb);
    virtual ~CMediaSyncObserver();
  
    void SubscribeForChangeNotificationL();
    
    void UnsubscribeForChangeNotificationL();
    
public:
    // From MMdESessionObserver
    void HandleSessionOpened(CMdESession& aSession, TInt aError);
    void HandleSessionError(CMdESession& aSession, TInt aError);
    
    // From MMdEObjectObserver
    void HandleObjectNotification(CMdESession& aSession,
                                 TObserverNotificationType aType,
                                 const RArray<TItemId>& aObjectIdArray);
    
    // From MMdEObjectPresentObserver
    void HandleObjectPresentNotification(CMdESession& aSession, 
                                        TBool aPresent, 
                                        const RArray<TItemId>& aObjectIdArray);
    
private:
    
    CMediaSyncObserver(CMediaSyncDatabase* aDb);
    void ConstructL();
    
    void HandleSessionCallback(TInt aError);
    
    void HandleObjectNotificationL(CMdESession& aSession,
                                   TObserverNotificationType aType,
                                   const RArray<TItemId>& aObjectIdArray);  
    
    void HandleObjectPresentNotificationL(CMdESession& aSession, 
                                        TBool aPresent, 
                                        const RArray<TItemId>& aObjectIdArray);
    
private: //not has ownership
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMdESession*           iSession;    
    CMediaSyncDatabase*    iDb;    
    CActiveSchedulerWait*  iSessionWait;  
    TInt                   iMdeSessionError;
    TBool iSubscribed;  
  };

#endif /*CMEDIASYNCOBSERVER_H*/
