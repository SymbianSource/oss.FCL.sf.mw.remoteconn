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

#ifndef C_PARSER_H
#define C_PARSER_H

#include "hidreportroot.h"

class TItem;

/**
 * HID Parser error codes.  For a description of the error code
 * format, see "HID Parser Error Codes", Revision 1.2, USB
 * Implementers' Forum, February 2000.
 */
enum THidParserError
    {
    // General:
    //
    EUnknownItem                    = 0xbf00,
    ELongItemDefined                = 0x3e00,
    //
    // Global:
    //
    EZeroUsagePage                  = 0x8004,
    EUsagePageOutOfRange            = 0x8104,
    ELogicalMinInvalidForArray      = 0x8214,
    ELogicalMaxInvalidForArray      = 0x8224,
    ELonelyPhysicalMin              = 0x8034,
    ELonelyPhysicalMax              = 0x8044,
    EZeroReportId                   = 0x8084,
    EReportIdTooBig                 = 0x8184,
    ELateReportId                   = 0x8284,
    EReportIdOutsideTopLevel        = 0x8484,
    EZeroReportCount                = 0x8094,
    EPushWithoutPop                 = 0x80a4,
    EPushHasData                    = 0x81a4,
    EPopWithoutPush                 = 0x80b4,
    EPopHasData                     = 0x81b4,
    ERedundantGlobalItem            = 0x80f4,
    EReservedUsagePage              = 0x0004,
    ELogicalMinExceedsMax           = 0x0014,
    EPhysicalMinExceedsMax          = 0x0034,
    EExponentReservedBitsNonZero    = 0x0054,
    EUnitReservedBitsNonZero        = 0x0064,
    //
    // Local:
    //
    ELonelyUsageMin                 = 0x8018,
    EUsageMinExceedsMax             = 0x8118,
    EUsagePageMismatchMin           = 0x8318,
    ELonelyUsageMax                 = 0x8028,
    EUsagePageMismatchMax           = 0x8228,
    ELonelyDesignatorMin            = 0x8048,
    EDesignatorMinExceedsMax        = 0x8148,
    ELonelyDesignatorMax            = 0x8058,
    ELonelyStringMin                = 0x8088,
    EStringMinExceedsMax            = 0x8188,
    ELonelyStringMax                = 0x8098,
    EUnknownDelimiter               = 0x80a8,
    ENestedDelimiter                = 0x81a8,
    ELonelyDelimiter                = 0x82a8,
    EInvalidItemWithinDelimiter     = 0x83a8,
    EDelimiterAtTopLevel            = 0x84a8,
    EZeroUsage                      = 0x0008,
    //
    // Main:
    //
    EInputMissingItems              = 0x8080,
    EInputItemWithinDelimiter       = 0x8180,
    EInputReportSize                = 0x8280,
    EInputMinExceedsMax             = 0x8380,
    EOutputMissingItems             = 0x8090,
    EOutputItemWithinDelimiter      = 0x8190,
    EOutputReportSize               = 0x8290,
    EOutputMinExceedsMax            = 0x8390,
    EFeatureMissingItems            = 0x80b0,
    EFeatureItemWithinDelimiter     = 0x81b0,
    EFeatureReportSize              = 0x82b0,
    EFeatureMinExceedsMax           = 0x83b0,
    ENoMatchingBeginCollection      = 0x80c0,
    EEndCollectionWithinDelimiter   = 0x81c0,
    EReportMustBeEightBitMultiple   = 0x82c0,
    ENoMatchingEndCollection        = 0x80a0,
    EBeginCollectionWithinDelimiter = 0x81a0,
    EApplicationCollectionLevel     = 0x82a0,
    EInputReservedBitsNonZero       = 0x0080,
    EInputLocalMultipleUse          = 0x0280,
    EOutputReservedBitsNonZero      = 0x0090,
    EOutputLocalMultipleUse         = 0x0290,
    EFeatureReservedBitsNonZero     = 0x00b0,
    EFeatureLocalMultipleUse        = 0x02b0,
    ECollectionLocalUnused          = 0x00a0,
    ECollectionTypeUnknownReserved  = 0x01a0,
    EEndCollectionLocalUnused       = 0x00c0,
    EEndCollectionHasData           = 0x01c0,
    //
    // Our ("vendor specific") codes:
    //
    EInvalidItemLength              = 0xC000,
    EApplicationHasDelimiter        = 0x40a0,
    EDelimiterWithinNamedArray      = 0x40a8,
    ECollectionHasNoUsage           = 0x40c0,
    ECollectionHasNoUsagePage       = 0x41c0
    };

