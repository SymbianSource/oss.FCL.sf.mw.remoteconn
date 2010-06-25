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
* Description:  This is the LCD Service Plugin construction parameters
*                declaration
*
*/


#ifndef T_LOCODSERVICEPLUGINPARAMS_H
#define T_LOCODSERVICEPLUGINPARAMS_H

class MLocodServicePluginObserver;

/**
 *  LCD Service Plugin Callback construction parameters
 *
 *  This interface class is used to pass construction parameters to the
 *  plugins.  These parameters include the callback interface (usually
 *  implemented by the LCD) and the ECOM plugin implementation UID 
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
class TLocodServicePluginParams
    {
public:
    /**
     * Constructor.
     *
     * @since S60 v3.2
     * @param  aUid      Implementation UID of the plugin being constructed
     * @param  aObserver Callback interface object
     */
    TLocodServicePluginParams(TUid aUid, MLocodServicePluginObserver& aObserver);

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
    MLocodServicePluginObserver& Observer() const;    

private: // data

    /**
     * The implementation UID
     */
    const TUid iImplementationUid;

    /**
     * Reference to the observer object
     */
    MLocodServicePluginObserver& iObserver;

    };

#include "locodservicepluginparams.inl"

#endif // T_LOCODSERVICEPLUGINPARAMS_H
