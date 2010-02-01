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

#include <ecom/ecom.h>
#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/mmtpdataproviderframework.h>

/**
MTP data provider plug-in factory method.
@param aImplementationUid A UID specifying the required data provider 
interface implementation.
@param aParams An MTP framework specific parameter block pointer. This pointer
should simply be passed through to this class's constructor by the derived 
class.
@return Pointer to the plug-in instance. Ownership IS transfered.
@leave KErrNoMemory If the plug-in instance could not be allocated.
@leave KErrArgument If an invalid aImplementationUid was specified.
*/
EXPORT_C CMTPDataProviderPlugin* CMTPDataProviderPlugin::NewL(TUid aImplementationUid, TAny* aParams)
    {
    CMTPDataProviderPlugin* self = reinterpret_cast<CMTPDataProviderPlugin*>(
        REComSession::CreateImplementationL(
            aImplementationUid,
            _FOFF(CMTPDataProviderPlugin, iDtorIdKey), 
            aParams));
    return self;
    }

/**
Destructor.
*/
EXPORT_C CMTPDataProviderPlugin::~CMTPDataProviderPlugin()
    {
	REComSession::DestroyedImplementation(iDtorIdKey);
    }

/**
Constructor.
@param aParams Parameter block pointer.
@panic USER 0 If the parameter block pointer is 0.
*/
EXPORT_C CMTPDataProviderPlugin::CMTPDataProviderPlugin(TAny* aParams) :
    iFramework(reinterpret_cast<MMTPDataProviderFramework*> (aParams))
    {
    __ASSERT_ALWAYS(iFramework, User::Invariant());
    }

EXPORT_C MMTPDataProviderFramework& CMTPDataProviderPlugin::Framework() const
    {
    return *iFramework;
    }
