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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rmtpframeworkTraces.h"
#endif


// Class constants.

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
    OstTraceFunctionEntry0( RMTPFRAMEWORK_OPENL_ENTRY );
    iSingletons = &CSingletons::OpenL();
    iNested     = iSingletons->iConstructing;
    OstTraceFunctionExit0( RMTPFRAMEWORK_OPENL_EXIT );
    }

/**
Opens the singletons reference. The singletons reference is pushed onto the
cleanup stack.
*/
EXPORT_C void RMTPFramework::OpenLC()
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_OPENLC_ENTRY );
    ::CleanupClosePushL(*this);
    OpenL();
    OstTraceFunctionExit0( RMTPFRAMEWORK_OPENLC_EXIT );
    }
    
/**
Closes the singletons reference.
*/
EXPORT_C void RMTPFramework::Close()
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_CLOSE_ENTRY );
    if ((iSingletons) && (!iNested))
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    OstTraceFunctionExit0( RMTPFRAMEWORK_CLOSE_EXIT );
    }

EXPORT_C CMTPConnectionMgr& RMTPFramework::ConnectionMgr() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_CONNECTIONMGR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonConnectionMgr, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_CONNECTIONMGR_EXIT );
    return *(iSingletons->iSingletonConnectionMgr);
    }
    
EXPORT_C CMTPDataProviderController& RMTPFramework::DpController() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_DPCONTROLLER_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonDpController, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_DPCONTROLLER_EXIT );
    return *(iSingletons->iSingletonDpController);
    }
   
EXPORT_C CMTPFrameworkConfig& RMTPFramework::FrameworkConfig() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_FRAMEWORKCONFIG_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonFrameworkConfig, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_FRAMEWORKCONFIG_EXIT );
    return *(iSingletons->iSingletonFrameworkConfig);
    }

EXPORT_C RFs& RMTPFramework::Fs() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_FS_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_FS_EXIT );
    return iSingletons->iSingletonFs;
    }

EXPORT_C CMTPObjectMgr& RMTPFramework::ObjectMgr() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_OBJECTMGR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonObjectMgr, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_OBJECTMGR_EXIT );
    return *(iSingletons->iSingletonObjectMgr);
    }

EXPORT_C CMTPReferenceMgr& RMTPFramework::ReferenceMgr() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_REFERENCEMGR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonRouter, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_REFERENCEMGR_EXIT );
    return (iSingletons->iSingletonObjectMgr->ObjectStore().ReferenceMgr());
    }

EXPORT_C CMTPParserRouter& RMTPFramework::Router() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_ROUTER_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonRouter, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_ROUTER_EXIT );
    return *(iSingletons->iSingletonRouter);
    }

EXPORT_C CMTPStorageMgr& RMTPFramework::StorageMgr() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_STORAGEMGR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingletonStorageMgr, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_STORAGEMGR_EXIT );
    return *(iSingletons->iSingletonStorageMgr);
    }

EXPORT_C CMTPDataCodeGenerator& RMTPFramework::DataCodeGenerator() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_DATACODEGENERATOR_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iSingleDataCodeGenerator, User::Invariant());
    OstTraceFunctionExit0( RMTPFRAMEWORK_DATACODEGENERATOR_EXIT );
    return *(iSingletons->iSingleDataCodeGenerator);
    }

EXPORT_C CMTPServiceMgr& RMTPFramework::ServiceMgr() const
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_SERVICEMGR_ENTRY );
   __ASSERT_DEBUG(iSingletons, User::Invariant());
   __ASSERT_DEBUG(iSingletons->iSingleServiceMgr, User::Invariant());
   OstTraceFunctionExit0( RMTPFRAMEWORK_SERVICEMGR_EXIT );
   return *(iSingletons->iSingleServiceMgr);
    }

RMTPFramework::CSingletons& RMTPFramework::CSingletons::OpenL()
    {
    OstTraceFunctionEntry0( CSINGLETONS_OPENL_ENTRY );
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
    OstTraceFunctionExit0( CSINGLETONS_OPENL_EXIT );
    return *self;
    }
    
void RMTPFramework::CSingletons::Close()
    {   
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        OstTraceFunctionEntry0( CSINGLETONS_CLOSE_ENTRY );
        self->Dec();
        if (self->AccessCount() == 0)
            {
            OstTraceFunctionExit0( CSINGLETONS_CLOSE_EXIT );
            delete self;
            Dll::SetTls(NULL);
            }
        else
            {
            OstTraceFunctionExit0( DUP1_CSINGLETONS_CLOSE_EXIT );
            }
        }
    }
    
RMTPFramework::CSingletons::~CSingletons()
    {
    OstTraceFunctionEntry0( RMTPFRAMEWORK_CSINGLETONS_CSINGLETONS_DES_ENTRY );
    delete iSingletonStorageMgr;
    delete iSingletonRouter;
    delete iSingletonDpController;
    delete iSingletonObjectMgr;
    delete iSingletonFrameworkConfig;
    delete iSingletonConnectionMgr;
    delete iSingleDataCodeGenerator;
    delete iSingleServiceMgr;
    
    iSingletonFs.Close();
    OstTraceFunctionExit0( RMTPFRAMEWORK_CSINGLETONS_CSINGLETONS_DES_EXIT );
    }
    
void RMTPFramework::CSingletons::ConstructL()
    {
    OstTraceFunctionEntry0( CSINGLETONS_CONSTRUCTL_ENTRY );
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
    OstTraceFunctionExit0( CSINGLETONS_CONSTRUCTL_EXIT );
    }
