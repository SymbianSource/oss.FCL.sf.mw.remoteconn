/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Hid headset field finder 
 *
*/

#ifndef T_FINDER_H
#define T_FINDER_H

#include <e32std.h>
#include "hidreportroot.h"

/**
 * Headset finder
 *
 * Field finder for the consumer/multimedia keys field.
 *
 */
class THeadsetFinder : public MHidFieldFinder
    {
public:
    // From MHidFieldFinder
    /**
     * From MHidFieldFinder
     * Called by THidFieldSearch::SearchL() during traversal of the
     * report descriptor tree when a CCollection is encountered. It
     * will be called once, and only once, for every CCollection in the
     * descriptor. It is not called for the root collection
     * (the CReportRoot).
     *     
     * @param aCollection A pointer to the collection object.     
     * @return ETrue if the contents of this collection (any child CField
     *         or CCollection objects) should be examined.  A driver would return
     *         EFalse if a collection was not of a compatible type, for
     *         example if the usage page was inappropriate.
     */
    virtual TBool BeginCollection( const CCollection *aCollection );

    /**
     * From MHidFieldFinder
     * Called by THidFieldSearch::SearchL() during traversal of the
     * report descriptor tree when all CFields and child CCollections
     * of a CCollection have been examined.  It will be called once,
     * and only once, for every CCollection in the descriptor. It is
     * not called for the root collection (the CReportRoot).
     *     
     * @param aCollection Collection pointer
     * @return ETrue if the search (tree traversal) should
     *         continue. A driver returns EFalse if it has finished examining
     *         the whole descriptor, in general this will be if it has
     *         established that it is compatible with the report descriptor.
     */
    virtual TBool EndCollection( const CCollection *aCollection );

    /**
     * From MHidFieldFinder
     * Called once for each CField in a CCollection by
     * THidFieldSearch::SearchL() during the traversal of a report
     * descriptor tree.
     *     
     * @param aField THe pointer to field     
     */
    virtual void Field( const CField* aField );

public:

    /** 
     * Constructor 
     */
    THeadsetFinder();

    /**
     * Check whether supported fields has been found.
     *     
     * @return ETrue if it has.
     */
    TBool Found() const;

    /**
     * Check whether a given field contains the consumer usages.
     *     
     * @param aField Pointer to the field to test.
     * @return ETrue if it does.
     */
    TBool IsConsumer( const CField* aField ) const;

    /**
     * Check whether a given field contains the telephony usages.
     *     
     * @param aField Pointer to the field to test.
     * @return ETrue if it does.
     */
    TBool IsTelephony( const CField* aField ) const;

    /**
     * Reset the field pointer array
     *
     */
    void EmptyList();

    /**
     * Returns number of supported fields.
     *
     * @return Field count
     */
    TInt FieldCount();

    /**
     * Read the field pointer from array at specified index
     *     
     * @param aIndex Index to the field pointer array
     * @return Pointer to the desired field
     */
    CField* GetFieldAtIndex( TInt aIndex );

private:

    /**
     *  Pointer to the top level application collection being searched. 
     *  Not own.
     */
    const CCollection* iAppCollection;

    /**
     *  Array of field pointers:
     *  Not own.
     */
    RPointerArray<CField> iFieldList;
    };
#endif
