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

__FLOG_STMT( _LIT8( KComponent, "mtpbearermonitor" ); )

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
    iMTPControllers.ResetAndDestroy();
    iMTPControllers.Close();
    __FLOG( _L8("+/-Dtor") );
    __FLOG_CLOSE;
    }

void CMTPBearerMonitor::ManageServiceCompleted( TLocodBearer aBearer, TBool aStatus, TInt aError )
    {
    Observer().ManageServiceCompleted( aBearer, aStatus, ImplementationUid(), aError );
    }

void CMTPBearerMonitor::ManageService( TLocodBearer aBearer, TBool aStatus )
    {
    __FLOG_2( _L8("+/-ManageService( 0x%08X, %d )"), aBearer, aStatus );
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
    }

CMTPBearerMonitor::CMTPBearerMonitor( TLocodServicePluginParams& aParams ):
    CLocodServicePlugin( aParams )
    {
    __FLOG_OPEN( KMTPSubsystem, KComponent );
    __FLOG( _L8("+/-Ctor") );
    }

void CMTPBearerMonitor::ConstructL()
    {
    __FLOG( _L8("+ConstructL") );
    
    CMTPBluetoothController* btController = CMTPBluetoothController::NewL( *this );
    CleanupStack::PushL(btController);
    iMTPControllers.AppendL( btController );
    CleanupStack::Pop(btController);
    
    __FLOG( _L8("-ConstructL") );
    }

