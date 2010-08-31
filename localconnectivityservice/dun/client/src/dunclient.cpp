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
* Description:  Main client side interface of DUN
*
*/


#include "dunclient.h"
#include "dunserverdef.h"
#include "DunDebug.h"
#include "dunactive.h"

const TInt KDunRetryCount        = 3;
const TInt KDunNumOfMessageSlots = 4;

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Create server thread/process
// ---------------------------------------------------------------------------
//
static TInt StartServer()
    {
    FTRACE(FPrint( _L("StartServer()") ));
    TInt retVal = KErrNone;
    TRequestStatus statusStarted;

    RProcess server;
    retVal = server.Create( KDialupServerName, KNullDesC, EOwnerThread );

    if ( retVal != KErrNone )
        {
        FTRACE(FPrint( _L("StartServer() complete (%d)"), retVal ));
        return retVal;
        }

    TRequestStatus stat;
    server.Rendezvous( stat );

    if ( stat != KRequestPending )
        {
        server.Kill( 0 );  //abort startup
        }
    else
        {
        server.Resume(); //logon OK - start the server
        }

    User::WaitForRequest( stat ); //wait the start or death of the server

    /*
    we can't use the 'exit reason' if the server panicked as this
    is the panic 'reason' and may be '0' which cannot be distinguished
    from KErrNone
    */
    retVal = stat.Int();
    if ( server.ExitType() == EExitPanic )
        {
        retVal = KErrServerTerminated;
        }

    server.Close();
    FTRACE(FPrint( _L("StartServer() complete (%d)"), retVal ));
    return retVal;
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// RDun::RDun()
// ---------------------------------------------------------------------------
//
RDun::RDun()
    {
    }

// ---------------------------------------------------------------------------
// This function starts DUN open/close for different medias
// ---------------------------------------------------------------------------
//
TInt RDun::ManageService( MDunActive* aRequest, TRequestStatus& aReqStatus )
    {
    FTRACE(FPrint( _L("RDun::ManageService()") ));
    SendReceive( EDunFuncManageService,
                 TIpcArgs(aRequest->Bearer(),
                          aRequest->BearerStatus()),
                 aReqStatus );
    FTRACE(FPrint( _L("RDun::ManageService() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// This function gets the active connection
// ---------------------------------------------------------------------------
//
TAny* RDun::ActiveConnection()
    {
    FTRACE(FPrint( _L("RDun::ActiveConnection()") ));
    TAny* connId = NULL;
    TPckg<TAny*> connIdPckg( connId );
    SendReceive( EDunFuncActiveConnection,
                 TIpcArgs(&connIdPckg) );
    FTRACE(FPrint( _L("RDun::ActiveConnection() complete") ));
    return connId;
    }

// ---------------------------------------------------------------------------
// Version number
// ---------------------------------------------------------------------------
//
TVersion RDun::Version() const
    {
    FTRACE(FPrint( _L("RDun::Version()") ));
    FTRACE(FPrint( _L("RDun::Version() complete") ));
    return ( TVersion(KDunServerMajorVersionNumber,
                      KDunServerMinorVersionNumber,
                      KDunServerBuildVersionNumber) );
    }

// ---------------------------------------------------------------------------
// This function connects to DUN server and creates a new session.
// ---------------------------------------------------------------------------
//
TInt RDun::Connect()
    {
    FTRACE(FPrint( _L("RDun::Connect()") ));
    TInt retry;
    TInt retVal = KErrNone;
    for ( retry=KDunRetryCount; retry>=0; retry-- )
        {
        retVal=CreateSession( KDialupServerName,
                              TVersion(KDunServerMajorVersionNumber,
                                       KDunServerMinorVersionNumber,
                                       KDunServerBuildVersionNumber),
                              KDunNumOfMessageSlots );
        if ( retVal == KErrNotFound || retVal == KErrServerTerminated )
            {
            retVal = StartServer();
            if ( retVal!=KErrNone && retVal!=KErrAlreadyExists )
                {
                FTRACE(FPrint( _L("RDun::Connect() complete (%d)"), retVal ));
                return retVal;
                }
            }
        else // KErrNone, or other error code.
            {
            FTRACE(FPrint( _L("RDun::Connect() complete (%d)"), retVal ));
            return retVal;
            }
        }
    FTRACE(FPrint( _L("RDun::Connect() complete (%d)"), retVal ));
    return retVal;
    }
