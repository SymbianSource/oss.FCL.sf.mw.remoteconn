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
* Description:  HID parser implementation
*
*/


// ----------------------------------------------------------------------

// References:
//
// [1] USB Device Class Definition for Human Interface Devices (HID),
//     Firmware Specification, Version 1.11, USB Implementers' Forum,
//     June 2001
//
// [2] HID Parser Error Codes (HID Parser Error Checking), Revision
//     1.2, USB Implementers' Forum, February 2000
//
// [3] USB HID Usage Tables, Version 1.11, USB Implementers' Forum,
//     June 2001
//
// ----------------------------------------------------------------------

#include <e32std.h>
#include <e32base.h>
#include <e32des8.h>
#include <e32svr.h>

#include "hidreportroot.h"
#include "hiditem.h"
#include "hidparser.h"
#include "debug.h"




// ----------------------------------------------------------------------
/*
// Define PARSER_DEBUG to activate trace output for WINS debug builds:
#undef PARSER_DEBUG

#if defined(PARSER_DEBUG) && defined(_DEBUG) && defined(__WINS__)
#define PDBG(a) a;
#define PDBG_ACTIVE
#else
#define PDBG(a)
#endif
*/
#define PDBG_ACTIVE

const TUint32 CParser::KLocalItemMask =
    EUsageMin | EUsageMax | EUsageId |
    EDesignatorIndex | EDesignatorMin | EDesignatorMax |
    EStringIndex | EStringMin | EStringMax;


const TUint32 KMaxStandardType   = 0x06;
const TUint32 KMinVendorType     = 0x80;
const TUint32 KMaxVendorType     = 0xFF;
const TUint32 KMaxUsagePage      = 0xffff;
const TUint32 KUnitData          = 0x0f;

const TUint32 CParser::KUnusedLocalItemsMask = KLocalItemMask & ~EUsageId;

const TUint32 KInputReservedBitsMask    = 0xffffff00;
const TUint32 KOutputReservedBitsMask   = 0xffffff00;
const TUint32 KFeatureReservedBitsMask  = 0xffffff00;
const TUint32 KUnitReservedBitsMask     = 0xf0000000;

const TInt KConstantFlag    = 1<<0;  // Constant (1) or Data (0)
const TInt KVariableFlag    = 1<<1;  // Array (0) or Variable (1)
const TInt KNullStateFlag   = 1<<6;

const TInt KExtendedDataSize  = 4;   // 32-bit extended data size
const TInt KExtendedDataShift = 16;  // 16 bit shift if extended usage page is used
const TInt KMaxReportIDMax    = 255;
const TInt K32Bit             = 32;

const TUint K32BitFirstBitOn   = 1u<<31;
const TInt  KUnitSystemMin     = 5;
const TInt  KUnitSystem15      = 15;

const TUint32 CParser::KMandatoryItemMask = EUsagePage |
        ELogicalMin | ELogicalMax | EReportSize | EReportCount;
        
const TUint32 CParser::KReportItemMask = EInputReport |
                  EOutputReport | EFeatureReport;        


// Reserved values as per the HUT document 1.11, [3]:
// This ranges are reserverd in future use.

