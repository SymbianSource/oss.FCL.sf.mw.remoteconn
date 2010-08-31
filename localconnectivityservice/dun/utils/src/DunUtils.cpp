/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Common utility methods for DUN
*
*/


#include <e32base.h>
#include "DunUtils.h"
#include "DunDebug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Connects to comms server
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunUtils::ConnectCommsServer( RCommServ& aCommServer )
    {
    FTRACE(FPrint(_L( "CDunUtils::ConnectCommsServer()") ));
    TInt retTemp;
#ifndef PRJ_USE_NETWORK_STUBS
    retTemp = StartC32();
    if ( retTemp!=KErrNone && retTemp!=KErrAlreadyExists )
        {
        FTRACE(FPrint(_L( "CDunUtils::ConnectCommsServer() StartC32 %d" ), retTemp));
        return retTemp;
        }
#endif
    retTemp = aCommServer.Connect();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunUtils::ConnectCommsServer() aCommServer.Connect %d" ), retTemp));
        return retTemp;
        }
    FTRACE(FPrint(_L( "CDunUtils::ConnectCommsServer() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets RComm buffer length
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunUtils::SetRCommBufferLength( RComm& aComm, TInt aLength )
    {
    FTRACE(FPrint(_L( "CDunUtils::SetRCommBufferLength()") ));
    TInt retTemp = aComm.SetReceiveBufferLength( aLength );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint(_L( "CDunUtils::SetRCommBufferLength() (set failed) complete (%d)"), retTemp));
        return retTemp;
        }
    TInt setLength = aComm.ReceiveBufferLength();
    if ( setLength != aLength )
        {
        FTRACE(FPrint(_L( "CDunUtils::SetRCommBufferLength() (get failed) complete") ));
        return KErrGeneral;
        }
    FTRACE(FPrint(_L( "CDunUtils::SetRCommBufferLength() complete") ));
    return KErrNone;
    }
