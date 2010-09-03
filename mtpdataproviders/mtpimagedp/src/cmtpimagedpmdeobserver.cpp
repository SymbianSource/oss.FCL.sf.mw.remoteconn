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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpimagedpmdeobserverTraces.h"
#endif


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
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_CONSTRUCTL_EXIT );
    }

CMTPImageDpMdeObserver::~CMTPImageDpMdeObserver()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_CMTPIMAGEDPMDEOBSERVER_ENTRY );
    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_CMTPIMAGEDPMDEOBSERVER_EXIT );
    }

void CMTPImageDpMdeObserver::CreateMetadataL(const TParsePtrC& aParse, TMTPFormatCode aFormat, TUint32 aParentHandle, CMTPObjectMetaData& aMetaData)
    {
    TInt32 storageId = MTPImageDpUtilits::FindStorage(iFramework, aParse.FullName());

    OstTraceExt1( TRACE_NORMAL, CMTPIMAGEDPMDEOBSERVER_CREATEMETADATAL, "Addition - New Image:%S", aParse.FullName() );
    aMetaData.SetUint(CMTPObjectMetaData::EDataProviderId, iFramework.DataProviderId());
    aMetaData.SetUint(CMTPObjectMetaData::EFormatCode, aFormat);
    aMetaData.SetUint(CMTPObjectMetaData::EStorageId, storageId);
    aMetaData.SetDesCL(CMTPObjectMetaData::ESuid, aParse.FullName());    
    aMetaData.SetUint(CMTPObjectMetaData::EParentHandle, aParentHandle);
    aMetaData.SetDesCL(CMTPObjectMetaData::EName, aParse.Name());
    
    //Increate the new pictures variable
    ++iNewPics;  
    }

void CMTPImageDpMdeObserver::ProcessMdeNotificationL(const RArray<TItemId>& aObjectIdArray, TObserverNotificationType /*aType*/)
    {    
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL_ENTRY );     
    
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
        OstTraceExt2( TRACE_NORMAL, CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL, 
                "Addition - ObjectId:%u, URI:%S", aObjectIdArray[i], mdeObject->Uri());

        //filter out all image files other jpeg files       
        TParsePtrC parse(mdeObject->Uri());
        TMTPFormatCode format = iDataProvider.FindFormat(parse.Ext().Mid(1));
        if (format != EMTPFormatCodeUndefined)
            {
            TUint32 handle = iFramework.ObjectMgr().HandleL(mdeObject->Uri());
            if (handle == KMTPHandleNone)
                {
                /**
                 * Try to find parent handle of new image file
                 * if we can't find parent handle we should issue a notification to framework, let framework to enumerate new folder if need
                 */
                TUint32 parentHandle = MTPImageDpUtilits::FindParentHandleL(iFramework, iDataProvider, parse.FullName());
                if (parentHandle == KMTPHandleNone)
                    {
                    OstTraceExt1( TRACE_NORMAL, DUP1_CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL, "Notify framework - URI:%S", mdeObject->Uri() );
                    TMTPNotificationParamsFolderChange param = {mdeObject->Uri()};                                       
                    iFramework.NotifyFrameworkL(EMTPAddFolder, &param);
                    
                    //try to get parent handle if framework complete it's tasks
                    parentHandle = MTPImageDpUtilits::FindParentHandleL(iFramework, iDataProvider, parse.FullName());
                    OstTrace1( TRACE_NORMAL, DUP2_CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL, "Framework enumeration parent handle:%u", parentHandle );
                    }
                
                if (parentHandle != KMTPHandleNone)
                    {
                    CMTPObjectMetaData* object = CMTPObjectMetaData::NewLC();
                    CreateMetadataL(parse, format, parentHandle, *object);
                    iFramework.ObjectMgr().InsertObjectL(*object);                
                    OstTrace1( TRACE_NORMAL, DUP3_CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL, 
                            "Addition - New Handle:%u", object->Uint(CMTPObjectMetaData::EHandle) );
                    
                    TMTPTypeEvent event;
                    event.SetUint16(TMTPTypeEvent::EEventCode, EMTPEventCodeObjectAdded);
                    event.SetUint32(TMTPTypeEvent::EEventSessionID, KMTPSessionAll);
                    event.SetUint32(TMTPTypeEvent::EEventTransactionID, KMTPTransactionIdNone);
                  
                    TUint32 handle = object->Uint(CMTPObjectMetaData::EHandle);            
                    event.SetUint32(TMTPTypeEvent::EEventParameter1, handle);                    
                    iFramework.SendEventL(event);                  
                    OstTrace1( TRACE_NORMAL, DUP4_CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL, 
                            "ProcessMdeNotificationL - EMTPEventCodeObjectAdd, object handle:%u", handle );
                    
                    CleanupStack::PopAndDestroy(object);//object                 
                    }
                }
            }
        
        CleanupStack::PopAndDestroy(mdeObject);//mdeObject             
        }    
    
    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_PROCESSMDENOTIFICATIONL_EXIT );
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
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_HANDLEOBJECTNOTIFICATIONL_ENTRY );
    OstTrace1( TRACE_NORMAL, CMTPIMAGEDPMDEOBSERVER_HANDLEOBJECTNOTIFICATIONL, "changeType = %d", aType ); 
   
    ProcessMdeNotificationL(aObjectIdArray, aType);

    //Update RProperty to notify the subscribers.
    if (iNewPics > 0)
        {
        OstTrace1( TRACE_NORMAL, DUP1_CMTPIMAGEDPMDEOBSERVER_HANDLEOBJECTNOTIFICATIONL, "CMTPImageDpMdeObserver::CMTPImageDpMdeObserver - New Pics: %u", iNewPics );
        iDataProvider.IncreaseNewPictures(iNewPics);
        iNewPics = 0;
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_HANDLEOBJECTNOTIFICATIONL_EXIT );
    }

/*
 * Subscribe MdS notifications
 */
void CMTPImageDpMdeObserver::SubscribeForChangeNotificationL()
    {    
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_SUBSCRIBEFORCHANGENOTIFICATIONL_ENTRY );

    if (!iSubscribed)
        {
        CMdESession& mdeSession = iDataProvider.PropertyMgr().MdeSession();    
        
        CMdENamespaceDef& def = mdeSession.GetDefaultNamespaceDefL();
        CMdEObjectDef& imageObjDef = def.GetObjectDefL(MdeConstants::Image::KImageObject);
        
        //Only subscribe add observer        
        CMdELogicCondition* addCondition = CMdELogicCondition::NewLC(ELogicConditionOperatorAnd);        
        addCondition->AddObjectConditionL(imageObjDef);                 
        mdeSession.AddObjectObserverL(*this, addCondition, ENotifyAdd);
        CleanupStack::Pop(addCondition);
        
        iSubscribed = ETrue;
        }

    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_SUBSCRIBEFORCHANGENOTIFICATIONL_EXIT );
    }

/*
 * Unsubscribe MdS notifications
 */
void CMTPImageDpMdeObserver::UnsubscribeForChangeNotificationL()
    {
    OstTraceFunctionEntry0( CMTPIMAGEDPMDEOBSERVER_UNSUBSCRIBEFORCHANGENOTIFICATIONL_ENTRY );
    
    if (iSubscribed)
        {
        CMdESession& mdeSession = iDataProvider.PropertyMgr().MdeSession();  
        
        mdeSession.RemoveObjectObserverL(*this);//add observer
        iSubscribed = EFalse;
        }
    
    OstTraceFunctionExit0( CMTPIMAGEDPMDEOBSERVER_UNSUBSCRIBEFORCHANGENOTIFICATIONL_EXIT );
    }
