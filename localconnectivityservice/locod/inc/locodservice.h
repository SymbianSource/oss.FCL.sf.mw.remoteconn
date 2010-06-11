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
* Description:  A service which constains a service plugin and its status.
*
*/


#ifndef C_LOCODSERVICE_H
#define C_LOCODSERVICE_H

#include <e32base.h>
#include <startupdomainpskeys.h>
#include <locodbearerpluginobserver.h>
#include <locodservicepluginobserver.h>

class CLocodServicePlugin;
class TLocodServiceRequest;

/**
 *  Specific service that is managed by locod
 *   Locod creates all service and add it to 
 *   a an array
 *  @ euser.lib
 *  @since S60 3.2
 */        
class CLocodService : public CBase
    {
public:

    /**
     *  Factory function
     * @since S60 3.2
     * @param aPlugin  the sevice plug ins that will be managed (for example DUN, obex)
     * @return an instance of CLocodService
     */
    static CLocodService* NewL(CLocodServicePlugin& aPlugin);
    
    ~CLocodService();
    
    CLocodServicePlugin& Plugin();
    
    /**
     *  Called to manage service when the bearer status has been changed
     * @since S60 3.2
     * @param aBearer  the bearer whose status has been changed
     * @param aStatus  status of the bearer connected, disconnected or on or off
     * @return  TInt
     */
    TInt ManageService(TLocodBearer aBearer, 
        TBool aStatus); 

    /**
     *  Called when the service has been managed by service plug ins
     * @since S60 3.2
     * @param aBearer  the bearer whose service has been managed
     * @param aStatus  status of the bearer connected, disconnected or on or off
     * @param err   error code that may occured during managing the service
     * @return  TInt
     */
    void ManageServiceCompleted(TLocodBearer aBearer, 
        TBool aStatus,
        TInt err); 

    /**
     *  Check if there are any service pending
     * @since S60 3.2
     * @return  TBool if there are service pending
     */
    TBool HasServiceToManage() const;

private:

    CLocodService(CLocodServicePlugin& aPlugin);
    
    void ConstructL();

private:
    // the service plugin, owned
    CLocodServicePlugin* iPlugin;
    
    // The latest status of this service plugin
    TInt iServiceStatus;
    
    // Queue of ManageService request
    RArray<TLocodServiceRequest> iRequests;
    };
    
/**
 * A ManageService request
 */
class TLocodServiceRequest
    {
public:

    TLocodServiceRequest(TLocodBearer aBearer, TBool aStatus);
    
    // The bearer whose status has changed
    TLocodBearer iBearer;
    
    // The new bearer status
    TBool iStatus;
    
    // The request status
    TBool iRequesting;
    };

#endif // C_LOCODSERVICE_H
