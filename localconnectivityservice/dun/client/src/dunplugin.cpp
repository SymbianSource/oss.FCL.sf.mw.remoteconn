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
* Description:  EComm interface implementation and the client side of DUN
*
*/


#include "dunplugin.h"
#include "dunactive.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunPlugin* CDunPlugin::NewL( TLocodServicePluginParams& aParams )
    {
    CDunPlugin* self = new (ELeave) CDunPlugin( aParams );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunPlugin::~CDunPlugin()
    {
    FTRACE(FPrint( _L("CDunPlugin::~CDunPlugin()") ));
    iActiveContainer.ResetAndDestroy();
    iActiveContainer.Close();
    if ( iServer.Handle() != KNullHandle )
        {
        iServer.Close();
        }
    FTRACE(FPrint( _L("CDunPlugin::~CDunPlugin() complete") ));
    }

// ---------------------------------------------------------------------------
// Return owned RServer
// ---------------------------------------------------------------------------
//
RDun& CDunPlugin::Server()
    {
    FTRACE(FPrint( _L("CDunPlugin::Server()") ));
    FTRACE(FPrint( _L("CDunPlugin::Server() complete") ));
    return iServer;
    }

// ---------------------------------------------------------------------------
// Called by instance of CDunActive to inform Observer of service completed.
// Destruct the active object.
// ---------------------------------------------------------------------------
//
void CDunPlugin::ServiceCompleted( MDunActive* aRequest, TInt aError )
    {
    FTRACE(FPrint( _L("CDunPlugin::ServiceCompleted()") ));
    TInt i;
    TLocodBearer bearer = aRequest->Bearer();
    TBool status = aRequest->BearerStatus();
    for ( i=iActiveContainer.Count()-1; i>=0; i-- )
        {
        if ( iActiveContainer[i] == aRequest )
            {
            delete iActiveContainer[i];
            iActiveContainer.Remove( i );
            break;
            }
        }
    ReportCompletion( bearer, status, aError );
    FTRACE(FPrint( _L("CDunPlugin::ServiceCompleted() completed (%d)"), aError ));
    }

// ---------------------------------------------------------------------------
// From class CLocodServicePlugin.
// Implements interface virtual function
// ---------------------------------------------------------------------------
//
void CDunPlugin::ManageService( TLocodBearer aBearer, TBool aBearerStatus )
    {
    FTRACE(FPrint( _L("CDunPlugin::ManageService()") ));
    TInt retTemp;
    if ( iServer.Handle() == KNullHandle )
        {
        retTemp = iServer.Connect();
        if ( retTemp != KErrNone )
            {
            ReportCompletion( aBearer, aBearerStatus, retTemp );
            FTRACE(FPrint( _L("CDunPlugin::ManageService() (failed!) complete (%d)"), retTemp ));
            return;
            }
        }
    CDunActive* stateRequest = NULL;
    TRAPD( retTrap, stateRequest=CDunActive::NewL( this,
                                                   aBearer,
                                                   aBearerStatus ));
    if ( retTrap != KErrNone )
        {
        ReportCompletion( aBearer, aBearerStatus, retTrap );
        FTRACE(FPrint( _L("CDunPlugin::ManageService() (failed!) complete (%d)"), retTrap ));
        return;
        }
    if ( !stateRequest )
        {
        ReportCompletion( aBearer, aBearerStatus, KErrGeneral );
        FTRACE(FPrint( _L("CDunPlugin::ManageService() (failed!) complete (%d)"), KErrGeneral ));
        return;
        }
    retTemp = iActiveContainer.Append( stateRequest );
    if ( retTemp != KErrNone )
        {
        ReportCompletion( aBearer, aBearerStatus, retTemp );
        FTRACE(FPrint( _L("CDunPlugin::ManageService() (failed!) complete (%d)"), retTemp ));
        return;
        }
    stateRequest->ServiceRequest();
    FTRACE(FPrint( _L("CDunPlugin::ManageService() complete") ));
    }

// ---------------------------------------------------------------------------
// CDunPlugin::CDunPlugin
// ---------------------------------------------------------------------------
//
CDunPlugin::CDunPlugin( TLocodServicePluginParams& aParams )
    : CLocodServicePlugin( aParams )
    {
    }

// ---------------------------------------------------------------------------
// CDunPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CDunPlugin::ConstructL()
    {
    FTRACE(FPrint( _L("CDunPlugin::ConstructL()") ));
    FTRACE(FPrint( _L("CDunPlugin::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Reports completion status to LOCOD
// ---------------------------------------------------------------------------
//
void CDunPlugin::ReportCompletion( TLocodBearer aBearer,
                                   TBool aBearerStatus,
                                   TInt aErr)
    {
    FTRACE(FPrint( _L("CDunPlugin::ReportCompletion()") ));
    Observer().ManageServiceCompleted( aBearer,
                                       aBearerStatus,
                                       ImplementationUid(),
                                       aErr);
    FTRACE(FPrint( _L("CDunPlugin::ReportCompletion() completed (%d)"), aErr ));
    }
