/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Daemon class implementation.
*
*/


#include <startupdomainpskeys.h>

#include <featmgr.h>
#include <locodplugin.hrh>
#include <locodbearerplugin.h>

#include "locodaemon.h"
#include "locodserviceman.h"
#include "utils.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CLocoDaemon* CLocoDaemon::NewLC()
    {
    CLocoDaemon* self = new (ELeave) CLocoDaemon();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// C++ destrctor
// ---------------------------------------------------------------------------
//
CLocoDaemon::~CLocoDaemon()
    {
    FeatureManager::UnInitializeLib();    
    Cancel();
    iSystemPS.Close();
    delete iServiceMan;
    iBearers.ResetAndDestroy();
    iBearers.Close();
    REComSession::FinalClose();
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// Called when the status of system pubsub keys are 
// changed. Normally this is called after the system boot.
// ---------------------------------------------------------------------------
//
void CLocoDaemon::RunL()
    {
    TRACE_INFO((_L(" RunL %d"), iStatus.Int()))
    if (iStatus == KErrNone)
        {
		iSystemPS.Subscribe(iStatus);
		SetActive();

        TRACE_INFO((_L(" [SYSTEM] prev %d"), iSystemState))
        LEAVE_IF_ERROR(iSystemPS.Get(iSystemState));
        TRACE_INFO((_L(" [SYSTEM] now %d"), iSystemState))
        
        if (iSystemState == ESwStateNormalRfOn ||
             iSystemState == ESwStateNormalRfOff ||
            iSystemState == ESwStateCharging ||  
            iSystemState == ESwStateNormalBTSap)
            { // System is up, construct service man and load bearers.
            if(!iServiceMan)
                {
                iServiceMan = CLocodServiceMan::NewL();
                }
            if (!iBearers.Count())
                {
                LoadBearesL();
                }
            }
  /*
     ESwStateShuttingDown and  ESWStateShuttingDown event is received when 
     the device is about to shut down
  */
        else if (iSystemState == ESwStateShuttingDown)
            {
            TRACE_INFO((_L(" [SYSTEM] Shuting down and deleting")))
            delete iServiceMan;
            iServiceMan = NULL;
            iBearers.ResetAndDestroy();
            return;
            }        
        }
        RProcess::Rendezvous(KErrNone);
    }

// ---------------------------------------------------------------------------
// CActive method cancel listening pubsub keys
// ---------------------------------------------------------------------------
//
void CLocoDaemon::DoCancel()
    {
    TRACE_FUNC
    iSystemPS.Cancel();
    }
  
// ---------------------------------------------------------------------------
// CActive method
// ---------------------------------------------------------------------------
//  
TInt CLocoDaemon::RunError(TInt /*aReason*/)
    {
    TRACE_FUNC
    return KErrNone;
    }
  
// ---------------------------------------------------------------------------
// C++ Constructor
// ---------------------------------------------------------------------------
//  
CLocoDaemon::CLocoDaemon() : CActive(CActive::EPriorityStandard)
    {
    CActiveScheduler::Add(this);
    TRACE_FUNC_THIS
    }

// ---------------------------------------------------------------------------
// 2nd phase construction
// ---------------------------------------------------------------------------
//    
void CLocoDaemon::ConstructL()
    {
    FeatureManager::InitializeLibL();
    LEAVE_IF_ERROR(iSystemPS.Attach(KPSUidStartup, KPSGlobalSystemState));
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* sta = &iStatus;
    User::RequestComplete(sta, KErrNone);
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// LoadBearesL Loads the bearer plug ins
// ---------------------------------------------------------------------------
//
void CLocoDaemon::LoadBearesL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Load interface 0x%08X"), KLOCODBEARERINTERFACEUID))
	const TUid KBearerPluginInterface = TUid::Uid(KLOCODBEARERINTERFACEUID);
	RImplInfoPtrArray implementations;
	const TEComResolverParams noResolverParams;
	REComSession::ListImplementationsL(KBearerPluginInterface, 
		noResolverParams,
		KRomOnlyResolverUid,
		implementations);
	CleanupResetDestroyClosePushL(implementations);
	const TUint count = implementations.Count();
    TRACE_INFO((_L(" Bearer count = %d"), count))
	for ( TUint ii = 0 ; ii < count ; ++ii )
		{
		CImplementationInformation* impl = implementations[ii];
		TRACE_INFO((_L("Bearer: feature %d, name '%S', ROM only %d"), 
		    impl->ImplementationUid().iUid, &(impl->DisplayName()), impl->RomOnly()))
    	if (FeatureManager::FeatureSupported(impl->ImplementationUid().iUid))
    		{
    		TRACE_INFO((_L("Feature found")))
    		TLocodBearerPluginParams params(impl->ImplementationUid(), *iServiceMan);
    		CLocodBearerPlugin* bearer = CLocodBearerPlugin::NewL(params);
    		CleanupStack::PushL(bearer);
    		iBearers.AppendL(bearer);
    		CleanupStack::Pop(bearer);
    		}
		}
	CleanupStack::PopAndDestroy(&implementations);
    TRACE_FUNC_EXIT
    }

