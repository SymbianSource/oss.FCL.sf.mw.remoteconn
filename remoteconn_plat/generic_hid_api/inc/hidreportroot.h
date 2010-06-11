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
* Description:  Declares hid report class
*
*/


#ifndef C_HIDREPORTROOT_H
#define C_HIDREPORTROOT_H

#include <e32base.h>

#include "hidfield.h"
#include "hidcollection.h"

#include "hidvalues.h"

class CReportRoot;



/**
 * Hid field finder class
 * The MHidFieldFinder class defines the call-back interface used by
 * THidFieldSearch. A device driver uses THidFieldSearch::SearchL()
 * along with a custom MHidFieldFinder object to determine whether it
 * is compatible with a given report descriptor.
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class MHidFieldFinder
    {
public:

    /**
     * Called by THidFieldSearch::SearchL() during traversal of the
     * report descriptor tree when a CCollection is encountered. It
     * will be called once, and only once, for every CCollection in the
     * descriptor. It is not called for the root collection
     * (the CReportRoot).
     *
     * @since S60 v5.0
     * @param aCollection A pointer to the collection object.
     * @return ETrue if the contents of this collection (any child CField
     * or CCollection objects) should be examined.  A driver would return
     * EFalse if a collection was not of a compatible type, for
     * example if the usage page was inappropriate.
     */
    virtual TBool BeginCollection(const CCollection *aCollection) = 0;

    /**
     * Called by THidFieldSearch::SearchL() during traversal of the
     * report descriptor tree when all CFields and child CCollections
     * of a CCollection have been examined.  It will be called once,
     * and only once, for every CCollection in the descriptor. It is
     * not called for the root collection (the CReportRoot).
     *
     * @since S60 v5.0
     * @param aCollection A pointer to the collection object.
     * @return ETrue if the search (tree traversal) should
     * continue. A driver returns EFalse if it has finished examining
     * the whole descriptor, in general this will be if it has
     * established that it is compatible with the report descriptor.
     */
    virtual TBool EndCollection(const CCollection *aCollection) = 0;

    /**
     * Called once for each CField in a CCollection by
     * THidFieldSearch::SearchL() during the traversal of a report
     * descriptor tree.
     *
     * @since S60 v5.0
     * @param aField The pointer to field
     * @return None.
     */
    virtual void Field(const CField* aField) = 0;
    };


/**
 *  Report Size
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class TReportSize
    {
public:

    /**
     * Constructor.
     *
     * @since S60 v5.0
     * @param aReportId The report id
     * @param aType Field type
     * @return Pointer to report size object
     */
    TReportSize(TInt aReportId, CField::TType aType);


    /**
     * TReportSize comparision
     *
     * @since S60 v5.0
     * @param aFirst first report size to be compared.
     * @param aSecond second report size to be compared
     * @return None
     */
    static TBool Match(const TReportSize& aFirst,
        const TReportSize& aSecond);

    /**
     * Report id
     */
    TInt iReportId;

    /**
     * Fiel type
     */
    CField::TType iType;

    /**
     *  Report size
     */
    TInt iSize;
    };


/**
 * Hid field search
 * THidFieldSearch provides a mechanism for traversing a parsed report
 * descriptor tree (a CReportRoot object). It is intended for a device
 * driver to use when it is looking to see if it is compatible with a
 * newly connected device, i.e. provides the appropriate types of
 * report. The driver must provide an object of a class that implements
 * the MHidFieldFinder interface.
 *
 * @since S60 v5.0
 */
class THidFieldSearch
    {
public:

    /**
     * Traverse a parsed report descriptor (a tree of CCollections
     * and CFields) calling the MHidFieldFinder member functions as
     * appropriate.
     *
     * @since S60 v5.0
     * @param aReportRoot THe pointer to field
     * @param aFinder An object that will establish if the report
     *                descriptor is suitable.
     * @return None.
     */
    IMPORT_C void SearchL(const CReportRoot* aReportRoot,
        MHidFieldFinder* aFinder);

private:

    /**
     * Traverse a parsed report descriptor (a tree of CCollections
     * and CFields) calling the MHidFieldFinder member functions as
     * appropriate.
     *
     * @since S60 v5.0
     * @param aReportRoot THe pointer to field
     * @return ETrue when search is done.
     */
    TBool DoSearchL(const CCollection* aCollection);

private:
    MHidFieldFinder* iFinder;
    };

/**
 *  Root report
 *  The top level of the tree of fields and collections in a HID
 *  report descriptor.
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class CReportRoot : public CCollection
    {
public:
    static CReportRoot* NewLC();
    static CReportRoot* NewL();

    /**
     * Destructor
     */
    virtual ~CReportRoot();

    /**
     * Increase the size of a given report by a given number of bits
     *
     * @since S60 v5.0
     * @param aReportId The report id, which size shoud be increased
     * @param aType a Type of field
     * @param aIncrement size of increase
     * @return None
     */
    void IncrementReportSizeL(TInt aReportId,
        CField::TType aType, TInt aIncrement);

    /**
     * Get the size of a given report in bits
     *
     * @since S60 v5.0
     * @param aReportId The report id, which size shoud be increased
     * @param aType a Type of field
     * @return Size of report in bits
     */
    TInt ReportSize(TInt aReportId, CField::TType aType) const;

    /**
     * Get the size of a given report in bytes
     *
     * @since S60 v5.0
     * @param aReportId The report id, which size shoud be increased
     * @param aType a Type of field
     * @return Size of report in bytys
     */
    IMPORT_C TInt ReportSizeBytes(TInt aReportId, CField::TType aType) const;

    /**
     * Get the number of reports
     *
     * @since S60 v5.0
     * @return number of reports
     */
    TInt NumberOfReports() const;

    /**
     * Get the size of a given report by array index, rather than type & ID
     *
     * @since S60 v5.0
     * @param aIndex report index
     * @return report size
     */
    TInt ReportSize(TInt aIndex) const;

private:
    CReportRoot();

    /**
     * Find report index
     *
     * @since S60 v5.0
     * @param aReportId The report id to be found
     * @param aType a Type of field
     * @return report size
     */
    TInt FindReportSizeIndex(TInt aReportId, CField::TType aType) const;

    /**
     * Array to store the size of each report
     */
    RArray<TReportSize> iSizes;
    };

#endif
