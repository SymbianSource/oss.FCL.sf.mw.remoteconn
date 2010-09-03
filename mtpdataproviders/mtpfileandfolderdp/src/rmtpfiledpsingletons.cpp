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

#include "rmtpfiledpsingletons.h"

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/cmtpobjectmetadata.h>

#include "cmtpfiledpconfigmgr.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rmtpfiledpsingletonsTraces.h"
#endif


// Class constants.

/**
Constructor.
*/
RMTPFileDpSingletons::RMTPFileDpSingletons() :
    iSingletons(NULL)
    {
    }

/**
Opens the singletons reference.
*/
void RMTPFileDpSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_OPENL_ENTRY );
    iSingletons = &CSingletons::OpenL(aFramework);
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_OPENL_EXIT );
    }
    
/**
Closes the singletons reference.
*/
void RMTPFileDpSingletons::Close()
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_CLOSE_ENTRY );
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_CLOSE_EXIT );
    }

/**
Provides a handle to the MTP file data provider's configuration manager.
@return The file data provider configuration manager.
*/

CMTPFileDpConfigMgr& RMTPFileDpSingletons::FrameworkConfig()
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_FRAMEWORKCONFIG_ENTRY );
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iConfigMgr, User::Invariant());
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_FRAMEWORKCONFIG_EXIT );
    return *iSingletons->iConfigMgr;
    }

RMTPFileDpSingletons::CSingletons* RMTPFileDpSingletons::CSingletons::NewL(MMTPDataProviderFramework& aFramework)
    {
    CSingletons* self(new(ELeave) CSingletons());
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    return self;
    }

RMTPFileDpSingletons::CSingletons& RMTPFileDpSingletons::CSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_CSINGLETONS_OPENL_ENTRY );
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
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_CSINGLETONS_OPENL_EXIT );
    return *self;
    }
    
void RMTPFileDpSingletons::CSingletons::Close()
    {
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_CSINGLETONS_CLOSE_ENTRY );
        self->Dec();
        if (self->AccessCount() == 0)
            {
            OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_CSINGLETONS_CLOSE_EXIT );
            delete self;
            Dll::SetTls(NULL);
            }
        else
            {
            OstTraceFunctionExit0( DUP1_RMTPFILEDPSINGLETONS_CSINGLETONS_CLOSE_EXIT );
            }
        }
    }
    
RMTPFileDpSingletons::CSingletons::~CSingletons()
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_CSINGLETONS_CSINGLETONS_ENTRY );
    delete iConfigMgr;
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_CSINGLETONS_CSINGLETONS_EXIT );
    }
    
void RMTPFileDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    OstTraceFunctionEntry0( RMTPFILEDPSINGLETONS_CSINGLETONS_CONSTRUCTL_ENTRY );
    iConfigMgr = CMTPFileDpConfigMgr::NewL(aFramework);
    OstTraceFunctionExit0( RMTPFILEDPSINGLETONS_CSINGLETONS_CONSTRUCTL_EXIT );
    }



