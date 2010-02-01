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

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"FileDpSingletons");)

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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("OpenL - Entry"));
    iSingletons = &CSingletons::OpenL(aFramework);
    __FLOG(_L8("OpenL - Exit"));
    }
    
/**
Closes the singletons reference.
*/
void RMTPFileDpSingletons::Close()
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
Provides a handle to the MTP file data provider's configuration manager.
@return The file data provider configuration manager.
*/

CMTPFileDpConfigMgr& RMTPFileDpSingletons::FrameworkConfig()
    {
    __FLOG(_L8("FrameworkConfig - Entry"));
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iConfigMgr, User::Invariant());
    __FLOG(_L8("FrameworkConfig - Exit"));
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
    
void RMTPFileDpSingletons::CSingletons::Close()
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
    
RMTPFileDpSingletons::CSingletons::~CSingletons()
    {
    __FLOG(_L8("CSingletons::~CSingletons - Entry"));
    delete iConfigMgr;
    __FLOG(_L8("CSingletons::~CSingletons - Exit"));
    __FLOG_CLOSE;
    }
    
void RMTPFileDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CSingletons::ConstructL - Entry"));
    iConfigMgr = CMTPFileDpConfigMgr::NewL(aFramework);
    __FLOG(_L8("CSingletons::ConstructL - Exit"));
    }



