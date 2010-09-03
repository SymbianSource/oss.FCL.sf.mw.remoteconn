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
 @internalComponent
*/

#include <ecom/implementationproxy.h>

#include "cmtpusbconnection.h"
#include "cmtpusbtransport.h"
#include "mmtpconnectionmgr.h"
#include "mtpusbpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpusbtransportTraces.h"
#endif


/**
USB MTP device class transport plug-in factory method.
@return A pointer to an MTP device class transport plug-in. Ownership IS 
transfered.
@leave One of the system wide error codes, if a processing failure occurs.
*/
TAny* CMTPUsbTransport::NewL(TAny* aParameter)
    {
    if ( aParameter != NULL )
    	{
        OstTrace0( TRACE_ERROR, CMTPUSBTRANSPORT_NEWL, "Parameter should be NULL!" );
    	User::Leave(KErrArgument);
    	}
    CMTPUsbTransport* self = new (ELeave) CMTPUsbTransport;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
    
/**
Destructor.
*/
CMTPUsbTransport::~CMTPUsbTransport()
    {
    OstTraceFunctionEntry0( CMTPUSBTRANSPORT_CMTPUSBTRANSPORT_DES_ENTRY );
    delete iConnection;
    OstTraceFunctionExit0( CMTPUSBTRANSPORT_CMTPUSBTRANSPORT_DES_EXIT );
    }
    
/**
This method returns a refrence to CMTPUsbConnection object.
@return a reference to CMTPUsbConnection object. 
*/
const CMTPUsbConnection& CMTPUsbTransport::MTPUsbConnection()
    {
    return *iConnection;
    }

void CMTPUsbTransport::ModeChanged(TMTPOperationalMode /*aMode*/)
    {
    OstTraceFunctionEntry0( CMTPUSBTRANSPORT_MODECHANGED_ENTRY );
    OstTraceFunctionExit0( CMTPUSBTRANSPORT_MODECHANGED_EXIT );
    }
    
void CMTPUsbTransport::StartL(MMTPConnectionMgr& aConnectionMgr)
    {
    OstTraceFunctionEntry0( CMTPUSBTRANSPORT_STARTL_ENTRY );
    __ASSERT_ALWAYS(!iConnection, Panic(EMTPUsbConnectionAlreadyExist));
    iConnection = CMTPUsbConnection::NewL(aConnectionMgr);
    OstTraceFunctionExit0( CMTPUSBTRANSPORT_STARTL_EXIT );
    }

void CMTPUsbTransport::Stop(MMTPConnectionMgr& /*aConnectionMgr*/)
    {
    OstTraceFunctionEntry0( CMTPUSBTRANSPORT_STOP_ENTRY );
    if(iConnection)
	    {
	    delete iConnection;
	    iConnection = NULL;
	    }
    OstTraceFunctionExit0( CMTPUSBTRANSPORT_STOP_EXIT );
    }

TAny* CMTPUsbTransport::GetExtendedInterface(TUid /*aInterfaceUid*/)
    {
    return 0;    
    }

/**
Constructor
*/    
CMTPUsbTransport::CMTPUsbTransport()
    {
    // Do nothing.
    }
    
/**
Second phase constructor.
*/
void CMTPUsbTransport::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPUSBTRANSPORT_CONSTRUCTL_ENTRY );
    OstTrace0( TRACE_NORMAL, CMTPUSBTRANSPORT_CONSTRUCTL, "USB MTP Device class plug-in loaded." );
    OstTraceFunctionExit0( CMTPUSBTRANSPORT_CONSTRUCTL_EXIT );
    }

// Define the implementation UID of MTP USB transport implementation.
static const TImplementationProxy ImplementationTable[] =
    {
        
        IMPLEMENTATION_PROXY_ENTRY((0x102827B2), CMTPUsbTransport::NewL)
        
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }

// Dummy dll entry point.
TBool E32Dll()
    {
    return ETrue;
    }

