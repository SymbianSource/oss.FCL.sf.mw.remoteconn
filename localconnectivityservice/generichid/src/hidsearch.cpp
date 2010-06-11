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
* Description:  HID field search
*
*/

#include <e32std.h>
#include <e32svr.h>

#include "hidreportroot.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// SearchL()
// ---------------------------------------------------------------------------
//
EXPORT_C void THidFieldSearch::SearchL(const CReportRoot* aReportRoot,
    MHidFieldFinder* aFinder)
    {
    iFinder = aFinder;

    if (aReportRoot)
        {
        TBool done = EFalse;
        for (TInt i=0; (i < aReportRoot->CollectionCount()) && !done; ++i)
            {
            done = !DoSearchL(aReportRoot->CollectionByIndex(i));
            }
        }
    }

// ---------------------------------------------------------------------------
// DoSearchL()
// ---------------------------------------------------------------------------
//
TBool THidFieldSearch::DoSearchL(const CCollection* aCollection)
    {
    if (iFinder->BeginCollection(aCollection))
        {
        TInt i;

        for (i=0; i < aCollection->FieldCount(); ++i)
            {
            iFinder->Field(aCollection->FieldByIndex(i));
            }

        for (i=0; i < aCollection->CollectionCount(); ++i)
            {
            if (!DoSearchL(aCollection->CollectionByIndex(i)))
                {
                return EFalse;
                }
            }
        }

    return iFinder->EndCollection(aCollection);
    }
