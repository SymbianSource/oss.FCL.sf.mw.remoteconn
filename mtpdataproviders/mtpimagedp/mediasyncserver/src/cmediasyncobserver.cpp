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

//
#include <mdequery.h>
#include <mdeconstants.h>

#include "cmediasyncdatabase.h"
#include "cmediasyncobserver.h"

__FLOG_STMT(_LIT8(KComponent,"MediaSyncObserver");)

CMediaSyncObserver*  CMediaSyncObserver::NewL(CMediaSyncDatabase* aDb)
    {       
    CMediaSyncObserver* self = new (ELeave) CMediaSyncObserver(aDb);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CMediaSyncObserver::ConstructL()
    {
    __FLOG_OPEN(KMSSSubsystem, KComponent);
    __FLOG(_L8("CMediaSyncObserver::ConstructL - Entry"));
    
    iSessionWait = new (ELeave) CActiveSchedulerWait();
    iSession = CMdESession::NewL(*this);
    iSessionWait->Start();
    
    User::LeaveIfError(iMdeSessionError);    
    
//    SubscribeForChangeNotificationL();
    
    __FLOG(_L8("CMediaSyncObserver::ConstructL - Exit"));
    }

CMediaSyncObserver::CMediaSyncObserver(CMediaSyncDatabase* aDb)
    :iDb(aDb),
    iSubscribed(EFalse)
    {    
    }

CMediaSyncObserver::~CMediaSyncObserver()
    {
    __FLOG(_L8("CMediaSyncObserver::~CMediaSyncObserver - Entry"));  
    
//    TRAP_IGNORE(UnsubscribeForChangeNotificationL());
    
    delete iSession;
    delete iSessionWait;
	
    __FLOG(_L8("CMediaSyncObserver::~CMediaSyncObserver - Exit"));
    __FLOG_CLOSE;
    }

void CMediaSyncObserver::SubscribeForChangeNotificationL()
    {
    __FLOG(_L8("CMediaSyncObserver::SubscribeForChangeNotificationL - Entry"));
    
    if (!iSubscribed)
        {        
        CMdENamespaceDef& def = iSession->GetDefaultNamespaceDefL();
        CMdEObjectDef& imageObjDef = def.GetObjectDefL(MdeConstants::Image::KImageObject);
        
        // add observer        
        CMdELogicCondition* addCondition = CMdELogicCondition::NewLC(ELogicConditionOperatorAnd);          
        CMdEPropertyDef& itemTypePropDef = imageObjDef.GetPropertyDefL(MdeConstants::Object::KItemTypeProperty);       
        addCondition->AddPropertyConditionL(itemTypePropDef, ETextPropertyConditionCompareEndsWith, _L("jpeg"));                       
        iSession->AddObjectObserverL(*this, addCondition, ENotifyAdd);
        CleanupStack::Pop(addCondition);
        
        // modify observer
        CMdELogicCondition* modifyCondition = CMdELogicCondition::NewLC(ELogicConditionOperatorAnd);          
        CMdEPropertyDef& titlePropDef = imageObjDef.GetPropertyDefL(MdeConstants::Object::KTitleProperty);
        modifyCondition->AddPropertyConditionL(titlePropDef);
        iSession->AddObjectObserverL(*this, modifyCondition, ENotifyModify);
        CleanupStack::Pop(modifyCondition);
        
        // remove observer
        iSession->AddObjectObserverL(*this, NULL, ENotifyRemove);
        
        // present observer
        iSession->AddObjectPresentObserverL(*this);
                        
        iSubscribed = ETrue;               
        }

    __FLOG(_L8("CMediaSyncObserver::SubscribeForChangeNotificationL - Exit"));
    }

void CMediaSyncObserver::UnsubscribeForChangeNotificationL()
    {
    __FLOG(_L8("CMediaSyncObserver::UnsubscribeForChangeNotificationL - Entry"));
    
    if (iSubscribed)
        {
        iSession->RemoveObjectObserverL(*this);//add observer
        iSession->RemoveObjectObserverL(*this);//modify observer
        iSession->RemoveObjectObserverL(*this);//remove observer
        iSession->RemoveObjectPresentObserverL(*this);
        iSubscribed = EFalse;
        }
    
    __FLOG(_L8("CMediaSyncObserver::UnsubscribeForChangeNotificationL - Exit"));
    }

// From MMdESessionObserver
void CMediaSyncObserver::HandleSessionOpened(CMdESession& /*aSession*/, TInt aError)
    {
    __FLOG(_L8("CMediaSyncObserver::HandleSessionOpened - Entry"));
    
    HandleSessionCallback(aError);
    
    __FLOG(_L8("CMediaSyncObserver::HandleSessionOpened - Exit"));
    }

void CMediaSyncObserver::HandleSessionError(CMdESession& /*aSession*/, TInt aError)
    {
    __FLOG(_L8("CMediaSyncObserver::HandleSessionError - Entry"));
    
    HandleSessionCallback(aError);
    
    __FLOG(_L8("CMediaSyncObserver::HandleSessionError - Exit"));
    }

void CMediaSyncObserver::HandleSessionCallback(TInt aError)
    {
    __ASSERT_DEBUG(iSessionWait, User::Invariant());
    iMdeSessionError = aError;    
    if (iSessionWait->IsStarted())
        {
        iSessionWait->AsyncStop();
        }
    }

/*
 * After receiving object change notification, check if there is any dp subscribed right now.
 * if none, store change into database
 * if yes, check the type of file with subscribed providers, if there is any match, just forward
 * the change to that dp, if none, store change into database.
 */
void CMediaSyncObserver::HandleObjectNotification(CMdESession& aSession,
                                            TObserverNotificationType aType,
                                            const RArray<TItemId>& aObjectIdArray)
    {
    TRAPD(err, HandleObjectNotificationL(aSession, aType, aObjectIdArray));
    
    if (err != KErrNone)
        {
        __FLOG(_L8("CMediaSyncObserver::HandleObjectNotification - Rollback database"));        
        iDb->Rollback();
        }
    }

/*
 * L Function
 */
void CMediaSyncObserver::HandleObjectNotificationL(CMdESession& /*aSession*/,
                                                   TObserverNotificationType aType,
                                                   const RArray<TItemId>& aObjectIdArray)
    {
    __FLOG(_L8("CMediaSyncObserver::HandleObjectNotificationL - Entry"));
    
    iDb->SaveNotificationsL(aObjectIdArray, aType, *iSession);
    
    __FLOG(_L8("CMediaSyncObserver::HandleObjectNotificationL - Exit"));
    }

/*
 * Called to notify the observer that objects has been set
 * to present or not present state in the metadata engine database.
 */
void CMediaSyncObserver::HandleObjectPresentNotification(CMdESession& aSession,
                                                        TBool aPresent, 
                                                        const RArray<TItemId>& aObjectIdArray)
    {
    TRAPD(err, HandleObjectPresentNotificationL(aSession, aPresent, aObjectIdArray));
    
    if (err != KErrNone)
        {
        __FLOG(_L8("CMediaSyncObserver::HandleObjectPresentNotification - Rollback database"));
        iDb->Rollback();
        }    
    }

/*
 * L Function
 */
void CMediaSyncObserver::HandleObjectPresentNotificationL(CMdESession& /*aSession*/,
                                                        TBool aPresent, 
                                                        const RArray<TItemId>& aObjectIdArray)
    {
    __FLOG(_L8("CMediaSyncObserver::HandleObjectPresentNotificationL - Entry"));
     
    iDb->SaveNotificationsL(aObjectIdArray, aPresent, *iSession);
       
    __FLOG(_L8("CMediaSyncObserver::HandleObjectPresentNotificationL - Exit"));
    }
