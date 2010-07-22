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
* Description:  Controls asynchronous requests between LOCOD and DUN server
*
*/


#ifndef C_CDUNACTIVE_H
#define C_CDUNACTIVE_H

#include <locodservicepluginobserver.h>

class CDunPlugin;

/**
 *  Class for accessing bearer statuses of CDunActive class
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunActive )
    {

public:

    /**
     * Bearer
     *
     * @since S60 3.2
     * @return Bearer
     */
    virtual TLocodBearer Bearer() = 0;

    /**
     * Bearer status
     *
     * @since S60 3.2
     * @return Bearer status
     */
    virtual TBool BearerStatus() = 0;

    };

/**
 *  Class that controls asynchronous requests between LOCOD and DUN server
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunActive ) : public CActive, public MDunActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aPlugin Plugin
     * @param aBearer Brearer
     * @param aBearerStatus Bearer status
     * @return Instance of self
     */
    static CDunActive* NewL( CDunPlugin* aPlugin,
                             TLocodBearer aBearer,
                             TBool aBearerStatus );

    /**
     * Two-phased constructor.
     * @param aPlugin Plugin
     * @param aBearer Brearer
     * @param aBearerStatus Bearer status
     * @return Instance of self
     */
    static CDunActive* NewLC( CDunPlugin* aPlugin,
                              TLocodBearer aBearer,
                              TBool aBearerStatus );

    /**
    * Destructor.
    */
    virtual ~CDunActive();

    /**
     * Request function
     *
     * @since S60 3.2
     * @return Symbian Error code
     */
    TInt ServiceRequest();

private:

    CDunActive( CDunPlugin* aPlugin,
                TLocodBearer aBearer,
                TBool aBearerStatus );

    void ConstructL();

// from base class CActive

    /**
     * From CActive.
     * Called when read or write operation is ready.
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Cancel current activity.
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();

// from base class MDunActive

    /**
     * From MDunActive.
     * Bearer
     *
     * @since S60 3.2
     * @return Bearer
     */
    TLocodBearer Bearer();

    /**
     * From MDunActive.
     * Bearer status
     *
     * @since S60 3.2
     * @return Bearer status
     */
    TBool BearerStatus();

private:  // data

    /**
     * CDunPlugin
     * Not own. Passed by DunPlugin
     */
    CDunPlugin* iPlugin;

    /**
     * Bearer
     * Not own. Passed by DunPlugin
     */
    TLocodBearer iBearer;

    /**
     * Service state
     * Not own. Passed by DunPlugin
     */
    TBool iBearerStatus;

    };

#endif  // C_CDUNACTIVE_H
