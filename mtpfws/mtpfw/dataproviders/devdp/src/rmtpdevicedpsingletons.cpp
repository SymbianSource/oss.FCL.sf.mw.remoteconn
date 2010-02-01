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

#include <e32std.h>
#include "cmtpdevicedatastore.h"
#include "cmtpdevicedpconfigmgr.h"
#include "cmtpstoragewatcher.h"
#include "rmtpdevicedpsingletons.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"DeviceDpSingletons");)

/**
Constructor.
*/
RMTPDeviceDpSingletons::RMTPDeviceDpSingletons() :
    iSingletons(NULL)
    {
    
    }

/**
Opens the singletons reference.
*/
void RMTPDeviceDpSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    iSingletons = &CSingletons::OpenL(aFramework);
    __FLOG(_L8("OpenL - Exit"));
    }
    
/**
Closes the singletons reference.
*/
void RMTPDeviceDpSingletons::Close()
    {
    __FLOG(_L8("Close - Entry"));
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    __FLOG(_L8("Close - Exit"));
    __FLOG_CLOSE;
    }

/**
Provides a handle to the MTP device data provider's device information data 
store singleton.
@return The device information data store singleton.
*/
CMTPDeviceDataStore& RMTPDeviceDpSingletons::DeviceDataStore()
    {
    __FLOG(_L8("DeviceDataStore - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iDeviceDataStore, User::Invariant());
    __FLOG(_L8("DeviceDataStore - Exit"));
    return *iSingletons->iDeviceDataStore;
    }

/**
Provides a handle to the MTP device data provider's config manager singleton.
@return The device config manager singleton.
*/
    
CMTPDeviceDpConfigMgr& RMTPDeviceDpSingletons::ConfigMgr()
	{
    __FLOG(_L8("ConfigMgr - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iConfigMgr, User::Invariant());
    __FLOG(_L8("ConfigMgr - Exit"));
    return *iSingletons->iConfigMgr;
	}

RMTPDeviceDpSingletons::CSingletons* RMTPDeviceDpSingletons::CSingletons::NewL(MMTPDataProviderFramework& aFramework)
    {
    CSingletons* self(new(ELeave) CSingletons());
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    return self;
    }

RMTPDeviceDpSingletons::CSingletons& RMTPDeviceDpSingletons::CSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Entry"));
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (!self)
        {
        self = CSingletons::NewL(aFramework);
        Dll::SetTls(reinterpret_cast<TAny*>(self));
        }
    else
        {        
        self->Inc();
        }
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Exit"));
    return *self;
    }
    
void RMTPDeviceDpSingletons::CSingletons::Close()
    {
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        __FLOG(_L8("CSingletons::Close - Entry"));
        self->Dec();
        if (self->AccessCount() == 0)
            {
            __FLOG(_L8("CSingletons::Close - Exit"));
            delete self;
            Dll::SetTls(NULL);
            }
        else
            {
            __FLOG(_L8("CSingletons::Close - Exit"));
            }
        }
    }
    
RMTPDeviceDpSingletons::CSingletons::~CSingletons()
    {
    __FLOG(_L8("CSingletons::~CSingletons - Entry"));
    delete iConfigMgr;
    delete iDeviceDataStore;
    __FLOG(_L8("CSingletons::~CSingletons - Exit"));
    __FLOG_CLOSE;
    }
    
void RMTPDeviceDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CSingletons::ConstructL - Entry"));
    iDeviceDataStore = CMTPDeviceDataStore::NewL();
    iConfigMgr = CMTPDeviceDpConfigMgr::NewL(aFramework);
    __FLOG(_L8("CSingletons::ConstructL - Exit"));
    }

