/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  EComm interface implementation and the client side of DUN
*
*/


#ifndef C_CDUNPLUGIN_H
#define C_CDUNPLUGIN_H

#include <locodbearer.h>
#include <locodserviceplugin.h>
#include "dunclient.h"

class CDunActive;

/**
 *  Class of EComm interface implementation and the client side of DUN
 *
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunPlugin ) : public CLocodServicePlugin
    {

public:

    /**
     * Two-phased constructor.
     * @param aParams LOCOD service plugin parameters
     * @return Instance of self
     */
    static CDunPlugin* NewL( TLocodServicePluginParams& aParams );

    /**
    * Destructor.
    */
    ~CDunPlugin();

    /**
     * Return owned RServer
     *
     * @since S60 3.2
     * @return RDun
     */
    RDun& Server();

    /**
     * Called by instance of CDunActive to inform Observer of service completed.
     * Destruct the active object.
     *
     * @since S60 3.2
     * @param aRequest Request to service
     * @param aError Error value
     * @return None
     */
    void ServiceCompleted( MDunActive* aRequest, TInt aError );

// from base class CLocodServicePlugin

    /**
     * From CLocodServicePlugin.
     * Implements interface virtual function
     *
     * @since S60 3.2
     * @param aBearer, BT/IR/USB bearer defined in locodbearer.h
     * @param aBearerStatus, the status of this bearer,
              ETrue if it is available;
              EFalse otherwise.
     * @return None
     */
    void ManageService( TLocodBearer aBearer, TBool aBearerStatus );

private:

    CDunPlugin( TLocodServicePluginParams& aParams );

    void ConstructL();

    /**
     * Reports completion status to LOCOD
     *
     * @since S60 3.2
     * @param aBearer Bearer to report
     * @param aBearerStatus Bearer status to report
     * @param aErr Error condition to report
     * @return None
     */
    void ReportCompletion( TLocodBearer aBearer,
                           TBool aBearerStatus,
                           TInt aErr);

private:  // data

    /**
     * Client side of DUN Server
     */
    RDun iServer;

    /**
     * Array of Active Object
     */
    RPointerArray<CDunActive> iActiveContainer;

    };

#endif  // C_CDUNPLUGIN_H
