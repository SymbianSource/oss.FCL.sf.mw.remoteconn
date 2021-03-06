/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Driver list item implementation
*
*/


#include <e32std.h>
#include <e32base.h>
#include <e32uid.h>

#include "hiddriveritem.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CDriverListItem::CDriverListItem(TInt aConnectionId)
    : iConnectionId(aConnectionId)
    {
    }
// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CDriverListItem::~CDriverListItem()
    {
    delete iDriver;    
    }

// ---------------------------------------------------------------------------
// ConnectionId()
// ---------------------------------------------------------------------------
//
TInt CDriverListItem::ConnectionId() const
    {
    return iConnectionId;
    }

// ---------------------------------------------------------------------------
// Driver()
// ---------------------------------------------------------------------------
//
CHidDriver* CDriverListItem::Driver() const
    {
    return iDriver;
    }

// ---------------------------------------------------------------------------
// SetDriver()
// ---------------------------------------------------------------------------
//
void CDriverListItem::SetDriver(CHidDriver* aDriver)
    {
    delete iDriver;
    iDriver = aDriver;
    }

