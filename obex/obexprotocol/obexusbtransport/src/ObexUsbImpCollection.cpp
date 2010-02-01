// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <ecom/ecom.h>
#include <ecom/implementationproxy.h>
#include "ObexUsbTransportController.h"

/**
Define the private interface UIDs
2 implementations are defined to cater for the USB and USB-V2 variants. Both
interfaces call the same NewL function.
ECOM passes a TObexTransportInfo parameter. The iTransportName member will
be used to cast the parameter to the correct type.
*/
const TImplementationProxy ObexTCImplementationTable[] =
    {
	IMPLEMENTATION_PROXY_ENTRY(0x1020de88, CObexUsbTransportController::NewL),//USB implementation
	IMPLEMENTATION_PROXY_ENTRY(0x1020dea4, CObexUsbTransportController::NewL),//USB V2 implementation
    };
    
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ObexTCImplementationTable) / sizeof(TImplementationProxy);

    return ObexTCImplementationTable;
    }
    
