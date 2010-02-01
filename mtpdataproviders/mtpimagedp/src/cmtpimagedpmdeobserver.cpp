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

#include <mdesession.h> 
#include <mdeconstants.h>
#include <mdequery.h>

#include <centralrepository.h>

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mmtpobjectmgr.h>
#include <mtp/cmtpobjectmetadata.h>
#include <mtp/tmtptypeevent.h>

#include "mtpdebug.h"
#include "mtpimagedpconst.h"
#include "cmtpimagedpmdeobserver.h"
#include "cmtpimagedp.h"
#include "cmtpimagedpobjectpropertymgr.h"
#include "mtpimagedputilits.h"

__FLOG_STMT(_LIT8(KComponent, "CMTPImageDpMdeObserver");)

CMTPImageDpMdeObserver* CMTPImageDpMdeObserver::NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider)
    {
    CMTPImageDpMdeObserver* self = new (ELeave) CMTPImageDpMdeObserver(aFramework, aDataProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------
//
CMTPImageDpMdeObserver::CMTPImageDpMdeObserver(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider) :
    iFramework(aFramework),
    iDataProvider(aDataProvider),
    iSubscribed(EFalse)
    {
    }

void CMTPImageDpMdeObserver::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPImageDpMdeObserver::ConstructL - Entry"));
    __FLOG(_L8("CMTPImageDpMdeObserver::ConstructL - Exit"));
    }

CMTPImageDpMdeObserver::~CMTPImageDpMdeObserver()
    {
    __FLOG(_L8("CMTPImageDpMdeObserver::~CMTPImageDpMdeObserver - Entry"));   
    __FLOG(_L8("CMTPImageDpMdeObserver::~CMTPImageDpMdeObserver - Exit"));   
    __FLOG_CLOSE;
    }

void CMTPImageDpMdeObserver::CreateMetadataL(const CMdEObject& aMdeObject, CMTPObjectMetaData& aMetaData)
    {
    const TDesC& uri = aMdeObject.Uri();
    TParsePtrC parse(uri);  
    TMTPFormatCode format = iDataProvider.FindFormatL(parse.Ext().Mid(1));
    TInt32 storageId = MTPImageDpUtilits::FindStorage(iFramework, uri);
    TUint32 parentHandle = MTPImageDpUtilits::FindParentHandleL(iFramework, iDataProvider, uri);
    
    aMetaData.SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
    aMetaData.SetUint(CMTPObjectMetaData::EFormatCode, format);
    aMetaData.SetUint(CMTPObjectMetaData::EStorageId, storageId);
    aMetaData.SetDesCL(CMTPObjectMetaData::ESuid, uri);    
    aMetaData.SetUint(CMTPObjectMetaData::EParentHandle, parentHandle);
    aMetaData.SetDesCL(CMTPObjectMetaData::EName, parse.Name());
    
    //Increate the new pictures variable
    ++iNewPics;
    }

void CMTPImageDpMdeObserver::ProcessMdeNotificationL(const RArray<TItemId>& aObjectIdArray, TObserverNotificationType /*aType*/)
    {    
    __FLOG_VA((_L8("CMTPImageDpMdeObserver::ProcessMdeNotificationL Start")));       
    
    //Because image dp only subscribe add notification from MdS, only the add notifictions can reach here
    //we ignore checking ENotifyAdd notification type to reduce complexity.    
    CMdESession& mdeSession = iDataProvider.PropertyMgr().MdeSession();
    CMdENamespaceDef& defaultNamespaceDef = mdeSession.GetDefaultNamespaceDefL();
    CMdEObjectDef& imageObjDef = defaultNamespaceDef.GetObjectDefL(MdeConstants::Image::KImageObject);    
                   
    TInt objectCount = aObjectIdArray.Count();
    for (TInt i(0); i < objectCount; ++i)
        {
        CMdEObject* mdeObject = mdeSession.GetObjectL(aObjectIdArray[i], imageObjDef);
        if (mdeObject == NULL)
            {
            continue;
            }
        CleanupStack::PushL(mdeObject);
        
        TUint32 handle = iFramework.ObjectMgr().HandleL(mdeObject->Uri());
        if (handle == KMTPHandleNone)
            {
            CMTPObjectMetaData* object = CMTPObjectMetaData::NewLC();
            CreateMetadataL(*mdeObject, *object);
            iFramework.ObjectMgr().InsertObjectL(*object);                
            __FLOG_VA((_L16("Addition - Handle:%u, ObjectId:%u, URI:%S"), object->Uint(CMTPObjectMetaData::EHandle), aObjectIdArray[i], &mdeObject->Uri()));
            
            TMTPTypeEvent event;
            event.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeObjectAdded);
            event.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
            event.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
          
            TUint32 handle = object->Uint(CMTPObjectMetaData::EHandle);            
            event.SetUint32(TMTPTypeEvent::EEventParameter1, handle);                    
            iFramework.SendEventL(event);
            __FLOG_VA((_L8("ProcessMdeNotificationL - EMTPEventCodeObjectAdd, object handle:%u"), handle));
            
            CleanupStack::PopAndDestroy(object);//object 
            }
        
        CleanupStack::PopAndDestroy(mdeObject);//mdeObject             
        }    
    
    __FLOG_VA((_L8("CMTPImageDpMdeObserver::ProcessMdeNotificationL - Exit")));
    }

