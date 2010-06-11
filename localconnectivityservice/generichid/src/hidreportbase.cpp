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


#include "hidfield.h"
#include "hidtranslate.h"
#include "hidinterfaces.h"

const TUint KValueMask      = 0xFFFFFFFF;
const TUint KSignBitMask    = 0x80000000;
const TInt  KThreeLSB       = 7;
const TInt  KThreeLSBShift  = 3;
// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// GetIndexOfUsage()
// ---------------------------------------------------------------------------
//
TBool TReportUtils::GetIndexOfUsage(const CField* aField,
    TInt aUsageId, TInt& aUsageIndex)
    {
    TArray<TInt> usages(aField->UsageArray());

    if ( usages.Count() > 0 )
        {
        // Find the usage in the array
        for ( TInt i = 0; i < usages.Count(); i++ )
            {
            if ( usages[i] == aUsageId )
                {
                aUsageIndex = i;
                return ETrue;
                }
            }
        }
    else
        {
        // The field includes all usages between the min and max
        if ( aField->UsageMin() <= aUsageId && aUsageId <= aField->UsageMax() )
            {
            aUsageIndex = aUsageId - aField->UsageMin();
            return ETrue;
            }
        }

    return EFalse;
    }

// ---------------------------------------------------------------------------
// UsageAtIndex()
// ---------------------------------------------------------------------------
//
TInt TReportUtils::UsageAtIndex(const CField* aField, TInt aUsageIndex)
    {
    TInt usageId = 0;
    TArray<TInt> usages(aField->UsageArray());

    if ( usages.Count() > 0 )
        {
        if ( aUsageIndex < 0 )
            {
            // Null state for array control
            }
        else if ( aUsageIndex < usages.Count() )
            {
            // Get the usage ID from the set of possible usages
            usageId = usages[aUsageIndex];
            }
        else
            {
            // If there aren't enough usages in the set, the last one repeats
            usageId = usages[usages.Count() - 1];
            }
        }
    else
        {
        // Get the usage ID from the range
        if ( 0 <= aUsageIndex
                && aUsageIndex <= (aField->UsageMax() - aField->UsageMin()) )
            {
            usageId = aField->UsageMin() + aUsageIndex;
            }
        }

    return usageId;
    }

// ---------------------------------------------------------------------------
// WriteData()
// ---------------------------------------------------------------------------
//
TInt TReportUtils::WriteData(HBufC8& aData, const CField* aField,
    TInt aIndex, TInt aValue)
    {
    if ( 0 <= aIndex && aIndex < aField->Count() )
        {
        // The offset in bits from the start of the report to the value
        TInt offset = aField->Offset() + aIndex * aField->Size();

        // How many bits in the least significant byte are not part of the value
        TInt bitsToShift = offset & KThreeLSB;

        TUint mask = KValueMask >> ((KSizeOfByte * sizeof(TInt)) - aField->Size());
        mask <<= bitsToShift;
        aValue <<= bitsToShift;

        TPtr8 data = aData.Des();

        // Write out the bytes, least significant first
        for ( TInt i = offset >> KThreeLSBShift; mask && i < aData.Length(); i++ )
            {
            TUint8 maskByte = static_cast<TUint8>(mask);

            // The extra cast is because MSVC6 thinks that or-ing 2
            // TUint8s together gives an int.
            data[i] = static_cast<TUint8>(
                (static_cast<TUint8>(aValue) & maskByte)
                | (aData[i] & ~maskByte));
            mask >>= KSizeOfByte;
            aValue >>= KSizeOfByte;
            }

        return KErrNone;
        }

    return KErrBadControlIndex;
    }

// ---------------------------------------------------------------------------
// ReadData()
// ---------------------------------------------------------------------------
//
TInt TReportUtils::ReadData(const TDesC8& aData, const CField* aField,
    TInt aIndex, TInt& aValue)
    {
    if ( 0 <= aIndex && aIndex < aField->Count() )
        {
        // The offset in bits from the start of the report to the value
        TInt offset = aField->Offset() + aIndex * aField->Size();

        // How many bits in the least significant byte are not part of
        // the value
        TInt bitsToShift = offset & KThreeLSB;

        // How many consecutive bytes we need to read to get the whole
        // value. According to the HID spec, a value cannot span more
        // than 4 bytes in a report
        TInt bytesToRead = (bitsToShift + aField->Size() + KThreeLSB) / KSizeOfByte;

        // Make sure we don't read past the end of the data
        if ( (offset >> KThreeLSBShift) + bytesToRead > aData.Length() )
            {
            bytesToRead = aData.Length() - (offset >> KThreeLSBShift);
            }

        TInt value = 0;

        // Read in the bytes, most significant first
        for ( TInt i = bytesToRead - 1; i >= 0; i-- )
            {
            value = (value << KSizeOfByte) | aData[(offset >> KThreeLSBShift) + i];
            }

        value >>= bitsToShift;

        // Make masks for the whole value and just the sign bit
        TUint valueMask = KValueMask >> ((KSizeOfByte * sizeof(TInt)) - aField->Size());
        TUint signMask = KSignBitMask >> ((KSizeOfByte * sizeof(TInt)) - aField->Size());

        if ( aField->LogicalMin() < 0 && (value & signMask) )
            {
            // The value is negative, so the leading bits should be 1s
            aValue = value | ~valueMask;
            }
        else
            {
            // The value is positive, so the leading bits should be 0s
            aValue = value & valueMask;
            }

        return KErrNone;
        }

    return KErrBadControlIndex;
    }
