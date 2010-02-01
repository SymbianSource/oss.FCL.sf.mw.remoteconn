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

#include "cmtptransportplugin.h"

EXPORT_C CMTPTransportPlugin* CMTPTransportPlugin::NewL(TUid aImplementationUid)
    {
    return NewL( aImplementationUid, NULL );
    }

EXPORT_C CMTPTransportPlugin* CMTPTransportPlugin::NewL(TUid aImplementationUid, const TAny* aParameter)
    {
    TAny* parameter = const_cast<TAny*>(aParameter);
    CMTPTransportPlugin* self = reinterpret_cast<CMTPTransportPlugin*>(
        REComSession::CreateImplementationL(
            aImplementationUid,
            _FOFF(CMTPTransportPlugin, iDtorIdKey),parameter));
    return self;
    }

EXPORT_C CMTPTransportPlugin::~CMTPTransportPlugin()
    {
    REComSession::DestroyedImplementation(iDtorIdKey);
    }

EXPORT_C CMTPTransportPlugin::CMTPTransportPlugin()
    {

    }


