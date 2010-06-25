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
* Description:  HID field descriptor definition
*
*/

#ifndef C_FIELD_H
#define C_FIELD_H


#include <e32base.h>


const TInt   KFieldAttributeConstant      = 1<<0;  //!< Data (0) or Constant (1)
const TInt   KFieldAttributeVariable      = 1<<1;  //!< Array (0) or Variable (1)
const TInt   KFieldAttributeRelative      = 1<<2;  //!< Absolute (0) or Relative (1)
const TInt   KFieldAttributeWrap          = 1<<3;  //!< No wrap (0) or Wrap (1)
const TInt   KFieldAttributeNonLinear     = 1<<4;  //!< Linear (0) or Non-linear (1)
const TInt   KFieldAttributeNoPreferred   = 1<<5;  //!< Preferred state (0) or not (1)
const TInt   KFieldAttributeNullState     = 1<<6;  //!< No null position (0) or null state (1)
const TInt   KFieldAttributeVolatile      = 1<<7;  //!< Non-volatile (0) or volatile (1)
const TInt   KFieldAttributeBufferedBytes = 1<<8;  //!< Bit field (0) or buffered bytes (1)
    
const TInt KSizeOfByte      = 8;
/**
 *  HID report field presentation
 *  Represents an individual field in a HID report, ie. the attributes
 *  of an input, output or feature Main item defined in the HID report
 *  descriptor.
 *
 *  The CField contains information such as the usage IDs sent in the
 *  the report, the type of the report (input, output or feature) and
 *  the logical range.
 *
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class CField : public CBase
  {
public:
    enum TType
        {
        EInput,    //!< Input report
        EOutput,   //!< Output report
        EFeature   //!< Feature report
        };

    

public:
  static CField* NewL();
  static CField* NewLC();

  /**
   * Destructor.
   */
  virtual ~CField();

