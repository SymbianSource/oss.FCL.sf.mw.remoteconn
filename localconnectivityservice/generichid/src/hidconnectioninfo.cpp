/*
* Copyright (c) 2008-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
*
*/



#include "hidconnectioninfo.h"
#include "hidreportroot.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CConnectionInfo::CConnectionInfo(TInt aConnectionID, CReportRoot* aReportRoot):
iConnectionID(aConnectionID),
iReportRoot(aReportRoot)
    { 
    TRACE_FUNC       
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CConnectionInfo* CConnectionInfo::NewL(TInt aConnectionID, CReportRoot* aReportRoot)
    {
    CConnectionInfo* self = CConnectionInfo::NewLC(aConnectionID, aReportRoot);
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CConnectionInfo* CConnectionInfo::NewLC(TInt aConnectionID, CReportRoot* aReportRoot)
    {
    CConnectionInfo* self = new( ELeave ) CConnectionInfo( aConnectionID, aReportRoot);
    CleanupStack::PushL( self );    
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CConnectionInfo::~CConnectionInfo()
    {
    TRACE_FUNC_ENTRY
    delete iReportRoot;
    iReportRoot = NULL;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ConnectionID
// ---------------------------------------------------------------------------
//
TInt CConnectionInfo::ConnectionID()    
    {
    return iConnectionID;
    }

// ---------------------------------------------------------------------------
// ReportRoot
// ---------------------------------------------------------------------------
//
CReportRoot* CConnectionInfo::ReportRoot()
    {    
    return iReportRoot;
    }

// ---------------------------------------------------------------------------
// SetLastCommandHandler
// ---------------------------------------------------------------------------
//
void CConnectionInfo::SetLastCommandHandler(CHidDriver* aHidDriverItem)
    {
    iLastCmdDriver = aHidDriverItem;
    }
// ---------------------------------------------------------------------------
// ReturnLastCommandHandler
// ---------------------------------------------------------------------------
//
CHidDriver* CConnectionInfo::ReturnLastCommandHandler()
    {
    return iLastCmdDriver; 
    }
