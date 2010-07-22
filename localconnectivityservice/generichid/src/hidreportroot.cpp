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
* Description:  Reportroot implementation
*
*/


#include <e32debug.h>

#include "hidreportroot.h"
#include "debug.h"

const TInt KSevenBits  = 7;

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// NewLC()
// -----------------------------------------------------------------------------
//
CReportRoot* CReportRoot::NewLC()
    {
    CReportRoot* self = new (ELeave) CReportRoot();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }
// -----------------------------------------------------------------------------
// NewL()
// -----------------------------------------------------------------------------
//
CReportRoot* CReportRoot::NewL()
    {
    CReportRoot* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }
// -----------------------------------------------------------------------------
// ~CReportRoot()
// -----------------------------------------------------------------------------
//
CReportRoot::~CReportRoot()
    {
    iSizes.Reset();
    iSizes.Close();
    }
// -----------------------------------------------------------------------------
// CReportRoot()
// -----------------------------------------------------------------------------
//
CReportRoot::CReportRoot()
    {
    // Nothing to do here
    }



// -----------------------------------------------------------------------------
// IncrementReportSizeL()
// Manage the report ID <-> size "map":
// -----------------------------------------------------------------------------
//
void CReportRoot::IncrementReportSizeL(TInt aReportId,
    CField::TType aType, TInt aIncrement)
    {
    TInt index = FindReportSizeIndex(aReportId, aType);

    if ( index == KErrNotFound )
        {
        // Add a new entry for this report ID to the "map":
        User::LeaveIfError(iSizes.Append(TReportSize(aReportId, aType)));
        index = iSizes.Count() - 1;

        TRACE_INFO((_L("Adding size record for report %d:%d (%d)\n"),
                  aReportId, aType, iSizes[index].iSize));
        }
    TRACE_INFO((_L("Size for report %d:%d changes %d + %d\n"),
              aReportId, aType, iSizes[index].iSize, aIncrement));
    iSizes[index].iSize += aIncrement;
    }

// -----------------------------------------------------------------------------
// NumberOfReports()
// -----------------------------------------------------------------------------
//
TInt CReportRoot::NumberOfReports() const
    {
    return iSizes.Count();
    }
// -----------------------------------------------------------------------------
// ReportSize()
// -----------------------------------------------------------------------------
//
TInt CReportRoot::ReportSize(TInt aIndex) const
    {
    TInt size = 0;

    if (aIndex < iSizes.Count())
        {
        size = iSizes[aIndex].iSize;
        }

    return size;
    }
// -----------------------------------------------------------------------------
// ReportSize()
// -----------------------------------------------------------------------------
//
TInt CReportRoot::ReportSize(TInt aReportId, CField::TType aType) const
    {
    TInt offset = 0;

    TInt index = FindReportSizeIndex(aReportId, aType);
    if (index != KErrNotFound && index < iSizes.Count() )
        {
        offset = iSizes[index].iSize;
        }
    return offset;
    }

// -----------------------------------------------------------------------------
// ReportSizeBytes()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CReportRoot::ReportSizeBytes(TInt aReportId,
    CField::TType aType) const
    {
    TInt reportBytes = (ReportSize(aReportId, aType) + KSevenBits) / KSizeOfByte;

    // Add an extra byte if the report ID is used
    return ( aReportId ) ? reportBytes + 1 : reportBytes;
    }

// -----------------------------------------------------------------------------
// FindReportSizeIndex()
// -----------------------------------------------------------------------------
//
TInt CReportRoot::FindReportSizeIndex(TInt aReportId, CField::TType aType) const
    {
    TIdentityRelation<TReportSize> matcher(TReportSize::Match);
    TInt index = iSizes.Find(TReportSize(aReportId, aType), matcher);

    TRACE_INFO((_L("FindReportSizeIndex(%d, %d) = %d\n"),
              aReportId, aType, index));
    return index;
    }
