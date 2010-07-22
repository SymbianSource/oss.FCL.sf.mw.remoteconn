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

#include <ecom/ecom.h>

#include "cmtpconnectionmgr.h"
#include "cmtpdataprovidercontroller.h"
#include "cmtpserver.h"
#include "cmtpserversession.h"
#include "cmtpshutdown.h"
#include "mtpclientserver.h"
#include "mtpdebug.h"
#include "rmtpframework.h"

#define UNUSED_VAR(a) (a) = (a)

__FLOG_STMT(_LIT8(KComponent,"Server");)

/**
PlatSec policy.
*/ 
const TUint8 KMTPPolicyElementNetworkAndLocal = 0;

const TInt KMTPFunctionCodeRanges[] = 
    {    
    EMTPClientStartTransport, 
    EMTPClientNotSupported,
    };

const TUint KMTPFunctionCodeRangeCount(sizeof(KMTPFunctionCodeRanges) / sizeof(KMTPFunctionCodeRanges[0]));

const TUint8 KMtpElementsIndex[KMTPFunctionCodeRangeCount] =
    {
    KMTPPolicyElementNetworkAndLocal,
    CPolicyServer::ENotSupported,
    };

const CPolicyServer::TPolicyElement KMtpPolicyElements[] = 
    { 
    {_INIT_SECURITY_POLICY_C2(ECapabilityNetworkServices, ECapabilityLocalServices), CPolicyServer::EFailClient},
    };

const static CPolicyServer::TPolicy KMTPServerPolicy =
    {
    CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
    KMTPFunctionCodeRangeCount,
    KMTPFunctionCodeRanges,
    KMtpElementsIndex,     // what each range is compared to 
    KMtpPolicyElements     // what policies range is compared to
    };
    
/**
Destructor.
*/
CMTPServer::~CMTPServer()
    {
    __FLOG(_L8("~CMTPServer - Entry"));
    delete iShutdown;
    iShutdown = NULL;
    iFrameworkSingletons.ConnectionMgr().StopTransports();
    iFrameworkSingletons.DpController().UnloadDataProviders();
    iFrameworkSingletons.Close();
    REComSession::FinalClose();
    __FLOG(_L8("~CMTPServer - Exit"));
    __FLOG_CLOSE;
    }

/**
Creates and executes a new CMTPServer instance.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPServer::RunServerL()
    {
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("RunServerL - Entry"));
    
    // Naming the server thread after the server helps to debug panics
    User::LeaveIfError(User::RenameProcess(KMTPServerName));
    
    // Create and install the active scheduler.
    CActiveScheduler* scheduler(new(ELeave) CActiveScheduler);
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    
    // Create the server and leave it on the cleanup stack.
    CMTPServer* server(CMTPServer::NewLC());
    
    // Initialisation complete, signal the client
    RProcess::Rendezvous(KErrNone);
    
    // Execute the server.
    CActiveScheduler::Start();

	// Server shutting down. 
	CleanupStack::PopAndDestroy(server);
        
    CleanupStack::PopAndDestroy(1); // scheduler
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("RunServerL - Exit"));
    } 

/**
Adds a new CMTPServer session.
*/
void CMTPServer::AddSession()
    {  
    __FLOG(_L8("AddSession - Entry"));
    if(iShutdown && iShutdown->IsActive())
        {  
        iShutdown->Cancel();
        }
    ++iSessionCount;
    __FLOG(_L8("AddSession - Exit"));
    }

/**
Removes a CMTPServer session. If there are no active MTP client API sessions 
remaining and no active MTP connections, then a shutdown timer is started to 
terminate the server thread.
*/
void CMTPServer::DropSession()
    {
    __FLOG(_L8("DropSession - Entry"));
         
    if (--iSessionCount==0 && iFrameworkSingletons.ConnectionMgr().TransportCount() == 0)
        {
        // No active MTP client API sessions remain, start the shutdown timer.
        if (iShutdown)
            {
            __FLOG(_L8("Shutdown Started - Entry"));
            iShutdown->Start();
            }
        }
    __FLOG(_L8("DropSession - Exit"));
    }
    
CSession2* CMTPServer::NewSessionL(const TVersion&,const RMessage2&) const
    {
    __FLOG(_L8("NewSessionL - Entry"));
    __FLOG(_L8("NewSessionL - Exit"));
    return new(ELeave) CMTPServerSession();
    }
       
/**
CMTPServer factory method. A pointer to the constructed CMTPServer instance is
placed on the cleanup stack.
@return A pointer to a new CMTPServer instance. Ownership IS transfered.
@leave One of the system wide error codes if a processing failure occurs.
*/
CMTPServer* CMTPServer::NewLC()
    {
    CMTPServer* self=new(ELeave) CMTPServer;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

/**
Constructor.
*/    
CMTPServer::CMTPServer() : 
    CPolicyServer(CActive::EPriorityStandard, KMTPServerPolicy)
    {

    }
    
/**
second-phase constructor.
*/
void CMTPServer::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    StartL(KMTPServerName);
    iFrameworkSingletons.OpenL();
    if (!iShutdown)
        {
        TRAPD(error, iShutdown = CMTPShutdown::NewL());
        __FLOG(_L8("CMTPShutdown Loaded- Entry"));
        UNUSED_VAR(error);    
        }    
    __FLOG(_L8("ConstructL - Exit"));
    }
        
/*
RMessage::Panic() also completes the message. This is:
(a) important for efficient cleanup within the kernel
(b) a problem if the message is completed a second time
@param aMessage Message to be paniced.
@param aPanic Panic code.
*/
void PanicClient(const RMessagePtr2& aMessage,TMTPPanic aPanic)
    {
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("PanicClient - Entry"));
    __FLOG_STATIC_VA((KMTPSubsystem, KComponent, _L8("Panic = %d"), aPanic));
    _LIT(KPanic,"MTPServer");
    aMessage.Panic(KPanic, aPanic);
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("PanicClient - Exit"));
    }

/**
Process entry point
*/
TInt E32Main()
    {
    __UHEAP_MARK;
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("E32Main - Entry"));
    __MTP_HEAP_FLOG
    
    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt ret = KErrNoMemory;
    if (cleanup)
        {
#ifdef __OOM_TESTING__
        TInt i = 0;
        while (ret == KErrNoMemory || ret == KErrNone)
             {
             __UHEAP_SETFAIL(RHeap::EDeterministic,i++);
             __UHEAP_MARK;

             TRAP(nRet, RunServerL());

             __UHEAP_MARKEND;
             __UHEAP_RESET;
             }
#else
        TRAP(ret, CMTPServer::RunServerL());
      
#endif        
        delete cleanup;
        }
    __FLOG_STATIC(KMTPSubsystem, KComponent, _L8("E32Main - Exit"));
    __UHEAP_MARKEND;
    return ret;
    }
