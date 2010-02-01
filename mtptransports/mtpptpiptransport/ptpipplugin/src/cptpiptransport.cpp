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

#include "cptpiptransport.h"
#include "cptpipconnection.h"
#include "mmtpconnectionmgr.h"
#include "ptpippanic.h"

__FLOG_STMT(_LIT8(KComponent,"PTPIPTransport");)

/**
PTPIP transport plug-in factory method.
@return A pointer to a PTP IP transport plug-in. Ownership IS transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CPTPIPTransport::NewL(TAny* aParameter)
	{
	if ( aParameter != NULL )
		{
		User::Leave(KErrArgument);
		}

	CPTPIPTransport* self = new (ELeave) CPTPIPTransport;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
	

/**
Destructor
*/
CPTPIPTransport::~CPTPIPTransport()
	{
    __FLOG(_L8("~Destructor - Entry"));
    delete iConnection;
    __FLOG(_L8("~Destructor - Exit"));
    __FLOG_CLOSE;
	}

/**
Second phase constructor. 
*/
void CPTPIPTransport::ConstructL()
	{
	__FLOG_OPEN(KMTPSubsystem, KComponent);
	__FLOG(_L8("ConstructL - Entry"));
	__FLOG(_L8("PTPIP MTP Device class plug-in loaded."));    
	__FLOG(_L8("ConstructL - Exit"));	
	}

/**
Constructor
*/
CPTPIPTransport::CPTPIPTransport()
	{
	// Do nothing.
	}
	
/**
Starts the Transport. Creates the connection object which controls the TCP/IP sockets.
*/
void CPTPIPTransport::StartL(MMTPConnectionMgr& aConnectionMgr)
	{
    __FLOG(_L8("CPTPIPTransport::StartL - Entry"));
    __ASSERT_ALWAYS(!iConnection, Panic(EPTPIPConnectionAlreadyExist));
    iConnection = CPTPIPConnection::NewL(aConnectionMgr);
    aConnectionMgr.ConnectionOpenedL(*iConnection);
    __FLOG(_L8("CPTPIPTransport::StartL - Exit"));
	}

/**
Stops the transport. Deletes the connection object controlling the TCP/IP sockets
*/
void CPTPIPTransport::Stop(MMTPConnectionMgr& aConnectionMgr)
	{
	__FLOG(_L8("Stop - Entry"));
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
	
	__FLOG(_L8("Stop - Exit"));
	}

/**
Nothing to do in mode change. 
*/
void CPTPIPTransport::ModeChanged(TMTPOperationalMode /*aMode*/)
	{
	__FLOG(_L8("ModeChanged - Entry"));
	__FLOG(_L8("ModeChanged - Exit"));
	}

/**
No Extended Interface.   
*/
TAny* CPTPIPTransport::GetExtendedInterface(TUid /*aInterfaceUid*/)
	{
	__FLOG(_L8("GetExtendedInterface - Entry"));
	__FLOG(_L8("GetExtendedInterface - Exit"));	
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
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }

/**
Dummy dll entry point.
*/
TBool E32Dll()
    {
    return ETrue;
    }



