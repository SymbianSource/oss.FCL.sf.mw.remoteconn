// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef CMTPPICTBRIDGEDP_H
#define CMTPPICTBRIDGEDP_H

#include <mtp/cmtpdataproviderplugin.h>
#include <comms-infras/commsdebugutility.h>

#include "mmtppictbridgeenumeratorcallback.h"
#include "cmtprequestprocessor.h"

class CMTPPictBridgeEnumerator;
class CPtpServer;

/** 
Implements the pictbridge data provider plugin.
*/

class CMTPPictBridgeDataProvider:
    public CMTPDataProviderPlugin, 
    public MMTPPictBridgeEnumeratorCallback
    {
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPPictBridgeDataProvider();
    const CPtpServer* PtpServer()const {return iServerP;}

public:
    TUint32 DeviceDiscoveryHandle() const;

public: // From CMTPDataProviderPlugin

    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported (TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;
    void SupportedL(TMTPSupportCategory aCategory, CDesCArray& aStrings)  const;

private: // From MMTPEnumerationCallback

    void NotifyStorageEnumerationCompleteL();
    void NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError);

private:
    CMTPPictBridgeDataProvider(TAny* aParams);
    void ConstructL();
    TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    TInt LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);    
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);

private:
    CMTPPictBridgeEnumerator* iPictBridgeEnumeratorP;  // owned
    CPtpServer* iServerP;  // owned

    /**
    The active request processors table.
    */ 
    RPointerArray<MMTPRequestProcessor> iActiveProcessors;
    };
    
#endif // CMTPPICTBRIDGEDP_H

