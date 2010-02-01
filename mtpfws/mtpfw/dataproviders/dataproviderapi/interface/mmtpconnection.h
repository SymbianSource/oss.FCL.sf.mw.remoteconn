// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
 @released
*/

#ifndef MMTPCONNECTION_H
#define MMTPCONNECTION_H

#include <e32cmn.h>
#include <e32def.h>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPSession;

/** 
Defines the MTP data provider framework MTP connection interface.

@publishedPartner
@released
*/
class MMTPConnection
    {
public:

    /**
    Provides the unique MTP data provider framework assigned identifier of the
    connection.
    @return The MTP data provider framework assigned connection identifier.
    */
    virtual TUint ConnectionId() const = 0;

    /**
    Provides a count of the number of sessions currently active on the 
    connection.
    @return The number of sessions currently active on the connection.
    */
    virtual TUint SessionCount() const = 0;

    /**
    Indicates if a session with the specified MTP connection assigned
    identifier is currently active on the connection.
    @param aMTPId The MTP connection assigned identifier of the session.
    @return ETrue, if a session with the specified session identifier is 
    currently active on the connection, otherwise EFalse.
    */
    virtual TBool SessionWithMTPIdExists(TUint32 aMTPId) const = 0;
    
    /**
    Provides a reference to the session with the specified MTP connection 
    assigned identifier. 
    @param aMTPId The MTP connection assigned identifier of the 
    requested session.
    @return The reference of the session with the specified MTP connection 
    assigned identifier.
    @leave KErrNotFound, if the specified session identifier is not currently
    active on the connection.
    @see SessionWithMTPIdExists
    */
    virtual MMTPSession& SessionWithMTPIdL(TUint32 aMTPId) const = 0;

    /**
    Indicates if a session with the specified MTP connection assigned
    identifier is currently active on the connection.
    @param aUniqueId The MTP connection assigned identifier of the session.
    @return ETrue, if a session with the specified session identifier is 
    currently active on the connection, otherwise EFalse.
    */
    virtual TBool SessionWithUniqueIdExists(TUint32 aUniqueId) const = 0;
    
    /**
    Provides a reference to the session with the specified MTP connection 
    assigned identifier. 
    @param aUniqueId The MTP connection assigned identifier of the 
    requested session.
    @return The reference of the session with the specified MTP connection 
    assigned identifier.
    @leave KErrNotFound, if the specified session identifier is not currently
    active on the connection.
    @see SessionWithMTPIdExists
    */
    virtual MMTPSession& SessionWithUniqueIdL(TUint32 aUniqueId) const = 0;
    };
    
#endif // MMTPCONNECTION_H
