// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalComponent
*/
#include "cmtpcontrollerbase.h"

TLocodBearer CMTPControllerBase::Bearer() const
    {
    return iBearer;
    }

CMTPControllerBase::CMTPControllerBase( CMTPBearerMonitor& aMon, TLocodBearer aBearer ):
    iMon( aMon ),
    iBearer( aBearer )
    {
    }

CMTPBearerMonitor& CMTPControllerBase::Monitor() const
    {
    return iMon;
    }
