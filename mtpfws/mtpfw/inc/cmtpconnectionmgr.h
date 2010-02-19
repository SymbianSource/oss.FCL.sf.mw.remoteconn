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
 @internalComponent
*/


#ifndef CMTPCONNECTIONMGR_H
#define CMTPCONNECTIONMGR_H

#include <e32base.h>
#include "mmtpconnectionmgr.h"
#include "mtpdebug.h"

class CMTPConnection;
class CMTPTransportPlugin;
class MMTPTransportConnection;

/**
Implements the connection manager singleton. The connection manager is a 
container class responsible for loading, storing, and managing 
@see CMTPTransportPlugin instances. The connection manager uses the ECOM 
framework to load and unload transport plug-ins on demand. 

The connection manager is not responsible for controlling or scheduling 
transport connections, but is notified by the loaded transport plug-ins as 
@see MMTPTransportConnection instances are created. At which time it will 
create and bind a new @see CMTPConnection instance to the 
@see MMTPTransportConnection instance being registered.
*/
class CMTPConnectionMgr : 
    public CBase,
    public MMTPConnectionMgr
    {
public:

    static CMTPConnectionMgr* NewL();
    ~CMTPConnectionMgr();
    
    IMPORT_C CMTPConnection& ConnectionL(TUint aConnectionId) const;
    TUint ConnectionCount() const;
    CMTPConnection& operator[](TInt aIndex) const;
	void ConnectionCloseComplete(const TUint& aConnUid);
    IMPORT_C void StartTransportL(TUid aTransport);
    IMPORT_C void StartTransportL(TUid aTransport, const TAny* aParameter);  
    IMPORT_C void QueueTransportL( TUid aTransport, const TAny* aParameter );
    IMPORT_C void SetClientSId(TUid aSecureId);  
    IMPORT_C void StopTransport(TUid aTransport);
    IMPORT_C void StopTransport( TUid aTransport, TBool aByBearer );
    IMPORT_C void StopTransports();
    IMPORT_C TInt TransportCount() const;
    IMPORT_C TUid TransportUid();
                  
private: // From MMTPConnectionMgr

    TBool ConnectionClosed(MMTPTransportConnection& aTransportConnection);
    void ConnectionOpenedL(MMTPTransportConnection& aTransportConnection);
    TUid ClientSId();
    
private:

    CMTPConnectionMgr();
    
    TInt ConnectionFind(TUint aConnectionId) const;
    static TInt ConnectionOrderCompare(const CMTPConnection& aFirst, const CMTPConnection& aSecond);

    void SuspendTransportL( TUid aTransport );
    void UnsuspendTransport( TUid aTransport );
    void ResumeSuspendedTransport();
    static TInt DoResumeSuspendedTransport( TAny* aSelf );
private:

    RPointerArray<CMTPConnection>   iConnections;
    TLinearOrder<CMTPConnection>    iConnectionOrder;
    TInt                            iShutdownConnectionIdx;
    CMTPTransportPlugin*            iTransport;
    TUid                            iTransportUid;
    TUint							iTransportCount;
    TUid 							iSecureId;
    TBool                           iIsTransportStopping;
    
    /**
     * Array storing the UIDs of the suspended transport plugins
     */
    RArray< TUid >                  iSuspendedTransports;
    
    /**
     * Active object which starts suspended transport asynchronously
     */
    CAsyncCallBack*                 iTransportTrigger;
    
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
#endif // CMTPCONNECTIONMGR_H
