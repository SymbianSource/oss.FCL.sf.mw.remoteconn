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
* Description:  Defines the service manager class.
*
*/


#ifndef T_LOCODSERVICEMAN_H
#define T_LOCODSERVICEMAN_H

#include <e32base.h>
#include <cenrepnotifyhandler.h>          
#include <CoreApplicationUIsSDKCRKeys.h>
#include <startupdomainpskeys.h>
#include <locodbearerpluginobserver.h>
#include <locodservicepluginobserver.h>

class CLocodService;

/**
 *  The service plugin manager.
 *
 *  It loads, unloads service plugins on demand and manages a service availability
 *  according to bearer status.
 *  @euser.lib
 *  @since S60 v3.2
 */
class CLocodServiceMan : 
    public CActive, 
    public MLocodBearerPluginObserver, 
    public MLocodServicePluginObserver
    {
public:

    /**
     *  Factory method, leave the object in cleanupstack.
     *
     *  @lib ?library
     *  @since S60 v3.2
     */
    static CLocodServiceMan* NewL();

    ~CLocodServiceMan();

public:

private:
    
    // From CActive
    
    void RunL();
    
    void DoCancel();
    
    TInt RunError(TInt aReason);

private:

    // From MLocodBearerPluginObserver
    /**
     * This is a callback function used by the plugins to inform when the
     * bearer's availability changes.
     *
     * @since S60 v3.2
     * @param  aBearer the bearer which calls this nothification
     * @param  aStatus the status of this bearer, ETrue if it is available;
     *                 EFalse otherwise.
     */
    void NotifyBearerStatus(TLocodBearer aBearer, 
        TBool aStatus);
        
    // From MLocodServicePluginObserver
    /**
     * This is a callback function used by the plugins to inform when
     * managing the service have completed.  The parameters passed should be
     * identical to the ones used when the plugin's ManageService() was called,
     * plus this service plugin's implemnetation UID and the completion status.
     *
     * @since  S60 v3.2
     * @param  aBearer the bearer identification passed in ManageService()
     * @param  aStatus the status of this bearer passed in ManageService()
     * @param  aServiceImplUid, the implementation UID of this service plugin.
     * @param  err     KErrNone if the operation succeeded; otherwise a Symbian
     *                 error code.
     */
    void ManageServiceCompleted(TLocodBearer aBearer, 
        TBool aStatus,
        TUid aServiceImplUid,
        TInt err);

private:

    CLocodServiceMan();

    void ConstructL();
    
    void LoadServicesL();    

private: // data
    RPointerArray<CLocodService> iServices;
    TInt iBearerStatus;
    TUid iUidDun;  // DUN id
    };


#endif // T_LOCODSERVICEMAN_H
