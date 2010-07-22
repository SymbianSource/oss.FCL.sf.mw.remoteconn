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
* Description:  This is the LCD Bearer Plugin construction parameters
*                declaration
*
*/


#ifndef T_LOCODBEARERPLUGINPARAMS_H
#define T_LOCODBEARERPLUGINPARAMS_H

class MLocodBearerPluginObserver;

/**
 *  LCD Bearer Plugin Callback construction parameters
 *
 *  This interface class is used to pass construction parameters to the
 *  plugins.  These parameters include the callback interface and the ECOM 
 *  plugin implementation UID 
 *
 *  @euser.lib
 *  @since S60 v3.2
 */
class TLocodBearerPluginParams
    {
public:

    /**
     * Constructor.
     *
     * @since S60 v3.2
     * @param  aUid      Implementation UID of the plugin being constructed
     * @param  aObserver Callback interface object
     */
    TLocodBearerPluginParams(const TUid aUid, MLocodBearerPluginObserver& aObserver);

    /**
     * Gets the implementation UID of the plugin
     *
     * @since S60 v3.2
     * @return Implementaion UID
     */
    TUid ImplementationUid() const;

    /**
     * Gets the observer interface object
     *
     * @since S60 v3.2
     * @return The observer object
     */
    MLocodBearerPluginObserver& Observer() const;    

private: // data

    /**
     * The implementation UID
     */
    const TUid iImplementationUid;

    /**
     * Reference to the observer object
     */
    MLocodBearerPluginObserver& iObserver;

    };

#include "locodbearerpluginparams.inl"

#endif // T_LOCODBEARERPLUGINPARAMS_H
