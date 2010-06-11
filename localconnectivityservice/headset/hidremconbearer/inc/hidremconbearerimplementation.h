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
* Description:  Declaration of class CHidRemconBearer.
 *
*/


#ifndef C_HIDREMCONBEARER_H
#define C_HIDREMCONBEARER_H

#include <e32base.h>
#include <c32comm.h>
#include <e32property.h>
#include <remcon/remconbearerplugin.h>
#include <remcon/remconbearerinterface.h>
#include <remcon/messagetype.h>
#include <AknCapServerClient.h>

#include "hidremconbearerobserver.h"

//CONSTANTS
const TInt KDataBufferSize = 10;

/**
 *  CHidRemconBearer
 *  Implementation of the Rem Con Bearer Plugin
 * 
 */
class CHidRemconBearer : public CRemConBearerPlugin,
        public MRemConBearerInterface,
        public MCallBackReceiver
    {
public:

    /**
     * Two-phased constructor.
     * @param  aParams required for the CRemConBearerPlugin
     * @return hidremconbearer plugin
     */
    static CHidRemconBearer* NewL( TBearerParams& aParams );

    /**
     * Destructor.
     */
    virtual ~CHidRemconBearer();

private:

    /**
     * Constructor
     * @param  aParams bearer parameters
     */
    CHidRemconBearer( TBearerParams& aParams );

    /**
     * Two - phase construction.
     */
    void ConstructL();

private:
    // from CRemConBearerPlugin

    /** 
     * From class CRemConBearerPlugin
     * Called by RemCon server to get a pointer to an object which implements
     * the bearer API with UID aUid. This is a mechanism for allowing future 
     * change to the bearer API without breaking BC in existing (non-updated) 
     * bearer plugins.
     *         
     * @param aUid Inteface uid
     * @return  Return pointer to interface
     */
    TAny* GetInterface( TUid aUid );

private:
    // from MRemConBearerInterface

    /** 
     * From class MRemConBearerInterface
     * Called by RemCon to retrieve a response on a connection.
     *         
     * @param  aInterfaceUid Inteface uid
     * @param  aTransactionId Transaction id
     * @param  aOperationId operation ID
     * @param  aData API-specific message data.
     * @param  aAddr The connection.
     * @return Error code
     */
    TInt GetResponse( TUid& aInterfaceUid, TUint& aTransactionId,
            TUint& aOperationId, RBuf8& aData, TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Send Remcon command
     *         
     * @param  aInterfaceUid Inteface uid
     * @param  aCommand command id
     * @param  aTransactionId Transaction id         
     * @param  aData API-specific message data.
     * @param  aAddr The connection.
     * @return Error code
     */
    TInt SendCommand( TUid aInterfaceUid, TUint aCommand,
            TUint aTransactionId, RBuf8& aData, const TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Get Remcon command
     *
     * @param  aInterfaceUid Inteface uid
     * @param  aTransactionId Transaction id                  
     * @param  aCommand command id
     * @param  aData API-specific message data.
     * @param  aAddr The connection.
     * @return Error code
     */
    TInt GetCommand( TUid& aInterfaceUid, TUint& aTransactionId,
            TUint& aCommand, RBuf8& aData, TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Send Response
     *         
     * @param  aInterfaceUid Inteface uid
     * @param  aOperationID operation id
     * @param  aTransactionId Transaction id                  
     * @param  aData API-specific message data.
     * @param  aAddr The connection.
     * @return Error code
     */
    TInt SendResponse( TUid aInterfaceUid, TUint aOperationId,
            TUint aTransactionId, RBuf8& aData, const TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Connect request
     *         
     * @param  aAddr The connection.         
     */
    void ConnectRequest( const TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Disconnect request
     *         
     * @param  aAddr The connection.         
     */
    void DisconnectRequest( const TRemConAddress& aAddr );

    /** 
     * From class MRemConBearerInterface
     * Clientstatus
     *
     * @param  aControllerPresent Controllerpresent status
     * @param  aTargetPresent Target present status         
     */
    void ClientStatus( TBool aControllerPresent, TBool aTargetPresent );

    /** 
     * From class MRemConBearerInterface
     * Return security policy
     *
     * @return Security policy
     */
    TSecurityPolicy SecurityPolicy() const;

private:
    // from MCallBackReceiver    

    /** 
     * From class MCallBackReceiver
     * Return security policy
     *
     * @param aScanCode a scancode
     * @param aKeyType a Keytype         
     */
    void ReceivedKeyEvent( TInt aScanCode, TInt aKeyType );

private:
    /** 
     * From class MCallBackReceiver
     * Set SVK Operation ID and Data
     *
     * @param aEnumValue OperationID and data field
     */
    void SetSVKOperationIdAndData( TInt aEnumValue );

    /** 
     * From class MCallBackReceiver
     * Set Mute Operation ID and Data
     *
     * @param aEnumValue OperationID and data field         
     */
    void SetMuteKeyOperationIdAndData( TInt aEnumValue );

    /** 
     * From class MCallBackReceiver
     * Set Media key Operation ID and Data
     *
     * @param aEnumValue OperationID and data field         
     */
    void SetMediaKeyOperationIdAndData( TInt aEnumValue );

    /** 
     * From class MCallBackReceiver
     * Set Media key Operation ID and Data
     *         
     * @param aEnumValue OperationID and data field         
     */
    void SetPhoneKeyOperationIdAndData( TInt aEnumValue );

    /**
     * RestartExpired observer
     *
     * @param aKeyType type of key         
     */
    void RestartKeyObeserver( TInt aKeyType );

    /**
     * Restart Expired observer
     *         
     * @param aEnumValue a value of key
     * @param aKeyType  A type of key         
     */
    void HandleKeyOperation( TInt aEnumValue, TInt aKeyType );

private:
    // Data

    /**
     * Interface uid of last correct-decoded message
     */
    TUid iInterfaceUid;

    /**
     * Operation uid of last correct-decoded message
     */
    TUint iOperationId;

    /**
     * Data of last correct-decoded message
     */
    TBuf8<KDataBufferSize> iData;

    /**
     * Transactionid uid of last correct-decoded message
     */
    TUint iTransactionId;

    /**
     * Media key observer
     * Own.
     */
    CHidRemconBearerObserver* iMediaKeyObserver;

    /**
     * Accessory volume key observer
     * Own.
     */
    CHidRemconBearerObserver* iAccessoryVolKeyObserver;

    /**
     * Mute key observer
     * Own.
     */
    CHidRemconBearerObserver* iMuteKeyObserver;

    /**
     * Phone key observer
     * Own.
     */
    CHidRemconBearerObserver* iHookKeyObserver;

    };

#endif // HIDREMCONBEARERIMPLEMENTATION_H