/**
 * HID parser panics
 */
enum THidParsingError
    {
    // ASSERT_ALWAYS:
    //
    EZeroLengthItem = 1,       //!< Zero descriptor length in TItem constructor
    ENoReportRoot = 2,         //!< iReportRoot is 0 in CreateFieldL()
    ENoCurrentCollection = 3,  //!< No current collection in Collection()
    //
    // ASSERT_DEBUG:
    //
    ENoCollectionToCheck = 10, //!< No collection in CheckForCollectionErrors()
    EPopFailed = 11,           //!< Empty collection stack in PopCollection()
    EIndexOutOfRange = 12,     //!< Index out of range in TItem::operator[]
    EItemTooLong = 13          //!< Data() called for item with size > 4
    };


/**
 *  Parser global states
 *
 *  CParser uses TParserGlobalState objects to store the global item
 *  state during parsing. The global state can be saved and restored
 *  using the HID descriptor POP and PUSH tags, therefore CParser
 *  incorporates a stack of TParserGlobalState objects.
 *
 *
 *  @lib  generichid.lib
 *  @since S60 v5.0
 */
class TParserGlobalState
    {
    friend class CParser;

public:

    TParserGlobalState();

    /**
     * Copies all data members into a CField object. The other
     * members of the CField object (those corresponding to local
     * HID descriptor tags) are unaffected.
     *
     * @since S60 v5.0
     * @param aField Pointer to the field object to populate.
     * @return None.
     */
    void Populate(CField *aField) const;

private:
    /**
     * Usage page
     */
    TInt iUsagePage;

    /**
     * Logical minimum
     */
    TInt iLogicalMin;

    /**
     * Logical maximum
     */
    TInt iLogicalMax;

    /**
     * Physical minimum
     */
    TInt iPhysicalMin;

    /**
     * Physical maximum
     */
    TInt iPhysicalMax;

    /**
     * Unit type
     */
    TInt iUnit;

    /**
     *  Unit exponent
     */
    TInt iUnitExponent;

    /**
     * Associated report ID
     */
    TInt iReportId;

    /**
     * Report size
     */
    TInt iSize;

    /**
     * Report count
     */
    TInt iCount;
    };


/**
 *
 * HID parser
 * CParser parses a HID report descriptor.  It outputs a CReportRoot
 * containing a tree of CCollection and CField objects representing
 * the descriptor information.  A HID device driver can then use this,
 * in conjunction with TReportTranslator and CReportGenerator objects,
 * to facilitate communication with a HID device.
 *
 * The parsing process conforms to the HID class specification
 * document: "USB Device Class Definition for Human Interface Devices
 * (HID)", Firmware Specification, Version 1.11, USB Implementers' Forum,
 * June 2001.
 *
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class CParser : public CBase
    {
public:
    IMPORT_C static CParser* NewL();
    IMPORT_C static CParser* NewLC();

    virtual ~CParser();

    /**
     * ParseL() parses a string of bytes representing a HID report
     * descriptor. It returns a tree of collection and field objects (a
     * CReportRoot). Ownership of the CReportRoot is transferred to the
     * caller.
     *
     * Warning messages produced during the parse process are added to
     * a list which the driver can retrieve using the Warnings()
     * function. Note that the list will be overwritten by the next
     * ParseL() operation.
     *
     * Errors and warnings are represented using the format described
     * in "HID Parser Error Codes", Revision 1.2, USB Implementers'
     * Forum, February 2000.
     *
     * @since S60 v5.0
     * @param aRawData The report descriptor data to be processed
     * @return The root collection, a tree of CCollection and CField objects
     */
    IMPORT_C CReportRoot* ParseL(const TDesC8& aRawData);

    /**
     * Return field count
     *
     * @since S60 v5.0
     * @return field count
     */
    TInt FieldCount();
     
  

        

