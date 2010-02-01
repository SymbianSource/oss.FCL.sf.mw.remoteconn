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
 @internalComponent
*/

#include "cmtpusbconnection.h"
#include "cmtpusbepwatcher.h"

__FLOG_STMT(_LIT8(KComponent,"UsbEpWatcher");)

/**
USB MTP device class endpoint stall status observer factory method.
@return A pointer to an USB MTP device class endpoint stall status 
observer object. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
CMTPUsbEpWatcher* CMTPUsbEpWatcher::NewL(CMTPUsbConnection& aConnection)
    {
    CMTPUsbEpWatcher* self = new (ELeave) CMTPUsbEpWatcher(aConnection);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor.
*/
CMTPUsbEpWatcher::~CMTPUsbEpWatcher()
    {
    __FLOG(_L8("~CMTPUsbEpWatcher - Entry"));
    Cancel();
    __FLOG(_L8("~CMTPUsbEpWatcher - Exit"));
    __FLOG_CLOSE;
    }

void CMTPUsbEpWatcher::Start()
    {
    __FLOG(_L8("Start - Entry"));
    if (!IsActive())
        {
        RequestIssue();
        }
    __FLOG(_L8("Start - Exit"));
    }
    
void CMTPUsbEpWatcher::Stop()
    {
    __FLOG(_L8("Stop - Entry"));
    Cancel();
    __FLOG(_L8("Stop - Exit"));
    }
    
void CMTPUsbEpWatcher::DoCancel()
    {
    __FLOG(_L8("DoCancel - Entry"));    
    RequestCancel();
    __FLOG(_L8("DoCancel - Exit"));  
    }
    
#ifdef __FLOG_ACTIVE
TInt CMTPUsbEpWatcher::RunError(TInt aError)
#else
TInt CMTPUsbEpWatcher::RunError(TInt /*aError*/)
#endif
    {
    __FLOG(_L8("RunError - Entry"));
    __FLOG_VA((_L8("Error = %d"), aError));
    __FLOG(_L8("RunError - Exit"));
    RequestIssue();
    return KErrNone;  
    }
    
void CMTPUsbEpWatcher::RunL()
    {
    __FLOG(_L8("RunL - Entry"));
    iConnection.EndpointStallStatusChangedL(iEndpointStatus);
    RequestIssue();
    __FLOG(_L8("RunL - Exit"));  
    }

CMTPUsbEpWatcher::CMTPUsbEpWatcher(CMTPUsbConnection& aConnection) :
    CActive(EPriorityStandard),
    iConnection(aConnection)
    {
    CActiveScheduler::Add(this);
    }
    
void CMTPUsbEpWatcher::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));    
    __FLOG(_L8("ConstructL - Exit")); 
    }

void CMTPUsbEpWatcher::RequestCancel()
    {
    __FLOG(_L8("RequestCancel - Entry"));    
    iConnection.Ldd().EndpointStatusNotifyCancel();
    __FLOG(_L8("RequestCancel - Exit")); 
    }
    
void CMTPUsbEpWatcher::RequestIssue()
    {
    __FLOG(_L8("RequestIssue - Entry"));    
    iConnection.Ldd().EndpointStatusNotify(iStatus, iEndpointStatus);
    SetActive();
    __FLOG(_L8("RequestIssue - Exit")); 
    }
