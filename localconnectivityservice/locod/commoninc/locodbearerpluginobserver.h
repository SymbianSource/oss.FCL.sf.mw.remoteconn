/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the LCD Bearer Plugin observer interface definition.
*
*/


#ifndef M_LOCODBEARERPLUGINOBSERVER_H
#define M_LOCODBEARERPLUGINOBSERVER_H

#include "locodbearer.h"

/**
 *  LCD Bearer Plugin Observer interface class
 *
 *  This is the bearer plugin observer interface definition used by LCD
 *  Bearer Plugins to inform the daemon about the plugin's event.
 *
 *  @euser.lib
 *  @since S60 v3.2
 */
class MLocodBearerPluginObserver
    {
public:

    /**
     * This is a callback function used by the plugins to inform when the
     * bearer's availability changes.
     *
     * @since S60 v3.2
     * @param  aBearer the bearer which calls this nothification
     * @param  aStatus the status of this bearer, ETrue if it is available;
     *                 EFalse otherwise.
     */
    virtual void NotifyBearerStatus(TLocodBearer aBearer, TBool aStatus) = 0;
    };

#endif // M_LOCODBEARERPLUGINOBSERVER_H
