// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <ecom/implementationproxy.h>
#include "cmtpbearermonitor.h"

// Define the implementation UID of MTP Service Plugin implementation.
static const TImplementationProxy ImplementationTable[] =
    {
    // KFeatureIdS60MtpController = 272
    IMPLEMENTATION_PROXY_ENTRY( 272, CMTPBearerMonitor::NewL )
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }

