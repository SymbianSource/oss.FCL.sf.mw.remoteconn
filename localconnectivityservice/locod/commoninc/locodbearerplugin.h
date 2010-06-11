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
* Description:  This is the LCD Bearer Plugin base class declaration.
*
*/


#ifndef C_LOCODBEARERPLUGIN_H
#define C_LOCODBEARERPLUGIN_H

#include <e32base.h>
#include "locodbearerpluginparams.h"

/**
 *  Bearer Plugin base class
 *
 *  This is the base class from which bearer plugins inherit.
 *
 * See locodplugin.hrh for the resource registration definitions.
 * 
 *  @lib euser.lib
 *  @since S60 v3.2
 */
class CLocodBearerPlugin : public CBase
    {
public:

    static CLocodBearerPlugin* NewL(TLocodBearerPluginParams& aParams);

    virtual ~CLocodBearerPlugin();

    /**
     * Gets the implementation uid of this plugin
     *
     * @since S60 v3.2
     * @return The implementation uid
     */
    TUid ImplementationUid() const;
    
protected:

    CLocodBearerPlugin(TLocodBearerPluginParams& aParams);

    /**
     * Gets the observer interface object associated with this plugin
     *
     * @since S60 v3.2
     * @return The observer object
     */
    MLocodBearerPluginObserver& Observer() const;

private: // data

    /**
     * UID set by ECOM when the instance is created.  Used when the instance
     * is destroyed.
     */
    TUid iInstanceUid;

    /**
     * Implementation UID of the concrete instance.
     */
    TUid iImplementationUid;

    /**
     * Holds the observer object which will be notified when the operations
     * complete
     * Not own.
     */
    MLocodBearerPluginObserver& iObserver;

    };

#include "locodbearerplugin.inl"

#endif // C_LOCODBEARERPLUGIN_H