public:
  // Accessors:

  /**
   * Set Report root
   *
   * @since S60 v5.0
   * @return None
   */
  IMPORT_C TInt UsagePage() const;

  /**
   * Return report id
   *
   * @since S60 v5.0
   * @return report id
   */
  IMPORT_C TInt ReportId() const;

  /**
   * Return offset
   *
   * @since S60 v5.0
   * @return offset
   */
  IMPORT_C TInt Offset() const;

  /**
   * Return size
   *
   * @since S60 v5.0   
   * @return None
   */
  IMPORT_C TInt Size() const;

  /**
   * Return count
   *
   * @since S60 v5.0
   * @return count
   */
  IMPORT_C TInt Count() const;

  /**
   * Return logical minimium
   *
   * @since S60 v5.0
   * @return Logical minimium
   */
  IMPORT_C TInt LogicalMin() const;

  /**
   * Return logical maximum
   *
   * @since S60 v5.0   
   * @return logical maximum
   */
  IMPORT_C TInt LogicalMax() const;

  /**
   * Return usage min
   *
   * @since S60 v5.0
   * @return None
   */
  IMPORT_C TInt UsageMin() const;

  /**
   * Return usage max
   *
   * @since S60 v5.0   
   * @return usage max
   */
  IMPORT_C TInt UsageMax() const;

  /**
   * Return Physical minimum
   *
   * @since S60 v5.0
   * @return physical minimum
   */
  IMPORT_C TInt PhysicalMin() const;

  /**
   * Return Physical max
   *
   * @since S60 v5.0
   * @return return physical max
   */
  IMPORT_C TInt PhysicalMax() const;

  /**
   * Return unit
   *
   * @since S60 v5.0
   * @return Unit
   */
  IMPORT_C TInt Unit() const;

  /**
   * Return unit exponent
   *
   * @since S60 v5.0
   * @return unit exponent
   */
  IMPORT_C TInt UnitExponent() const;

  /**
   * Return Designator index
   *
   * @since S60 v5.0
   * @return Designator Index
   */
  IMPORT_C TInt DesignatorIndex() const;

  /**
   * Return designator minimium
   *
   * @since S60 v5.0
   * @return return designator index minimium
   */
  IMPORT_C TInt DesignatorMin() const;

  /**
   * Return designator maximium
   *
   * @since S60 v5.0
   * @return designator maximium
   */
  IMPORT_C TInt DesignatorMax() const;

  /**
   * Return string minimum
   *
   * @since S60 v5.0
   * @return None
   */
  IMPORT_C TInt StringMin() const;

  /**
   * Return string maximum
   *
   * @since S60 v5.0
   * @return string maximum
   */
  IMPORT_C TInt StringMax() const;

  /**
   * Return string index
   *
   * @since S60 v5.0
   * @return string index
   */
  IMPORT_C TInt StringIndex() const;

  /**
   * Return attributes
   *
   * @since S60 v5.0
   * @return attributes
   */
  IMPORT_C TUint32 Attributes() const;

  /**
   * Return type
   *
   * @since S60 v5.0
   * @return type
   */
  IMPORT_C TType Type() const;

  /**
   * Return variable status
   *
   * @since S60 v5.0
   * @return variable status
   */
  IMPORT_C TBool IsVariable() const;

  /**
   * Return array status
   *
   * @since S60 v5.0
   * @return arrau status
   */
  IMPORT_C TBool IsArray() const;

  /**
   * Return data status
   *
   * @since S60 v5.0
   * @return data status
   */
  IMPORT_C TBool IsData() const;

  /**
   * Return constant status
   *
   * @since S60 v5.0
   * @return constant status
   */
  IMPORT_C TBool IsConstant() const;

  /**
   * Return input status
   *
   * @since S60 v5.0
   * @return input status
   */
  IMPORT_C TBool IsInput() const;

  /**
   * Return output status
   *
   * @since S60 v5.0
   * @return None
   */
  IMPORT_C TBool IsOutput() const;

  /**
   * Return feature status
   *
   * @since S60 v5.0
   * @return feature status
   */
  IMPORT_C TBool IsFeature() const;

  /**
   * Check if reportid is in report
   *
   * @since S60 v5.0
   * @param aReportId report id
   * @return true if reportid is in report
   */
  IMPORT_C TBool IsInReport(TInt aReportId) const;

  /**
   * Check if usage exsist
   *
   * @since S60 v5.0
   * @param aUsage Usage id
   * @return None
   */
  IMPORT_C TBool HasUsage(TInt aUsage) const;

  /**
   * Return usage array
   *
   * @since S60 v5.0
   * @return usage array
   */
  IMPORT_C TArray<TInt> UsageArray() const;

  /**
   * Return usage
   *
   * @since S60 v5.0
   * @param aIndex usage array index
   * @return usage
   */
  IMPORT_C TInt Usage(TInt aIndex) const;

  /**
   * Return usage count
   *
   * @since S60 v5.0
   * @return usage count
   */
  IMPORT_C TInt UsageCount() const;

  /**
   * Return last usage
   *
   * @since S60 v5.0
   * @return last usage
   */
  IMPORT_C TInt LastUsage() const;

  /**
   * Set type
   *
   * @since S60 v5.0
   * @param aType type of field
   * @return None
   */
  IMPORT_C void SetType(const TType& aType);

  /**
   * Add usage
   *
   * @since S60 v5.0
   * @param aUsage usage to be added
   * @return None
   */
  IMPORT_C void AddUsageL(TInt aUsage);

  /**
   * Clear usage list
   *
   * @since S60 v5.0
   * @return None
   */
  IMPORT_C void ClearUsageList();

  /**
   * Set usage page
   *
   * @since S60 v5.0
   * @param aUsagePage usagepage to be set
   * @return None
   */
  IMPORT_C void SetUsagePage(TInt aUsagePage);

  /**
   * Set offset
   *
   * @since S60 v5.0
   * @param aOffset offset to be set
   * @return None
   */
  IMPORT_C void SetOffset(TInt aOffset);

  /**
   * Set size
   *
   * @since S60 v5.0
   * @param aSize size to be set
   * @return None
   */
  IMPORT_C void SetSize(TInt aSize);

  /**
   * Set count
   *
   * @since S60 v5.0
   * @param aCount Count to be set
   * @return None
   */
  IMPORT_C void SetCount(TInt aCount);

  /**
   * Set logical minimium
   *
   * @since S60 v5.0
   * @param aMin Logical minimium to be set
   * @return None
   */
  IMPORT_C void SetLogicalMin(TInt aMin);

  /**
   * Set logical maximum
   *
   * @since S60 v5.0
   * @param aMax logical maximum to be used
   * @return None
   */
  IMPORT_C void SetLogicalMax(TInt aMax);

  /**
   * Set usage min
   *
   * @since S60 v5.0
   * @param aMin  usage minimium to be set.
   * @return None
   */
  IMPORT_C void SetUsageMin(TInt aMin);

  /**
   * Set usage maximum
   *
   * @since S60 v5.0
   * @param aMax usage max to be set
   * @return None
   */
  IMPORT_C void SetUsageMax(TInt aMax);

  /**
   * Set Report ID
   *
   * @since S60 v5.0
   * @param aReportId report id to be set
   * @return None
   */
  IMPORT_C void SetReportId(TInt aReportId);

  /**
   * Set attributes
   *
   * @since S60 v5.0
   * @param aAttributes attributes to be set
   * @return None
   */
  IMPORT_C void SetAttributes(TUint32 aAttributes);

  /**
   * Set physical minimium
   *
   * @since S60 v5.0
   * @param aValue physical minimium value
   * @return None
   */
  IMPORT_C void SetPhysicalMin(TInt aValue);

  /**
   * Set Physical maximum
   *
   * @since S60 v5.0
   * @param aValue physical maximum value
   * @return None
   */
  IMPORT_C void SetPhysicalMax(TInt aValue);

  /**
   * Set unit value
   *
   * @since S60 v5.0
   * @param aValue unit value
   * @return None
   */
  IMPORT_C void SetUnit(TInt aValue);

  /**
   * Set unit exponent
   *
   * @since S60 v5.0
   * @param aValue unit exponent valut to be set
   * @return None
   */
  IMPORT_C void SetUnitExponent(TInt aValue);

  /**
   * Set Designator index
   *
   * @since S60 v5.0
   * @param aValue Designator index value
   * @return None
   */
  IMPORT_C void SetDesignatorIndex(TInt aValue);

  /**
   * Set designator minimium
   *
   * @since S60 v5.0
   * @param aValue designator minimum value
   * @return None
   */
  IMPORT_C void SetDesignatorMin(TInt aValue);

  /**
   * Set designator maximium value
   *
   * @since S60 v5.0
   * @param aValue designator maximium value
   * @return None
   */
  IMPORT_C void SetDesignatorMax(TInt aValue);

  /**
   * Set string minimium value
   *
   * @since S60 v5.0
   * @param aValue string minimium value
   * @return None
   */
  IMPORT_C void SetStringMin(TInt aValue);

  /**
   * Set string maximum value
   *
   * @since S60 v5.0
   * @param aValue string maximum value to be set
   * @return None
   */
  IMPORT_C void SetStringMax(TInt aValue);

  /**
   * Set string index
   *
   * @since S60 v5.0
   * @param aValue string index
   * @return None
   */
  IMPORT_C void SetStringIndex(TInt aValue);

  /**
   * Set Logical range
   *
   * @since S60 v5.0
   * @param aMin logical range minimium value
   * @param aMax logical range maximum value
   * @return None
   */
  IMPORT_C void SetLogicalRange(TInt aMin, TInt aMax);

  /**
   * Set Usage range
   *
   * @since S60 v5.0
   * @param aMin usage range minimium value
   * @param aMax usage range maximum value
   * @return None
   */
  IMPORT_C void SetUsageRange(TInt aMin, TInt aMax);

  /**
   * Set Physical range
   *
   * @since S60 v5.0
   * @param aMin physical range minimium value
   * @param aMax physical range maximum value
   * @return None
   */
  IMPORT_C void SetPhysicalRange(TInt aMin, TInt aMax);

  /**
   * Set string range
   *
   * @since S60 v5.0
   * @param aMin string range minimium value
   * @param aMax string range maximum value
   * @return None
   */
  IMPORT_C void SetStringRange(TInt aMin, TInt aMax);

  /**
   * Set designator range
   *
   * @since S60 v5.0
   * @param aMin designator range minimium value
   * @param aMax designator range maximum value
   * @return None
   */
  IMPORT_C void SetDesignatorRange(TInt aMin, TInt aMax);

