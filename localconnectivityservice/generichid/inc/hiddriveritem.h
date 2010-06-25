/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares driver list itemapplication class.
*
*/



#ifndef C_DRIVERLISTITEM_H
#define C_DRIVERLISTITEM_H

#include "hidreportroot.h"
#include "hidinterfaces.h"

/**
 *  Container class so we can maintain a list of driver instances
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
class CDriverListItem : public CBase
    {
    // Give access to the TSglQueLink offset:
    friend class CGenericHid;

public :

    explicit CDriverListItem(TInt aConnectionId);

    /**
    * Destructor.
    */
    virtual ~CDriverListItem();


    /**
     * Set driver instance
     *
     * @since S60 v5.0
     * @param aDriver Driver instance
     * @return None
     */
    void SetDriver(CHidDriver* aDriver);

    /**
     * Return connection ID
     *
     * @since S60 v5.0
     * @return Connection id
     */
    TInt ConnectionId() const;

    /**
     * Return driver instance
     *
     * @since S60 v5.0
     * @return driver instance
     */
    CHidDriver* Driver() const;
        

private :

    /**
     * Queue link
     */
    TSglQueLink iSlink;

    /**
     * The transport-layer ID for the connection
     */
    TInt iConnectionId;

    /**
     * The device driver
     * Own.
     */
    CHidDriver* iDriver;
    
    };
#endif

