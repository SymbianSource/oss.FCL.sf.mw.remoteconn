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
* Description:  This is the LCD Service Plugin base class declaration.
*
*/


#ifndef C_LOCODSERVICEPLUGIN_H
#define C_LOCODSERVICEPLUGIN_H

#include <e32base.h>

#include "locodservicepluginparams.h"
#include "locodbearer.h"
    
/**
 *  Service Plugin base class
 *
 *  This is the base class from which service plugins inherit.
 *
 * When a service plugin is constructed, it should not start any service by default.
 * A service is started on demand when the bearer is available.
 *
 *  @euser.lib
 *  @since S60 v3.2
 */
class CLocodServicePlugin : public CBase
    {
public:

    static CLocodServicePlugin* NewL(TLocodServicePluginParams& aParams);

    virtual ~CLocodServicePlugin();

    /**
     * Tell service plugin which service should be enabled or disabled according to
     * specified bearer and its status.
     * 
     * On completion, the plugin should call 
     * MLocodServicePluginObserver::ManageServiceCompleted() to inform the result.
     *
     * @since S60 v3.2
     * @param  aBearer the bearer identification 
     * @param  aStatus the status of this bearer, ETrue if it is available;
     *                 EFalse otherwise.
     */
    virtual void ManageService(TLocodBearer aBearer, TBool aStatus) = 0;

    TUid ImplementationUid() const;
    
protected:

    CLocodServicePlugin(TLocodServicePluginParams& aParams);

    /**
     * Gets the observer interface object associated with this plugin
     *
     * @since S60 v3.2
     * @return The observer object
     */
    MLocodServicePluginObserver& Observer() const;

private: // data

    /**
     * UID set by ECOM when the instance is created.  Used when the instance
     * is destroyed.
     */
    TUid iInstanceUid;

    /**
     * Implementation UID of the concrete instance.
     */
    const TUid iImplementationUid;

    /**
     * Holds the observer object which will be notified when the operations
     * complete
     * Not own.
     */
    MLocodServicePluginObserver& iObserver;

    };

#include <locodserviceplugin.inl>

#endif // C_LOCODSERVICEPLUGIN_H
