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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpserverTraces.h"
#endif


#define UNUSED_VAR(a) (a) = (a)

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
    OstTraceFunctionEntry0( CMTPSERVER_CMTPSERVER_DES_ENTRY ); 
    delete iShutdown;
    iShutdown = NULL;
    if(iFrameworkSingletonsOpened)
        {
        iFrameworkSingletons.ConnectionMgr().StopTransports();
        iFrameworkSingletons.DpController().UnloadDataProviders();
        iFrameworkSingletons.Close();	 
        iFrameworkSingletonsOpened = EFalse;
        }
    REComSession::FinalClose();
    OstTraceFunctionExit0( CMTPSERVER_CMTPSERVER_DES_EXIT );
    }

/**
Creates and executes a new CMTPServer instance.
@leave One of the system wide error codes, if a processing failure occurs.
*/
void CMTPServer::RunServerL()
    {
    OstTraceFunctionEntry0( CMTPSERVER_RUNSERVERL_ENTRY );
    
    // Naming the server thread after the server helps to debug panics
    TInt ret = User::RenameProcess(KMTPServerName);
    LEAVEIFERROR(ret, OstTrace0(TRACE_ERROR, CMTPSERVER_RUNSERVERL, "Rename process error"));
    
    // Create and install the active scheduler.
    CActiveScheduler* scheduler(new(ELeave) CActiveScheduler);
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    
    // Create the server and leave it on the cleanup stack.
    CMTPServer* server(CMTPServer::NewLC());
    
    // Initialisation complete, signal the client
    RProcess::Rendezvous(KErrNone);
    
    // Execute the server.
    OstTraceDef0( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, DUP1_CMTPSERVER_RUNSERVERL, "MTP server starts up" );
    CActiveScheduler::Start();
    OstTraceDef0( OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, DUP2_CMTPSERVER_RUNSERVERL, "MTP server closed" );

	// Server shutting down. 
	CleanupStack::PopAndDestroy(server);
        
    CleanupStack::PopAndDestroy(1); // scheduler
    OstTraceFunctionExit0( CMTPSERVER_RUNSERVERL_EXIT );
    } 

/**
Adds a new CMTPServer session.
*/
void CMTPServer::AddSession()
    {  
    OstTraceFunctionEntry0( CMTPSERVER_ADDSESSION_ENTRY );
    if(iShutdown && iShutdown->IsActive())
        {  
        iShutdown->Cancel();
        }
    ++iSessionCount;
    OstTraceFunctionExit0( CMTPSERVER_ADDSESSION_EXIT );
    }

/**
Removes a CMTPServer session. If there are no active MTP client API sessions 
remaining and no active MTP connections, then a shutdown timer is started to 
terminate the server thread.
*/
void CMTPServer::DropSession()
    {
    OstTraceFunctionEntry0( CMTPSERVER_DROPSESSION_ENTRY );
         
    if (--iSessionCount==0 && iFrameworkSingletonsOpened && iFrameworkSingletons.ConnectionMgr().TransportCount() == 0)
        {
        // No active MTP client API sessions remain, start the shutdown timer.
        if (iShutdown)
            {
            OstTrace0( TRACE_NORMAL, CMTPSERVER_DROPSESSION, "Shutdown Started" );           
            iShutdown->Start();
            }
        }
    OstTraceFunctionExit0( CMTPSERVER_DROPSESSION_EXIT );
    }
    
CSession2* CMTPServer::NewSessionL(const TVersion&,const RMessage2&) const
    {
    OstTraceFunctionEntry0( CMTPSERVER_NEWSESSIONL_ENTRY );
    OstTraceFunctionExit0( CMTPSERVER_NEWSESSIONL_Exit );
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
    CPolicyServer(CActive::EPriorityStandard, KMTPServerPolicy),iFrameworkSingletonsOpened(EFalse)
    {

    }
    
/**
second-phase constructor.
*/
void CMTPServer::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPSERVER_CONSTRUCTL_ENTRY );
    StartL(KMTPServerName);
    iFrameworkSingletons.OpenL();
    iFrameworkSingletonsOpened = ETrue;
    if (!iShutdown)
        {
        TRAPD(error, iShutdown = CMTPShutdown::NewL());
        OstTrace0( TRACE_NORMAL, CMTPSERVER_CONSTRUCTL, "CMTPShutdown Loaded" );            
        UNUSED_VAR(error);    
        }    
    OstTraceFunctionExit0( CMTPSERVER_CONSTRUCTL_EXIT );
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
    OstTraceFunctionEntry0( _PANICCLIENT_ENTRY );
    OstTraceDef1(OST_TRACE_CATEGORY_PRODUCTION, TRACE_IMPORTANT, _PANICCLIENT, "Panic = %d", aPanic );
    
    _LIT(KPanic,"MTPServer");
    aMessage.Panic(KPanic, aPanic);
    OstTraceFunctionExit0( _PANICCLIENT_EXIT );
    }

/**
Process entry point
*/
TInt E32Main()
    {
    __UHEAP_MARK;
    OstTraceFunctionEntry0( CMTPSERVER_E32MAIN_ENTRY );
#ifdef OST_TRACE_COMPILER_IN_USE
    __MTP_HEAP_OSTTRACE(OstTraceExt4(TRACE_NORMAL,CMTPSERVER_E32MAIN_HEAP,"Heap: Size = %d, Allocated = %d, Available = %d, Largest block = %d", size, allocated, available, largest));
#endif
    
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
    
    OstTraceFunctionExit0( CMTPSERVER_E32MAIN_EXIT );
    __UHEAP_MARKEND;

    return ret;
    }
