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

#ifndef CMTPDATAPROVIDERCONTROLLER_H
#define CMTPDATAPROVIDERCONTROLLER_H

#include <e32base.h>
#include <ecom/ecom.h>
#include <mtp/mtpdataproviderapitypes.h>

#include "rmtpframework.h"
#include <swi/sisregistrysession.h> 
#include <swi/sisregistryentry.h>
#include <swi/sisregistrypackage.h>


class CMTPDataProvider;
class CMTPDataProviderConfig;

/**
Implements the MTP framework data provider controller singleton. This is a 
container class responsible for loading, storing, and managing 
@see CMTPDataProviderPlugin instances and their associated 
@see CMTPDataProvider bindings. The ECOM framework is used to load data 
@see CMTPDataProviderPlugin instances when instructed to do so by the MTP 
framework, either on start-up or when a mode switch occurs. 
@internalTechnology
 
*/


	
class CMTPDataProviderController : public CActive
    {
public:
	/**
    The active data provider set enumeration states.
    */
    enum TEnumerationStates
        {
        EUnenumerated                       = 0,
        EEnumerationStarting                = 1,
        EEnumeratingFrameworkStorages       = 2,
        EEnumeratingDataProviderStorages    = 3,
        EEnumeratingFrameworkObjects        = 4,
        EEnumeratingDataProviderObjects     = 5,
        EEnumeratingPhaseOneDone            = 6,
        EEnumeratingSubDirFiles				= 7, //Only File DP care the status.
        EEnumeratingCleanDBSnapshot         = 8,
        EEnumeratedFulllyCompleted			= 9,
        };
        
public:

    static CMTPDataProviderController* NewL();
    ~CMTPDataProviderController();    
    
    IMPORT_C void LoadDataProvidersL();
    IMPORT_C void UnloadDataProviders();
    IMPORT_C void NotifyDataProvidersL(TMTPNotification aNotification, const TAny* aParams);
    IMPORT_C void NotifyDataProvidersL(TUint aDPId, TMTPNotification aNotification, const TAny* aParams);
    
    IMPORT_C TUint Count();
    IMPORT_C CMTPDataProvider& DataProviderL(TUint aId);
    IMPORT_C CMTPDataProvider& DataProviderByIndexL(TUint aIndex);
    IMPORT_C TBool IsDataProviderLoaded(TUint aId) const;
    IMPORT_C TInt DeviceDpId();
    IMPORT_C TInt DpId(TUint aUid);
    IMPORT_C TInt ProxyDpId();
    IMPORT_C TUint EnumerateState();
    IMPORT_C TInt FileDpId();
        
    void EnumerationStateChangedL(const CMTPDataProvider& aDp);    
	TMTPOperationalMode Mode();

    IMPORT_C void WaitForEnumerationComplete();
    TBool FreeEnumerationWaiter();
    
    IMPORT_C void SetNeedEnumeratingPhase2(TBool aNeed);
    IMPORT_C TBool NeedEnumeratingPhase2() const;
    
    IMPORT_C void RegisterPendingRequestDP(TUint aDpUid, TUint aTimeOut = 0);
    IMPORT_C void ExecutePendingRequestL();
    IMPORT_C TUint StorageEnumerateState(TUint aStorageId);
    

private: // From CActive

    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:

    CMTPDataProviderController();
    void ConstructL();
    void EstablishDBSnapshotL(TUint32 aStorage);
    
    CMTPDataProviderConfig* CreateConfigLC(const TDesC& aResourceFilename);
    void EnumerateDataProviderObjectsL(TUint aId);
	TBool IsObjectsEnumerationNeededL(CMTPDataProvider& dp);
    void  LoadInstalledDataProvidersL(const CImplementationInformation* aImplementations);
    TBool LoadROMDataProvidersL(const TDesC& aResourceFilename, const RImplInfoPtrArray& aImplementations);
    TBool LoadDataProviderL(const TDesC& aResourceFilename);
    TInt  Uid(const TDesC& aResourceFilename, TUint& aUid);
    void Schedule();
    void CreateRegistrySessionAndEntryL();
    void CloseRegistrySessionAndEntryL();
    
    static void ImplementationsCleanup(TAny* aData);
    static TInt ImplementationsLinearOrderUid(const TUid* aUid, const CImplementationInformation& aObject);
    static TInt ImplementationsLinearOrderUid(const CImplementationInformation& aL, const CImplementationInformation& aR);
    
private: // Owned
    
    friend class CMTPObjectStore;
    class CMTPPendingReqestTimer : public CTimer
        {
    public:

        static CMTPPendingReqestTimer* NewL(CMTPDataProviderController* aDPController); 
        virtual ~CMTPPendingReqestTimer();
          
        void Start(TUint aTimeOut);
        
    private: // From CTimer

        void RunL();
        
    private:

        CMTPPendingReqestTimer(CMTPDataProviderController* aDPController);
        void ConstructL();
        
    private:
        
        CMTPDataProviderController* iDPController;
        };

    /**
    The active data provider set container, ordered bit implementation 
    UID.
    */
    RPointerArray<CMTPDataProvider> iDataProviders;

    /**
    The active data provider identifier container, ordered by enumeration phase.
    */
    RArray<TUint>                   iDataProviderIds;
    
    /**
    The device data provider identifier.
    */
    TUint                           iDpIdDeviceDp;
    
    /**
    The proxy data provider identifier.
    */
    TUint                           iDpIdProxyDp;

    /**
    The file data provider identifier.
    */
    TUint                           iDpIdFileDp;
    
    /**
    The actively enumerating data providers.
    */
    RArray<TUint>                   iEnumeratingDps;
    
    /**
    The actively enumerating storage IDs.
    */
    RArray<TUint>                   iEnumeratingStorages;
    
    /**
    The active data provider set enumeration states.
    */
    TEnumerationStates              iEnumerationState;

    /**
    The active data provider set enumeration phases.
    */
    TUint                           iEnumerationPhase;

    /**
    The index used for iterating through the iDataProviderIds array.
    */
    TUint                           iDpIdArrayIndex;
    
    /**
    A general purpose index used for iterating through the active data provider
    set.
    */
    TUint                           iNextDpId;
    
    /**
    The framework singletons.
    */
    RMTPFramework                   iSingletons;
    
    /**
    Current operation mode
    */
    TMTPOperationalMode 			iMode ;
    
    /**
    Session for sisRegistry
    */
    Swi::RSisRegistrySession iSisSession;
    
    /**
    Registry Entry 
    */
    Swi::RSisRegistryEntry iSisEntry;
    
    /**
    Stub Found
    */
    TBool iStubFound;

    /**
    opensession waiter
    */
    CActiveSchedulerWait *iOpenSessionWaiter;

    /*
     The number of folders and files that will be enumerated.
     */
    TBool		iNeedEnumeratingPhase2;
    TUint32 	iNeedEnumeratingPhase2StorageId;
    
    TUint       iPendingRequestDpUid;
    
    CMTPPendingReqestTimer *iPendingRequestTimer;

    };

#endif
