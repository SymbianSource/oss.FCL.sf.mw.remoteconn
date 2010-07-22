/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Report base class implementation
*
*/


#include <e32std.h>

#include "hidtranslate.h"
#include "hidreportroot.h"
#include "hidinterfaces.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
EXPORT_C TReportTranslator::TReportTranslator(
    const TDesC8& aData,
    const CField *aField)
    : iData(aData), iField(aField)
    {
    }

// ---------------------------------------------------------------------------
// GetValue()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::GetValue(TInt& aValue, TInt aUsageId,
    TInt aControlOffset /*= 0*/) const
    {
    TInt usageIndex = 0;

    if ( iField && TReportUtils::GetIndexOfUsage(iField, aUsageId, usageIndex) )
        {
        if ( iField->IsArray() )
            {
            // Convert usage ID to logical value
            TInt logicalValue = usageIndex + iField->LogicalMin();

            // Find logical value in the array
            for ( TInt i = 0; i < iField->Count(); i++ )
                {
                TInt value = 0;
                TInt error = TReportUtils::ReadData(iData, iField, i, value);

                if ( KErrNone != error )
                    {
                    return error;
                    }

                if ( value == logicalValue )
                    {
                    aValue = ETrue;
                    return KErrNone;
                    }
                }

            aValue = EFalse;
            return KErrNone;
            }
        else
            {
            return TReportUtils::ReadData(iData, iField, usageIndex + aControlOffset, aValue);
            }
        }

    return KErrUsageNotFound;
    }

// ---------------------------------------------------------------------------
// ValueL()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::ValueL(TInt aUsageId,
    TInt aControlOffset /*= 0*/) const
    {
    TInt value = 0;

    User::LeaveIfError(GetValue(value, aUsageId, aControlOffset));

    return value;
    }

// ---------------------------------------------------------------------------
// GetUsageId()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::GetUsageId(TInt& aUsageId, TInt aIndex) const
    {
    TInt logicalValue = 0;
    TInt error = KErrNoMemory;

    if ( iField )
        {
        error = TReportUtils::ReadData(iData, iField, aIndex, logicalValue);

        if ( KErrNone == error )
            {
            if ( iField->IsArray() )
                {
                // The logical value is a usage index
                aUsageId = TReportUtils::UsageAtIndex(iField, logicalValue -
                    iField->LogicalMin());
                }
            else
                {
                // Treat the logical value as an on/off control for the usage
                aUsageId = ( logicalValue ) ? TReportUtils::UsageAtIndex(iField, 
                    aIndex) : 0;
                }
            }
        }

    return error;
    }

// ---------------------------------------------------------------------------
// GetUsageId()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::UsageIdL(TInt aIndex) const
    {
    TInt usageId = 0;

    User::LeaveIfError(GetUsageId(usageId, aIndex));

    return usageId;
    }

// ---------------------------------------------------------------------------
// RawValueL()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::RawValueL(TInt aIndex) const
    {
    TInt value = 0;
    User::LeaveIfNull(const_cast<CField*>(iField));
    User::LeaveIfError(TReportUtils::ReadData(iData, iField, aIndex, value));
    return value;
    }

// ---------------------------------------------------------------------------
// Count()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt TReportTranslator::Count() const
    {
    TInt count = 0;

    if (iField)
        {
        count = iField->Count();
        }

    return count;
    }
