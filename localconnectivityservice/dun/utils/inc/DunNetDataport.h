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
* Description:  Dataport specific network resource accessor implementation
*
*/


#ifndef C_CDUNNETDATAPORT_H
#define C_CDUNNETDATAPORT_H

#include <e32base.h>
#ifdef PRJ_USE_NETWORK_STUBS
#include <c32comm_stub.h>
#include <etelmm_stub.h>
#else
#include <etelmm.h>
#endif

/**
 *  Class to store data needed for each separate "call"
 *  One TDunDataportEntity entity contains objects needed for one network data
 *  connection
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( TDunDataportEntity )
    {

public:

    /**
     * Dataport for one call
     */
    RComm iDataport;

    /**
     * Call for this entity
     */
    RMobileCall iMobileCall;

    /**
     * Specifies whether this entity is in use or not
     */
    TBool iEntityInUse;

    };

/**
 *  Class for common network resource usage
 *  This class provides network side independent functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunNetwork )
    {

public:

    /**
     * Abstract; Initializes network
     * Must be called before any other operation
     *
     * @since S60 3.2
     * @return None
     */
    virtual void InitializeL() = 0;

    };

/**
 *  Class for Dataport specific network resource usage
 *  This class is provides Dataport (RComm) dependent functionality
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( MDunNetDataport ) : public MDunNetwork
    {

public:

    /**
     * Abstract; Called when channel was created by transporter
     * Initializes network for channel creation
     *
     * @since S60 3.2
     * @param aComm Network ID returned when allocation successful
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt AllocateChannel( RComm*& aComm ) = 0;

    /**
     * Abstract; Called when channel was deleted/closed by transporter
     * Uninitializes network for channel deletion/close
     *
     * @since S60 3.2
     * @param aComm Network ID of owning entity that will be freed
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt FreeChannel( RComm* aComm ) = 0;

    /**
     * Gets index by network ID for Dataport
     *
     * @since S60 3.2
     * @param aComm Index to get by this network ID
     * @return Symbian error code on error, index otherwise
     */
    virtual TInt GetIndexById( RComm* aComm ) = 0;

    };

/**
 *  Class for accessing network functionality through Dataport
 *  This class is needed by CDunTransporter to create abstract "channels"
 *  for data communication.
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunNetDataport ) : public CBase, public MDunNetDataport
    {

public:

    /**
     * Two-phased constructor.
     * @param aNumOfMaxChannels Maximum number of supported Dataport ports
     *                          for DUN
     * @return Instance of self
     */
	static CDunNetDataport* NewL( TInt aNumOfMaxChannels );

    /**
    * Destructor.
    */
    ~CDunNetDataport();

// from base class MDunNetwork (MDunNetDataport -> MDunNetwork)

    /**
     * From MDunNetwork (MDunNetDataport -> MDunNetwork).
     * Initializes network for Dataport
     * Must be called before any other operation
     *
     * @since S60 3.2
     * @return None
     */
    void InitializeL();

// from base class MDunNetDataport

    /**
     * From MDunNetDataport.
     * Called when channel was created by transporter for Dataport
     * Initializes network for channel creation
     *
     * @since S60 3.2
     * @param aComm Network ID returned when allocation successful
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AllocateChannel( RComm*& aComm );

    /**
     * From MDunNetDataport.
     * Called when channel was deleted/closed by transporter for Dataport
     * Uninitializes network for channel deletion/close
     *
     * @since S60 3.2
     * @param aComm Network ID of owning entity that will be freed
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt FreeChannel( RComm* aComm );

    /**
     * From MDunNetDataport.
     * Gets index by network ID for Dataport
     *
     * @since S60 3.2
     * @param aComm Index to get by this network ID
     * @return Symbian error code on error, index otherwise
     */
    TInt GetIndexById( RComm* aComm );

private:

    CDunNetDataport( TInt aNumOfMaxChannels );

    void ConstructL();

    /**
     * Allocates phone objects for use
     *
     * @since S60 3.2
     * @return None
     */
    void AllocatePhoneObjectsL();

    /**
     * Initializes first free entity
     *
     * @since S60 3.2
     * @return Symbian error code on error, found index otherwise
     */
    TInt InitializeFirstFreeEntity();

    /**
     * Remove network entity by index
     *
     * @since S60 3.2
     * @param aIndex Index which entity to remove
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt RemoveEntity( TInt aIndex );

    /**
     * Deletes own internal data
     *
     * @since S60 3.2
     * @return None
     */
    void DeleteNetwork();

    /**
     * Deletes one network entity at index aIndex for Dataport
     *
     * @since S60 3.2
     * @param aIndex Index where to delete an entity
     * @param aCheckFree Check free status before deletion
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt DeleteNetworkEntity( TInt aIndex, TBool aCheckFree );

private:  // data

    /**
     * Maximum number of supported Dataport ports for DUN
     */
    TInt iNumOfMaxChannels;

    /**
     * Array of abstractions of each "call".
     * One TDunDataportEntity entity contains objects needed for one network
     * data connection
     */
    RArray<TDunDataportEntity> iEntities;

    /**
     * Communication server needed for Dataport port opening, port information
     * fetching, and Dataport communication module loading and unloading
     */
    RCommServ iCommServer;

    /**
     * Telephony server needed for opening iMobilePhone mobile phone session
     * and for telephony module loading and unloading
     */
    RTelServer iTelServer;

    /**
     * Mobile phone server needed for opening iMobileLine mobile line session
     */
    RMobilePhone iMobilePhone;

    /**
     * Mobile line server needed for opening new calls to Dataport.
     * Each new call is also new network data connection
     */
    RMobileLine iMobileLine;

    };

#endif  // C_CDUNNETDATAPORT_H
