// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef MMTPSESSION_H
#define MMTPSESSION_H

#include <e32cmn.h>
#include <e32def.h>
#include <mtp/mtpdataproviderapitypes.h>

class TMTPTypeRequest;

/** 
Defines the MTP data provider framework MTP session interface.
@publishedPartner
@released
*/
class MMTPSession
    {
public:

    /**
    Provides a handle to the MTP request dataset of the session's active MTP 
    transaction. This method should only be invoked if the current MTP 
    transaction phase is other than EIdle (@see TransactionPhase).
    @return The active MTP transaction request dataset.
    @leave KErrNotReady, if the MTP transaction phase is EIdle.
    @see TransactionPhase.
    */
    virtual const TMTPTypeRequest& ActiveRequestL() const = 0;
    
    /**
    Provides the session identifier assigned by the MTP connection on which
    the session resides. This identifier is unique only to the connection,
    and may also be in use on any other active connection.
    @return The MTP connection's session identifier.
    */
    virtual TUint32 SessionMTPId() const = 0;

    /**
    Provides the unique session identifier assigned by the MTP data provider
    framework. This identifier is unique across all active connections.
    @return The unique session identifier.
    */
    virtual TUint SessionUniqueId() const = 0;
    };
#endif // MMTPSESSION_H
