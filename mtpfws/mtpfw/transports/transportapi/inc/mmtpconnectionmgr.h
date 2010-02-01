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
 @internalTechnology
*/

#ifndef MMTPCONNECTIONMGR_H
#define MMTPCONNECTIONMGR_H

#include <e32cmn.h>

class MMTPTransportConnection;

/** 
Defines the MTP connection manager interface
@internalTechnology
  
*/

class MMTPConnectionMgr
    {
public:

    /**
    Signals that an MTP transport layer connection has become unavailable.
    @param aTransportConnection Interface handle of the MTP transport layer 
    connection instance.
    */
    virtual void ConnectionClosed(MMTPTransportConnection& aTransportConnection) = 0;
    
    /**
    Signals that an MTP transport layer connection has become available.
    @param aTransportConnection Interface handle of the MTP transport layer 
    connection instance.
    @leave One of the system wide error codes, if a processing failure occurs.
    */
    virtual void ConnectionOpenedL(MMTPTransportConnection& aTransportConnection) = 0;
    
    /**
    Returns the Secure Id of the process that sent the request
    */
    virtual TUid ClientSId()=0;
            
    };
#endif // MMTPCONNECTIONMGR_H