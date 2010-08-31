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
* Description:  This is the LCD Service Plugin observer interface definition.
*
*/


#ifndef M_LOCODSERVICEPLUGINOBSERVER_H
#define M_LOCODSERVICEPLUGINOBSERVER_H

#include "locodbearer.h"

/**
 *  LCD Service Plugin Observer interface class
 *
 *  This is the service plugin observer interface definition used by LCD
 *  Service Plugins to inform the daemon when an action is complete.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class MLocodServicePluginObserver
    {
public:

    /**
     * This is a callback function used by the plugins to inform when
     * managing the service have completed.  The parameters passed should be
     * identical to the ones used when the plugin's ManageService() was called,
     * plus this service plugin's implemnetation UID and the completion status.
     *
     * @since S60 v3.2
     * @param  aBearer the bearer identification passed in ManageService()
     * @param  aStatus the status of this bearer passed in ManageService()
     * @param  aServiceImplUid, the implementation UID of this service plugin.
     * @param  err     KErrNone if the operation succeeded; otherwise a Symbian
     *                 error code.
     */
    virtual void ManageServiceCompleted(
        TLocodBearer aBearer,
        TBool aStatus,
        TUid aServiceImplUid,
        TInt err) = 0;
    };

#endif // M_LOCODSERVICEPLUGINOBSERVER_H
