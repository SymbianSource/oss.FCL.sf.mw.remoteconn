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

#ifndef CMTPSTORAGEWATCHER_H
#define CMTPSTORAGEWATCHER_H

#include <e32base.h>
#include <mtp/tmtptypeevent.h>

#include "mtpdebug.h"
#include "rmtpframework.h"
#include "rmtpdpsingletons.h"

class MMTPDataProviderFramework;

/** 
Implements the MTP device data provider system storage watcher.
@internalComponent
*/
class CMTPStorageWatcher : public CActive
   {    

public:

    static CMTPStorageWatcher* NewL(MMTPDataProviderFramework& aFramework);
    ~CMTPStorageWatcher();
    
    void EnumerateStoragesL();
    void Start();
    
private: // Form CActive

    void DoCancel();
    TInt RunError(TInt aError);
    void RunL();
    
private:

    CMTPStorageWatcher(MMTPDataProviderFramework& aFramework);
    void ConstructL();
    
    TUint32 DriveConfigurationL(TBool aAllDrives) const;
    TBool Excluded(TDriveNumber aDriveNumber) const;
    void RequestNotification();
    void SendEventL(TUint16 aEvent, TUint32 aStorageId);
    void StorageAvailableL(TDriveNumber aDriveNumber);
    void StorageUnavailableL(TDriveNumber aDriveNumber);    
    void AppendFolderExclusionListL();
    
private: // Owned

    enum TState    
        {
        EUndefined  = 0x00000000,
        EStarted    = 0x00000001
        };
    
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The available drives configuration.
    */
	TUint32                         iDrivesConfig;
    
    /**
    The drive exclusion list.
    */
    RArray<TUint>                   iDrivesExcluded;
    
    /**
    The MTP event dataset buffer.
    */
    TMTPTypeEvent                   iEvent;
    
    /**
    The MTP framework singletons.
    */
    RMTPFramework                   iFrameworkSingletons;
    
    /**
    The logical storages allocation enable flag.
    */
    TBool                           iAllocateLogicalStorages;
    
    /**
    The state variable.
    */
    TUint32                         iState;
    
    /**
    The folder exclusion list.
    */
    CDesCArray*						iFolderExclusionList;
    
    /**
    The DP Singleton.
    */
    RMTPDpSingletons				iDpSingletons;
    
private: // Not owned.

    MMTPDataProviderFramework&      iFramework;
    };
   
#endif // CMTPSTORAGEWATCHER_H
