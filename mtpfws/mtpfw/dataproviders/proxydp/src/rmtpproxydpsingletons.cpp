// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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



#include "rmtpproxydpsingletons.h"
#include "cmtpproxydpconfigmgr.h"

	
/**
Constructor.
*/
RMTPProxyDpSingletons::RMTPProxyDpSingletons() :
    iSingletons(NULL)
    {
    }

/**
Opens the singletons reference.
*/
void RMTPProxyDpSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
    iSingletons = &CSingletons::OpenL(aFramework);
    }
    
/**
Closes the singletons reference.
*/
void RMTPProxyDpSingletons::Close()
    {
    if (iSingletons)
        {
        iSingletons->Close();
        iSingletons = NULL;
        }
    }

/**
Provides a handle to the MTP proxy data provider's configuration manager.
@return The proxy data provider configuration manager.
*/
CMTPProxyDpConfigMgr& RMTPProxyDpSingletons::FrameworkConfig()
    {
    __ASSERT_DEBUG(iSingletons, User::Invariant());
    __ASSERT_DEBUG(iSingletons->iConfigMgr, User::Invariant());
    return *iSingletons->iConfigMgr;
    }

RMTPProxyDpSingletons::CSingletons* RMTPProxyDpSingletons::CSingletons::NewL(MMTPDataProviderFramework& aFramework)
    {
    CSingletons* self(new(ELeave) CSingletons());
    CleanupStack::PushL(self);
    self->ConstructL(aFramework);
    CleanupStack::Pop(self);
    return self;
    }

RMTPProxyDpSingletons::CSingletons& RMTPProxyDpSingletons::CSingletons::OpenL(MMTPDataProviderFramework& aFramework)
    {
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
    return *self;
    }
    
void RMTPProxyDpSingletons::CSingletons::Close()
    {
    CSingletons* self(reinterpret_cast<CSingletons*>(Dll::Tls()));
    if (self)
        {
        self->Dec();
        if (self->AccessCount() == 0)
            {
            delete self;
            Dll::SetTls(NULL);
            }
        }
    }
    
RMTPProxyDpSingletons::CSingletons::~CSingletons()
    {
    delete iConfigMgr;
    }
    
void RMTPProxyDpSingletons::CSingletons::ConstructL(MMTPDataProviderFramework& aFramework)
    {
    iConfigMgr = CMTPProxyDpConfigMgr::NewL(aFramework);
    }
