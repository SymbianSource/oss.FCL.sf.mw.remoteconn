// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPPLAYBACKCONTROLDP_H
#define CMTPPLAYBACKCONTROLDP_H

#include <mtp/cmtpdataproviderplugin.h>
#include <mtp/tmtptypeevent.h>

#include "mtpplaybackcontroldpconst.h"
#include "mmtpplaybackinterface.h"

class MMTPRequestProcessor;
class CMTPPlaybackMap;
class MMTPPlaybackControl;
class CMTPPlaybackProperty;
class CMTPPlaybackEvent;

/** 
Implements the MTP playback control data provider plug-in.
@internalComponent
*/
class CMTPPlaybackControlDataProvider :	public CMTPDataProviderPlugin, public MMTPPlaybackObserver
    {
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPPlaybackControlDataProvider();
    
    CMTPPlaybackMap& GetPlaybackMap() const;
    CMTPPlaybackProperty& GetPlaybackProperty() const;
    MMTPPlaybackControl& GetPlaybackControlL();
    void RequestToResetPbCtrl();
    
private: // From CMTPDataProviderPlugin

    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;

private: //From MMTPPlaybackObserver

    void HandlePlaybackEventL(CMTPPlaybackEvent* aEvent, TInt aErr);
    
private:
    
    CMTPPlaybackControlDataProvider(TAny* aParams);
    void ConstructL();
    
    TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);
    void SendEventL(TMTPDevicePropertyCode aPropCode);
    
private: // Owned

    /**
    The event dataset.
    */     
    TMTPTypeEvent                   iEvent;
    
    /**
    The active request processors table. Multiple request processors may be 
    active in a multi-session MTP environment.
    */
    RPointerArray<MMTPRequestProcessor> iActiveProcessors;
    TInt iActiveProcessor;
    TBool iActiveProcessorRemoved;
    
    /**
    The iPlaybackMap map the device property or operation to valid playback command
    */
    CMTPPlaybackMap* iPlaybackMap;
    
    /**
    The iPlaybackProperty is a container for playback properties.
    */    
    CMTPPlaybackProperty* iPlaybackProperty;
    
    /**
    The checker map the device property or operation to valid playback command
    */
    MMTPPlaybackControl* iPlaybackControl;
    
    TBool iRequestToResetPbCtrl;
    };

#endif // CMTPPLAYBACKCONTROLDP_H

