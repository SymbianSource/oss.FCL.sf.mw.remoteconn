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
* Description:  HID collection implementation
*
*/


#include <e32std.h>
#include <e32base.h>
#include <e32des8.h>
#include <e32svr.h>

#include "hidreportroot.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// NewLC()
// -----------------------------------------------------------------------------
//
CCollection* CCollection::NewLC()
    {
    CCollection* self = new (ELeave) CCollection();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }
// -----------------------------------------------------------------------------
// NewL()
// -----------------------------------------------------------------------------
//
CCollection* CCollection::NewL()
    {
    CCollection* self=NewLC();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// ConstructL()
// -----------------------------------------------------------------------------
//
void CCollection::ConstructL()
    {
    // Nothing to do here
    }

// -----------------------------------------------------------------------------
// CCollection()
// -----------------------------------------------------------------------------
//
CCollection::CCollection()
    {
    // Nothing to do here
    }

// -----------------------------------------------------------------------------
// ~CCollection()
// -----------------------------------------------------------------------------
//
CCollection::~CCollection()
    {
    iCollections.ResetAndDestroy();
    iFields.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// Type()
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CCollection::Type() const
    {
    return iType;
    }

// -----------------------------------------------------------------------------
// UsagePage()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCollection::UsagePage() const
    {
    return iUsagePage;
    }

// -----------------------------------------------------------------------------
// Usage()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCollection::Usage() const
    {
    return iUsage;
    }

// -----------------------------------------------------------------------------
// CollectionCount()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCollection::CollectionCount() const
    {
    return iCollections.Count();
    }

// -----------------------------------------------------------------------------
// FieldCount()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CCollection::FieldCount() const
    {
    return iFields.Count();
    }

// -----------------------------------------------------------------------------
// CollectionByIndex
// -----------------------------------------------------------------------------
//
EXPORT_C const CCollection* CCollection::CollectionByIndex(TInt aIndex) const
    {
    return (0 <= aIndex && aIndex < iCollections.Count()) ?
        iCollections[aIndex] : NULL;
    }

// -----------------------------------------------------------------------------
// FieldByIndex
// -----------------------------------------------------------------------------
//
EXPORT_C const CField* CCollection::FieldByIndex(TInt aIndex) const
    {
    return (0 <= aIndex && aIndex < iFields.Count()) ?
        iFields[aIndex] : NULL;
    }

// -----------------------------------------------------------------------------
// IsPhysical()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsPhysical() const
    {
    return iType == EPhysical;
    }

// -----------------------------------------------------------------------------
// IsApplication()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsApplication() const
    {
    return iType == EApplication;
    }

// -----------------------------------------------------------------------------
// IsLogical()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsLogical() const
    {
    return iType == ELogical;
    }

// -----------------------------------------------------------------------------
// IsReport()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsReport() const
    {
    return iType == EReport;
    }

// -----------------------------------------------------------------------------
// IsNamedArray()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsNamedArray() const
    {
    return iType == ENamedArray;
    }

// -----------------------------------------------------------------------------
// IsUsageSwitch()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsUsageSwitch() const
    {
    return iType == EUsageSwitch;
    }

// -----------------------------------------------------------------------------
// IsUsageModifier()
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CCollection::IsUsageModifier() const
    {
    return iType == EUsageModifier;
    }

// -----------------------------------------------------------------------------
// SetType()
// -----------------------------------------------------------------------------
//
void CCollection::SetType(TUint32 aType)
    {
    iType = aType;
    }

// -----------------------------------------------------------------------------
// SetUsagePage()
// -----------------------------------------------------------------------------
//
void CCollection::SetUsagePage(TInt aUsagePage)
    {
    iUsagePage = aUsagePage;
    }

// -----------------------------------------------------------------------------
// SetUsage()
// -----------------------------------------------------------------------------
//
void CCollection::SetUsage(TInt aUsage)
    {
    iUsage = aUsage;
    }
// -----------------------------------------------------------------------------
// AddFieldL()
// -----------------------------------------------------------------------------
//
CField* CCollection::AddFieldL()
    {
    CField* field = CField::NewL();
    CleanupStack::PushL(field);
    User::LeaveIfError(iFields.Append(field));
    CleanupStack::Pop(field);
    return field;
    }

// -----------------------------------------------------------------------------
// AddCollectionL()
// -----------------------------------------------------------------------------
//
CCollection* CCollection::AddCollectionL()
    {
    CCollection* collection = CCollection::NewL();
    CleanupStack::PushL(collection);
    User::LeaveIfError(iCollections.Append(collection));
    CleanupStack::Pop(collection);
    return collection;
    }





// -----------------------------------------------------------------------------
// Match()
// -----------------------------------------------------------------------------
//
TBool TReportSize::Match(const TReportSize& aFirst,
    const TReportSize& aSecond)
    {
    return (aFirst.iReportId == aSecond.iReportId) &&
        (aFirst.iType == aSecond.iType);
    }


// -----------------------------------------------------------------------------
// TReportSize()
// -----------------------------------------------------------------------------
//
TReportSize::TReportSize(TInt aReportId, CField::TType aType)
    : iReportId(aReportId), iType(aType), iSize(0)
    {
    // Nothing else to do
    }

