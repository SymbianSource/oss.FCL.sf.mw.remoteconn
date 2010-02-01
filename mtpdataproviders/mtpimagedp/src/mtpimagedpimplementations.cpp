// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#include <ecom/implementationproxy.h>
#include "cmtpimagedp.h"


// Define the interface UIDs
static const TImplementationProxy ImplementationTable[] =
    {
        {
        {0x2001FCAE}, (TProxyNewLPtr)(CMTPImageDataProvider::NewL)
        }
    };

/**
ECOM entry point
*/
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
