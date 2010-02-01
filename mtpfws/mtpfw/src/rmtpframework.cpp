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

#include "cmtpconnectionmgr.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpframeworkconfig.h"
#include "cmtpobjectmgr.h"
#include "cmtpobjectstore.h"
#include "cmtpparserrouter.h"
#include "cmtpstoragemgr.h"
#include "rmtpframework.h"
#include "cmtpdatacodegenerator.h"
#include "cmtpservicemgr.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"Framework");)

/**
Constructor.
*/
EXPORT_C RMTPFramework::RMTPFramework() :
    iSingletons(NULL)
    {
    
    }

/**
Opens the singletons reference.
*/
EXPORT_C void RMTPFramework::OpenL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    iSingletons = &CSingletons::OpenL();
    iNested     = iSingletons->iConstructing;
    __FLOG(_L8("OpenL - Exit"));
    }

/**
Opens the singletons reference. The singletons reference is pushed onto the
cleanup stack.
*/
EXPORT_C void RMTPFramework::OpenLC()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenLC - Entry"));
    ::CleanupClosePushL(*this);
    OpenL();
    __FLOG(_L8("OpenLC - Exit"));
    }
    
/**
Closes the singletons reference.
*/
EXPORT_C void RMTPFramework::Close()
    {
    __FLOG(_L8("Close - Entry"));
    if ((iSingletons) && (!iNested))
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    __FLOG(_L8("Close - Exit"));
    __FLOG_CLOSE;
    }

EXPORT_C CMTPConnectionMgr& RMTPFramework::ConnectionMgr() const
    {
    __FLOG(_L8("ConnectionMgr - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonConnectionMgr, User::Invariant());
    __FLOG(_L8("ConnectionMgr - Exit"));
    return *(iSingletons->iSingletonConnectionMgr);
    }
    
EXPORT_C CMTPDataProviderController& RMTPFramework::DpController() const
    {
    __FLOG(_L8("DpController - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonDpController, User::Invariant());
    __FLOG(_L8("DpController - Exit"));
    return *(iSingletons->iSingletonDpController);
    }
   
EXPORT_C CMTPFrameworkConfig& RMTPFramework::FrameworkConfig() const
    {
    __FLOG(_L8("FrameworkConfig - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonFrameworkConfig, User::Invariant());
    __FLOG(_L8("FrameworkConfig - Exit"));
    return *(iSingletons->iSingletonFrameworkConfig);
    }

EXPORT_C RFs& RMTPFramework::Fs() const
    {
    __FLOG(_L8("Fs - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __FLOG(_L8("Fs - Exit"));
    return iSingletons->iSingletonFs;
    }

EXPORT_C CMTPObjectMgr& RMTPFramework::ObjectMgr() const
    {
    __FLOG(_L8("ObjectMgr - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonObjectMgr, User::Invariant());
    __FLOG(_L8("ObjectMgr - Exit"));
    return *(iSingletons->iSingletonObjectMgr);
    }

EXPORT_C CMTPReferenceMgr& RMTPFramework::ReferenceMgr() const
    {
    __FLOG(_L8("ReferenceMgr - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonRouter, User::Invariant());
    __FLOG(_L8("ReferenceMgr - Exit"));
    return (iSingletons->iSingletonObjectMgr->ObjectStore().ReferenceMgr());
    }

EXPORT_C CMTPParserRouter& RMTPFramework::Router() const
    {
    __FLOG(_L8("Router - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonRouter, User::Invariant());
    __FLOG(_L8("Router - Exit"));
    return *(iSingletons->iSingletonRouter);
    }

EXPORT_C CMTPStorageMgr& RMTPFramework::StorageMgr() const
    {
    __FLOG(_L8("StorageMgr - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonStorageMgr, User::Invariant());
    __FLOG(_L8("StorageMgr - Exit"));
    return *(iSingletons->iSingletonStorageMgr);
    }

EXPORT_C CMTPDataCodeGenerator& RMTPFramework::DataCodeGenerator() const
    {
    __FLOG(_L8("DataCodeGenerator - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingleDataCodeGenerator, User::Invariant());
    __FLOG(_L8("DataCodeGenerator - Exit"));
    return *(iSingletons->iSingleDataCodeGenerator);
    }

EXPORT_C CMTPServiceMgr& RMTPFramework::ServiceMgr() const
    {
    __FLOG(_L8("ServiceMgr - Entry"));
   __ASSERT_DEBUG(iSingletons, User::Invariant());
   __ASSERT_DEBUG(iSingletons->iSingleServiceMgr, User::Invariant());
   __FLOG(_L8("ServiceMgr - Exit"));
   return *(iSingletons->iSingleServiceMgr);
    }

RMTPFramework::CSingletons& RMTPFramework::CSingletons::OpenL()
    {
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Entry"));
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (!self)
        {
        self = new(ELeave) CSingletons();
        Dll::SetTls(reinterpret_cast<TAny*>(self));
        self->ConstructL();
        }
    else if (!self->iConstructing)
        {        
        self->Inc();
        }
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("CSingletons::OpenL - Exit"));
    return *self;
    }
    
void RMTPFramework::CSingletons::Close()
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
    
RMTPFramework::CSingletons::~CSingletons()
    {
    __FLOG(_L8("CSingletons::~CSingletons - Entry"));
    delete iSingletonStorageMgr;
    delete iSingletonRouter;
    delete iSingletonDpController;
    delete iSingletonObjectMgr;
    delete iSingletonFrameworkConfig;
    delete iSingletonConnectionMgr;
    delete iSingleDataCodeGenerator;
    delete iSingleServiceMgr;
    
    iSingletonFs.Close();
    __FLOG(_L8("CSingletons::~CSingletons - Exit"));
    __FLOG_CLOSE;
    }
    
void RMTPFramework::CSingletons::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CSingletons::ConstructL - Entry"));
    iConstructing = ETrue;
    
	User::LeaveIfError(iSingletonFs.Connect());
    iSingletonFrameworkConfig   = CMTPFrameworkConfig::NewL();
    iSingletonConnectionMgr     = CMTPConnectionMgr::NewL();
    iSingletonObjectMgr         = CMTPObjectMgr::NewL();
    iSingletonDpController      = CMTPDataProviderController::NewL();
    iSingletonRouter            = CMTPParserRouter::NewL();
    iSingletonStorageMgr        = CMTPStorageMgr::NewL();
    iSingleDataCodeGenerator    = CMTPDataCodeGenerator::NewL();
    iSingleServiceMgr           = CMTPServiceMgr::NewL();
    
    iConstructing = EFalse;
    __FLOG(_L8("CSingletons::ConstructL - Exit"));
    }
