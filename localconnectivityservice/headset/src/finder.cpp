/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Generic hid finder implementation
 *
*/


#include <e32std.h>
#include <e32svr.h>

#include "finder.h"
#include "debug.h"

// ----------------------------------------------------------------------

// Define this for additional debug output (this file only):
#define EXTRA_DEBUG

#ifdef EXTRA_DEBUG
#define DBG(a) a;
#else
#define DBG(a)
#endif

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// THeadsetFinder()
// ---------------------------------------------------------------------------
//
THeadsetFinder::THeadsetFinder() :
    iAppCollection( 0 ), iFieldList()
    {
    // Nothing else to do
    }

// --------------------------------------------------------------------------
// From class MHidFieldFinder
// BeginCollection()
// --------------------------------------------------------------------------
//
TBool THeadsetFinder::BeginCollection( const CCollection* aCollection )
    {
    TBool examineCollection = ETrue;

    const TInt KConsumerControl = 0x01;

    // Only look at top-level application (consumer devices: consumer
    // control) collections:
    //

    if ( ( aCollection->IsApplication() ) && ( iAppCollection == 0 ) )
        {
        // Top-level application collection.

        if ( ( aCollection->UsagePage() == EUsagePageConsumer )
                && ( aCollection->Usage() == KConsumerControl ) )
            {
            TRACE_INFO(_L("[HID]\tTHeadsetFinder::BeginCollection: this is \
                    consumer collection "));
            // Collection is a consumer device:
            iAppCollection = aCollection;
            iFieldList.Reset();
            }
        else
            {
            TRACE_INFO(_L("[HID]\tTHeadsetFinder::BeginCollection: not \
                    consumer collection"));
            // Skip other types of top-level application collection:
            examineCollection = EFalse;
            }
        }
    return examineCollection;

    }

// ---------------------------------------------------------------------------
// From class MHidFieldFinder
// EndCollection()
// ---------------------------------------------------------------------------
//
TBool THeadsetFinder::EndCollection( const CCollection* aCollection )
    {
    TBool continueSearch = ETrue;

    TRACE_INFO(_L("[HID]\tTHeadsetFinder::EndCollection"));
    if ( aCollection == iAppCollection )
        {
        // Top-level application(Consumer Devices:Consumer Control) finished:
        //
        iAppCollection = 0;

        // Stop if we've found a device we can use in this
        // application collection:
        //
        continueSearch = !Found();
        }

    return continueSearch;
    }

// ---------------------------------------------------------------------------
// From class MHidFieldFinder
// Field()
// ---------------------------------------------------------------------------
//
void THeadsetFinder::Field( const CField* aField )
    {
    TRACE_INFO((_L("[HID]\tTHeadsetFinder::Field( 0x%08x)"),aField));
    TInt error = KErrNone;
    if ( iAppCollection )
        {

        if ( IsTelephony( aField ) )
            {
            TRACE_INFO((_L("[HID]\tTHeadsetFinder::Field, telephony")));
            error = iFieldList.Append( aField );
            if ( error != KErrNone )
                {
                TRACE_INFO((_L("[HID]\tTHeadsetFinder::Field, telephony \
                        failed")));
                }
            }
        else if ( IsConsumer( aField ) )
            {
            TRACE_INFO(_L("[HID]\tTHeadsetFinder::Field, consumer"));
            error = iFieldList.Append( aField );
            if ( error != KErrNone )
                {
                TRACE_INFO((_L("[HID]\tTHeadsetFinder::Field, telephony \
                        failed")));
                }
            }
        else
            {
            TRACE_INFO(_L("[HID]\tTHeadsetFinder::Field, other, or empty \
                    field"));
            }
        }
    }

// ---------------------------------------------------------------------------
// IsConsumer()
// ---------------------------------------------------------------------------
//
TBool THeadsetFinder::IsConsumer( const CField* aField ) const
    {
    TBool found = EFalse;

    if ( aField->IsInput() && aField->IsData() && ( aField->UsagePage()
            == EUsagePageConsumer ) )
        {
#ifdef _DEBUG
        for ( TInt i = 0; i < aField->UsageCount(); i++ )
            {
            TRACE_INFO((_L("[HID]\tTHeadsetFinder::IsConsumer: Usage %d: \
                    %02x"),i,aField->Usage(i)));
            }
#endif
            // *** Add usage test here ***
        TRACE_INFO(_L("[HID]\tTHeadsetFinder::IsConsumer:  Consumer keys \
                field found\r\n"));
        found = ETrue;
        }
    return found;
    }

// ---------------------------------------------------------------------------
// IsTelephony()
// ---------------------------------------------------------------------------
//
TBool THeadsetFinder::IsTelephony( const CField* aField ) const
    {
    TBool found = EFalse;

    if ( aField->IsInput() && aField->IsData() && ( aField->UsagePage()
            == EUsagePageTelephony ) )
        {
        const TInt KHookSwitch = 0x20;
        const TInt KPhoneMute = 0x2F;
#ifdef _DEBUG
        for ( TInt i = 0; i < aField->UsageCount(); i++ )
            {
            TRACE_INFO((_L("[HID]\tTHeadsetFinder::IsTelephony: \
                    Usage %d: %02x"),i,aField->Usage(i)));
            }
#endif
        if ( ( aField->UsageMin() <= KPhoneMute ) && ( aField->UsageMax()
                >= KHookSwitch ) )
            {
            TRACE_INFO(_L("[HID]\tTHeadsetFinder::IsTelephony:  Telephony \
                    field found"));
            found = ETrue;
            }
        }
    return found;
    }

// ---------------------------------------------------------------------------
// EmptyList()
// ---------------------------------------------------------------------------
//
void THeadsetFinder::EmptyList()
    {
    iFieldList.Reset();
    }

// ---------------------------------------------------------------------------
// FieldCount()
// ---------------------------------------------------------------------------
//
TInt THeadsetFinder::FieldCount()
    {
    return iFieldList.Count();
    }

// ---------------------------------------------------------------------------
// GetFieldAtIndex()
// ---------------------------------------------------------------------------
//
CField* THeadsetFinder::GetFieldAtIndex( TInt aIndex )
    {
    return iFieldList[aIndex];
    }

// ---------------------------------------------------------------------------
// Found()
// ---------------------------------------------------------------------------
//
TBool THeadsetFinder::Found() const
    {
    return ( iFieldList.Count() != 0 );
    }
