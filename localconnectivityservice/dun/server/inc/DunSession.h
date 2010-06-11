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
* Description:  The session object for DUN server
*
*/


#include "DunServer.h"

#ifndef C_CDUNSESSION_H
#define C_CDUNSESSION_H

/**
 *  Manages separate DUN server sessions for client(s)
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunSession ) : public CSession2
    {

public:

    CDunSession( CDunServer* aDunServer );

    /**
    * Destructor.
    */
    virtual ~CDunSession();

    /**
     * Return the version number
     *
     * @since S60 3.2
     * @return Version number
     */
    TVersion Version() const;

// from base class CSession2

    /**
     * From CSession2.
     * Handles the servicing of client requests passed to the server
     *
     * @since S60 3.2
     * @param aMessage Message containing the client request
     * @return None
     */
    void ServiceL( const RMessage2 &aMessage );

    /**
     * From CSession2.
     * Return a reference to DUN server
     *
     * @since S60 3.2
     * @return Server reference
     */
    CDunServer& Server() const;

private:

    /**
     * Handles incoming service messages
     *
     * @since S60 3.2
     * @param aMessage Message containing the client request
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoService( const RMessage2& aMessage );

    /**
     * Handles incoming test service messages
     *
     * @since S60 5.0
     * @param aMessage Message containing the client request
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DoTestService( const RMessage2& aMessage );

private:  // data

    /**
     * Message containing the client request
     */
    RMessage2   iMessage;

    /**
     * Pointer to DUN server
     * Not own.
     */
    CDunServer* iDunServer;

};

#endif  // C_CDUNSESSION_H
