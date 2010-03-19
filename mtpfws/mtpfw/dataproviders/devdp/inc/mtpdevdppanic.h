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

/**
 @file
 @internalComponent
*/

#ifndef MTPDEVDPPANIC_H
#define MTPDEVDPPANIC_H

#include <e32std.h>

enum TMTPDevDpPanic
    {
    EMTPDevDpReserved                   = 0,
    EMTPDevDpInvalidState               = 1,
    EMTPDevDpDeviceInfoPhaseInvalid     = 2,
    EMTPDevDpNoMatchingProcessor        = 3,
    EMTPDevDpConnectionNotFound         = 4,
    EMTPDevDpStorageNoMedia             = 5,
    EMTPDevDpResponseParameterTooMany   = 6,
    EMTPDevDpInvalidRequest             = 7,
    EMTPDevDpUnknownDeviceProperty      = 8,
    EMTPDevDpUnknownServiceID           = 9,
    EMTPDevDpObjectNull					= 10,

    };

_LIT(KMTPDevDpPanic, "MTPDeviceDp");
inline void Panic(TMTPDevDpPanic aCode)
    {
    User::Panic(KMTPDevDpPanic, aCode);
    };

#endif // MTPDEVDPPANIC_H

