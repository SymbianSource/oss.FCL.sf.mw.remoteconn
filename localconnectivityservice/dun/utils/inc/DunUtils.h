/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef C_CDUNUTILS_H
#define C_CDUNUTILS_H

#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#else
#include <c32comm.h>
#endif
#include <e32std.h>

/**
 *  This class contains common utility methods for DUN
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunUtils ) : public CBase
    {

public:

    /**
     * Connects to comms server
     *
     * @since S60 3.2
     * @param aCommServer Comms server where to connect
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C static TInt ConnectCommsServer( RCommServ& aCommServer );

    /**
     * Sets RComm buffer length
     *
     * @since S60 3.2
     * @param aComm RComm object whose length to set
     * @param aLength Length to set to aComm
     * @return Symbian error code on error, KErrNone otherwise
     */
    IMPORT_C static TInt SetRCommBufferLength( RComm& aComm, TInt aLength );

    };

#endif  // C_CDUNUTILS_H
