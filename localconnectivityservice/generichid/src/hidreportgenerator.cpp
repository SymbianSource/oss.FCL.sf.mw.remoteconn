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
* Description:  HID retport generator
*
*/



#include "hidreportgenerator.h"
#include "hidreportroot.h"
#include "hidinterfaces.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CReportGenerator* CReportGenerator::NewLC(const CReportRoot*
    aReportRoot, TInt aReportId, CField::TType aType)
    {
    CReportGenerator* self = new (ELeave) CReportGenerator(aReportId, aType);
    CleanupStack::PushL(self);
    self->ConstructL(aReportRoot);
    return self;
    }

// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CReportGenerator* CReportGenerator::NewL(const CReportRoot*
    aReportRoot, TInt aReportId, CField::TType aType)
    {
    CReportGenerator* self =
        CReportGenerator::NewLC(aReportRoot, aReportId, aType);
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CReportGenerator::CReportGenerator(TInt aReportId, CField::TType aType)
    : iReportId(aReportId), iType(aType)
    {
    // Nothing else to do
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CReportGenerator::ConstructL(const CReportRoot* aReportRoot)
    {
    User::LeaveIfNull(const_cast<CReportRoot*>(aReportRoot));
    iReportRoot = aReportRoot;

    // Allocate the buffer, initialise to all zeros, and fill in the
    // report ID if used.  Zero should be a null value for all array fields
    // according to the HID parser error checking spec.
    iBuf = HBufC8::NewMaxL(aReportRoot->ReportSizeBytes(iReportId, iType));
    TPtr8 bufptr = iBuf->Des(); 
    bufptr.FillZ();
    if ( 0 != iReportId )
        {
        bufptr[0] = static_cast<TUint8>(iReportId);
        }
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CReportGenerator::~CReportGenerator()
    {
    delete iBuf;
    }

// ---------------------------------------------------------------------------
// SetField()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CReportGenerator::SetField(const CField* aField,
    TInt aUsageId, TInt aValue, TInt aControlOffset /*= 0*/)
    {
    TInt usageIndex;

    if ( aField && TReportUtils::GetIndexOfUsage(aField, aUsageId, usageIndex) )
        {
        if ( aField->IsArray() )
            {
            // Convert usage ID to logical value
            TInt logicalValue = usageIndex + aField->LogicalMin();

            // Find unused position in the array and write the logical
            // value to it
            for ( TInt i = 0; i < aField->Count(); i++ )
                {
                TInt value;
                TInt error = TReportUtils::ReadData(*iBuf, aField, i, value);

                if ( KErrNone != error )
                    {
                    return error;
                    }

                if ( value == logicalValue )
                    {
                    // The array already contains this usage
                    return KErrNone;
                    }
                else if ( 0 == value || value < aField->LogicalMin() ||
                    aField->LogicalMax() < value )
                    {
                    // This is an unused position
                    //
                    // NOTE: The comparison with zero is because the buffer is
                    // initialised to all zeros, and some reports erroneously
                    // include zero in the logical range.
                    //
                    return TReportUtils::WriteData(*iBuf, aField, i, logicalValue);
                    }
                }

            return KErrNoSpaceInArray;
            }
        else
            {
            // Check the value to set is valid
            if ( aValue < aField->LogicalMin() ||
                aField->LogicalMax() < aValue )
                {
                return KErrValueOutOfRange;
                }

            return TReportUtils::WriteData(*iBuf, aField,
                usageIndex + aControlOffset, aValue);
            }
        }

    return KErrUsageNotFound;
    }

// ---------------------------------------------------------------------------
// SetField()
// ---------------------------------------------------------------------------
//
EXPORT_C TPtr8 CReportGenerator::Report()
    {
    return iBuf->Des();
    }
