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
 @internalTechnology
*/

#include <e32base.h>
#include <bautils.h>

#include "cmediasyncserver.h"
#include "cmediasyncserversession.h"
#include "cmediasyncobserver.h"
#include "cmediasyncserverdef.h"

__FLOG_STMT(_LIT8(KComponent, "MediaSyncServer");)

const TInt KMediaSyncFunctionCodeRanges[] = 
    {    
    EMediaSyncClientGetGSHHandle,
    EMediaSyncClientShutdown,
    EMediaSyncClientNotSupported,
    };

const TUint KMediaSyncFunctionCodeRangeCount = (sizeof(KMediaSyncFunctionCodeRanges) 
                                            / sizeof(KMediaSyncFunctionCodeRanges[0]));


const TUint8 KMediaSyncPolicyElementNetworkAndLocal = 0;
const TUint8 KMediaSyncPolicyElementPowerMgmt = 1;

const TUint8 KMediaSyncElementsIndex[KMediaSyncFunctionCodeRangeCount] =
    {
    KMediaSyncPolicyElementNetworkAndLocal,
    KMediaSyncPolicyElementPowerMgmt,
    CPolicyServer::ENotSupported,
    };

const CPolicyServer::TPolicyElement KMediaSyncPolicyElements[] = 
    { 
    {_INIT_SECURITY_POLICY_C2(ECapabilityNetworkServices, ECapabilityLocalServices), CPolicyServer::EFailClient},
    {_INIT_SECURITY_POLICY_C1(ECapabilityPowerMgmt), CPolicyServer::EFailClient},
    };

const CPolicyServer::TPolicy KMediaSyncServerPolicy =
    {
    CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
    KMediaSyncFunctionCodeRangeCount,
    KMediaSyncFunctionCodeRanges,
    KMediaSyncElementsIndex,     // what each range is compared to 
    KMediaSyncPolicyElements     // what policies range is compared to
    };


/**
Creates and executes a new CMTPServer instance.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMediaSyncServer::RunServerL()
    {           
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);
    
    TFileName lockFileName;
    fs.PrivatePath(lockFileName);
    TDriveUnit driveNum = RFs::GetSystemDrive();
    lockFileName.Insert(0, driveNum.Name());
    lockFileName.Append(KMssLockName);
    
    RFile lockFile;
    CleanupClosePushL(lockFile);
    TInt ret = KErrNone;
    if (!BaflUtils::FileExists(fs, lockFileName))
        {        
        BaflUtils::EnsurePathExistsL(fs, lockFileName);
        ret = lockFile.Create(fs, lockFileName, EFileShareExclusive|EFileWrite);
        }
    else
        {
        ret = lockFile.Open(fs, lockFileName, EFileShareExclusive|EFileWrite);
        }
    
    if (ret == KErrNone)
        {
        // Naming the server thread after the server helps to debug panics
        User::LeaveIfError(User::RenameProcess(KMediaSyncServerName));
        
        // Create and install the active scheduler.
        CActiveScheduler* scheduler = new (ELeave) CActiveScheduler;
        CleanupStack::PushL(scheduler);
        CActiveScheduler::Install(scheduler);
        
        // Create the server and leave it on the cleanup stack.
        CMediaSyncServer* server = CMediaSyncServer::NewLC(fs);
        
        // Initialisation complete, signal the client
        RProcess::Rendezvous(KErrNone);
        
        // Execute the server.
        CActiveScheduler::Start();

        // Server shutting down. 
        CleanupStack::PopAndDestroy(server);
            
        CleanupStack::PopAndDestroy(scheduler); // scheduler        
        }
    else
        {
        RProcess::Rendezvous(KErrNone);
        }
    
    CleanupStack::PopAndDestroy(&lockFile);
    CleanupStack::PopAndDestroy(&fs);    
    }

CMediaSyncServer* CMediaSyncServer::NewLC(RFs& aFs)
    {
    CMediaSyncServer* self = new (ELeave) CMediaSyncServer;
    CleanupStack::PushL(self);
    self->ConstructL(aFs);
    return self;
    }

CMediaSyncServer::CMediaSyncServer() : 
    CPolicyServer(CActive::EPriorityStandard, KMediaSyncServerPolicy)
    {   
    
    }

CMediaSyncServer::~CMediaSyncServer()
    {
    __FLOG(_L8("CMediaSyncServer::~CMediaSyncServer - Entry")); 
    
    delete iDb;
    delete iObserver;    
    
    __FLOG(_L8("CMediaSyncServer::~CMediaSyncServer - Exit"));
    __FLOG_CLOSE;
    }

void CMediaSyncServer::ConstructL(RFs& aFs)
    {  
    __FLOG_OPEN(KMSSSubsystem, KComponent);
    __FLOG(_L8("CMediaSyncServer::ConstructL - Entry"));
    
    iDb = CMediaSyncDatabase::NewL(aFs);
    iObserver = CMediaSyncObserver::NewL(iDb);
    iNeedFullSync = iDb->IsMssDbCorrupt();
    
    StartL(KMediaSyncServerName);

    __FLOG(_L8("CMediaSyncObserver::ConstructL - Exit"));    
    }

CSession2* CMediaSyncServer::NewSessionL(const TVersion&,const RMessage2&) const
    {
    __FLOG(_L8("CMediaSyncServer::NewSessionL - Entry"));
    
    CMediaSyncServer* ncThis = const_cast<CMediaSyncServer*>(this);
    
    __FLOG(_L8("CMediaSyncObserver::NewSessionL - Exit"));    
    return new(ELeave) CMediaSyncServerSession(ncThis);
    }

CMediaSyncObserver* CMediaSyncServer::MediaSyncObserver() const
    {
    return iObserver;
    }

CMediaSyncDatabase* CMediaSyncServer::MediaSyncDatabase() const
    {
    return iDb;
    }

TBool CMediaSyncServer::NeedFullSync()
    {
    return iNeedFullSync;
    }

void CMediaSyncServer::ClearFullSyncFlag()
    {
    __FLOG(_L8("CMediaSyncServer::ClearFullSyncFlag - Entry"));
    
    iNeedFullSync = EFalse;
    iDb->ClearMssDbCorrupt();
    
    __FLOG(_L8("CMediaSyncObserver::ClearFullSyncFlag - Exit"));    
    }
