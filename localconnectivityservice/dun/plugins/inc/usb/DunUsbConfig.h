/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  DUN USB configuration accessor and listener
*
*/


#ifndef C_CDUNUSBCONFIG_H
#define C_CDUNUSBCONFIG_H

#include <e32property.h>
#include <usb/acmconfig.h>
#include "DunPlugin.h"

/**
 *  Notification interface class to report USB ACM configuration change
 *
 *  @lib dunusb.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunUsbConfig )
	{

public:

    /**
     * Gets called when one or more ACM configurations are added
     *
     * @since S60 3.2
     * @param aIndex Index (also port number) of added ACM configuration
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyConfigAddition( TInt aIndex ) = 0;

    /**
     * Gets called when one or more ACM configurations are removed
     *
     * @since S60 3.2
     * @param aIndex Index (also port number) of removed ACM configuration
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt NotifyConfigRemoval( TInt aIndex ) = 0;

	};

/**
 *  Class for detecting USB ACM configuration change
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunUsbConfig ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Server
     * @param aCallback Pointer to listener callback
     * @param aProtocol Protocol number to listen
     * @return Instance of self
     */
	static CDunUsbConfig* NewL( MDunServerCallback* aServer,
                                MDunUsbConfig* aCallback,
                                TUint8 aProtocol );

    /**
    * Destructor.
    */
    virtual ~CDunUsbConfig();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Compares initialized protocol number to configuration by index
     *
     * @since S60 3.2
     * @param aIndex Index for which to compare protocol number
     * @param aValidity If initialized protocol matched at aIndex then ETrue
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetConfigValidityByIndex( TInt aIndex, TBool& aValidity );

    /**
     * Starts listening for ACM configuration changes
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops listening for ACM configuration changes
     *
     * @since S60 3.2
     * @return None
     */
    void Stop();

private:

    CDunUsbConfig( MDunServerCallback* aServer,
                   MDunUsbConfig* aCallback,
                   TUint8 aProtocol );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /*
     * Gets current ACM configuration
     */
    TInt GetConfiguration( TPublishedAcmConfigs& aConfig );

// from base class CActive

    /**
     * From CActive.
     * Gets called when ACM configuration changes
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();

private:  // data

    /**
     * Callback to call when notification via MDunServerCallback to be made
     * Not own.
     */
    MDunServerCallback* iServer;

    /**
     * Callback to call when notification(s) via MDunUsbConfig to be made
     */
    MDunUsbConfig* iCallback;

    /*
     * Protocol number of USB ACM to which listening is done
     */
    TUint8 iProtocol;

    /**
     * ACM config from the previous Get()
     * Used to check whether configs are added or removed and also array data
     * needed for check in the removal case
     */
    TPublishedAcmConfigs iConfig;

    /*
     * ACM configuration property
     */
    RProperty iAcmProperty;

    /*
     * Flag to indicate if configuration already received
     */
    TBool iConfigExist;

    };

#endif  // C_CDUNUSBCONFIG_H
