/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The session object for DUN server
*
*/


#include <e32std.h>
#include <c32comm.h>
#include <f32file.h>
#include <e32math.h>
#include <locodbearer.h>
#include <btfeaturescfg.h>
#include "dunserverdef.h"
#include "DunSession.h"
#include "DunServer.h"
#include "DunPlugin.h"
#include "DunDebug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDunSession::CDunSession
// ---------------------------------------------------------------------------
//
CDunSession::CDunSession( CDunServer* aDunServer ) :
    CSession2(),
    iDunServer( aDunServer )
    {
    FTRACE(FPrint( _L( "CDunSession::CDunSession()") ));
    iDunServer->IncRefCount();
    FTRACE(FPrint( _L( "CDunSession::CDunSession() complete") ));
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunSession::~CDunSession()
    {
    FTRACE(FPrint(_L("CDunSession::~CDunSession()")));
    if ( iDunServer )
        {
        iDunServer->DecRefCount();
        iDunServer = NULL;
        }
    FTRACE(FPrint(_L("CDunSession::~CDunSession() complete")));
    }

// ---------------------------------------------------------------------------
// Return the version number
// ---------------------------------------------------------------------------
//
TVersion CDunSession::Version(void) const
    {
    return (TVersion( KDunServerMajorVersionNumber,
                      KDunServerMinorVersionNumber,
                      KDunServerBuildVersionNumber ));
    }

// ---------------------------------------------------------------------------
// From class CSession2.
// Handles the servicing of client requests passed to the server
// ---------------------------------------------------------------------------
//
void CDunSession::ServiceL( const RMessage2& aMessage )
    {
    FTRACE(FPrint(_L("CDunSession::ServiceL()")));
    TBool hasCapability = aMessage.HasCapability( ECapabilityLocalServices );
    if ( !hasCapability )
        {
        aMessage.Complete( KErrPermissionDenied );
        FTRACE(FPrint(_L("CDunSession::ServiceL() (no capability) complete")));
        return;
        }
    TInt retVal = KErrNotSupported;
    switch ( aMessage.Function() )
        {
        case EDunFuncManageService:
            retVal = DoService( aMessage );
            break;
        case EDunFuncActiveConnection:
            retVal = DoTestService( aMessage );
            break;
        default:
            FTRACE(FPrint(_L("CDunSession::ServiceL() (not supported) complete")));
            break;
        }
    aMessage.Complete( retVal );
    FTRACE(FPrint(_L("CDunSession::ServiceL() complete")));
    }

// ---------------------------------------------------------------------------
// From class CSession2.
// Return a reference to DUN server
// ---------------------------------------------------------------------------
//
CDunServer& CDunSession::Server() const
    {
    return *iDunServer;
    }

// ---------------------------------------------------------------------------
// Handles incoming service messages
// ---------------------------------------------------------------------------
//
TInt CDunSession::DoService( const RMessage2& aMessage )
    {
    FTRACE(FPrint(_L("CDunSession::DoService()")));
    if ( !iDunServer )
        {
        FTRACE(FPrint(_L("CDunSession::DoService() (iDunServer) not initialized!")));
        return KErrGeneral;
        }

    TLocodBearer bearer = static_cast<TLocodBearer>( aMessage.Int0() );
    TBool bearerStatus = static_cast<TBool>( aMessage.Int1() );

    TUid pluginUid;
    switch ( bearer )
        {
        case ELocodBearerBT:
			{
            pluginUid = KDunBtPluginUid;
            FTRACE(FPrint(_L("CDunSession::DoService() Managing service for BT, bearer status %d"), bearerStatus));
			
			BluetoothFeatures::TEnterpriseEnablementMode mode = BluetoothFeatures::EDisabled;
			TRAPD(err, mode = BluetoothFeatures::EnterpriseEnablementL());
			if ( err )
				{
				FTRACE(FPrint(_L("CDunSession::DoService() complete: failed to get BluetoothFeatures::EnterpriseEnablementL")));
				return err;
				}
			if ( mode != BluetoothFeatures::EEnabled )
				{
				FTRACE(FPrint(_L("CDunSession::DoService() complete: !BluetoothFeatures::EEnabled")));
				return KErrNotSupported;
				}
			}
            break;
        case ELocodBearerIR:
            pluginUid = KDunIrPluginUid;
            FTRACE(FPrint(_L("CDunSession::DoService() Managing service for IR, bearer status %d"), bearerStatus));
            break;
        case ELocodBearerUSB:
            pluginUid = KDunUsbPluginUid;
            FTRACE(FPrint(_L("CDunSession::DoService() Managing service for USB, bearer status %d"), bearerStatus));
            break;
        default:
            FTRACE(FPrint(_L("CDunSession::DoService() Unknown service %d, bearer status %d!"), bearer, bearerStatus));
            return KErrNotSupported;
        }
    TInt retVal = KErrNone;
    if ( bearerStatus )  // ON
        {
        retVal = iDunServer->NotifyMediaOpenRequest( pluginUid );
        }
    else  // OFF
        {
        retVal = iDunServer->NotifyMediaCloseRequest( pluginUid );
        }
    FTRACE(FPrint(_L("CDunSession::DoService() complete")));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Handles incoming test service messages
// ---------------------------------------------------------------------------
//
TInt CDunSession::DoTestService( const RMessage2& aMessage )
    {
    FTRACE(FPrint(_L("CDunSession::DoTestService()")));
    if ( !iDunServer )
        {
        FTRACE(FPrint(_L("CDunSession::DoTestService() (iDunServer) not initialized!")));
        return KErrGeneral;
        }
    TConnId connId = iDunServer->ActiveConnection();
    TPckgBuf<TConnId> connIdPckg( connId );
    aMessage.Write( 0, connIdPckg );  // First argument in EDunFuncActiveConnection
    FTRACE(FPrint(_L("CDunSession::DoTestService() complete")));
    return KErrNone;
    }
