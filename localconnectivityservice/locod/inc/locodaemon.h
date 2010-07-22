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
* Description:  Defines the Daemon class.
*
*/


#ifndef T_LOCODAEMON_H
#define T_LOCODAEMON_H

#include <e32base.h>
#include <e32property.h>
class CLocodBearerPlugin;
class CLocodServiceMan;

/**
 *  The root of Daemon.
 *
 *  It monitors system state, constructs service manager and loads all bearer
 *  plugins when system is up.
 *  It unloads all bearer and service plugins when system is shuting down.
 *
 *  @euser.lib
 *  @since S60 v3.2
 */
class CLocoDaemon : public CActive
    {
public:

    /**
     *  Factory method, leave the object in cleanupstack.    
     *  @since S60 v3.2
     */
    static CLocoDaemon* NewLC();

    ~CLocoDaemon();

private:
    
    // From CActive
    
    void RunL();
    
    void DoCancel();
    
    TInt RunError(TInt aReason);

private:

    CLocoDaemon();

    void ConstructL();

    /**
     *  Loads the bearer plug ins
     *  @since S60 v3.2
     */
    void LoadBearesL();

private: // data

    // the bearer plugins
    RPointerArray<CLocodBearerPlugin> iBearers;
    
    // the service manager
    CLocodServiceMan* iServiceMan;
    
    // for system state monitoring
    RProperty iSystemPS;
    TInt iSystemState;
    };


#endif // T_LOCODAEMON_H