private:
    CParser();
    void ConstructL();

    /**
     * Handle a HID descriptor main item.  This includes the creation
     * of new field objects in the current collection and management of
     * the global item state stack.
     *
     * @since S60 v5.0
     * @param aItem The current report descriptor tag and its associated data
     * @return KErrNone (0) on success, otherwise a critical error code in
     *         the standard HID format.
     */
    TInt MainItemL(const TItem& aItem);

    /**
     * Handle a HID descriptor local item.  Stores the data in the
     * appropriate member of the local state object, *iLocal.
     *
     * @since S60 v5.0
     * @param aItem The current report descriptor tag and its associated data
     * @return KErrNone (0) on success, otherwise a critical error code in
     *         the standard HID format.
     */
    TInt LocalItemL(const TItem& aItem);

    /**
     * Handle a HID descriptor global item. Stores the data in the
     * appropriate member of the current global state object, iGlobal.
     *
     * @since S60 v5.0
     * @param aItem The current report descriptor tag and its associated data
     * @return KErrNone (0) on success, otherwise a critical error code in
     *         the standard HID format.
     */
    TInt GlobalItemL(const TItem& aItem);

    /**
     * Used to access the current collection object, i.e. the object
     * at the top of the collection stack (iCollectionStack).
     *     
     *
     * @since S60 v5.0
     * @return A pointer to the current collection object.
     *
     */
    CCollection* Collection();

    /**
     * Pushes a pointer to the current collection object on to the
     * collection stack.
     *
     * Note that the collection stack is used to create the tree of
     * collections. It should not be confused with the global state
     * stack, iGlobalStack.
     *
     * @since S60 v5.0
     * @param aCollection The collection to be pushed onto the stack
     * @return None.
     *
     */
    void PushCollectionL(const CCollection* aCollection);

    /**
     * Pushes a pointer to the current collection object on to the
     * collection stack.
     *
     * Note that the collection stack is used to create the tree of
     * collections. It should not be confused with the global state
     * stack, iGlobalStack.
     *
     * @since  S60 v5.0
     * @return None.
     *
     */
    void PopCollection();

    /**
     * CreateFieldL() is called to instantiate a new CField object
     * when an input, output or feature item is encountered in the
     * report descriptor.
     *
     * The current local and global states extracted from the report
     * descriptor are copied to the CField, which is placed into the
     * current CCollection.
     *
     * @since S60 v5.0
     * @param aType The type of field: input, output or feature
     * @param aAttributes The attributes for the field (e.g. Data, Array,
     *                    Absolute.)
     * @return KErrNone (0) on success, otherwise a critical error code
     *                  in the standard HID format.
     *
     */
    TInt CreateFieldL(CField::TType aType, TUint32 aAttributes);

    /**
     * ClearLocalState() clears the appropriate fields in the CField
     * object that represents the current local item state.
     *
     * @since S60 v5.0
     * @return None
     *
     */
    void ClearLocalState();

    /**
     * DumpStateTableL() copies the current local and global item state
     * into a CField object.
     *
     * @since S60 v5.0
     * @param aField The field object to receive the data
     * @return None
     *
     */
    void DumpStateTableL(CField *aField) const;

    /**
     * CreateCollectionL() is called to instantiate a new CCollection
     * object when a "begin collection" item is encountered in the
     * report descriptor.
     *
     * @since S60 v5.0
     * @param aType The type of the collection. This can be one of the
     *              seven standard types defined in CCollection::TType,
     *              or a vendor defined value.
     * @return KErrNone (0) on success, otherwise a critical error code in
     *         the standard HID format.
     */
    TInt CreateCollectionL(TUint32 aType);

    /**
     * CheckForCollectionErrors() validates the current collection
     * object. It adds non-critical errors to the warnings list. It
     * returns an error code if a critical error is encountered.
     *
     * @since S60 v5.0
     * @param aType The type of the collection (one of the CCollection::TType
     *              values, or vendor defined).
     * @return KErrNone (0) if there was no error, otherwise a critical
     *         error code in the standard HID format.
     */
    TInt CheckForCollectionErrors(TUint32 aType);

    /**
     * CheckForFieldErrors() validates the current global and local
     * item state in preparation for creating a CField object.  It adds
     * non-critical errors to the warnings list. It returns an error
     * code if a critical error is encountered.
     *
     * @since S60 v5.0
     * @param @param aType The type of the field (input, output or feature).
     * @param aAttributes The attributes for the field (e.g. Data, Array,
     *                    Absolute.)
     * @return KErrNone (0) if there was no error, otherwise a critical
     *                   error code in the standard HID format.
     */
    TInt CheckForFieldErrors(CField::TType aType, TUint32 aAttributes);

    /**
     * CheckForMainErrors() performs error checking common to
     * CheckForCollectionErrors() and CheckForFieldErrors(). It adds
     * non-critical errors to the warnings list. It returns an error
     * code if a critical error is encountered.
     *
     * @since S60 v5.0
     * @return KErrNone (0) if there was no error, otherwise a critical
     *                  error code in the standard HID format.
     */
    TInt CheckForMainErrors();

    /**
     * CheckAllReportSizes() checks if all report sizes are integral
     * multiples of 8 bits.
     *
     * @since S60 v5.0
     * @return ETrue if all report sizes are integral multiples of
     *         8 bits.
     */
    TBool CheckAllReportSizes() const;

    /**
     * BitsToRepresentRange() returns the number of bits required to
     * represent all values in a given range. It is used to check
     * that the report field size is appropriate for the given
     * logical minimum and maximum.
     *
     * If the range is all positive then it is assumed that there is no
     * sign bit, otherwise twos complement format is assumed, as per
     * the HID class specification, v1.11, Section 6.2.2.7.
     *
     * @since S60 v5.0
     * @param aMin Logical minimum
     * @param aMax Logical maximum
     * @return The number of bits required to represent the range aMin
     *         to aMax (inclusive).
     */
    static TInt BitsToRepresentRange(TInt aMin, TInt aMax);

    /**
     * NumberOfLeadingZeros() is used by BitsToRepresentRange(). It
     * returns the number of leading zeros in the binary representation
     * of a number, effectively performing a log_2 operation.
     *
     * @since S60 v5.0
     * @param aValue Unsigned 32-bit value
     * @return Number of leading zeros in the binary representation of aValue
     */
    static TInt NumberOfLeadingZeros(TUint32 aValue);

    /**
     * IssueWarning() adds a TParserWarning to the warning list.
     *
     * @since S60 v5.0
     * @param aHidWarningCode The error or warning code, which should
     *                        be in the standard HID format.
     * @return None
     */
    void IssueWarning(TInt aHidWarningCode);

    /**
     * IsReservedUsagePage() checks if the given HID usage page is
     * listed as reserved according to the HID clas specification,
     * v1.11.
     *
     * @since S60 v5.0
     * @param aUsagePage The usage page to check.
     * @result ETrue if the usage page is listed as reserved.
     */
    static TBool IsReservedUsagePage(TInt aUsagePage);


    /**
     * HandleMainInputTagL
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleMainInputTagL(const TItem& aItem);


    /**
     * HandleMainOutputTag
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleMainOutputTagL(const TItem& aItem);

    /**
     * HandleMainFeature
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleMainFeatureL( const TItem& aItem );

    /**
     * HandleMainCollection
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleMainCollectionL( const TItem& aItem );


    /**
     * MainEndCollection
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleMainEndCollection( const TItem& aItem);

    /**
     * HandleGlobalReportId
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalReportId( const TItem& aItem );

    /**
     * HandleGlobalUsagePage
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalUsagePage( const TItem& aItem );

    /**
     * HandleGlobalLogicalMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalLogicalMinimum( const TItem& aItem );

    /**
     * HandleGlobalLogicalMaximum
     *
     * @since S60 ?S60_version *** for example, S60 v3.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalLogicalMaximum( const TItem& aItem );

    /**
     * HandleGlobalPhysicalMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalPhysicalMinimum( const TItem& aItem );

    /**
     * HandleGlobalPhysicalMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalPhysicalMaximum( const TItem& aItem );

    /**
     * HandleGlobalUnit
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalUnit( const TItem& aItem );

    /**
     * HandleGlobalUnitExponent
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalUnitExponent( const TItem& aItem );

    /**
     * HandleGlobalReportSize
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalReportSize( const TItem& aItem );

    /**
     * HandleGlobalReportSize
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalReportCount( const TItem& aItem );

    /**
     * HandleGlobalPush
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalPushL( const TItem& aItem );

    /**
     * HandleGlobalPop
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleGlobalPop( const TItem& aItem );

    /**
     * HandleLocalUsage
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalUsageL( const TItem& aItem );

    /**
     * HandleLocalUsageMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalUsageMinimum( const TItem& aItem );

    /**
     * HandleLocalUsageMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalUsageMaximum( const TItem& aItem );

    /**
     * HandleLocalDesignatorIndex
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalDesignatorIndex( const TItem& aItem );

    /**
     * HandleLocalDesignatorMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalDesignatorMinimum( const TItem& aItem );

    /**
     * HandleLocalDesignatorMaximum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalDesignatorMaximum( const TItem& aItem );

    /**
     * HandleLocalStringIndex
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalStringIndex( const TItem& aItem );

    /**
     * HandleLocalStringMinimum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalStringMinimum( const TItem& aItem );

    /**
     * HandleLocalStringMaximum
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalStringMaximum( const TItem& aItem );

    /**
     * HandleLocalDelimiter
     *
     * @since S60 v5.0
     * @param aItem a Hid field item
     * @result error code
     */
    TInt HandleLocalDelimiter( const TItem& aItem );
    
    /**
     * CheckMandatoryFieldError
     *
     * @since S60 v5.0
     * @param aType a field type
     * @param aAttributes attributes
     * @result error code
     */
    TInt CheckMandatoryFieldExistence( CField::TType aType, TUint32 aAttributes );
    
    /**
     * CheckUsageMinAndMaxErrors
     *
     * @since S60 v5.0     
     * @result error code
     */
    TInt CheckUsageMinAndMaxErrors();
    
    /**
     * CheckDesignatorMinAndMaxErrors
     *
     * @since S60 v5.0     
     * @result error code
     */
    TInt CheckDesignatorMinAndMaxErrors();
    
    /**
     * CheckStringMinAndMaxErrors
     *
     * @since S60 v5.0     
     * @result error code
     */
    TInt CheckStringMinAndMaxErrors();
    
    /**
     * CheckMandatoryFieldError
     *
     * @since S60 v5.0
     * @param aType a field type
     * @param aAttributes attributes
     * @result error code
     */
    TInt CheckMandatoryFieldErrors( CField::TType aType, TUint32 aAttributes );
    
    /**
     * CheckLogicalMinAndMax
     * Check if logical minimum and maximum must match the number of usage
     * values 
     *
     * @since S60 v5.0     
     * @param aAttributes attributes
     * @result error code
     */
    void CheckLogicalMinAndMax( TUint32 aAttributes );
    
    /**
     * CheckMandatoryFieldError
     *
     * @since S60 v5.0
     * @param aType a field type
     * @param aAttributes attributes
     * @result error code
     */
    void CheckFieldBitNeeded( CField::TType aType, TUint32 aAttributes );
    
    /**
     * HandleItem
     *
     * @since S60 v5.0     
     * @result error code
     */
    void HandleItemL(TItem& aItem);
    
    /**
     * CheckParseErrors
     *
     * @since S60 v5.0     
     * @result error code
     */
    void CheckParseErrors();
    
    /**
     * ResetParser
     *
     * @since S60 v5.0     
     * @result error code
     */
    void ResetParserL();

