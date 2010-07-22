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
* Description:  Hid field implementation
*
*/


#include <e32std.h>
#include <e32base.h>
#include <e32des8.h>
#include <e32svr.h>

#include "hidreportroot.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
CField* CField::NewL()
    {
    CField* self = NewLC();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
CField* CField::NewLC()
    {
    // Two-phase construction isn't necessary at present:
    CField* self = new (ELeave) CField;
    CleanupStack::PushL(self);
    return self;
    }

// ---------------------------------------------------------------------------
// CField()
// ---------------------------------------------------------------------------
//
CField::CField()
    {
    // nothing else to do
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CField::~CField()
    {
    iUsageList.Close();
    }

// ---------------------------------------------------------------------------
// UsagePage()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::UsagePage() const
    {
    return iUsagePage;
    }

// ---------------------------------------------------------------------------
// ReportId()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::ReportId() const
    {
    return iReportId;
    }

// ---------------------------------------------------------------------------
// Offset()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::Offset() const
    {
    return (iReportId == 0) ? iPos : (iPos+KSizeOfByte);
    }

// ---------------------------------------------------------------------------
// Size()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::Size() const
    {
    return iSize;
    }

// ---------------------------------------------------------------------------
// Count()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::Count() const
    {
    return iCount;
    }

// ---------------------------------------------------------------------------
// LogicalMin()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::LogicalMin() const
    {
    return iLogicalMin;
    }

// ---------------------------------------------------------------------------
// LogicalMax()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::LogicalMax() const
    {
    return iLogicalMax;
    }

// ---------------------------------------------------------------------------
// UsageMin()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::UsageMin() const
    {
    return iUsageMin;
    }

// ---------------------------------------------------------------------------
// UsageMax()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::UsageMax() const
    {
    return iUsageMax;
    }

// ---------------------------------------------------------------------------
// PhysicalMin()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::PhysicalMin() const
    {
    return iPhysicalMin;
    }

// ---------------------------------------------------------------------------
// PhysicalMax()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::PhysicalMax() const
    {
    return iPhysicalMax;
    }

// ---------------------------------------------------------------------------
// Unit()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::Unit() const
    {
    return iUnit;
    }

// ---------------------------------------------------------------------------
// UnitExponent()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::UnitExponent() const
    {
    return iUnitExponent;
    }

// ---------------------------------------------------------------------------
// DesignatorIndex()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::DesignatorIndex() const
    {
    return iDesignatorIndex;
    }

// ---------------------------------------------------------------------------
// DesignatorMin()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::DesignatorMin() const
    {
    return iDesignatorMin;
    }

// ---------------------------------------------------------------------------
// DesignatorMax()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::DesignatorMax() const
    {
    return iDesignatorMax;
    }

// ---------------------------------------------------------------------------
// StringMin()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::StringMin() const
    {
    return iStringMin;
    }

// ---------------------------------------------------------------------------
// StringMax()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::StringMax() const
    {
    return iStringMax;
    }

// ---------------------------------------------------------------------------
// StringIndex()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::StringIndex() const
    {
    return iStringIndex;
    }

// ---------------------------------------------------------------------------
// UsageArray()
// ---------------------------------------------------------------------------
//
EXPORT_C TArray<TInt> CField::UsageArray() const
    {
    return iUsageList.Array();
    }

// ---------------------------------------------------------------------------
// HasUsage()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::HasUsage(const TInt aUsage) const
    {
    return iUsageList.Find(aUsage) != KErrNotFound;
    }

// ---------------------------------------------------------------------------
// AddUsageL
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::AddUsageL(const TInt aUsage)
    {
    User::LeaveIfError(iUsageList.Append(aUsage));
    }

// ---------------------------------------------------------------------------
// ClearUsageList()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::ClearUsageList()
    {
    iUsageList.Reset();
    }

// ---------------------------------------------------------------------------
// LastUsage()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::LastUsage() const
    {
    TInt usage = 0;
    if (iUsageList.Count() > 0)
        {
        usage = iUsageList[iUsageList.Count()-1];
        }
    return usage;
    }

// ---------------------------------------------------------------------------
// Attributes()
// ---------------------------------------------------------------------------
//
EXPORT_C TUint32 CField::Attributes() const
    {
    return iAttributes;
    }

// ---------------------------------------------------------------------------
// Type()
// ---------------------------------------------------------------------------
//
EXPORT_C CField::TType CField::Type() const
    {
    return iType;
    }

// ---------------------------------------------------------------------------
// IsVariable()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsVariable() const
    {
    return (( iAttributes & KFieldAttributeVariable ) != 0);
    }

// ---------------------------------------------------------------------------
// IsArray()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsArray() const
    {
    return ( ( iAttributes & KFieldAttributeVariable ) == 0);
    }

// ---------------------------------------------------------------------------
// IsConstant()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsConstant() const
    {
    return (( iAttributes & KFieldAttributeConstant ) != 0 );
    }

// ---------------------------------------------------------------------------
// IsData()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsData() const
    {
    // Field atribute first bit present if data is constat or is it data.
    // If first bit is zero field attibute data is set.
    return ( ( iAttributes & KFieldAttributeConstant ) == 0 );
    }

// ---------------------------------------------------------------------------
// SetType()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetType(const TType& aType)
    {
    iType = aType;
    }

// ---------------------------------------------------------------------------
// SetUsagePage()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUsagePage(const TInt aUsagePage)
    {
    iUsagePage = aUsagePage;
    }

// ---------------------------------------------------------------------------
// Destructor()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetReportId(const TInt aId)
    {
    iReportId = aId;
    }

// ---------------------------------------------------------------------------
// SetOffset()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetOffset(const TInt aOffset)
    {
    iPos = aOffset;
    }

// ---------------------------------------------------------------------------
// SetSize()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetSize(const TInt aSize)
    {
    iSize = aSize;
    }

// ---------------------------------------------------------------------------
// SetCount()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetCount(const TInt aCount)
    {
    iCount = aCount;
    }

// ---------------------------------------------------------------------------
// SetLogicalMin()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetLogicalMin(const TInt aMin)
    {
    iLogicalMin = aMin;
    }

// ---------------------------------------------------------------------------
// SetLogicalMax()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetLogicalMax(const TInt aMax)
    {
    iLogicalMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetUsageMin()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUsageMin(const TInt aMin)
    {
    iUsageMin = aMin;
    }

// ---------------------------------------------------------------------------
// SetUsageMax()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUsageMax(const TInt aMax)
    {
    iUsageMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetAttributes()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetAttributes(const TUint32 aAttributes)
    {
    iAttributes = aAttributes;
    }

// ---------------------------------------------------------------------------
// SetPhysicalMin()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetPhysicalMin(TInt aValue)
    {
    iPhysicalMin = aValue;
    }

// ---------------------------------------------------------------------------
// SetPhysicalMax()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetPhysicalMax(TInt aValue)
    {
    iPhysicalMax = aValue;
    }

// ---------------------------------------------------------------------------
// SetUnit()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUnit(TInt aValue)
    {
    iUnit = aValue;
    }

// ---------------------------------------------------------------------------
// SetUnitExponent()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUnitExponent(TInt aValue)
    {
    iUnitExponent = aValue;
    }

// ---------------------------------------------------------------------------
// SetDesignatorIndex()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetDesignatorIndex(TInt aValue)
    {
    iDesignatorIndex = aValue;
    }

// ---------------------------------------------------------------------------
// SetDesignatorMin()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetDesignatorMin(TInt aValue)
    {
    iDesignatorMin = aValue;
    }

// ---------------------------------------------------------------------------
// SetDesignatorMax()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetDesignatorMax(TInt aValue)
    {
    iDesignatorMax = aValue;
    }

// ---------------------------------------------------------------------------
// SetStringMin()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetStringMin(TInt aValue)
    {
    iStringMin = aValue;
    }

// ---------------------------------------------------------------------------
// SetStringMax()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetStringMax(TInt aValue)
    {
    iStringMax = aValue;
    }

// ---------------------------------------------------------------------------
// SetStringIndex()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetStringIndex(TInt aValue)
    {
    iStringIndex = aValue;
    }

// ---------------------------------------------------------------------------
// SetLogicalRange()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetLogicalRange(TInt aMin, TInt aMax)
    {
    iLogicalMin = aMin;
    iLogicalMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetUsageRange()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetUsageRange(TInt aMin, TInt aMax)
    {
    iUsageMin = aMin;
    iUsageMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetPhysicalRange
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetPhysicalRange(TInt aMin, TInt aMax)
    {
    iPhysicalMin = aMin;
    iPhysicalMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetStringRange()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetStringRange(TInt aMin, TInt aMax)
    {
    iStringMin = aMin;
    iStringMax = aMax;
    }

// ---------------------------------------------------------------------------
// SetDesignatorRange()
// ---------------------------------------------------------------------------
//
EXPORT_C void CField::SetDesignatorRange(TInt aMin, TInt aMax)
    {
    iDesignatorMin = aMin;
    iDesignatorMax = aMax;
    }

// ---------------------------------------------------------------------------
// IsInput()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsInput() const
    {
    return iType == EInput;
    }

// ---------------------------------------------------------------------------
// IsOutput()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsOutput() const
    {
    return iType == EOutput;
    }

// ---------------------------------------------------------------------------
// IsFeature()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsFeature() const
    {
    return iType == EFeature;
    }

// ---------------------------------------------------------------------------
// UsageCount()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::UsageCount() const
    {
    return iUsageList.Count();
    }

// ---------------------------------------------------------------------------
// Usage()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CField::Usage(TInt aIndex) const
    {
    return iUsageList[aIndex];
    }

// ---------------------------------------------------------------------------
// IsInReport()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CField::IsInReport(TInt aReportId) const
    {
    return ( 0 == iReportId || aReportId == iReportId );
    }

