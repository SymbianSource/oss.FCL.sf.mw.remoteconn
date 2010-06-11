/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BTHid item
*
*/


#ifndef T_ITEM_H
#define T_ITEM_H

#include <e32std.h>

/**
 * HID item class
 * A HID report descriptor is comprised of a sequence of "items"
 * that provide information about the device.  Items consist of a
 * header and zero or more bytes of data.  The header contains the item
 * type (main/global/local), an item "tag" (subtype) and the item size.
 * Items can be "short" (containing a maximum of 4 bytes of data) or
 * "long" (a maximum of 255 bytes of data).

 * A TItem represents a single HID report descriptor item. It is
 * constructed from an eight bit non-modifiable (Symbian-) descriptor,
 * which is assumed to contain report (HID-) descriptor data.
 *
 * @lib generichid.lib
 * @since S60 v5.0
 */
class TItem
    {
public:
    /**
     * The possible item types: main, global or local.  Item type is
     * a two-bit field -- the 11b value is reserved for "long items", which
     * are only used for vendor-specific commands.
     */
    enum TItemType
        {
        EMain = 0,    //!< Input, output, feature and begin or end collection
        EGlobal = 1,  //!< Global item items affect all subsequent fields
        ELocal = 2,   //!< Local items only persist until the next main item
        EReserved = 3 //!< No defined use in the current HID standard (v1.11)
        };

public:

    /**
     * The constructor takes a an eight bit non-modifiable (Symbian-)
     * descriptor containing report (HID-) descriptor data.
     * The item tag, type, data size and offset are calculated here.
     *
     * @since S60 v5.0
     * @param aRawData The raw item data, represented as a Symbian
     * descriptor. The item data is assumed to be at the beginning of the
     * descriptor. The size of the TDesC8 may be greater than the size
     * of the item, as the item size will be calculated from the raw HID
     * descriptor data.  Of course, the TDesC8 should be at least as
     * long as the raw item data.
     */
    TItem(const TDesC8& aRawData);

    /**
     * DataSize() returns the size of the item data in bytes. This
     * is the total item size minus the size of any header information.
     * (A short item header is a single byte, a long item header is
     * three bytes long.)
     *
     * @since S60 v5.0
     * @return The size, in bytes, of the item data. Range 0-255.
     */
    TInt DataSize() const;

    /**
     * Tag() returns the tag (subtype) value for this item.
     *
     * @since S60 v5.0
     * @return The item tag.
     */
    TInt Tag() const;

    /**
     * Data() returns the data associated with the item, represented as a
     * 32-bit unsigned integer.  This is only meaningful if the data length
     * is less than 4 bytes.
     *
     * @since S60 v5.0
     * @return The item data as a 32-bit unsigned integer.
     */
    TUint32 Data() const;

    /**
     * SignedData() returns the data associated with the item,
     * represented as a 32-bit signed integer.  This is only
     * meaningful if the data length is less than 4 bytes.
     *
     * @since S60 v5.0
     * @return The item data as a 32-bit signed integer.
     */
    TInt32 SignedData() const;

    /**
     * Returns the byte at the given offset within the item data block.     
     *
     * @since S60 v5.0
     * @param aIndex The index within the data block for the current tag.
     * @return The data at the specified index.
     */
    TUint8 operator[](TInt aIndex) const;

    /**
     * Type() returns the item type (e.g. "global").
     *
     * @since S60 v5.0     
     * @return The item type.
     */
    TItemType Type() const;

    /**
     * Check if item is main
     *
     * @since S60 v5.0
     * @return ETrue if item type is "main"
     */
    TBool IsMain() const;

    /**
     * Check if item is local
     *
     * @since S60 v5.0
     * @return ETrue if item type is "local"
     */
    TBool IsLocal() const;

    /**
     * Check if item is global
     *
     * @since S60 v5.0
     * @return ETrue if item type is "global"
     */
    TBool IsGlobal() const;

    /**
     * Check if item is global
     *
     * @since S60 v5.0
     * @return ETrue if this is a long item
     */
    TBool IsLong() const;

    /**
     * ItemSize() returns the total size of this item, including the
     * header data.
     *
     * @since S60 v5.0
     * @return The total size of this item, in bytes.
     */
    TInt ItemSize() const;

private:
    /**
     * Pointer to the raw report descriptor data
     */
    TPtrC8 iRawData;

    /**
     * Item data size
     */

    TInt iSize;
    /**
     * Item tag value
     */
    TInt iTag;

    /**
     * Offset in bytes of the data block
     */
    TInt iDataOffset;

    /**
     * Item type
     */
    TItemType iType;
    };

#endif

