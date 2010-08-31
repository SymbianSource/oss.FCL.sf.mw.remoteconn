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


#include <featmgr.h> 
#include <locodplugin.hrh>
#include <locodserviceplugin.h>

#include "locodserviceman.h"
#include "locodservice.h"
#include "debug.h"
#include "utils.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CLocodServiceMan* CLocodServiceMan::NewL()
    {
    CLocodServiceMan* self = new (ELeave) CLocodServiceMan();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// C++ destructor
// ---------------------------------------------------------------------------
//
CLocodServiceMan::~CLocodServiceMan()
    {
    Cancel();
    iServices.ResetAndDestroy();
    iServices.Close();

    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// CActive method, the request is only completed when there is no service
// to manage, so all the service plugins are destroyed
// ---------------------------------------------------------------------------
//
void CLocodServiceMan::RunL()
    {
    TRACE_FUNC_ENTRY
        if(iStatus ==KErrNone)
        {
        iServices.ResetAndDestroy();
        }
        TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CActive method
// ---------------------------------------------------------------------------
//
void CLocodServiceMan::DoCancel()
    {
    
    }
  
// ---------------------------------------------------------------------------
// CActive method
// ---------------------------------------------------------------------------
//  
TInt CLocodServiceMan::RunError(TInt/* aReason */)
    {
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// NotifyBearerStatus
// ---------------------------------------------------------------------------
//
void CLocodServiceMan::NotifyBearerStatus(TLocodBearer aBearer, TBool aStatus)
    {
    TRACE_INFO((_L(" [BRRST] status of 0x%04x changes to %d"), aBearer, aStatus))
    TInt oldStatus = iBearerStatus;
    if (aStatus)
        {
        iBearerStatus |= aBearer;
        }
    else
        {
        iBearerStatus &= (~aBearer);
        }
    TRACE_INFO((_L(" [BRRST] [OLD] 0x%08x [NEW] 0x%08x"), oldStatus, iBearerStatus))
    if (iBearerStatus && !iServices.Count())
        {
        TRAP_IGNORE(LoadServicesL());
        }
    TInt count = iServices.Count();
    for (TInt i = 0; i < count; i++)
        {
        if(iBearerStatus != oldStatus)
	        {
            iServices[i]->ManageService(aBearer, aStatus);
	        }
        }
    }

// ---------------------------------------------------------------------------
// Call back method from service plugins. This is called when the services has been 
// managed by the plug ins.
// ---------------------------------------------------------------------------
//
void CLocodServiceMan::ManageServiceCompleted(TLocodBearer aBearer, 
    TBool aStatus, TUid aServiceImplUid, TInt err)
    {
    TInt count = iServices.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iServices[i]->Plugin().ImplementationUid() == aServiceImplUid)
            {
            iServices[i]->ManageServiceCompleted(aBearer, aStatus, err);
            break;
            }
        }

    if (!iBearerStatus)
        {
        for (TInt i = 0; i < count; i++)
            {
            if (iServices[i]->HasServiceToManage())
                {
                return;
                }
            }
    
        if(!IsActive())
        {
            iStatus = KRequestPending;
            SetActive();
            TRequestStatus* status = &iStatus;
            User::RequestComplete(status, KErrNone);
        }

        }
    }
  
// ---------------------------------------------------------------------------
// c++ constructor
// ---------------------------------------------------------------------------
//          
CLocodServiceMan::CLocodServiceMan() : CActive(CActive::EPriorityStandard),iUidDun(TUid::Uid(KFeatureIdDialupNetworking))
    {        
    CActiveScheduler::Add(this);
    TRACE_FUNC_THIS
    }
  
// ---------------------------------------------------------------------------
// 2nd phase construction
// ---------------------------------------------------------------------------
//  
void CLocodServiceMan::ConstructL()
    {

    }

// ---------------------------------------------------------------------------
// Loaded all service plug ins who has implemented the KLOCODSERVICEINTERFACEUID
// ---------------------------------------------------------------------------
//
void CLocodServiceMan::LoadServicesL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Load interface 0x%08X"), KLOCODSERVICEINTERFACEUID))
	const TUid KServicePluginInterface = TUid::Uid(KLOCODSERVICEINTERFACEUID);
	RImplInfoPtrArray implementations;
	const TEComResolverParams noResolverParams;
	REComSession::ListImplementationsL(KServicePluginInterface, 
		noResolverParams,
		KRomOnlyResolverUid,
		implementations);
	CleanupResetDestroyClosePushL(implementations);
	const TUint count = implementations.Count();
    TRACE_INFO((_L(" [BRRST] Service Plug in found %d"), count))
	for ( TUint ii = 0 ; ii < count ; ++ii )
		{
		CImplementationInformation* impl = implementations[ii];
		TRACE_INFO((_L("Service: feature %d, name '%S', ROM only %d"), 
		    impl->ImplementationUid().iUid, &(impl->DisplayName()), impl->RomOnly()))
    	if (FeatureManager::FeatureSupported(impl->ImplementationUid().iUid))
    		{
    		TRACE_INFO((_L("Feature found")))
            TLocodServicePluginParams params(impl->ImplementationUid(), *this);
    		CLocodServicePlugin* srvcplugin = CLocodServicePlugin::NewL(params);
    		CleanupStack::PushL(srvcplugin);
    		CLocodService* service = CLocodService::NewL(*srvcplugin);
    		CleanupStack::Pop(srvcplugin);
    		CleanupStack::PushL(service);
    		iServices.AppendL(service);
    		CleanupStack::Pop(service);
    		}
		}
	CleanupStack::PopAndDestroy(&implementations);
    TRACE_FUNC_EXIT    
    }



