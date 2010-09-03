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

#ifndef CMTPPROXYDP_H
#define CMTPPROXYDP_H

#include <mtp/cmtpdataproviderplugin.h>
#include "rmtpdpsingletons.h"
#include "rmtpproxydpsingletons.h"
class MMTPRequestProcessor;

/** 
Implements the proxy data provider.
@internalComponent
  
*/
class CMTPProxyDataProvider : public CMTPDataProviderPlugin
    {
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPProxyDataProvider();
	
private: // From CMTPDataProviderPlugin

	void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;
 	void SupportedL(TMTPSupportCategory aCategory, CDesCArray& aStrings) const;
	void ConstructL();

private:

    CMTPProxyDataProvider(TAny* aParams);
	
	TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
	TInt LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);	 
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);
private:

	RPointerArray<MMTPRequestProcessor>		iActiveProcessors;	//keep track of the active processors (might from different sessions)
  
    /* DP utility singletons
    */
    RMTPDpSingletons					iDpSingletons;
    
    TInt									iActiveProcessor;
    TBool									iActiveProcessorRemoved;
    RMTPProxyDpSingletons   iSingletons;

    };
#endif // CMTPPROXYDP_H