const TInt KReservedUsage = 0x0e;
const TInt KReservedUsageRange1Min = 0x11;
const TInt KReservedUsageRange1Max = 0x13;
const TInt KReservedUsageRange2Min = 0x15;
const TInt KReservedUsageRange2Max = 0x3f;
const TInt KReservedUsageRange3Min = 0x41;
const TInt KReservedUsageRange3Max = 0x7f;
const TInt KReservedUsageRange4Min = 0x88;
const TInt KReservedUsageRange4Max = 0x8b;
const TInt KReservedUsageRange5Min = 0x92;
const TInt KReservedUsageRange5Max = 0xfeff;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CParser* CParser::NewLC()
    {
    CParser* self = new (ELeave) CParser;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CParser* CParser::NewL()
    {
    CParser* self = NewLC();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CParser::ConstructL()
    {
    TRACE_FUNC_THIS
    iLocal = CField::NewL();
    }

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CParser::CParser():
    iFieldCount(0)
    {
    TRACE_FUNC_THIS
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CParser:: ~CParser()
    {
    TRACE_FUNC_THIS

    // Free all RArray storage:
    iGlobalStack.Reset();


    // Although iCollectionStack is an RPointerArray, we aren't doing
    // a ResetAndDestroy() here, as all the collections are owned by
    // the report root object, iReportRoot:
    iCollectionStack.Reset();

    delete iReportRoot;
    delete iLocal;
    }

// ---------------------------------------------------------------------------
// CreateCollectionL
// ---------------------------------------------------------------------------
//
TInt CParser::CreateCollectionL(TUint32 aType)
    {
    TInt err = CheckForCollectionErrors(aType);

    if (err == KErrNone)
        {
        CCollection* collection = Collection()->AddCollectionL(); // Created collection added
                                                                  // Collection's collection array
        collection->SetType(aType);
        collection->SetUsagePage(iGlobal.iUsagePage);
        collection->SetUsage(iLocal->LastUsage());
        PushCollectionL(collection);         
        }
    return err;
    }

// ---------------------------------------------------------------------------
// CheckForMainErrors()
// ---------------------------------------------------------------------------
//
TInt CParser::CheckForMainErrors()
    {
    if ( ( iItemsDefined & ELogicalMin ) && ( iItemsDefined & ELogicalMax ) )
        {
        if ( iGlobal.iLogicalMin > iGlobal.iLogicalMax )
            {
            IssueWarning( ELogicalMinExceedsMax );
            }
        }
    if ( iItemsDefined & (EPhysicalMin | EPhysicalMax ) )
        {
        if ( !( iItemsDefined & EPhysicalMax ) )
            {
            return ELonelyPhysicalMin;
            }
        if (!( iItemsDefined & EPhysicalMin ))
            {
            return ELonelyPhysicalMax;
            }

        if ( iGlobal.iPhysicalMin > iGlobal.iPhysicalMax )
            {
            IssueWarning(EPhysicalMinExceedsMax);
            }
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CheckForCollectionErrors()
// ---------------------------------------------------------------------------
//
TInt CParser::CheckForCollectionErrors(TUint32 aType)
    {    
    if (iCollectionStack.Count() == 0 )
        {
        return ENoCollectionToCheck;
        }

    if (aType > KMaxStandardType)
        {
        if ((aType < KMinVendorType) || (aType > KMaxVendorType))
            {
            IssueWarning( ECollectionTypeUnknownReserved );
            }
        }
    
    if ( iItemsDefined & KUnusedLocalItemsMask )
        {
        IssueWarning( ECollectionLocalUnused );
        }

    TInt numUsages = iLocal->UsageCount();

    if ( numUsages > 1 )
        {
        // Only a single usage value can be associated with a collection:
        IssueWarning( ECollectionLocalUnused );
        }

    if ( numUsages == 0 )
        {
        // A usage tag must be associated with a collection (see [1],
        // Section 6.2.2.6):
        IssueWarning( ECollectionHasNoUsage );
        }

    if ( !( iItemsDefined & EUsagePage ) )
        {
        // A usage page must be associated with a collection (see [1],
        // Section 6.2.2.6):
        IssueWarning( ECollectionHasNoUsagePage );
        }

    if (( aType == CCollection::EApplication ) && ( iItemsDefined & EDelimiter ))
        {
        // Delimiters can't be used when defining usages that apply to
        // Application Collections ([1], Section 6.2.2.8):
        IssueWarning(EApplicationHasDelimiter);

        // It is an error to declare a delimiter for a top-level
        // application collection, [2]:
        if (iCollectionStack.Count() == 1)
            {
            return EDelimiterAtTopLevel;
            }
        }
    return CheckForMainErrors();
    }


// ---------------------------------------------------------------------------
// CheckForFieldErrors()
// ---------------------------------------------------------------------------
//
TInt CParser::CheckForFieldErrors(CField::TType aType, TUint32 aAttributes)
    {  
    TInt ret = KErrNone;
    ret = CheckMandatoryFieldErrors(aType, aAttributes);
    if ( ret != KErrNone )
        {
        return ret;    
        }
    
    const TInt KLimitsError[] =
        { EInputMinExceedsMax, EOutputMinExceedsMax, EFeatureMinExceedsMax };

    if ( iGlobal.iLogicalMin > iGlobal.iLogicalMax )
        {
        return KLimitsError[aType];
        }

    if ( ( iItemsDefined & ( EPhysicalMin | EPhysicalMax ) )
        && ( iGlobal.iPhysicalMin > iGlobal.iPhysicalMax ))
        {
        return KLimitsError[aType];
        }   
    CheckLogicalMinAndMax( aAttributes );     
    CheckFieldBitNeeded( aType, aAttributes );
    return CheckForMainErrors();
    }


// ---------------------------------------------------------------------------
// BitsToRepresentRange()
// ---------------------------------------------------------------------------
//
TInt CParser::BitsToRepresentRange(TInt aMin, TInt aMax)
    {
    // The number of bits required to represent all values in the
    // range aMin to aMax inclusive.  If the range is all positive
    // then there is no sign bit, otherwise twos complement format is
    // assumed. ([1], Section 6.2.2.7.)

    TInt bitsNeeded = 0;

    if (aMin != aMax)
        {
        TUint absVal = static_cast<TUint>(Max(Abs(aMin), Abs(aMax)));

        bitsNeeded = K32Bit - NumberOfLeadingZeros(absVal);

        // If either are negative, we'll need space for the sign bit:
        //
        if ((aMax < 0) || (aMin < 0))
            {
            bitsNeeded++;

            // However, 2s complement allows us to represent one extra
            // negative number than positive, and so our calculation
            // may be one bit over. Catch this with a special case:
            //
            if (bitsNeeded > 1)
                {
                TInt n = 1 << (bitsNeeded - 2);
                if ((aMin == -n) && (aMax < n))
                    {
                    bitsNeeded--;
                    }
                }
            }
        }

    return bitsNeeded;
    }

// ---------------------------------------------------------------------------
// NumberOfLeadingZeros()
// ---------------------------------------------------------------------------
//
TInt CParser::NumberOfLeadingZeros(TUint32 aValue)
    {
    TInt count = 0;

    TUint32 pos = K32BitFirstBitOn;
    while ((pos != 0) && ((aValue & pos) == 0))
        {
        count++;
        pos >>= 1;
        }

    return count;
    }


// ---------------------------------------------------------------------------
// CheckAllReportSizes()
// ---------------------------------------------------------------------------
//
TBool CParser::CheckAllReportSizes() const
    {
    // Final report sizes must be an integral number of bytes, [2]:

    TBool sizesOk = ETrue;

    for (TInt i=0; sizesOk && (i<iReportRoot->NumberOfReports()); ++i)
        {
        TInt bits = iReportRoot->ReportSize(i);

        if ((bits == 0) || ((bits % 8) != 0))
            {
            sizesOk = EFalse;
            }
        }
    return sizesOk;
    }


// ---------------------------------------------------------------------------
// CreateFieldL()
// ---------------------------------------------------------------------------
//
TInt CParser::CreateFieldL(CField::TType aType, TUint32 aAttributes)
    {
    TInt err = CheckForFieldErrors( aType, aAttributes );
    //Microsoft Elite 2 keyboard HID bug fix
	if ( err == EInputMissingItems && iGlobal.iUsagePage == 0x07 )
		if ( iLocal->UsageMin( ) == 0xe0 && iLocal->UsageMax( ) == 0xe7 )
			{
			iGlobal.iLogicalMin = 0x0;
			iGlobal.iLogicalMax = 0x1;
			err = KErrNone;
			}
		else
			{
			iGlobal.iLogicalMin = 0x0;
			err = KErrNone;
			}

    if (err == KErrNone)
        {
        // Create a new field object:
        CField* field = Collection()->AddFieldL( );   // Created field added
                                                      // to collection's field array
        DumpStateTableL( field );
        field->SetType( aType );
        field->SetAttributes( aAttributes );

        // Set the field offset to the current report size, and
        // increase the report size by the size of this field:        
        if ( !iReportRoot )
            {
            User::Leave(ENoReportRoot);
            }
        field->SetOffset( iReportRoot->ReportSize( field->ReportId( ), aType) );
        iReportRoot->IncrementReportSizeL( field->ReportId(),
            aType, field->Count() * field->Size() );
        TRACE_INFO(_L("CParser::CreateFieldL Field added"));
        if ( field->UsageCount() )
            {
            iFieldCount++;
            }        
        }
    return err;
    }

// ---------------------------------------------------------------------------
// DumpStateTableL()
// ---------------------------------------------------------------------------
//
void CParser::DumpStateTableL(CField *aField) const
    {
    TRACE_INFO((_L("DumpStateTableL(0x%08x)\n"), aField));

    // Copy global state:
    //
    iGlobal.Populate(aField);

    // Copy local state:
    //
    aField->SetUsageRange( iLocal->UsageMin(), iLocal->UsageMax() );
    aField->SetDesignatorIndex( iLocal->DesignatorIndex() );
    aField->SetDesignatorRange( iLocal->DesignatorMin(),
        iLocal->DesignatorMax() );
    aField->SetStringIndex( iLocal->StringIndex() );
    aField->SetStringRange( iLocal->StringMin(), iLocal->StringMax() );

    // Copy usage list (local state) and calculate the usage range, if
    // it hasn't already been explicitly specified:
    //
    if ( iLocal->UsageCount() > 0 )
        {
        TInt minUsage, maxUsage;
        minUsage = maxUsage = iLocal->Usage( 0 );

        for (TInt i=0; i<iLocal->UsageCount(); ++i)
            {
            TInt value = iLocal->Usage( i );
            aField->AddUsageL( value );
            if ( value < minUsage )
                {
                minUsage = value;
                }
            if ( value > maxUsage )
                {
                maxUsage = value;
                }
            }
        if ( (iItemsDefined & (EUsageMin | EUsageMax) ) == 0)
            {
            aField->SetUsageRange( minUsage, maxUsage );
            }
        }
    }


// ---------------------------------------------------------------------------
// Collection()
// ---------------------------------------------------------------------------
//
CCollection* CParser::Collection()
    {    
    CCollection* lastcollection = NULL;
    if ( iCollectionStack.Count( ) > 0 )
        {
        lastcollection = iCollectionStack[ iCollectionStack.Count() - 1 ];
        }
    return lastcollection;
    }

// ---------------------------------------------------------------------------
// PushCollectionL()
// ---------------------------------------------------------------------------
//
void CParser::PushCollectionL(const CCollection* aCollection)
    {
    User::LeaveIfError( iCollectionStack.Append( aCollection ) );
    }

// ---------------------------------------------------------------------------
// PopCollection()
// ---------------------------------------------------------------------------
//
void CParser::PopCollection()
    {
    
    if ( iCollectionStack.Count() > 0 )
        {
        iCollectionStack.Remove( iCollectionStack.Count() - 1 );
        }
    }


// ---------------------------------------------------------------------------
// IssueWarning()
// ---------------------------------------------------------------------------
//
void CParser::IssueWarning(TInt aHidWarningCode)
    {
    TRACE_ERROR((_L("Item %3d: Warning 0x%04x\n"), iItemNumber, aHidWarningCode));
    (void)aHidWarningCode;
    }

// ---------------------------------------------------------------------------
// ParseL()
// ---------------------------------------------------------------------------
//
EXPORT_C CReportRoot* CParser::ParseL(const TDesC8& aRawData)
    {
    ResetParserL();    
    // Now we can parse the descriptor data:
    const TInt length = aRawData.Length();
    TInt posn = 0;
    TRACE_INFO((_L("CParser::ParseL() Start parsing length %d"), length));
    while ((posn < length) && (iErrorCode == 0))
        {
        iItemNumber++;
        TRACE_INFO((_L("posn is %d"), posn));

        TItem item(aRawData.Right(length-posn));

#ifdef PDBG_ACTIVE
        TRACE_INFO((_L("Item: size %d, tag %d, type %d\n"),
                   item.DataSize(), item.Tag(), item.Type()));
        for (TInt i=0; i<item.DataSize(); ++i)
            {
            TRACE_INFO((_L("  Data[%d] = 0x%02x (%d)\n"),
                       i, item[i], item[i]))
            }
#endif

        HandleItemL( item );        
        posn += item.ItemSize();
        if (posn > length)
            {
            iErrorCode = EInvalidItemLength;
            }
        }

    PopCollection();

    // PUSH without POP:
    CheckParseErrors();

    iGlobalStack.Reset();
    iCollectionStack.Reset();

    TRACE_INFO((_L("CParser::ParseL() error code is %d"), iErrorCode));
    // Finished, transfer ownership to caller:
    CReportRoot* reportRoot = iReportRoot;
    iReportRoot = 0;
    return reportRoot;
    }

// ---------------------------------------------------------------------------
// CParser::FieldCount()
// ---------------------------------------------------------------------------
//
TInt CParser::FieldCount()
    {
    return iFieldCount;
    }

// ---------------------------------------------------------------------------
// CParser::MainItemL()
// ---------------------------------------------------------------------------
//
TInt CParser::MainItemL(const TItem& aItem)
    {
    TInt retVal = 0;

    switch (aItem.Tag())
        {
        case EMainInput:
            retVal = HandleMainInputTagL( aItem );
            break;

        case EMainOutput:
            retVal = HandleMainOutputTagL( aItem );
            break;

        case EMainFeature:
            retVal = HandleMainFeatureL( aItem );
            break;

        case EMainCollection:
            retVal = HandleMainCollectionL( aItem );
            break;

        case EMainEndCollection:
            retVal = HandleMainEndCollection( aItem );
            break;
        default:
            TRACE_ERROR(_L("Error: unknown main item\n"));
            retVal = EUnknownItem;
            break;
        }

    // All main items cause local state to be cleared:
    ClearLocalState();

    // For checking if global items declared more than once between
    // main items:
    iGlobalItemsDefined = 0;

    return retVal;
    }
// ---------------------------------------------------------------------------
// ClearLocalState()
// ---------------------------------------------------------------------------
//
void CParser::ClearLocalState()
    {
    iLocal->ClearUsageList();
    iLocal->SetUsageRange(0, 0);
    iLocal->SetStringIndex(0);
    iLocal->SetStringRange(0, 0);
    iLocal->SetDesignatorIndex(0);
    iLocal->SetDesignatorRange(0, 0);

    iItemsDefined &= ~(KLocalItemMask | EDelimiter);
    iLocalMultipleUse = EFalse;
    }

// ---------------------------------------------------------------------------
// GlobalItemL()
// ---------------------------------------------------------------------------
//
TInt CParser::GlobalItemL(const TItem& aItem)
    {
    TInt retVal = 0;
    switch (aItem.Tag())
        {
        case EGlobalReportId:
            retVal = HandleGlobalReportId( aItem );
            break;
        case EGlobalUsagePage:
            retVal = HandleGlobalUsagePage( aItem );
            break;
        case EGlobalLogicalMinimum:
            retVal = HandleGlobalLogicalMinimum( aItem );
            break;
        case EGlobalLogicalMaximum:
            retVal = HandleGlobalLogicalMaximum( aItem );
            break;
        case EGlobalPhysicalMinimum:
            retVal = HandleGlobalPhysicalMinimum( aItem );
            break;
        case EGlobalPhysicalMaximum:
            retVal = HandleGlobalPhysicalMaximum( aItem );
            break;
        case EGlobalUnit:
            retVal = HandleGlobalUnit( aItem );
            break;
        case EGlobalUnitExponent:
            retVal = HandleGlobalUnitExponent( aItem );
            break;
        case EGlobalReportSize:
            retVal = HandleGlobalReportSize( aItem );
            break;
        case EGlobalReportCount:
            retVal = HandleGlobalReportCount(aItem );
            break;
        case EGlobalPush:
            retVal = HandleGlobalPushL( aItem );
            break;
        case EGlobalPop:
            retVal = HandleGlobalPop( aItem );
            break;
        default:
            TRACE_ERROR(_L("Error: unknown global item\n"));
            retVal = EUnknownItem;
            break;
        }

    return retVal;
    }

// ---------------------------------------------------------------------------
// LocalItemL()
// ---------------------------------------------------------------------------
//
TInt CParser::LocalItemL(const TItem& aItem)
    {
    TInt retVal = 0;
    switch (aItem.Tag())
        {
        case ELocalUsage:
            retVal = HandleLocalUsageL( aItem );
            break;
        case ELocalUsageMinimum:
            retVal = HandleLocalUsageMinimum( aItem );
            break;
        case ELocalUsageMaximum:
            retVal = HandleLocalUsageMaximum( aItem );
            break;
        case ELocalDesignatorIndex:
            retVal = HandleLocalDesignatorIndex( aItem );
            break;
        case ELocalDesignatorMinimum:
            retVal = HandleLocalDesignatorMinimum( aItem );
            break;
        case ELocalDesignatorMaximum:
            retVal = HandleLocalDesignatorMaximum( aItem );
            break;
        case ELocalStringIndex:
            retVal = HandleLocalStringIndex( aItem );
            break;
        case ELocalStringMinimum:
            retVal = HandleLocalStringMinimum( aItem );
            break;
        case ELocalStringMaximum:
            retVal = HandleLocalStringMaximum( aItem );
            break;
        // "HID parsers must handle Delimiters however, the support
        // for the alternative usages that they define is optional.
        // Usages other than the first (most preferred) usage defined
        // may not be made accessible by system software.", [1],
        // Section 6.2.2.8.
        //
        // This parser only supports the first usage in a delimiter list.
        case ELocalDelimiter:
            retVal = HandleLocalDelimiter( aItem );
            break;
        default:
            TRACE_ERROR(_L("Error: unknown local item\n"));
            retVal = EUnknownItem;
            break;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// IsReservedUsagePage()
// ---------------------------------------------------------------------------
//
TBool CParser::IsReservedUsagePage(TInt aId)
    {   
    return (aId == KReservedUsage) ||                     
        ((aId >= KReservedUsageRange1Min ) && (aId <= KReservedUsageRange1Max)) ||     
        ((aId >= KReservedUsageRange2Min ) && (aId <= KReservedUsageRange2Max)) ||     
        ((aId >= KReservedUsageRange3Min) && (aId <= KReservedUsageRange3Max)) ||     
        ((aId >= KReservedUsageRange4Min ) && (aId <= KReservedUsageRange4Max)) ||     
        ((aId >= KReservedUsageRange5Min) && (aId <= KReservedUsageRange5Max));     
    }

// ---------------------------------------------------------------------------
// Populate()
// ---------------------------------------------------------------------------
//
void TParserGlobalState::Populate(CField *aField) const
    {
    aField->SetUsagePage(iUsagePage);
    aField->SetReportId(iReportId);
    aField->SetLogicalRange(iLogicalMin, iLogicalMax);
    aField->SetSize(iSize);
    aField->SetCount(iCount);
    aField->SetUnit(iUnit);
    aField->SetUnitExponent(iUnitExponent);

    // If the physical min and max are both zero, then the HID class
    // document specifies that they should be assumed to be equal to
    // the corresponding logical values ([1], Section 6.2.2.7):
    //
    if ((iPhysicalMin == 0) && (iPhysicalMax == 0))
        {
        aField->SetPhysicalRange(iLogicalMin, iLogicalMax);
        }
    else
        {
        aField->SetPhysicalRange(iPhysicalMin, iPhysicalMax);
        }
    }


// ---------------------------------------------------------------------------
// TParserGlobalState()
// ---------------------------------------------------------------------------
//
TParserGlobalState::TParserGlobalState()
    : iUsagePage(0), iLogicalMin(0), iLogicalMax(0),
      iPhysicalMin(0), iPhysicalMax(0), iUnit(0),
      iUnitExponent(0), iReportId(0), iSize(0), iCount(0)
    {
    // Nothing else to do
    }

// ---------------------------------------------------------------------------
// HandleMainInputTagL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleMainInputTagL(const TItem& aItem)
    {
    // Section 6.2.2.4 of the HID class specification, [1],
    // states that an Input item may have a data size of zero
    // bytes:
    //
    //   "In this case the value of each data bit for the item
    //   can be assumed to be zero. This is functionally
    //   identical to using a item tag that specifies a 4-byte
    //   data item followed by four zero bytes."
    //
    // For a data size of zero, TItem::Data() will return zero
    // and so we will get the required behaviour.
    TRACE_INFO((_L("Input %d\n"), aItem.Data()));
    TInt retVal=0;

    iItemsDefined |= EInputReport;
    
    if ( aItem.Data() & KInputReservedBitsMask )
        {
        IssueWarning( EInputReservedBitsNonZero );
        }
    if ( iLocalMultipleUse )
        {
        IssueWarning( EInputLocalMultipleUse );
        }
    if ( iWithinDelimiter )
        {
        retVal = EInputItemWithinDelimiter;
        }
    else
        {
        retVal = CreateFieldL( CField::EInput, aItem.Data() );
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleMainOutputTagL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleMainOutputTagL(const TItem& aItem)
    {
    TRACE_INFO((_L("Output %d\n"), aItem.Data()));
    TInt retVal=0;
    iItemsDefined |= EOutputReport;
    
    if ( aItem.Data() & KOutputReservedBitsMask )
        {
        IssueWarning( EOutputReservedBitsNonZero );
        }

    if ( iLocalMultipleUse )
        {
        IssueWarning( EOutputLocalMultipleUse );
        }

    if ( iWithinDelimiter )
        {
        retVal = EOutputItemWithinDelimiter;
        }
    else
       {
       retVal = CreateFieldL( CField::EOutput, aItem.Data() );
       }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleMainFeatureL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleMainFeatureL( const TItem& aItem )
    {
    TRACE_INFO((_L("Feature %d\n"), aItem.Data()));
    TInt retVal=0;
    iItemsDefined |= EFeatureReport;
        
    if ( aItem.Data() & KFeatureReservedBitsMask )
        {
        IssueWarning(EFeatureReservedBitsNonZero );
        }

    if ( iLocalMultipleUse )
        {
        IssueWarning( EFeatureLocalMultipleUse );
        }

    if ( iWithinDelimiter )
        {
        retVal = EFeatureItemWithinDelimiter;
        }
     else
        {
        retVal = CreateFieldL( CField::EFeature, aItem.Data() );
        }

    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleMainCollectionL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleMainCollectionL( const TItem& aItem )
    {
    TRACE_INFO((_L("Start collection %d\n"), aItem.Data()));
    TInt retVal = 0;

    if ( iWithinDelimiter )
        {
        retVal = EBeginCollectionWithinDelimiter;
        }
    else
        {
        // Application collections can only be declared at
        // top-level:
        if ((aItem.Data() == CCollection::EApplication) &&
           (iCollectionStack.Count() != 1))
            {
            retVal = EApplicationCollectionLevel;
            }
        else
            {
            retVal = CreateCollectionL(aItem.Data());
            }
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleMainEndCollection
// ---------------------------------------------------------------------------
//
TInt CParser::HandleMainEndCollection( const TItem& aItem )
    {
    TRACE_INFO((_L("Start collection %d\n"), aItem.Data()));
    TInt retVal = 0;

    if (aItem.DataSize() != 0)
        {
        IssueWarning(EEndCollectionHasData);
        }

    if (iItemsDefined & KLocalItemMask)
        {
        IssueWarning(EEndCollectionLocalUnused);
        }

    if (iCollectionStack.Count() > 1)
        {
        PopCollection();
        }
    else
        {
        retVal = ENoMatchingBeginCollection;
        }

    if (iWithinDelimiter)
        {
        retVal = EEndCollectionWithinDelimiter;
        }

    return  retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalReportId
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalReportId( const TItem& aItem )
    {
    TRACE_INFO((_L("Global report ID %d\n"), aItem.Data()));
    TInt retVal = 0;
    TUint reportId = aItem.Data();

    if (reportId == 0)
        {
        retVal = EZeroReportId;
        }
    if (reportId > KMaxReportIDMax)
        {
        retVal = EReportIdTooBig;
        }

    // If there are to be any report IDs specified at all,
    // then a report ID must be defined before the first
    // input, output or feature report:
    //    
    if ((iGlobal.iReportId == 0) && (iItemsDefined & KReportItemMask))
        {
        retVal = ELateReportId;
        }

    // Report ID defined outside a top level collection (Microsoft
    // restriction)
    //
    if (iCollectionStack.Count() == 1)
        {
        retVal = EReportIdOutsideTopLevel;
        }

    // Same item shouldn't have been declared since last main item:
    //
    if (iGlobalItemsDefined & EReportId)
        {
        // This is an error according to [2], but as it isn't
        // a critical problem, and as some real-world devices
        // fail this check, we issue a warning instead:
        IssueWarning(ERedundantGlobalItem);
        }
    iGlobalItemsDefined |= EReportId;

    iItemsDefined |= EReportId;
    iGlobal.iReportId = reportId;

    return  retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalUsagePage
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalUsagePage( const TItem& aItem )
    {
    TRACE_INFO((_L("Global usage page %d\n"), aItem.Data()));
    TInt retVal = 0;

    iGlobal.iUsagePage = aItem.Data();

    if (aItem.Data() == 0)
        {
        retVal = EZeroUsagePage;
        }
    
    if (aItem.Data() > KMaxUsagePage)
        {
        retVal = EUsagePageOutOfRange;
        }

    if (IsReservedUsagePage(aItem.Data()))
        {
        IssueWarning(EReservedUsagePage);
        }

    iItemsDefined |= EUsagePage;

    if (iGlobalItemsDefined & EUsagePage)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EUsagePage;

    return  retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalLogicalMinimum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalLogicalMinimum( const TItem& aItem )
    {
    TRACE_INFO((_L("Global logical min %d\n"), aItem.SignedData()));
    TInt retVal = 0;
    iGlobal.iLogicalMin = aItem.SignedData();
    iItemsDefined |= ELogicalMin;

    if (iGlobalItemsDefined & ELogicalMin)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= ELogicalMin;

    // "Until Physical Minimum and Physical Maximum are
    // declared in a report descriptor they are assumed by the
    // HID parser to be equal to Logical Minimum and Logical
    // Maximum, respectively.", [1], Section 6.2.2.7.
    //
    if (!(iItemsDefined & EPhysicalMin))
        {
        iGlobal.iPhysicalMin = aItem.SignedData();
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalLogicalMaximum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalLogicalMaximum( const TItem& aItem )
    {
    TRACE_INFO((_L("Global logical max %d\n"), aItem.SignedData()));
    TInt retVal = 0;

    iGlobal.iLogicalMax = aItem.SignedData();
    if ( !(iItemsDefined & EPhysicalMax) )
        {
        iGlobal.iPhysicalMax = aItem.SignedData();
        }
    iItemsDefined |= ELogicalMax;

    if (iGlobalItemsDefined & ELogicalMax)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= ELogicalMax;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalPhysicalMinimum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalPhysicalMinimum( const TItem& aItem )
    {
    TRACE_INFO((_L("Global physical min %d\n"), aItem.SignedData()));
    TInt retVal = 0;
    iGlobal.iPhysicalMin = aItem.SignedData();
    iItemsDefined |= EPhysicalMin;

    if (iGlobalItemsDefined & EPhysicalMin)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EPhysicalMin;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalPhysicalMaximum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalPhysicalMaximum( const TItem& aItem )
    {
    TRACE_INFO((_L("Global physical max %d\n"), aItem.SignedData()));

    TInt retVal = 0;
    iGlobal.iPhysicalMax = aItem.SignedData();
    iItemsDefined |= EPhysicalMax;

    if ( iGlobalItemsDefined & EPhysicalMax )
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EPhysicalMax;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalUnit
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalUnit( const TItem& aItem )
    {
    TRACE_INFO((_L("Global unit %d\n"), aItem.Data()));
    TInt retVal = 0;
    iGlobal.iUnit = aItem.Data();
    iItemsDefined |= EUnit;

    if (iGlobalItemsDefined & EUnit)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EUnit;

    TInt unitSystem = aItem.Data() & KUnitData;
    if (((unitSystem >= KUnitSystemMin ) && (unitSystem != KUnitSystem15)) ||
        (aItem.Data() & KUnitReservedBitsMask ))
        {               
        IssueWarning(EUnitReservedBitsNonZero);
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalUnitExponent
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalUnitExponent( const TItem& aItem )
    {
    TRACE_INFO((_L("Global unit exponent %d\n"), aItem.Data()));
    TInt retVal = 0;
    iGlobal.iUnitExponent = aItem.Data();
    iItemsDefined |= EUnitExponent;

    if (iGlobalItemsDefined & EUnitExponent)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EUnitExponent;
    const TUint32 KReservedBits = ~0x0fUL;
    if (aItem.Data() & KReservedBits)
        {
        IssueWarning( EExponentReservedBitsNonZero );
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalReportSize
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalReportSize( const TItem& aItem )
    {
    TRACE_INFO((_L("Global report size %d\n"), aItem.Data()));
    TInt retVal = 0;
    iGlobal.iSize = aItem.Data();
    iItemsDefined |= EReportSize;

    if (iGlobalItemsDefined & EReportSize)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EReportSize;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalReportCount
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalReportCount( const TItem& aItem )
    {
    TRACE_INFO((_L("Global report count %d\n"), aItem.Data()));
    TInt retVal = 0;
    iGlobal.iCount = aItem.Data();
    if (aItem.Data() == 0)
        {
        retVal = EZeroReportCount;
        }
    iItemsDefined |= EReportCount;

    if (iGlobalItemsDefined & EReportCount)
        {
        retVal = ERedundantGlobalItem;
        }
    iGlobalItemsDefined |= EReportCount;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalPushL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalPushL( const TItem& aItem )
    {
    TRACE_INFO(_L("Global push\n"));
    TInt retVal = 0;
    if (aItem.DataSize() != 0)
        {
        retVal = EPushHasData;
        }
    User::LeaveIfError(iGlobalStack.Append(iGlobal));
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleGlobalPop
// ---------------------------------------------------------------------------
//
TInt CParser::HandleGlobalPop( const TItem& aItem )
    {
    TRACE_INFO(_L("Global pop\n"));
    TInt retVal = 0;
    if (aItem.DataSize() != 0)
        {
        retVal = EPopHasData;
        }

    if (iGlobalStack.Count() > 0)
        {
        iGlobal = iGlobalStack[iGlobalStack.Count()-1];
        iGlobalStack.Remove(iGlobalStack.Count()-1);
        }
    else
        {
        retVal = EPopWithoutPush;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalUsageL
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalUsageL( const TItem& aItem )
    {
    TRACE_INFO((_L("Local usage %d\n"), aItem.Data()));
    TInt retVal = 0;
    
    if (aItem.DataSize() == KExtendedDataSize )
        {
        // Extended (32-bit) usage:
        TInt usagePage = (aItem.Data() >> KExtendedDataShift);
        if (IsReservedUsagePage(usagePage))
            {
            IssueWarning(EReservedUsagePage);
            }
        }

    if ((aItem.Data() & 0xffff) == 0)
        {
        IssueWarning(EZeroUsage);
        }

    if (!iWithinDelimiter || (iAliasCount++ == 0))
        {
        iLocal->AddUsageL(aItem.Data());
        iItemsDefined |= EUsageId;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalUsageMinimum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalUsageMinimum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local usage min %d\n"), aItem.Data()));
    TInt retVal = 0;
    if (!iWithinDelimiter || (iAliasCountMin++ == 0))
        {
        TInt usagePage = iGlobal.iUsagePage;

        if (aItem.DataSize() == KExtendedDataSize )
            {
            // Extended usage specified.
            usagePage = aItem.Data() >> KExtendedDataShift;
            }

        if (iItemsDefined & EUsageMax)
            {
            TInt maxPage =
            static_cast<TUint32>(iLocal->UsageMax()) >> KExtendedDataShift;
            if (maxPage == 0)
                {
                maxPage = iGlobal.iUsagePage;
                }
            if (usagePage != maxPage)
                {
                retVal = EUsagePageMismatchMin;
                }
            }
        iLocal->SetUsageMin(aItem.Data());
        }
    if (iItemsDefined & EUsageMin)
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EUsageMin;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalUsageMaximum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalUsageMaximum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local usage max %d\n"), aItem.Data()));
    
    TInt retVal = 0;
    
    if (!iWithinDelimiter || (iAliasCountMax++ == 0))
        {
        TInt usagePage = iGlobal.iUsagePage;

        if (aItem.DataSize() == KExtendedDataSize )
            {
            // Extended usage specified.
            usagePage = aItem.Data() >> KExtendedDataShift;
            }
        if (iItemsDefined & EUsageMin)
            {
            TInt minPage =
                static_cast<TUint32>(iLocal->UsageMin()) >> KExtendedDataShift;
            if (minPage == 0)
                {
                minPage = iGlobal.iUsagePage;
                }
            if (usagePage != minPage)
                {
                retVal = EUsagePageMismatchMax;
                }
            }
        iLocal->SetUsageMax(aItem.Data());
        }
    if (iItemsDefined & EUsageMax)
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EUsageMax;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalDesignatorIndex
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalDesignatorIndex( const TItem& aItem )
    {
    TRACE_INFO((_L("Local designator index %d\n"),
            aItem.Data()));
    TInt retVal = 0;
    iLocal->SetDesignatorIndex(aItem.Data());
    if (iWithinDelimiter)
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    if (iItemsDefined & EDesignatorIndex)
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EDesignatorIndex;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalDesignatorMinimum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalDesignatorMinimum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local designator min %d\n"), aItem.Data()));
    TInt retVal = 0;
    iLocal->SetDesignatorMin(aItem.Data());
    if ( iWithinDelimiter )
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    if (iItemsDefined & EDesignatorMin)
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EDesignatorMin;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalDesignatorMaximum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalDesignatorMaximum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local designator max %d\n"), aItem.Data()));
    TInt retVal = 0;
    iLocal->SetDesignatorMax(aItem.Data());
    if ( iWithinDelimiter )
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    if ( iItemsDefined & EDesignatorMax )
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EDesignatorMax;
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalStringIndex
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalStringIndex( const TItem& aItem )
    {
    TRACE_INFO((_L("Local string index %d\n"), aItem.Data()));
    TInt retVal = 0;
    iLocal->SetStringIndex(aItem.Data());
    if ( iItemsDefined & EStringIndex )
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EStringIndex;
    if ( iWithinDelimiter )
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalStringMinimum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalStringMinimum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local string min %d\n"),
            aItem.Data()));
    TInt retVal = 0;
    iLocal->SetStringMin(aItem.Data());
    if ( iItemsDefined & EStringMin)
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EStringMin;
    if (iWithinDelimiter)
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalStringMaximum
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalStringMaximum( const TItem& aItem )
    {
    TRACE_INFO((_L("Local string max %d\n"),
            aItem.Data()));
    TInt retVal = 0;
    iLocal->SetStringMax(aItem.Data());
    if ( iItemsDefined & EStringMax )
        {
        iLocalMultipleUse = ETrue;
        }
    iItemsDefined |= EStringMax;
    if ( iWithinDelimiter )
        {
        retVal = EInvalidItemWithinDelimiter;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// HandleLocalDelimiter
// ---------------------------------------------------------------------------
//
TInt CParser::HandleLocalDelimiter( const TItem& aItem )
    {
    const TInt KBeginDelimiter = 1;
    const TInt KEndDelimiter = 0;
    TInt retVal = 0;

    switch ( aItem.Data() )
        {
        case KBeginDelimiter:
            if ( iWithinDelimiter )
                {
                retVal = ENestedDelimiter;
                }
            // Delimiters can't be used when defining usages
            // that apply to array items ([1], Section 6.2.2.8):
            //
            if ( Collection()->Type() == CCollection::ENamedArray )
                {
                IssueWarning( EDelimiterWithinNamedArray );
                }
            iWithinDelimiter = ETrue;
            iAliasCount = 0;
            iAliasCountMin = 0;
            iAliasCountMax = 0;
            break;

        case KEndDelimiter:
            if ( !iWithinDelimiter )
                {
                retVal = ELonelyDelimiter;
                }
            iWithinDelimiter = EFalse;
            break;

        default:
            retVal = EUnknownDelimiter;
            TRACE_ERROR((_L("Error: Unknown delimiter type %d\n"),
                    aItem.Data()));
            break;
        }
    return retVal;
    }

// ---------------------------------------------------------------------------
// CheckMandatoryFieldExistence
// ---------------------------------------------------------------------------
//
TInt CParser::CheckMandatoryFieldExistence( CField::TType aType, TUint32 aAttributes )
    {
    TInt retVal = KErrNone;
        
    if ( ( ( iItemsDefined & KMandatoryItemMask ) != KMandatoryItemMask )
        && (!(aAttributes & KConstantFlag)))
        {
        if ( aType == CField::EInput )
            {
            retVal = EInputMissingItems;
            }
        if ( aType == CField::EOutput )
            {
            retVal = EOutputMissingItems;
            }
        if ( aType == CField::EFeature )
            {    
            retVal = EFeatureMissingItems;
            }
        }    
    return retVal;   
    }

// ---------------------------------------------------------------------------
// CheckUsageMinAndMaxErrors
// ---------------------------------------------------------------------------
//    
TInt CParser::CheckUsageMinAndMaxErrors()
    {
    TInt retVal = KErrNone;
        
    if ( iItemsDefined & ( EUsageMin | EUsageMax ))
        {
        if (!( iItemsDefined & EUsageMax ))
            {
            retVal = ELonelyUsageMin;
            }
        if (!( iItemsDefined & EUsageMin ))
            {
            retVal = ELonelyUsageMax;
            }
        if ( iLocal->UsageMin() > iLocal->UsageMax() )
            {
            retVal = EUsageMinExceedsMax;
            }
        }    
    return retVal;       
    }
    
// ---------------------------------------------------------------------------
// CheckDesignatorMinAndMaxErrors
// ---------------------------------------------------------------------------
//      
TInt CParser::CheckDesignatorMinAndMaxErrors()
    {
    TInt retVal = KErrNone;
        
    if ( iItemsDefined & ( EDesignatorMin | EDesignatorMax ))
        {
        if ( !( iItemsDefined & EDesignatorMax ) )
            {
            retVal = ELonelyDesignatorMin;
            }
        if ( !( iItemsDefined & EDesignatorMin ) )
            {
            retVal = ELonelyDesignatorMax;
            }
        if ( iLocal->DesignatorMin( ) > iLocal->DesignatorMax( ) )
            {
            retVal = EDesignatorMinExceedsMax;
            }
        }    
    return retVal;           
    }    

// ---------------------------------------------------------------------------
// CheckStringMinAndMaxErrors
// ---------------------------------------------------------------------------
//  
TInt CParser::CheckStringMinAndMaxErrors()
    {
    TInt retVal = KErrNone;
            
    if (iItemsDefined & (EStringMin | EStringMax))
        {
        if ( !( iItemsDefined & EStringMax ) )
            {
            retVal = ELonelyStringMin;
            }
        if ( !( iItemsDefined & EStringMin ) )
            {
            retVal = ELonelyStringMax;
            }
        if ( iLocal->StringMin( ) > iLocal->StringMax( ) )
            {
            retVal = EStringMinExceedsMax;
            }
        }    
    return retVal;               
    }


// ---------------------------------------------------------------------------
// CheckStringMinAndMaxErrors
// ---------------------------------------------------------------------------
//  
TInt CParser::CheckMandatoryFieldErrors( CField::TType aType, TUint32 aAttributes )
    {
    TInt ret = KErrNone;
    // Check for mandatory items:       
    ret = CheckMandatoryFieldExistence( aType, aAttributes );    
    if ( ret != KErrNone )
        {
        return ret;
        }
    ret = CheckUsageMinAndMaxErrors();
    if ( ret != KErrNone )
        {
        return ret;
        }    
    ret = CheckDesignatorMinAndMaxErrors();
    if ( ret != KErrNone )
        {
        return ret;
        }     
    ret = CheckStringMinAndMaxErrors();
    if ( ret != KErrNone )
        {
        return ret;
        }          
    return KErrNone;               
    }

// ---------------------------------------------------------------------------
// CheckLogicalMinAndMax
// ---------------------------------------------------------------------------
//  
void CParser::CheckLogicalMinAndMax( TUint32 aAttributes )
    {
    // Logical minimum and maximum must match the number of usage
    // values defined if the Array flag is set (Var=0).  (Ignore this
    // check for constant fields)
    //
    if ( !( aAttributes & KVariableFlag ) && ( ! ( aAttributes & KConstantFlag ) ) )
        {
        // Logical minimum must equal 1:
        //
        if ( iGlobal.iLogicalMin != 1 )
            {
            // This is an error according to [2], but we issue a
            // warning instead, as many devices (including the
            // Logitech diNovo keyboard) fail this check:
            IssueWarning( ELogicalMinInvalidForArray );
            }

        // Logical maximum must equal the number of defined usages:
        //
        TInt numUsages = iLocal->UsageCount();
        if ( numUsages == 0 )
            {
            numUsages = iLocal->UsageMax( ) - iLocal->UsageMin( ) + 1;
            }
        if ( iGlobal.iLogicalMax != numUsages )
            {
            // Again, we issue a warning rather than an error:
            IssueWarning( ELogicalMaxInvalidForArray );
            }
        }            
    
    }

// ---------------------------------------------------------------------------
// CheckFieldBitNeeded
// ---------------------------------------------------------------------------
//      
void CParser::CheckFieldBitNeeded( CField::TType aType, TUint32 aAttributes )
    {
    // "The bit field declared by Report Size must be large enough to
    // hold all values declared by Logical Minimum and Logical
    // Maximum. This includes a sign bit if either are less than
    // 0. Also if the Null flag is set then the field must be capable
    // of reporting all values declared by Logical Minimum and Logical
    // Maximum, and a null value.", [2] (footnote 5).

    TInt bitsNeeded = 0;

    if ( !( aAttributes & KConstantFlag ) )
        {        
        if ( aAttributes & KNullStateFlag )
            {
            // The null state flag is set, so there needs to be at
            // least one extra "out of range" value. This could be
            // below the lowest value or above the highest, whichever
            // will fit better:
            bitsNeeded = Min(
                BitsToRepresentRange(iGlobal.iLogicalMin - 1,
                    iGlobal.iLogicalMax),
                BitsToRepresentRange(iGlobal.iLogicalMin,
                    iGlobal.iLogicalMax + 1));
            }
        else
            {
            // No null state declared:
            bitsNeeded = BitsToRepresentRange(iGlobal.iLogicalMin,
                iGlobal.iLogicalMax);
            }
        }

    if ( iGlobal.iSize < bitsNeeded )
        {
        // The Logitech diNovo is missing a Logical Min and Logical
        // Max pair and so will trigger a range error here.  As a
        // workaround, we will treat this as a warning rather than
        // a critical error:
        const TInt KRangeError[] =
            { EInputReportSize, EOutputReportSize, EFeatureReportSize };
        IssueWarning(KRangeError[aType]);
        }        
    }
    
// ---------------------------------------------------------------------------
// HandleItem
// ---------------------------------------------------------------------------
//     
void CParser::HandleItemL( TItem& aItem )
    {    
    iErrorCode = EUnknownItem;
    if (aItem.IsLocal())
        {
        iErrorCode = LocalItemL(aItem);
        }   
    else
        {
        // Not allowed non-local items within a delimiter pair:
        //
        if ( iWithinDelimiter )
            {
            iErrorCode = EInvalidItemWithinDelimiter;
            }
        else
            {
            if ( aItem.IsMain() )
                {
                iErrorCode = MainItemL(aItem);
                }

            if ( aItem.IsGlobal() )
                {
                iErrorCode = GlobalItemL(aItem);
                }
            if ( aItem.IsLong() )
                {
                IssueWarning(ELongItemDefined);
                iErrorCode = 0;
                }
            }
        }        
    }
    
// ---------------------------------------------------------------------------
// CheckParseErrors()
// ---------------------------------------------------------------------------
//      
void CParser::CheckParseErrors()
    {        
    if ( !iErrorCode && ( iGlobalStack.Count() > 0) )
        {
        iErrorCode = EPushWithoutPop;
        }

    // COLLECTION without END_COLLECTION:
    if ( !iErrorCode && ( iCollectionStack.Count() != 0 ) )
        {
        iErrorCode = ENoMatchingEndCollection;
        }

    // DELIMITER(Open) without DELIMITER(Close):
    if ( !iErrorCode && iWithinDelimiter )
        {
        iErrorCode = ELonelyDelimiter;
        }

    // Final size of all reports must be a multiple of eight bits:
    if ( !CheckAllReportSizes() )
        {        
        IssueWarning( EReportMustBeEightBitMultiple );
        }            
    }
    
// ---------------------------------------------------------------------------
// ResetParser
// ---------------------------------------------------------------------------
//  
void CParser::ResetParserL()    
    {
    // Create the root collection, which is the container for all
    // other collections and fields:
    //
    delete iReportRoot;   // may exist if there has been a Leave()
    iReportRoot = 0;
    iReportRoot = CReportRoot::NewL();
    iCollectionStack.Reset();
    PushCollectionL(iReportRoot);

    // Clear the error code and the warnings list:
    //
    iErrorCode = 0;    

    // Reset the parser internal state:
    //
    iGlobal = TParserGlobalState();
    iGlobalStack.Reset();
    iWithinDelimiter = EFalse;
    iItemsDefined = 0;
    iGlobalItemsDefined = 0;
    iItemNumber = 0;    
    ClearLocalState();
    }