/*
 * After receiving object change notification, check if there is any dp subscribed right now.
 * if none, store change into database
 * if yes, check the type of file with subscribed providers, if there is any match, just forward
 * the change to that dp, if none, store change into database.
 */
void CMTPImageDpMdeObserver::HandleObjectNotification(CMdESession& aSession,
                                            TObserverNotificationType aType,
                                            const RArray<TItemId>& aObjectIdArray)
    {
    TRAP_IGNORE(HandleObjectNotificationL(aSession, aType, aObjectIdArray));
    }

/*
 * L Function
 */
void CMTPImageDpMdeObserver::HandleObjectNotificationL(CMdESession& /*aSession*/,
                                                    TObserverNotificationType aType,
                                                    const RArray<TItemId>& aObjectIdArray)
    {   
    __FLOG_VA((_L8("CMTPImageDpMdeObserver::HandleObjectNotificationL - changeType = %d"), aType));   
   
    ProcessMdeNotificationL(aObjectIdArray, aType);

    //Update RProperty to notify the subscribers.
    if (iNewPics > 0)
        {
        __FLOG_1(_L16("CMTPImageDpMdeObserver::CMTPImageDpMdeObserver - New Pics: %u"), iNewPics);
        MTPImageDpUtilits::UpdateNewPicturesValue(iDataProvider, iNewPics, ETrue);
        iNewPics = 0;
        }
    
    __FLOG(_L8("CMTPImageDpMdeObserver::CMTPImageDpMdeObserver - Exit"));
    }

/*
 * Subscribe MdS notifications
 */
void CMTPImageDpMdeObserver::SubscribeForChangeNotificationL()
    {    
    __FLOG_VA((_L8("CMTPImageDpMdeObserver::SubscribeForChangeNotificationL"))); 

    if (!iSubscribed)
        {
        CMdESession& mdeSession = iDataProvider.PropertyMgr().MdeSession();    
        
        CMdENamespaceDef& def = mdeSession.GetDefaultNamespaceDefL();
        CMdEObjectDef& imageObjDef = def.GetObjectDefL(MdeConstants::Image::KImageObject);
        
        //Only subscribe add observer        
        CMdELogicCondition* addCondition = CMdELogicCondition::NewLC(ELogicConditionOperatorAnd);          
        CMdEPropertyDef& itemTypePropDef = imageObjDef.GetPropertyDefL(MdeConstants::Object::KItemTypeProperty);       
        addCondition->AddPropertyConditionL(itemTypePropDef, ETextPropertyConditionCompareEndsWith, _L("jpeg"));                       
        mdeSession.AddObjectObserverL(*this, addCondition, ENotifyAdd);
        CleanupStack::Pop(addCondition);
        
        iSubscribed = ETrue;
        }
    
    __FLOG(_L8("CMTPImageDpMdeObserver::SubscribeForChangeNotificationL - Exit")); 
    }

/*
 * Unsubscribe MdS notifications
 */
void CMTPImageDpMdeObserver::UnsubscribeForChangeNotificationL()
    {
    __FLOG_VA((_L8("CMTPImageDpMdeObserver::UnsubscribeForChangeNotificationL"))); 
    
    if (iSubscribed)
        {
        CMdESession& mdeSession = iDataProvider.PropertyMgr().MdeSession();  
        
        mdeSession.RemoveObjectObserverL(*this);//add observer
        iSubscribed = EFalse;
        }
    
    __FLOG(_L8("CMTPImageDpMdeObserver::UnsubscribeForChangeNotificationL - Exit")); 
    }
