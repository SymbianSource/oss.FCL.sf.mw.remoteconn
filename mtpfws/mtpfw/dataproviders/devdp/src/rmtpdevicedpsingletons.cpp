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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rmtpdevicedpsingletonsTraces.h"
#endif


// Class constants.

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
    iSingletons = &CSingletons::OpenL(aFramework);
    }
    
/**
Closes the singletons reference.
*/
void RMTPDeviceDpSingletons::Close()
    {
    OstTraceFunctionEntry0( RMTPDEVICEDPSINGLETONS_CLOSE_ENTRY );
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    OstTraceFunctionExit0( RMTPDEVICEDPSINGLETONS_CLOSE_EXIT );
    }

/**
Provides a handle to the MTP device data provider's device information data 
store singleton.
@return The device information data store singleton.
*/
CMTPDeviceDataStore& RMTPDeviceDpSingletons::DeviceDataStore()
    {
    OstTraceFunctionEntry0( RMTPDEVICEDPSINGLETONS_DEVICEDATASTORE_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iDeviceDataStore, User::Invariant());
    OstTraceFunctionExit0( RMTPDEVICEDPSINGLETONS_DEVICEDATASTORE_EXIT );
    return *iSingletons->iDeviceDataStore;
    }

/**
Provides a handle to the MTP device data provider's config manager singleton.
@return The device config manager singleton.
*/
    
CMTPDeviceDpConfigMgr& RMTPDeviceDpSingletons::ConfigMgr()
	{
    OstTraceFunctionEntry0( RMTPDEVICEDPSINGLETONS_CONFIGMGR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iConfigMgr, User::Invariant());
    OstTraceFunctionExit0( RMTPDEVICEDPSINGLETONS_CONFIGMGR_EXIT );
    return *iSingletons->iConfigMgr;
	}

RMTPDeviceDpSingletons::CSingletons* RMTPDeviceDpSingletons::CSingletons::NewL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( CSINGLETONS_NEWL_ENTRY );
    CSingletons* self(new(ELeave) CSingletons());
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CSINGLETONS_NEWL_EXIT );
    return self;
    }

RMTPDeviceDpSingletons::CSingletons& RMTPDeviceDpSingletons::CSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( CSINGLETONS_OPENL_ENTRY );
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
    OstTraceFunctionExit0( CSINGLETONS_OPENL_EXIT );
    return *self;
    }
    
void RMTPDeviceDpSingletons::CSingletons::Close()
    {
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        OstTraceFunctionEntry0( RMTPDEVICEDPSINGLETONS_CSINGLETONS_CLOSE_ENTRY );
        self->Dec();
        if (self->AccessCount() == 0)
            {
            OstTraceFunctionExit0( RMTPDEVICEDPSINGLETONS_CSINGLETONS_CLOSE_EXIT );
            delete self;
            Dll::SetTls(NULL);
            }
        else
            {
            OstTraceFunctionExit0( DUP1_RMTPDEVICEDPSINGLETONS_CSINGLETONS_CLOSE_EXIT );
            }
        }
    }
    
RMTPDeviceDpSingletons::CSingletons::~CSingletons()
    {
    OstTraceFunctionEntry0( CSINGLETONS_CSINGLETONS_ENTRY );
    delete iConfigMgr;
    delete iDeviceDataStore;
    iPendingStorages.Close();
    OstTraceFunctionExit0( CSINGLETONS_CSINGLETONS_EXIT );
    }
    
void RMTPDeviceDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( CSINGLETONS_CONSTRUCTL_ENTRY );
    iDeviceDataStore = CMTPDeviceDataStore::NewL();
    iConfigMgr = CMTPDeviceDpConfigMgr::NewL(aFramework);
    OstTraceFunctionExit0( CSINGLETONS_CONSTRUCTL_EXIT );
    }

RArray<TUint>& RMTPDeviceDpSingletons::PendingStorages()
	{
    OstTraceFunctionEntry0( RMTPDEVICEDPSINGLETONS_PENDINGSTORAGES_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    OstTraceFunctionExit0( RMTPDEVICEDPSINGLETONS_PENDINGSTORAGES_EXIT );
    return iSingletons->iPendingStorages;
	}

