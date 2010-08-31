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
* Description:  Item implementation
*
*/


// References:
//
// [1] USB Device Class Definition for Human Interface Devices (HID),
//     Firmware Specification, Version 1.11, USB Implementers' Forum,
//     June 2001
//
// ----------------------------------------------------------------------

#include <e32std.h>

#include "hiditem.h"
#include "hidparser.h"

const TInt KSizeMask        = 0x03;
const TInt KTypeMask        = 0x03;
const TInt KTypePosn        = 2;
const TInt KTagMask         = 0x0F;
const TInt KTagPosn         = 4;
const TInt KLongItemId      = 0xfe;
const TInt KMaxItemlength   = 4;
const TInt KSizeThree       = 3;
const TInt KSizeFour        = 4;
const TInt KLongItemMin     = 3;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
TItem::TItem(const TDesC8& aRawData)
    : iRawData(aRawData)
    {    
    if (aRawData.Length() < 1)
        {
        return;
        }
    TBool isShortItem = (aRawData[0] != KLongItemId);

    if (isShortItem)
        {
        //  +----+----+----+----+----+----+----+----+
        //  |        Tag        |   Type  |  Size   |    [data...]
        //  |    .    .    .    |    .    |    .    |
        //  +----+----+----+----+----+----+----+----+
        //    7    6    5    4    3    2    1     0
        //
        // A short item is followed by 0, 1, 2 or 4 bytes of data, as
        // specified in the size field. See [1], Section 6.2.2.2.        
        iSize = aRawData[0] & KSizeMask;
        if (iSize == KSizeThree)
            {
            iSize = KSizeFour;
            }        
        iType = static_cast<TItemType>((aRawData[0] >> KTypePosn) & KTypeMask);
        iTag = (aRawData[0] >> KTagPosn) & KTagMask;
        iDataOffset = 1;
        }
    else
        {
        //        Byte 0               Byte 1          Byte 2
        // +------+------+------+   +-----------+   +-----------+
        // | Tag  | Type | Size |   | Data size |   | Long item |   [data...]
        // | 1111 |  11  |  10  |   |  (0-255)  |   |    tag    |
        // +------+------+------+   +-----------+   +-----------+
        //   7654    32     10
        //
        // A long item is followed by 0-255 bytes of data, as specified
        // in the data size byte. See [1], Section 6.2.2.3.

        iType = EReserved;

        if (aRawData.Length() > KLongItemMin)
            {
            iSize = aRawData[1];
            iTag = aRawData[2];
            iDataOffset = KLongItemMin;
            }
        else
            {
            iSize = 0;
            iTag = 0;
            iDataOffset = 0;
            }
        }

#ifdef DBG_ACTIVE
    for (TInt i=0; (i<aRawData.Length()) && (i<(iSize+iDataOffset)); ++i)
        {
        RDebug::Print(_L("aRawData[%d] = 0x%02x"), i, aRawData[i]);
        }
#endif
    }

// ---------------------------------------------------------------------------
// DataSize()
// ---------------------------------------------------------------------------
//
TInt TItem::DataSize() const
    {
    return iSize;
    }

// ---------------------------------------------------------------------------
// Tag()
// ---------------------------------------------------------------------------
//
TInt TItem::Tag() const
    {
    return iTag;
    }

// ---------------------------------------------------------------------------
// Data()
// ---------------------------------------------------------------------------
//
TUint32 TItem::Data() const
    {
    TInt size = DataSize();

    // Calling Data() makes no sense for long items that have a data
    // size over 4 bytes, as it only returns a TUint32:
    //
    if (size > KMaxItemlength)
        {
        size = KMaxItemlength;
        }

    // Ensure we don't overrun the source buffer:
    //
    if (size > (iRawData.Length() - 1))
        {
        size = iRawData.Length() - 1;
        }

    // Concatenate each byte into a TUint32.  Note that this function must
    // return zero if the data size is zero (see Parser.cpp, MainItemL()).
    //
    TUint32 tagData = 0;
    for (TInt i=0; i<size; ++i)
        {
        tagData |= (iRawData[1 + i] << (KSizeOfByte*i));
        }

    return tagData;
    }

// ---------------------------------------------------------------------------
// SignedData()
// ---------------------------------------------------------------------------
//
TInt32 TItem::SignedData() const
    {
    TUint32 data = Data();

    // For 8 and 16 bit negative values, we need to
    // sign extend to 32-bits:
    //
    if ((DataSize() == 1) && (data & 0x80))
        {
        data |= 0xffffff00;
        }
    if ((DataSize() == 2) && (data & 0x8000))
        {
        data |= 0xffff0000;
        }

    // Note that this assumes that the machine uses 2s complement
    // representation internally.  All current Symbian devices do,
    // including ARM devices and the WINS emulator. This is almost
    // certain to remain the case in the future.
    //
    return static_cast<TInt32>(data);
    }

// ---------------------------------------------------------------------------
// Operator []
// ---------------------------------------------------------------------------
//
TUint8 TItem::operator[](TInt aIndex) const
    {    

    TUint8 value = 0;
    if ((aIndex >= 0) && (aIndex < iSize))
        {
        value = iRawData[iDataOffset + aIndex];
        }
    return value;
    }

// ---------------------------------------------------------------------------
// Type()
// ---------------------------------------------------------------------------
//
TItem::TItemType TItem::Type() const
    {
    return iType;
    }

// ---------------------------------------------------------------------------
// IsMain()
// ---------------------------------------------------------------------------
//
TBool TItem::IsMain() const
    {
    return iType == TItem::EMain;
    }

// ---------------------------------------------------------------------------
// IsLocal()
// ---------------------------------------------------------------------------
//
TBool TItem::IsLocal() const
    {
    return iType == TItem::ELocal;
    }

// ---------------------------------------------------------------------------
// IsGlobal()
// ---------------------------------------------------------------------------
//
TBool TItem::IsGlobal() const
    {
    return iType == TItem::EGlobal;
    }

// ---------------------------------------------------------------------------
// IsLong()
// ---------------------------------------------------------------------------
//
TBool TItem::IsLong() const
    {
    return iType == TItem::EReserved;
    }

// ---------------------------------------------------------------------------
// ItemSize()
// ---------------------------------------------------------------------------
//
TInt TItem::ItemSize() const
    {
    return iSize + iDataOffset;
    }

