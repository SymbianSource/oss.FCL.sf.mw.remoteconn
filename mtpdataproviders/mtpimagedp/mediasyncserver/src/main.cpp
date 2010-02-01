// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalTechnology
*/

#include <e32base.h>
#include "cmediasyncserver.h"


//  Global Functions
/**
Process entry point
*/
TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt ret = KErrNoMemory;
    if (cleanup)
        {
        // Run application code inside TRAP harness, wait keypress when terminated
        TRAP(ret, CMediaSyncServer::RunServerL());
        delete cleanup;
        }    
    __UHEAP_MARKEND;
    return ret;
    }
