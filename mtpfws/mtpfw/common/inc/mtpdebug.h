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

/*
 *  Todo: keep these macros temporarily, will discard them at last
 */
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

/*
 * Unlike __FLOG_ACTIVE, every component has its own macro OST_TRACE_COMPILER_IN_USE .
 * Thus the macro won't be used in this header file.
 */
#define __MTP_HEAP_OSTTRACE(r) \
    { \
    TInt allocated; \
    TInt largest; \
    TInt available(User::Heap().Available(largest)); \
    TInt size(User::Heap().Size()); \
    User::Heap().AllocSize(allocated); \
    r;\
    }

#define LEAVEIFERROR(err, trace) \
{ \
TInt munged_err=(err); \
if (munged_err < 0) \
{ \
trace; \
User::Leave(munged_err);\
}\
}

#define TRACEPANIC(aReason, trace) \
        { \
        tp; \
        User::Panic(KMyCategory, (aReason)); \
        }
        
#if defined(_DEBUG)
#define __ASSERT_DEBUG_OST(c,trace,p) if(!c) {trace; p;}
#else
#define __ASSERT_DEBUG_OST(c,trace,p)
#endif

#define __ASSERT_ALWAYS_OST(c,trace,p) if(!c) {trace; p;}

#endif // MTPDEBUG_H