private:
  CField();

  

private:

  /**
   * Input, output or feature report
   */
  TType iType;

  // For a detailed description of the use of the following members,
  // see "USB Device Class Definition for Human Interface Devices
  // (HID)", Firmware Specification, Version 1.11, USB Implementers'
  // Forum, June 2001.

  /**
   * The usage page this field is associated with (G)
   */
  TInt iUsagePage;

  /**
   * ID for the HID report containing this field (G)
   */
  TInt iReportId;

  /**
   * Field offset (in bits) from start of report
   */
  TInt iPos;

  /**
   * Bit size of each item in the current field (G)
   */
  TInt iSize;

  /**
   * Number of items in the report field (G)
   */
  TInt iCount;

  /**
   * Minimum extent value in logical units (G)
   */
  TInt iLogicalMin;

  /**
   * Maximum extent value in logical units (G)
   */
  TInt iLogicalMax;

  /**
   * Starting usage associated with array / bitmap (L)
   */
  TInt iUsageMin;

  /**
   * Ending usage associated with array / bitmap (L)L)
   */
  TInt iUsageMax;

  /**
   * Unit value (G)
   */
  TInt iUnit;

  /**
   * Value of the unit exponent in base 10 (G)
   */
  TInt iUnitExponent;

  /**
   * Body part used for a control (L)
   */
  TInt iDesignatorIndex;

  /**
   * String associated with a control (L)
   */
  TInt iStringIndex;

  /**
   * Minimum physical extent for a variable item (G)
   */
  TInt iPhysicalMin;

  /**
   * Maximum physical extent for a variable item (G)
   */
  TInt iPhysicalMax;

  /**
   * First string index for a group of strings (L)
   */
  TInt iStringMin;

  /**
   * Last string index for a group of strings (L)
   */
  TInt iStringMax;

  /**
   * Starting designator index (L)
   */
  TInt iDesignatorMin;

  /**
   * Ending designator index (L)
   */
  TInt iDesignatorMax;

  /**
   * Flags associated with a main item (e.g. "array")
   */
  TUint32 iAttributes;

  /**
   * The usage indexes associated with this field (G)
   */
  RArray<TInt> iUsageList;

  // (G) = Global item
  // (L) = Local item
  };

#endif

