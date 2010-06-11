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
* Description:  Main client side interface of DUN
*
*/


#ifndef R_RDUNCLIENT_H
#define R_RDUNCLIENT_H

#include <e32def.h>
#include <e32std.h>
#include <locodserviceplugin.h>

class MDunActive;

/**
 *  Main client side interface of DUN
 *  Manages service from/to LOCOD
 *
 *  @since S60 v3.2
 */
class RDun : public RSessionBase
    {

public:

    RDun();

    /**
     * This function starts DUN open/close for different medias
     *
     * @since S60 3.2
     * @param aRequest Request to manage
     * @param aReqStatus Active object's request status
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt ManageService( MDunActive* aRequest, TRequestStatus& aReqStatus );

    /**
     * This function gets the active connection
     * (For testing purposes only).
     *
     * @since S60 5.0
     * @return Active connection if found, NULL otherwise
     */
    TAny* ActiveConnection();

    /**
     * Version number
     *
     * @since S60 3.2
     * @return Version number of DUN
     */
    TVersion Version() const;

    /**
     * This function connects to DUN server and creates a new session.
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Connect();

    };

#endif  // R_RDUNCLIENT_H
