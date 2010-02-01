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

#ifndef MTPDEBUG_H
#define MTPDEBUG_H

#include <comms-infras/commsdebugutility.h>

__FLOG_STMT(_LIT8(KMTPSubsystem, "MTP");)

#ifdef __FLOG_ACTIVE
#define __MTP_HEAP_FLOG \
    { \
    TInt allocated; \
    TInt largest; \
    TInt available(User::Heap().Available(largest)); \
    TInt size(User::Heap().Size()); \
    User::Heap().AllocSize(allocated); \
    __FLOG_STATIC_VA((KMTPSubsystem, KComponent, _L8("Heap: Size = %d, Allocated = %d, Available = %d, Largest block = %d"), size, allocated, available, largest)); \
    }
#else
#define __MTP_HEAP_FLOG 
#endif // __FLOG_ACTIVE

#endif // MTPDEBUG_H
