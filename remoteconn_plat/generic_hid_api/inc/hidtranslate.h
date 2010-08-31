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
* Description:  Report translator
*
*/

#ifndef T_TRANSLATE_H
#define T_TRANSLATE_H

#include <e32std.h>

class CField;

/**
 * HID report base object
 *
 * Base class for report translator and report generator.
 * Contains only static functions.
 *
 * @lib  generichid.lib
 * @since S60 v5.0
 */
class TReportUtils
    {
public:
    /**
     *  Find the index within the usages for a field of a given usage ID.
     *
     *  @since S60 v5.0
     *  @param aField Pointer to the field.
     *  @param aUsageId Usage ID to find.
     *  @param aUsageIndex Reference to variable to receive the index.
     *  @return True if the usage ID is found.
     */
    static TBool GetIndexOfUsage(const CField* aField,
        TInt aUsageId, TInt& aUsageIndex);
    /**
     *  Find the usage ID at a given index within the usages for a field.
     *
     *  @since S60 v5.0
     *  @param aField Pointer to the field.
     *  @param aUsageIndex The index.
     *  @return The usage ID at the given index.
     */
    static TInt UsageAtIndex(const CField* aField, TInt aUsageIndex);

    /**
     *  Write a value to a field at a given index.
     *
     *  @since S60 v5.0
     *  @param aData Buffer containing the HID report.
     *  @param aField The field in which to write.
     *  @param aIndex Position in the field to write.
     *  @param aValue Value to write to the field.
     *  @return Error code indicating success or reason for failure.
     */
    static TInt WriteData(HBufC8& aData, const CField* aField,
        TInt aIndex, TInt aValue);

    /**
     *  Read a value from a field at a given index.
     *
     *  @since S60 v5.0
     *  @param aData Buffer containing the HID report.
     *  @param aField The field from which to read.
     *  @param aIndex Position in the field to read.
     *  @param aValue Reference to variable to receive the value read
     *                from the field.
     *  @return Error code indicating success or reason for failure.
     */
    static TInt ReadData(const TDesC8& aData, const CField* aField,
        TInt aIndex, TInt& aValue);
    };

/**
 * HID report translator
 *
 * Allows a device driver to extract data items from a device report, based on
 * the results of the report descriptor parser stage (at device connection)
 *
 * @lib  generichid.lib
 * @since S60 v5.0
 */
class TReportTranslator
    {
public:

    /**
     * Constructor.
     *
     * @since S60 v5.0
     * @param aData Data to be extracted
     * @param aField  HID field
     * @return return TReportTranslator
     */
    IMPORT_C TReportTranslator(const TDesC8& aData, const CField* aField);

    /**
     * For variable fields, reads the logical value of the control with the
     * given usage ID.  For arrays, searches for the usage ID and gives the
     * value as ETrue if found and EFalse if not.
     *
     * @since S60 v5.0
     * @param aValue Reference to variable to receive the value read
     *  from the field.
     * @param aUsageId Usage ID of the control to read.
     * @param aControlOffset Which control to read when more than one
     *                       have the same usage ID.
     * @return Error code indicating success or reason for failure.
     */
    IMPORT_C TInt GetValue(TInt& aValue, TInt aUsageId,
        TInt aControlOffset = 0) const;

    /**
     * Alternate version of the above method for convenience.  Returns the
     * value read directly and leaves if an error occurs.
     *
     * @since S60 v5.0
     * @param aUsageId Usage ID of the control to read.
     * @param aControlOffset Which control to read when more than one
     *                       have the same usage ID.
     * @return The logical value of a variable, or true/false for an array.
     */
    IMPORT_C TInt ValueL(TInt aUsageId, TInt aControlOffset = 0) const;

    /**
     * Gets the usage ID at a given index in an array field.  For variable
     * fields, if the logical value of the control at the given index is non-
     * zero, returns the usage ID of the control, otherwise returns zero.
     *
     * @since S60 v5.0
     * @param aUsageId Reference to variable to receive the usage ID.
     * @param aIndex Index in the array to read.
     * @return Error code indicating success or reason for failure.
     */
    IMPORT_C TInt GetUsageId(TInt& aUsageId, TInt aIndex) const;

    /**
     * Alternate version of the above method for convenience.  Returns the
     * usage ID directly and leaves if an error occurs.
     *
     * @since S60 v5.0
     * @param aIndex Index in the array to read.
     * @return The usage ID.
     */
    IMPORT_C TInt UsageIdL(TInt aIndex) const;

    /**
     * Gets the logical value at a given index in a field.  Leaves if an
     * error occurs.
     *
     * @since S60 v5.0
     * @param aIndex Index in the field to read.
     * @return The logical value.
     */
    IMPORT_C TInt RawValueL(TInt aIndex) const;

    /**
     * Gets the number of controls in the field.
     *
     * @since S60 v5.0
     * @return The number of controls.
     */
    IMPORT_C TInt Count() const;

private:

    /**
     * Data to be extracted
     */
    const TDesC8& iData;

    /**
     * HID field
     * Not own.
     */
    const CField* iField;
    };

#endif
