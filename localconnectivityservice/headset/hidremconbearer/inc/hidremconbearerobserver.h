/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declaration of class CHidRemconBearerObserver
 *
*/


#ifndef C_HIDREMCONBEAREROBSERVER_H
#define C_HIDREMCONBEAREROBSERVER_H

#include <e32svr.h>
#include <e32property.h>

/**
 * Observer key types
 */
enum TObserverKeyType
    {
    EMediaKeys = 0, 
    EAccessoryVolumeKeys, 
    EMuteKey, 
    EHookKeys
    };

/**
 *  Call Back Receiver
 *  On receiving event from the P & S, CHidRemconBearerObserver calls 
 *  this function
 * 
 */
class MCallBackReceiver
    {
public:
    /**
     * Called when event has arrived from P&S.
     *  
     * @param aEnumValue Enum value representing the key and action
     * @param aKeyType will indicate whether it is Volume key or Media key     
     */
    virtual void ReceivedKeyEvent( TInt aEnumValue, TInt aKeyType ) = 0;
    };

/**
 *  Hid remcon bearer observer
 *  This class inplements the Active Object. There will be two instance 
 *  of this ,one for handling Volume keys and other for Media Keys
 * 
 */
class CHidRemconBearerObserver : public CActive
    {
public:
    // Constructors and destructor       

    /**
     * Two-phased constructor.
     * @param aCallback reference of the class which implements 
     *                  MCallBackReceiver for handling call back 
     *                  functions
     * @param aKeyType  Indiacates whether it is side volume key or 
     *                   media key
     * @return remconbearer observer                   
     */
    static CHidRemconBearerObserver* NewL( MCallBackReceiver& aCallback,
            TInt aKeyType );
    /**
     * Destructor.
     */
    virtual ~CHidRemconBearerObserver();

public:
    /**
     * Start remconbearer observer key events
     *  
     * @return Error code
     */
    TInt Start();

    /**
     * Stop remconbearer observer key events
     *
     */
    void Stop();

private:
    /**
     * Two-phased constructor.
     * @param aCallback reference of the class which implements 
     *                  MCallBackReceiver for handling call back 
     *                  functions
     */
    CHidRemconBearerObserver( MCallBackReceiver& aCallback );

    /**
     * Two-phased constructor.
     * @param aKeyType  Indiacates whether it is side volume key or 
     *                   media key                            
     */
    void ConstructL( TInt aKeyType );

private:
    // From Base class CActive       

    /**
     * From CActive
     * Handles an active object's request completion event.
     *         
     */
    void RunL();

    /**
     * From CActive
     * Canceling outstanding request
     *         
     */
    void DoCancel();

private:
    // Data
    /**
     * RProperty for key subscribing key values
     */
    RProperty iProperty;

    /**
     * Callback to request handler
     */
    MCallBackReceiver& iCallback;

    /**
     * Key type
     */
    TInt iKeyType; // Vol key or media Key
    };

#endif // HIDREMCONBEAREROBSERVER_H
// End of file
