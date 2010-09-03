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

#ifndef CMTPDEVICEDP_H
#define CMTPDEVICEDP_H
#include <e32base.h>
#include <d32usbc.h>
#include <mtp/cmtpdataproviderplugin.h>
#include "mmtpenumerationcallback.h"
#include "rmtpdevicedpsingletons.h"
#include "rmtpdpsingletons.h"
#include "mextndevplugincallback.h"
#include "mmtpdevdpextn.h"
#include "rmtpframework.h"

#include "cmtpgetdevicepropdesc.h"
#include "cmtpdevicedatastore.h"
#include "cmtpdevdpextnmap.h"
#include <mtp/tmtptypeevent.h>

class CMTPDevDpExclusionMgr;
class CMTPFSEnumerator;
class CMTPStorageWatcher;
class MMTPRequestProcessor;
class CMtpExtnDevicePropPlugin;
class CMTPDeviceDataStore;
class CMTPDeviceInfoTimer;
/** 
Implements the MTP device data provider plug-in.
@internalComponent
*/
class CMTPDeviceDataProvider : 
	public CMTPDataProviderPlugin,
	public MMTPEnumerationCallback , public MExtnDevPluginCallback
    {
private:
	enum TEnumerationState
		{
		EUndefined,
		EEnumeratingDeviceDataStore,
		EEnumeratingFolders,
		EEnumerationComplete,
		};
    
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPDeviceDataProvider();
    void SetConnectMac();    
private: // From CMTPDataProviderPlugin

    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;

private: // MMTPEnumerationCallback

	void NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError);

private:
    
    CMTPDeviceDataProvider(TAny* aParams);
    void ConstructL();
    
    TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    TInt LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);    
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);
    TInt FindExtnPlugin(TUint aOpcode);
    void LoadExtnPluginsL();
    void AddFolderRecursiveL( const TMTPNotificationParamsFolderChange& aFolder );
    TUint32 AddEntryL( const TDesC& aPath, TUint32 aParentHandle, TUint32 aStorageId, CMTPObjectMetaData& objectInfo );
    TUint32 GetStorageIdL( const TDesC& aPath );
    void OnDeviceFolderChangedL( TMTPEventCode aEventCode, CMTPObjectMetaData& objectInfo );
    
   public:
    //from MExtnDevPluginCallback
    void OnDevicePropertyChangedL(TMTPDevicePropertyCode& aPropCode);
	
    /**
      * This method return reference MMTPDataProviderFramework
      */
    MMTPDataProviderFramework& DataProviderFramework ();
    
private: // Owned


    /**
    The active request processors table. Multiple request processors may be 
    active in a multi-session MTP environment.
    */
    RPointerArray<MMTPRequestProcessor> iActiveProcessors;
    
    /**
    The device DP singletons.
    */
    RMTPDeviceDpSingletons              iDevDpSingletons;
    
    /**
    The dp utilities singleton.
    */
    RMTPDpSingletons					iDpSingletons;
    
    /**
    The device dp object enumeration state.
    */
    TEnumerationState					iEnumeratingState;
    
    /**
    The device dp folder enumerator.
    */
    CMTPFSEnumerator*					iEnumerator;
    
    /**
    The device dp exlusion manager. 
    */
    CMTPDevDpExclusionMgr*				iExclusionMgr;
    
    /**
    The list of storages to be enumerated.
    */	
    RArray<TUint32>                     iPendingEnumerations;
    
    /**
    The system storages watcher.
    */
    CMTPStorageWatcher*                 iStorageWatcher;
    
    /**
    The device Dp extension plugin map array.
    */   
    RPointerArray<CDevDpExtnPluginMap> iExtnPluginMapArray;//2113
  
    /**
    The event dataset.
    */     
    TMTPTypeEvent                   iEvent;
    
    //not owned by this class
    CMTPDeviceDataStore *iPtrDataStore;
    
    TInt								iActiveProcessor;
    TBool								iActiveProcessorRemoved;

    //Timer for  DeviceInfo
    CMTPDeviceInfoTimer*                iDeviceInfoTimer;
    RMTPFramework                       iFrameWork;
    enum TCommandState
        {
        EIdle,
        EOpenSession,
        EStartDeviceInfoTimer,
        ESetIsMac
        };
    TCommandState                       iCommandState;


    };
	/**
	Implements the MTP GetDeviceinfo timer. 
	@internalComponent
	*/
class CMTPDeviceInfoTimer : public CTimer
    {
public:

    static CMTPDeviceInfoTimer* NewL(CMTPDeviceDataProvider& aDeviceProvider); 
    virtual ~CMTPDeviceInfoTimer();
    void Start();
    
private: // From CTimer

    void RunL();
    
private:

    CMTPDeviceInfoTimer(CMTPDeviceDataProvider& aDeviceProvider);
    void ConstructL();    

private:    
    
    CMTPDeviceDataProvider& iDeviceProvider;
    RDevUsbcClient          iLdd;

enum TTimerState
    {
    EIdle,
    EStartTimer,
    EUSBReEnumerate
    };
    TTimerState iState;
    };    
#endif // CMTPDEVICEDP_H