private:
    /**
     * A bitmask containing the THidItem flags which represent local
     * (as opposed to main or global) items.
     */
    static const TUint32 KLocalItemMask;
    
    /**
     * A bitmask containing the THidItem flags which represent local
     * unused items.
     */
    static const TUint32 KUnusedLocalItemsMask;
    
    /**
     * A bitmask containing the THidItem flags which represent mandatory
     * items.
     */
    static const TUint32 KMandatoryItemMask;
    
    /**
     * A bitmask containing the THidItem flags which represent report
     * items.
     */
    static const TUint32 KReportItemMask;

private:

    /**
     * THidItem defineds a set of flags used for keeping track of
     * which items have been encountered in the report descriptor for
     * the current report field. The flags are used with the
     * iItemsDefined and iGlobalItemsDefined data members.
     */
    enum THidItem
        {
        EUsagePage       = 1<<0,
        ELogicalMin      = 1<<1,
        ELogicalMax      = 1<<2,
        EReportSize      = 1<<3,
        EReportCount     = 1<<4,
        EReportId        = 1<<5,
        EUsageMin        = 1<<6,
        EUsageMax        = 1<<7,
        EPhysicalMin     = 1<<8,
        EPhysicalMax     = 1<<9,
        EUnit            = 1<<10,
        EUnitExponent    = 1<<11,
        EDesignatorIndex = 1<<12,
        EDesignatorMin   = 1<<13,
        EDesignatorMax   = 1<<14,
        EStringIndex     = 1<<15,
        EStringMin       = 1<<16,
        EStringMax       = 1<<17,
        EInputReport     = 1<<18,
        EOutputReport    = 1<<19,
        EFeatureReport   = 1<<20,
        EDelimiter       = 1<<21,
        EUsageId         = 1<<22
       };

    /**
     * HID local tags
     */
    enum THidLocalTags
        {
        ELocalUsage             = 0x00,
        ELocalUsageMinimum      = 0x01,
        ELocalUsageMaximum      = 0x02,
        ELocalDesignatorIndex   = 0x03,
        ELocalDesignatorMinimum = 0x04,
        ELocalDesignatorMaximum = 0x05,
        ELocalStringIndex       = 0x07,
        ELocalStringMinimum     = 0x08,
        ELocalStringMaximum     = 0x09,
        ELocalDelimiter         = 0x0a
        };

    /**
     *  HID global tags
     */
    enum THidGlobalTags
        {
        EGlobalUsagePage       = 0x00,
        EGlobalLogicalMinimum  = 0x01,
        EGlobalLogicalMaximum  = 0x02,
        EGlobalPhysicalMinimum = 0x03,
        EGlobalPhysicalMaximum = 0x04,
        EGlobalUnitExponent    = 0x05,
        EGlobalUnit            = 0x06,
        EGlobalReportSize      = 0x07,
        EGlobalReportId        = 0x08,
        EGlobalReportCount     = 0x09,
        EGlobalPush            = 0x0a,
        EGlobalPop             = 0x0b
        };

    /**
     * HID main tags
     */
    enum THidMainTags
        {
        EMainInput         = 0x08,
        EMainOutput        = 0x09,
        EMainFeature       = 0x0b,
        EMainCollection    = 0x0a,
        EMainEndCollection = 0x0c
        };

    /**
     * The local state, cleared after each "Main" item
     * Own.
     */
    CField* iLocal;

    /**
     * Current global state
     */
    TParserGlobalState iGlobal;

    /**
     * Global state stack, to allow HID descriptor PUSH and POP commands
     */
    RArray<TParserGlobalState> iGlobalStack;

    // For delimiter handling:
    //
    /**
     *  ETrue if currently within a delimiter pair
     */
    TBool iWithinDelimiter;

    /**
     *  Count of alternate USAGE declarations so far
     */
    TInt iAliasCount;

    /**
     *  Count of alternate USAGE_MIN declarations
     */
    TInt iAliasCountMin;

    /**
     *  Count of alternate USAGE_MAX declarations
     */
    TInt iAliasCountMax;

    /**
     * The root node of the collection tree
     * Own.
     */
    CReportRoot* iReportRoot;

    /**
     * Stack used to create the collection tree
     */
    RPointerArray<CCollection> iCollectionStack;

    // Keep track of which items have been seen in the descriptor:
    //
    /**
     * All items forming the current item state
     */
    TUint32 iItemsDefined;

    /**
     * Global items seen since the last main tag
     */
    TUint32 iGlobalItemsDefined;

    /**
     * ETrue if a redundant local item is seen
     */
    TBool iLocalMultipleUse;

    /**
     * Number of the item we're currently processing
     */
    TInt iItemNumber;

    /**
     * Record of any critical error encountered
     */
    TInt iErrorCode;
    
    /**
     * Field count
     */
    TInt iFieldCount;
    };

#endif
