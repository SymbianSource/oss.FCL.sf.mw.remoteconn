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

__FLOG_STMT(_LIT8(KComponent,"UsbTransport");)

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
    __FLOG(_L8("~CMTPUsbTransport - Entry"));
    delete iConnection;
    __FLOG(_L8("~CMTPUsbTransport - Exit"));
    __FLOG_CLOSE;
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
    __FLOG(_L8("ModeChanged - Entry"));
    __FLOG(_L8("ModeChanged - Exit"));
    }
    
void CMTPUsbTransport::StartL(MMTPConnectionMgr& aConnectionMgr)
    {
    __FLOG(_L8("StartL - Entry"));
    __ASSERT_ALWAYS(!iConnection, Panic(EMTPUsbConnectionAlreadyExist));
    iConnection = CMTPUsbConnection::NewL(aConnectionMgr);
    __FLOG(_L8("StartL - Exit"));
    }

void CMTPUsbTransport::Stop(MMTPConnectionMgr& /*aConnectionMgr*/)
    {
    __FLOG(_L8("Stop - Entry"));
    if(iConnection)
	    {
	    delete iConnection;
	    iConnection = NULL;
	    }
    __FLOG(_L8("Stop - Exit")); 
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
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("ConstructL - Entry"));
    __FLOG(_L8("USB MTP Device class plug-in loaded."));    
    __FLOG(_L8("ConstructL - Exit"));
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

