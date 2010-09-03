// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include <ecom/implementationproxy.h>

#include "mtpdebug.h"
#include "cptpiptransport.h"
#include "cptpipconnection.h"
#include "mmtpconnectionmgr.h"
#include "ptpippanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpiptransportTraces.h"
#endif


/**
PTPIP transport plug-in factory method.
@return A pointer to a PTP IP transport plug-in. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CPTPIPTransport::NewL(TAny* aParameter)
	{
	OstTraceFunctionEntry0( CPTPIPTRANSPORT_NEWL_ENTRY );
	if ( aParameter != NULL )
		{
        LEAVEIFERROR(KErrArgument, 
                        OstTrace0( TRACE_ERROR, CPTPIPTRANSPORT_NEWL, "Error argument" ));
		}

	CPTPIPTransport* self = new (ELeave) CPTPIPTransport;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	OstTraceFunctionExit0( CPTPIPTRANSPORT_NEWL_EXIT );
	return self;
	}
	
	

/**
Destructor
*/
CPTPIPTransport::~CPTPIPTransport()
	{
    OstTraceFunctionEntry0( CPTPIPTRANSPORT_CPTPIPTRANSPORT_ENTRY );
    delete iConnection;
	OstTraceFunctionExit0( CPTPIPTRANSPORT_CPTPIPTRANSPORT_EXIT );
	}

/**
Second phase constructor. 
*/
void CPTPIPTransport::ConstructL()
	{
	OstTraceFunctionEntry0( CPTPIPTRANSPORT_CONSTRUCTL_ENTRY );
	OstTrace0( TRACE_NORMAL, CPTPIPTRANSPORT_CONSTRUCTL, "PTPIP MTP Device class plug-in loaded." );
	OstTraceFunctionExit0( CPTPIPTRANSPORT_CONSTRUCTL_EXIT );
	}

/**
Constructor
*/
CPTPIPTransport::CPTPIPTransport()
	{
	OstTraceFunctionEntry0( DUP1_CPTPIPTRANSPORT_CPTPIPTRANSPORT_ENTRY );
	// Do nothing.
	OstTraceFunctionExit0( DUP1_CPTPIPTRANSPORT_CPTPIPTRANSPORT_EXIT );
	}
	
/**
Starts the Transport. Creates the connection object which controls the TCP/IP sockets.
*/
void CPTPIPTransport::StartL(MMTPConnectionMgr& aConnectionMgr)
	{
    OstTraceFunctionEntry0( CPTPIPTRANSPORT_STARTL_ENTRY );
    
    __ASSERT_ALWAYS(!iConnection, Panic(EPTPIPConnectionAlreadyExist));
    iConnection = CPTPIPConnection::NewL(aConnectionMgr);
    aConnectionMgr.ConnectionOpenedL(*iConnection);
	OstTraceFunctionExit0( CPTPIPTRANSPORT_STARTL_EXIT );
	}

/**
Stops the transport. Deletes the connection object controlling the TCP/IP sockets
*/
void CPTPIPTransport::Stop(MMTPConnectionMgr& aConnectionMgr)
	{
	OstTraceFunctionEntry0( CPTPIPTRANSPORT_STOP_ENTRY );
    if(iConnection)
	    {
	    // Check that we did not earlier close the connection due to some
	    // error. If so then the connectionclosed would already have been closed. 
	    if (iConnection->ConnectionOpen())
	    	{
	    	aConnectionMgr.ConnectionClosed(*iConnection);
	    	}
	    delete iConnection;
	    iConnection = NULL;
	    }
	
	OstTraceFunctionExit0( CPTPIPTRANSPORT_STOP_EXIT );
	}

/**
Nothing to do in mode change. 
*/
void CPTPIPTransport::ModeChanged(TMTPOperationalMode /*aMode*/)
	{
	OstTraceFunctionEntry0( CPTPIPTRANSPORT_MODECHANGED_ENTRY );
	OstTraceFunctionExit0( CPTPIPTRANSPORT_MODECHANGED_EXIT );
	}

/**
No Extended Interface.   
*/
TAny* CPTPIPTransport::GetExtendedInterface(TUid /*aInterfaceUid*/)
	{
	OstTraceFunctionEntry0( CPTPIPTRANSPORT_GETEXTENDEDINTERFACE_ENTRY );
	OstTraceFunctionExit0( CPTPIPTRANSPORT_GETEXTENDEDINTERFACE_EXIT );
	return 0;
	}

/**
Define the implementation UID of PTPIP transport implementation.
*/
static const TImplementationProxy ImplementationTable[] =
    {
        
    IMPLEMENTATION_PROXY_ENTRY((0xA0004A60), CPTPIPTransport::NewL)
        
    };

/**
PTPIP transport implementation table. 
*/
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    OstTraceFunctionEntry0( _IMPLEMENTATIONGROUPPROXY_ENTRY );
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    OstTraceFunctionExit0( _IMPLEMENTATIONGROUPPROXY_EXIT );
    return ImplementationTable;
    }

/**
Dummy dll entry point.
*/
TBool E32Dll()
    {
    OstTraceFunctionEntry0( _E32DLL_ENTRY );
    OstTraceFunctionExit0( _E32DLL_EXIT );
    return ETrue;
    }



