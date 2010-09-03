// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cmtpbearermonitor.h"
#include <locodservicepluginobserver.h>

#include "cmtpbluetoothcontroller.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpbearermonitorTraces.h"
#endif


CMTPBearerMonitor* CMTPBearerMonitor::NewL( TLocodServicePluginParams& aParams )
    {
    CMTPBearerMonitor* self = new( ELeave ) CMTPBearerMonitor( aParams );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

CMTPBearerMonitor::~CMTPBearerMonitor()
    {
    OstTraceFunctionEntry0( CMTPBEARERMONITOR_DES_ENTRY );
    iMTPControllers.ResetAndDestroy();
    iMTPControllers.Close();
    OstTraceFunctionExit0( CMTPBEARERMONITOR_DES_EXIT );
    }

void CMTPBearerMonitor::ManageServiceCompleted( TLocodBearer aBearer, TBool aStatus, TInt aError )
    {
    OstTraceFunctionEntry0( CMTPBEARERMONITOR_MANAGESERVICECOMPLETED_ENTRY );
    Observer().ManageServiceCompleted( aBearer, aStatus, ImplementationUid(), aError );
    OstTraceFunctionExit0( CMTPBEARERMONITOR_MANAGESERVICECOMPLETED_EXIT );
    }

void CMTPBearerMonitor::ManageService( TLocodBearer aBearer, TBool aStatus )
    {
    OstTraceFunctionEntry0( CMTPBEARERMONITOR_MANAGESERVICE_ENTRY );
    OstTraceExt2( TRACE_NORMAL, CMTPBEARERMONITOR_MANAGESERVICE, "The bear is 0x%08X, The status is %d", aBearer, aStatus );
    
    TInt count = iMTPControllers.Count();
    TBool foundController = EFalse;
    for ( TInt i = 0; i < count; ++i )
        {
        if ( aBearer == iMTPControllers[i]->Bearer() )
            {
            iMTPControllers[i]->ManageService( aStatus );
            foundController = ETrue;
            }
        }
    if ( !foundController )
        {
        ManageServiceCompleted( aBearer, aStatus, KErrNone );
        }
    OstTraceFunctionExit0( CMTPBEARERMONITOR_MANAGESERVICE_EXIT );
    }

CMTPBearerMonitor::CMTPBearerMonitor( TLocodServicePluginParams& aParams ):
    CLocodServicePlugin( aParams )
    {
    OstTraceFunctionEntry0( CMTPBEARERMONITOR_CONS_ENTRY );
    OstTraceFunctionExit0( CMTPBEARERMONITOR_CONS_EXIT );
    }

void CMTPBearerMonitor::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPBEARERMONITOR_CONSTRUCTL_ENTRY );
    CMTPBluetoothController* btController = CMTPBluetoothController::NewL( *this );
	CleanupStack::PushL(btController);
    iMTPControllers.AppendL( btController );
	CleanupStack::Pop(btController);
    OstTraceFunctionExit0( CMTPBEARERMONITOR_CONSTRUCTL_EXIT );
    }

