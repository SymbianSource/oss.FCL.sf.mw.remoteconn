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

#ifndef CMTPIMAGEDPMDEOBSERVER_H
#define CMTPIMAGEDPMDEOBSERVER_H

#include <e32cmn.h>
#include <e32base.h>
#include <e32def.h>

#include "cmtpimagedpobjectpropertymgr.h"

class MMTPDataProviderFramework;
class CMTPImageDataProvider;

/** 
Implements the observer interfaces of MDE.
@internalComponent
*/
class CMTPImageDpMdeObserver : public CBase, 
                               public MMdEObjectObserver
    {
public:

    static CMTPImageDpMdeObserver* NewL(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    ~CMTPImageDpMdeObserver();
    
public:
    // From MMdEObjectObserver
    void HandleObjectNotification(CMdESession& aSession,
                                 TObserverNotificationType aType,
                                 const RArray<TItemId>& aObjectIdArray);
    
    void SubscribeForChangeNotificationL();
    void UnsubscribeForChangeNotificationL();
    
private:
    CMTPImageDpMdeObserver(MMTPDataProviderFramework& aFramework, CMTPImageDataProvider& aDataProvider);
    void ConstructL();   
    
private:    
    void HandleObjectNotificationL(CMdESession& aSession,
                                   TObserverNotificationType aType,
                                   const RArray<TItemId>& aObjectIdArray);
            
    void ProcessMdeNotificationL(const RArray<TItemId>& aObjectIdArray, TObserverNotificationType aType);     
    void CreateMetadataL(const TParsePtrC& aParse, TMTPFormatCode aFormat, TUint32 aParentHandle, CMTPObjectMetaData& aMetaData); 
    
private:
    
    MMTPDataProviderFramework&    iFramework;    
    CMTPImageDataProvider&        iDataProvider;
    TBool                         iSubscribed;
    TInt                          iNewPics;
    };

#endif // CMTPIMAGEDPMDEOBSERVER_H
